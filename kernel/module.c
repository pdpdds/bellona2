#include "bellona2.h"

static SysModuleStt	sys_module;
static ModuleStt	bell_mod;

// bellona2 ����� ����� ������ �ܺο��� �ε��Ѵ�.
int load_bellona2_dbginfo( char *pPath )
{
	ModuleStt		*pM;
	MyCoffDbg2Stt	*pDbg;

	pM = &bell_mod;

	if( pM->pMyDbg != NULL )
	{	// ����� ������ �̹� �����Ǿ� �ִ�.
		kdbg_printf( "load_bellona2_dbginfo: debuginfo is already loaded\n" );
		return( -1 );
	}

	// ����� ���� ������ ���� �ε��Ѵ�.
	pDbg = load_mydbg2_info( pPath );
	if( pDbg == NULL )
	{	// ����� ���� ������ �ε��� �� ����.
		return( -1 );
	}

	// ����� ������ �����Ѵ�.	
	pM->pMyDbg = pDbg;

	return( 0 );
}

// sys_module�� �ּҸ� �˾Ƴ���.
SysModuleStt *get_sys_module()
{
	return( &sys_module );
}

// ��� ����ü�� �ּҸ� ����Ѵ�.
static int insert_module_struct( ModuleStt *pM )
{
	int nI;

	if( sys_module.nTotal >= MAX_MODULE )
		return( -1 );	// ���̻� �߰��� �� ����.

	// ������ �����Ͱ� �ִ��� ã�´�.
	for( nI = 0; nI < MAX_MODULE; nI++ )
	{	// �̹� ������ �ּҰ� ��ϵǾ� �ִ�.
		if( sys_module.mod[nI] == pM )
			return( 0 );
	}	

	// NULL ��Ʈ���� ã�´�.
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

// ��� ����ü�� ��� �����Ѵ�.
static int delete_module_struct( ModuleStt *pM )
{
	int nI;
	
	if( sys_module.nTotal <= 0 || pM == NULL )
		return( -1 );

	// NULL ��Ʈ���� ã�´�.
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

// �ý��� ��� ����ü�� �ʱ�ȭ �ϰ� BELLONA2 ��⿡ ���� ��Ʈ���� �߰��Ѵ�.
int init_sys_module_struct()
{
	memset( &sys_module, 0, sizeof( SysModuleStt ) );
	
	memset( &bell_mod, 0, sizeof( bell_mod ) );

	// BELLONA2�� export table �ּҸ� RELOC_BASE�� �����Ѵ�.
	realize_exp_table( bell.pExp, (DWORD)RELOC_BASE );

	// BELLONA2�� ��� ����ü�� �߰��Ѵ�.
	strcpy( bell_mod.szAlias, "Bellona2.org" );
	bell_mod.dwLoadAddr   = RELOC_BASE;
	bell_mod.pExp		  = bell.pExp;
	bell_mod.dwEntryPoint = (DWORD)bellona2_main;
	bell_mod.pMyDbg       = pMy;
	bell_mod.nTotalPage   = ( bell.dwLastImageByte - RELOC_BASE ) / 4096;
	
	// ��� ����ü�� ����Ѵ�.
	insert_module_struct( &bell_mod );
					  
	return( 0 );
}

// Alias�� ����� ã�´�.
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
				return( NULL );	// ã�� �� ����.	

			pM = pSM->mod[nI];
			if( pM == NULL || pM->dwLoadAddr >= 0x80000000 )
				continue;
			
			if( pM->dwLoadAddr <= dwAddr && dwAddr < pM->dwLoadAddr + (DWORD)(pM->nTotalPage * 4096) ) 
				return( pM );	// ã�Ҵ�.
		}
	}
	
	// user area
	pProcess = k_get_current_process();
	if( pProcess == NULL )
		return( NULL );
		
	return( pProcess->pModule );
}	

// ����� ������.
int unload_module( char *pAlias )
{
	ModuleStt	*pM;
	int			nR, nI;

	// ����� ã�´�.
	pM = find_module_by_alias( pAlias, &nI );
	if( pM == NULL )
		return( -1 );

	// ��� ����ü�� ����.
	delete_module_struct( pM );

	// ������ �����Ѵ�.
	nR = nReleaseMapping( bell.pPD, (char*)pM->dwLoadAddr, (long)(pM->nTotalPage * 4096) );
						
	return( 0 );
}	

// import table�� ó���Ѵ�.
int import_linking( MY_IMAGE_IMPORT_DESCRIPTOR *pImp, DWORD dwBaseAddr )
{
	ModuleStt	*pM;
	char		*pFuncName;
	int			nK, nI, nTotalImp;
	DWORD		*pIAT, *pILT, dwAddr;

	if( pImp == NULL )
		return( 0 );

	// Imp�� ������ ã�´�.
	for( nTotalImp = nI = 0; pImp[nI].pIATRVA != 0; nI++ )
	{
		// ������ �ּҸ� �����Ѵ�.
		pImp[nI].pNameRVA = (char*)( (DWORD)pImp[nI].pNameRVA + dwBaseAddr );

		// �ش� ����� �̹� �ε�Ǿ� �ִ��� ã�´�.
		pM = find_module_by_alias( pImp[nI].pNameRVA, NULL );
		if( pM == NULL )
		{	// ������ ����� ã�� �� ����.
			kdbg_printf( "import_linking() : module %s not found!\n", pImp[nI].pNameRVA );
			return( -1 );
		}
										  				
		// ILT, IAT�� �ּҸ� �����Ѵ�.
		pIAT = pImp[nI].pIATRVA = (DWORD*)( (DWORD)pImp[nI].pIATRVA + dwBaseAddr );
		pILT = pImp[nI].pILTRVA = (DWORD*)( (DWORD)pImp[nI].pILTRVA + dwBaseAddr );

		for( nK = 0; ; nK++ )
		{
			if( (DWORD)pILT[nK] == 0 )
				break;

			if( pILT[nK] & MY_IMAGE_ORDINAL_FLAG )
			{	// ORDINAL�� ���� Import??
				kdbg_printf( "import_linking: Import by ordinal! (pILT[nK]=0x%X)\n", pILT[nK] );
				return( -1 );
			}

			pIAT[nK] = (DWORD)( (DWORD)pIAT[nK] + dwBaseAddr );
			pILT[nK] = (DWORD)( (DWORD)pILT[nK] + dwBaseAddr );

			pFuncName = (char*)pILT[nK];
			pFuncName += 2;	// 2�� ���� �ش�.

			// ������ ��⿡�� Ư�� �Լ��� ã�´�.
			dwAddr = find_function_address( pM, pFuncName );
			if( dwAddr == 0 )
			{
				kdbg_printf( "import_linking() : Address of function %s not found!\n", pFuncName );
				return( -1 );
			}
			
			// ã�� �Լ� �ּҸ� �����Ѵ�.
			pIAT[nK] = dwAddr;

			//kdbg_printf( "IMP_LINK : 0x%08X <- 0x%08X %s/%s\n", (DWORD)&pIAT[nK], pIAT[nK], pImp[nI].pNameRVA, pFuncName );
		}	
		
		nTotalImp ++;
	}

	return( 0 );
}	 

// ����� �ε��Ѵ�.
ModuleStt *load_pe( char *pFileName, UINT16 wType )
{
	int					nR;
	LDRStt				ldr;
	ModuleStt			*pModule;
	MODULE_MAIN_FUNC	pModuleMain;
	DWORD				dwEntryPoint;

	// ��� ����ü�� �Ҵ��Ѵ�.
	pModule = (ModuleStt*)kmalloc( sizeof(ModuleStt) );
	if( pModule == NULL )
		return( NULL );
	memset( pModule, 0, sizeof( ModuleStt ) );
	memset( &ldr, 0, sizeof( LDRStt ) );	

	// PE ����� �ε��� �� ��Ʈ�� ����Ʈ�� �����Ѵ�.
	// dwLoadAddr�� "0"�̸� �ε� ������ �ּҸ� ���� ã�´�.
	if( wType == MTYPE_MODULE )
		dwEntryPoint = load_pe_file( pFileName, 0, LOAD_MODULE_TEMP_ADDR, &ldr );
	else
		dwEntryPoint = load_pe_file( pFileName, LDR_BASE_ADDR, LDR_TEMP_ADDR, &ldr );
	if( dwEntryPoint == 0 )
	{	// �޸� �����ϰ� ���ư���.
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

	// Export Table�� �����͸� �����Ѵ�.
	realize_exp_table( pModule->pExp, pModule->dwLoadAddr );

	// Import Table�� ó���Ѵ�.
	nR = import_linking( ldr.pImp, pModule->dwLoadAddr );
	if( nR < 0 )
	{	// ��� ���ؽ�Ʈ�� �������� �����Ѵ�. (������� ���ؼ� �޸𸮴� �������� �ʰ� �д�.)
		//nReleaseMapping( bell.pPD, (char*)ldr.dwLoadAddr, ldr.nTotalPage * 4096 );
		
		// ��� ����ü�� �����Ѵ�.
		kfree( pModule );
		return( NULL );
	}				   

	// ��� ����ü�� ����Ѵ�.
	insert_module_struct( pModule );

	// ����� ��쿡�� R0 Context �󿡼� ���� main�� ȣ���Ѵ�.
	if( wType == MTYPE_MODULE )
	{
		pModuleMain = (MODULE_MAIN_FUNC)dwEntryPoint;
		pModuleMain( (DWORD)pModule, 0, NULL );
	}

	return( pModule );
}

// R0 ������ ����� �ε��Ѵ�.
ModuleStt *load_module( char *pFileName )
{
	ModuleStt *pM;
	
	pM = load_pe( pFileName, MTYPE_MODULE );
	
	return( pM );
}

// R3 ������ ����� �ε��Ѵ�.  
// �ϴ� ����� �ε��� �� Page�� ������ �����Ѵ�.
ModuleStt *load_user_module( char *pFileName )
{
	int			nI;
	ModuleStt	*pM;
	DWORD		dwAddr;
	
	pM = load_pe( pFileName, MTYPE_MODULE );
	if( pM == NULL )
		return( NULL );

	// ���ε� �������� user level�� �����Ѵ�.
	dwAddr = pM->dwLoadAddr;
	for( nI = 0; nI < pM->nTotalPage; nI++ )
	{
		change_mapping_to_user( dwAddr );
		dwAddr += 0x1000;
	}
	
	pM->byUserLevel = 1;
	// Ŀ�� �����̸� �ٸ� ���μ����鿡 ����ǵ��� �Ѵ�.
	if( pM->dwLoadAddr < (DWORD)0x80000000 )
	{	// 2003-08-23 ���� ����. (�̰��� ������ Ŀ�� ������ ����� ������ 
		// bell.pPD�� �ݿ����� �ʾƼ� �ٸ� ���μ����� ���޵��� �ʴ´�.
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

// ��� ����Ʈ�� ����Ѵ�.
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

// ����� ���μ��� ī���͸� ������Ų��.
int inc_process_count( ModuleStt *pM )
{
	if( pM == NULL )
		return( -1 );

	pM->nProcessCount++;

	return( pM->nProcessCount );
}

// ����� ���μ��� ī���͸� ���ҽ�Ų��.
int dec_process_count( ModuleStt *pM )
{
	if( pM == NULL )
		return( -1 );

	pM->nProcessCount--;

	if( pM->nProcessCount == 0 )
	{	// ��� ����ü�� �����Ѵ�.
		delete_module_struct( pM );	// pM�� ���������� free�ȴ�.
	}

	return( 0 );
}



