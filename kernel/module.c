#include "bellona2.h"

static SysModuleStt	sys_module;
static ModuleStt	bell_mod;

// bellona2 모듈의 디버깅 정보를 외부에서 로드한다.
int load_bellona2_dbginfo( char *pPath )
{
	ModuleStt		*pM;
	MyCoffDbg2Stt	*pDbg;

	pM = &bell_mod;

	if( pM->pMyDbg != NULL )
	{	// 디버깅 정보가 이미 설정되어 있다.
		kdbg_printf( "load_bellona2_dbginfo: debuginfo is already loaded\n" );
		return( -1 );
	}

	// 디버그 정보 파일을 새로 로드한다.
	pDbg = load_mydbg2_info( pPath );
	if( pDbg == NULL )
	{	// 디버그 정보 파일을 로드할 수 없다.
		return( -1 );
	}

	// 디버깅 정보를 설정한다.	
	pM->pMyDbg = pDbg;

	return( 0 );
}

// sys_module의 주소를 알아낸다.
SysModuleStt *get_sys_module()
{
	return( &sys_module );
}

// 모듈 구조체의 주소를 등록한다.
static int insert_module_struct( ModuleStt *pM )
{
	int nI;

	if( sys_module.nTotal >= MAX_MODULE )
		return( -1 );	// 더이상 추가할 수 없다.

	// 동일한 포인터가 있는지 찾는다.
	for( nI = 0; nI < MAX_MODULE; nI++ )
	{	// 이미 동일한 주소가 등록되어 있다.
		if( sys_module.mod[nI] == pM )
			return( 0 );
	}	

	// NULL 엔트리를 찾는다.
	for( nI = 0; nI < MAX_MODULE; nI++ )
	{
		if( sys_module.mod[nI] == NULL )
		{
			sys_module.mod[nI] = pM;
			sys_module.nTotal++;
			return( 0 );
		}				
	}					
	return( -1 );
}

// 모듈 구조체를 등록 해제한다.
static int delete_module_struct( ModuleStt *pM )
{
	int nI;
	
	if( sys_module.nTotal <= 0 || pM == NULL )
		return( -1 );

	// NULL 엔트리를 찾는다.
	for( nI = 0; nI < MAX_MODULE; nI++ )
	{
		if( sys_module.mod[nI] == pM )
		{
			sys_module.mod[nI] = NULL;
			sys_module.nTotal--;
			kfree( pM );
			return( 0 );
		}
	}

	return( -1 );
}

// 시스템 모듈 구조체를 초기화 하고 BELLONA2 모듈에 대한 엔트리를 추가한다.
int init_sys_module_struct()
{
	memset( &sys_module, 0, sizeof( SysModuleStt ) );
	
	memset( &bell_mod, 0, sizeof( bell_mod ) );

	// BELLONA2의 export table 주소를 RELOC_BASE로 보정한다.
	realize_exp_table( bell.pExp, (DWORD)RELOC_BASE );

	// BELLONA2의 모듈 구조체를 추가한다.
	strcpy( bell_mod.szAlias, "Bellona2.org" );
	bell_mod.dwLoadAddr   = RELOC_BASE;
	bell_mod.pExp		  = bell.pExp;
	bell_mod.dwEntryPoint = (DWORD)bellona2_main;
	bell_mod.pMyDbg       = pMy;
	bell_mod.nTotalPage   = ( bell.dwLastImageByte - RELOC_BASE ) / 4096;
	
	// 모듈 구조체에 등록한다.
	insert_module_struct( &bell_mod );
					  
	return( 0 );
}

// Alias로 모듈을 찾는다.
ModuleStt *find_module_by_alias( char *pAlias, int *pNi )
{
	ModuleStt	*pM;
	int			nI, nTemp;

	if( pNi == NULL )
		pNi = &nTemp;
	pNi[0] = -1;
	for( nI = 0; nI < sys_module.nTotal; nI++ )
	{
		pM = sys_module.mod[nI];
		if( strcmpi( pM->szAlias, pAlias ) == 0 )
		{
			pNi[0] = nI;
			return( pM );
		}
	}

	return( NULL );
}

// get mydbg struct and relocation base
ModuleStt *find_module_by_addr( DWORD dwAddr )
{
	int 			nI;
	ModuleStt		*pM;
	SysModuleStt	*pSM;
	ProcessStt		*pProcess;

	// kernel area
	if( (dwAddr & 0x80000000) == 0 )
	{
		pSM = get_sys_module();
		for( nI = 0; ; nI++ )
		{
			if( nI >= MAX_MODULE )
				return( NULL );	// 찾을 수 없다.	

			pM = pSM->mod[nI];
			if( pM == NULL || pM->dwLoadAddr >= 0x80000000 )
				continue;
			
			if( pM->dwLoadAddr <= dwAddr && dwAddr < pM->dwLoadAddr + (DWORD)(pM->nTotalPage * 4096) ) 
				return( pM );	// 찾았다.
		}
	}
	
	// user area
	pProcess = k_get_current_process();
	if( pProcess == NULL )
		return( NULL );
		
	return( pProcess->pModule );
}	

// 모듈을 내린다.
int unload_module( char *pAlias )
{
	ModuleStt	*pM;
	int			nR, nI;

	// 모듈을 찾는다.
	pM = find_module_by_alias( pAlias, &nI );
	if( pM == NULL )
		return( -1 );

	// 모듈 구조체를 뺀다.
	delete_module_struct( pM );

	// 매핑을 해제한다.
	nR = nReleaseMapping( bell.pPD, (char*)pM->dwLoadAddr, (long)(pM->nTotalPage * 4096) );
						
	return( 0 );
}	

// import table을 처리한다.
int import_linking( MY_IMAGE_IMPORT_DESCRIPTOR *pImp, DWORD dwBaseAddr )
{
	ModuleStt	*pM;
	char		*pFuncName;
	int			nK, nI, nTotalImp;
	DWORD		*pIAT, *pILT, dwAddr;

	if( pImp == NULL )
		return( 0 );

	// Imp의 개수를 찾는다.
	for( nTotalImp = nI = 0; pImp[nI].pIATRVA != 0; nI++ )
	{
		// 모듈명의 주소를 보정한다.
		pImp[nI].pNameRVA = (char*)( (DWORD)pImp[nI].pNameRVA + dwBaseAddr );

		// 해당 모듈이 이미 로드되어 있는지 찾는다.
		pM = find_module_by_alias( pImp[nI].pNameRVA, NULL );
		if( pM == NULL )
		{	// 지정된 모듈을 찾을 수 없다.
			kdbg_printf( "import_linking() : module %s not found!\n", pImp[nI].pNameRVA );
			return( -1 );
		}
										  				
		// ILT, IAT의 주소를 보정한다.
		pIAT = pImp[nI].pIATRVA = (DWORD*)( (DWORD)pImp[nI].pIATRVA + dwBaseAddr );
		pILT = pImp[nI].pILTRVA = (DWORD*)( (DWORD)pImp[nI].pILTRVA + dwBaseAddr );

		for( nK = 0; ; nK++ )
		{
			if( (DWORD)pILT[nK] == 0 )
				break;

			if( pILT[nK] & MY_IMAGE_ORDINAL_FLAG )
			{	// ORDINAL에 의한 Import??
				kdbg_printf( "import_linking: Import by ordinal! (pILT[nK]=0x%X)\n", pILT[nK] );
				return( -1 );
			}

			pIAT[nK] = (DWORD)( (DWORD)pIAT[nK] + dwBaseAddr );
			pILT[nK] = (DWORD)( (DWORD)pILT[nK] + dwBaseAddr );

			pFuncName = (char*)pILT[nK];
			pFuncName += 2;	// 2를 더해 준다.

			// 지정된 모듈에서 특정 함수를 찾는다.
			dwAddr = find_function_address( pM, pFuncName );
			if( dwAddr == 0 )
			{
				kdbg_printf( "import_linking() : Address of function %s not found!\n", pFuncName );
				return( -1 );
			}
			
			// 찾은 함수 주소를 저장한다.
			pIAT[nK] = dwAddr;

			//kdbg_printf( "IMP_LINK : 0x%08X <- 0x%08X %s/%s\n", (DWORD)&pIAT[nK], pIAT[nK], pImp[nI].pNameRVA, pFuncName );
		}	
		
		nTotalImp ++;
	}

	return( 0 );
}	 

// 모듈을 로드한다.
ModuleStt *load_pe( char *pFileName, UINT16 wType )
{
	int					nR;
	LDRStt				ldr;
	ModuleStt			*pModule;
	MODULE_MAIN_FUNC	pModuleMain;
	DWORD				dwEntryPoint;

	// 모듈 구조체를 할당한다.
	pModule = (ModuleStt*)kmalloc( sizeof(ModuleStt) );
	if( pModule == NULL )
		return( NULL );
	memset( pModule, 0, sizeof( ModuleStt ) );
	memset( &ldr, 0, sizeof( LDRStt ) );	

	// PE 모듈을 로드한 후 엔트리 포인트를 리턴한다.
	// dwLoadAddr이 "0"이면 로딩 가능한 주소를 직접 찾는다.
	if( wType == MTYPE_MODULE )
		dwEntryPoint = load_pe_file( pFileName, 0, LOAD_MODULE_TEMP_ADDR, &ldr );
	else
		dwEntryPoint = load_pe_file( pFileName, LDR_BASE_ADDR, LDR_TEMP_ADDR, &ldr );
	if( dwEntryPoint == 0 )
	{	// 메모리 해제하고 돌아간다.
		kfree( pModule );
		return( NULL );
	}

	pModule->wModuleType    = wType;
	pModule->pExp           = ldr.pExp;
	pModule->dwEntryPoint	= dwEntryPoint;
	pModule->dwLoadAddr		= ldr.dwLoadAddr;
	pModule->nTotalPage		= ldr.nTotalPage;
	pModule->dwFileSize		= (DWORD)ldr.lFileSize;
	strcpy( pModule->szAlias, get_pure_filename( pFileName ) );
	pModule->pMyDbg			= get_coff_dbg2( (char*)ldr.dwAfterRelocDbgAddr );

	// Export Table의 포인터를 보정한다.
	realize_exp_table( pModule->pExp, pModule->dwLoadAddr );

	// Import Table을 처리한다.
	nR = import_linking( ldr.pImp, pModule->dwLoadAddr );
	if( nR < 0 )
	{	// 모듈 컨텍스트의 페이지를 해제한다. (디버깅을 위해서 메모리는 해제하지 않고 둔다.)
		//nReleaseMapping( bell.pPD, (char*)ldr.dwLoadAddr, ldr.nTotalPage * 4096 );
		
		// 모듈 구조체를 해제한다.
		kfree( pModule );
		return( NULL );
	}				   

	// 모듈 구조체를 등록한다.
	insert_module_struct( pModule );

	// 모듈일 경우에만 R0 Context 상에서 직접 main을 호출한다.
	if( wType == MTYPE_MODULE )
	{
		pModuleMain = (MODULE_MAIN_FUNC)dwEntryPoint;
		pModuleMain( (DWORD)pModule, 0, NULL );
	}

	return( pModule );
}

// R0 레벨의 모듈을 로드한다.
ModuleStt *load_module( char *pFileName )
{
	ModuleStt *pM;
	
	pM = load_pe( pFileName, MTYPE_MODULE );
	
	return( pM );
}

// R3 레벨의 모듈을 로드한다.  
// 일단 모듈을 로드한 후 Page의 매핑을 변경한다.
ModuleStt *load_user_module( char *pFileName )
{
	int			nI;
	ModuleStt	*pM;
	DWORD		dwAddr;
	
	pM = load_pe( pFileName, MTYPE_MODULE );
	if( pM == NULL )
		return( NULL );

	// 매핑된 페이지를 user level로 변경한다.
	dwAddr = pM->dwLoadAddr;
	for( nI = 0; nI < pM->nTotalPage; nI++ )
	{
		change_mapping_to_user( dwAddr );
		dwAddr += 0x1000;
	}
	
	pM->byUserLevel = 1;
	// 커널 영역이면 다른 프로세스들에 복사되도록 한다.
	if( pM->dwLoadAddr < (DWORD)0x80000000 )
	{	// 2003-08-23 버그 수정. (이것이 없으면 커널 영역의 변경된 매핑이 
		// bell.pPD에 반영되지 않아서 다른 프로세스에 전달되지 않는다.
		_asm {
			MOV EAX, CR3;
			AND EAX, 0xFFFFF000
			MOV dwAddr, EAX
		}
		memcpy( bell.pPD, (void*)dwAddr, 4096/2 );
	
		update_kernel_mapping_flag();
	}
	return( pM );
}

// 모듈 리스트를 출력한다.
int disp_module()
{
	int			nI;
	ModuleStt	*pM;

	kdbg_printf( "Index  address   pages   exp_tbl     debug       entry     Alias\n" );
	for( nI = 0; nI < MAX_MODULE; nI++ )
	{
		pM = sys_module.mod[nI];
		if( pM == NULL )
			continue;
		
		kdbg_printf( "[%2d] 0x%08X  %3d   0x%08X  0x%08X  0x%08X  %-16s\n", 
		    nI, pM->dwLoadAddr, pM->nTotalPage, (DWORD)pM->pExp, (DWORD)pM->pMyDbg, 
		    pM->dwEntryPoint, pM->szAlias );
	}

	kdbg_printf( "Total %d registerd modules.\n", sys_module.nTotal );

	return( 0 );
}

// 모듈의 프로세스 카운터를 증가시킨다.
int inc_process_count( ModuleStt *pM )
{
	if( pM == NULL )
		return( -1 );

	pM->nProcessCount++;

	return( pM->nProcessCount );
}

// 모듈의 프로세스 카운터를 감소시킨다.
int dec_process_count( ModuleStt *pM )
{
	if( pM == NULL )
		return( -1 );

	pM->nProcessCount--;

	if( pM->nProcessCount == 0 )
	{	// 모듈 구조체를 해제한다.
		delete_module_struct( pM );	// pM은 내부적으로 free된다.
	}

	return( 0 );
}



