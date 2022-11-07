#include "bellona2.h"

// PE 헤더의 포인터들을 설정한다.
static int pre_load_analysis( LDRStt *pLDR, char *pB, long lSize )
{
	int		nI;
	DWORD	dwI, dwSize;
	
	// check MZ or ZM
	pLDR->pDH = (MY_IMAGE_DOS_HEADER*)pB;
	if( pLDR->pDH->e_magic != (unsigned short)0x4D5A && pLDR->pDH->e_magic != (unsigned short)0x5A4D )
	{
		kdbg_printf( "ERROR: pB( 0x%X ) is not an MZ file!\n", (DWORD)pB );
		return( -1 );
	}
	
	pLDR->pIF = (MY_IMAGE_FILE_HEADER*)&pB[pLDR->pDH->e_lfanew];
	// check PE
	if( pLDR->pIF->Signature != (DWORD)0x4550 )
	{
		kdbg_printf( "ERROR: not a PE file!\n" );
		return( -1 );
	}

	// get image optional header
	pLDR->pIO = (MY_IMAGE_OPTIONAL_HEADER*)&pB[ pLDR->pDH->e_lfanew + sizeof( MY_IMAGE_FILE_HEADER ) ];
	pLDR->dwSectOffs = dwI = (DWORD)pLDR->pDH->e_lfanew + (DWORD)sizeof( MY_IMAGE_FILE_HEADER ) + (DWORD)sizeof( MY_IMAGE_OPTIONAL_HEADER );

	// 개별 섹션을 처리한다.
	for( nI = 0; nI < pLDR->pIF->NumberOfSections; nI++ )
	{
		pLDR->p_sect[nI] = (MY_IMAGE_SECTION_HEADER*)&pB[ dwI ];

		// calc this section size
		dwSize = pLDR->p_sect[nI]->VirtualSize;
		if( dwSize < pLDR->p_sect[nI]->SizeOfRawData )
			dwSize = pLDR->p_sect[nI]->SizeOfRawData;

		// accumulate total page
		pLDR->nTotalPage += (int)( (dwSize + 4095)/ 4096 );

		dwI += (DWORD)sizeof( MY_IMAGE_SECTION_HEADER );
	}

	return( 0 );
}

static int rva_to_offset( MY_IMAGE_SECTION_HEADER **ppISH, int nTotalSect, DWORD dwRVA )
{
	int nI;

	for( nI = 0; nI < nTotalSect; nI++ )
	{
		if( ppISH[nI]->VirtualAddress <= dwRVA && dwRVA < ppISH[nI]->VirtualAddress + ppISH[nI]->VirtualSize )
			return( dwRVA - ppISH[nI]->VirtualAddress + ppISH[nI]->PointerToRawData );
	}	

	return( 0 );
}

// relocate pe image
static int relocate_pe_image( LDRStt *pLDR, char *pB, char *pDest )
{
	MY_IMAGE_BASE_RELOCATION	*pR;
	MY_IMAGE_SECTION_HEADER		*pSect;
	WORD						wW, *pW;
	DWORD						dwX, *pT;
	int							nI, nTotal;
	long						lRelocInfoStart, lRelocInfoEnd;

	// 앞부분 헤더 4096 바이트를 복사한다. 
	memcpy( pDest, pB, 4096 );

	// copy section's body
	for( nI = 0; nI < pLDR->pIF->NumberOfSections && nI < 16; nI++ )
	{
		pSect = pLDR->p_sect[nI];
        // Virtual Size만큼 초기화한 후 SizeOfRawData만큼 복사한다.
        memset( &pDest[ pSect->VirtualAddress ], 0, pSect->VirtualSize );
		memcpy( &pDest[ pSect->VirtualAddress ], &pB[ pSect->PointerToRawData ], pSect->SizeOfRawData );
	}

	//디버그 정보를 옮긴다.
	if( pLDR->pIO->dd_Debug_dwVAddr != 0 )
	{	// pLDR->pIO->dd_Debug_dwVAddr는 파일에서의 오프셋.
		memcpy( (void*)pLDR->dwAfterRelocDbgAddr, &pB[ pLDR->pIO->dd_Debug_dwVAddr ], pLDR->pIO->dd_Debug_dwSize );
	}
	pLDR->nTotalPage += pLDR->nTotalDbgPage;
	//kdbg_printf( "Total Debug Page : %d\n", pLDR->nTotalDbgPage );
	
	// 주소 재배치 
	if( pLDR->pIO->dd_Basereloc_dwVAddr == 0 )
	{
		kdbg_printf( "no relocation info.\n" );
		return( 0 );		// no relocation section
	}

	// RVA를 Offset으로 변경하여 처리하여야 한다.
	lRelocInfoStart	= rva_to_offset( pLDR->p_sect, pLDR->pIF->NumberOfSections, pLDR->pIO->dd_Basereloc_dwVAddr );
	lRelocInfoEnd	= lRelocInfoStart + pLDR->pIO->dd_Basereloc_dwSize;
	
	for( nTotal = 0; lRelocInfoStart < lRelocInfoEnd; )
	{
		pR = (MY_IMAGE_BASE_RELOCATION*)&pB[ lRelocInfoStart ];
		if( pR->VirtualAddress == 0 )
			break;

		// get total entries of this block
		pW = (WORD*)&pB[ lRelocInfoStart + 8 ];
		nI = ( pR->SizeOfBlock -8 ) / 2;		//number of relocation entries ( 2 bytes)

		for( ; nI > 0; nI-- )
		{
			wW = *pW;
			if( wW != 0 )  // if the value is 0 we will ignore that. maybe the last part is padded with 0...
			{
				wW = (WORD)wW & (WORD)0x0FFF;
				// 엔트리 하위 12비트를 블록의 Virtual Address에 더한 곳의 주소를 보정하면된다.
				dwX = pR->VirtualAddress + (DWORD)wW;  // ImageBase의 값이 더해지지 않은 RVA값. (당연)

				pT = (DWORD*)&pDest[dwX];
			    pT[0] -= pLDR->pIO->ImageBase;	  // sub the compile time image base
			    pT[0] += (DWORD)pDest;			  // add the new image base
			}
		    pW++; 
			nTotal++;
		}

		lRelocInfoStart += pR->SizeOfBlock;
	}				
	
	//kdbg_printf( "Total %d relocation entries.\n", nTotal );

	return( 0 );
}

// mapping address to relocate pe image
static int mapping_for_section( DWORD *pPD, LDRStt *pLDR, char *pAddr )
{
	MY_IMAGE_SECTION_HEADER		*pSect;
	int							nI, nR;
	DWORD						dwSize;

	// 기본적으로 앞부분 1페이지는 매핑한다.
	if( (DWORD)pAddr & (DWORD)0x80000000 )
		nR = nMappingUser( pPD, (DWORD)pAddr, 4096 );
	else
		nR = nMapping( pPD, (DWORD)pAddr, 4096 );

	// 각 섹션별 매핑을 잡는다.
	for( nI = 0; nI < pLDR->pIF->NumberOfSections; nI++ )
	{
		pSect = pLDR->p_sect[nI];

		if( pSect->SizeOfRawData > pSect->VirtualSize )
			dwSize = pSect->SizeOfRawData; 
		else
			dwSize = pSect->VirtualSize;
						   
		dwSize = (DWORD)( ( (dwSize + 4095) / 4096) * 4096 );

		// user level mapping
		if( (DWORD)pAddr & (DWORD)0x80000000 )
			nR = nMappingUser( pPD, pSect->VirtualAddress + (DWORD)pAddr, dwSize );
		else
			nR = nMapping( pPD, pSect->VirtualAddress + (DWORD)pAddr, dwSize );
		if( nR < 0 )
			return( -1 );

		pLDR->dwAfterRelocDbgAddr = pSect->VirtualAddress + (DWORD)pAddr + dwSize;
	}

	// 디버깅 정보를 옮길 영역을 매핑한다.
	if( pLDR->pIO->dd_Debug_dwVAddr != 0 )
	{
		pLDR->nTotalDbgPage = (int)( (DWORD)( pLDR->pIO->dd_Debug_dwSize + 4096 ) / 4096 );
		if( pLDR->dwAfterRelocDbgAddr & (DWORD)0x80000000 )
			nR = nMappingUser( pPD, pLDR->dwAfterRelocDbgAddr, pLDR->nTotalDbgPage * 4096 );
		else
			nR = nMapping( pPD, pLDR->dwAfterRelocDbgAddr, pLDR->nTotalDbgPage * 4096 );
	}
	else //  디버깅 정보가 없다.
		pLDR->dwAfterRelocDbgAddr = 0;

	return( 0 );
}

// load pe image and relocate that.(this function does not mapping program stack!!)
DWORD load_pe_file( char *pFileName, DWORD dwLoadAddr, DWORD dwTempAddr, LDRStt *pLDR )
{
    long		lSize;
	char		*pTemp;
	ThreadStt	*pThread;
	int			nR, nHandle;
	DWORD		dwEntryPoint, *pPageDir, dwDbgAddr;

	dwEntryPoint = 0;

	// 페이지 디렉토리를 구한다.
	pThread = get_current_thread();
	if( pThread == NULL )
	{
		kdbg_printf( "load_pe() - invalid thread!\n" );
		return( 0 );
	}
	pPageDir = (DWORD*)get_thread_page_dir( pThread );

	// 파일을 오픈하여 크기를 구한다.
	nHandle = kopen( pFileName, FM_READ );
    if( nHandle == -1 )
    {
        kdbg_printf( "load_pe() : %s - open error!\n", pFileName );
        return( 0 );
    }
	lSize = klseek( nHandle, 0, FSEEK_END );
	if( lSize < 0 )
	{
		kdbg_printf( "load_pe() - get file size failed!\n" );
		kclose( nHandle );
		return( 0);
	}	
	pLDR->lFileSize = lSize;
    klseek( nHandle, 0, FSEEK_SET );
	lSize = (( lSize + 4095 ) / 4096) * 4096;
	
	// 임시 영역을 매핑한다.
	if( dwTempAddr & (DWORD)0x80000000 )
		nR = nMappingUser( pPageDir, dwTempAddr, lSize );		// user mapping
	else
		nR = nMapping( pPageDir, dwTempAddr, lSize );			// kernel mapping
	if( nR < 0 )
	{
		kdbg_printf( "load_pe() : temp_mapping failed!\n" );
		kclose( nHandle );
		return( 0 );
	}
	pTemp = (char*)dwTempAddr;

	// 파일을 읽고 닫는다.
	nR = kread( nHandle, pTemp, lSize );
    kclose( nHandle );
	if( nR < 0 )
	{
		kdbg_printf( "load_pe() - file read error!\n" );
		goto BACK_1;
	}

    // MZ, PE 체크
    nR = pre_load_analysis( pLDR, pTemp, lSize );
	if( nR < 0 )
		goto BACK_1;

	// BASE ADDRESS가 "0"이면 필요한 페이지 만큼의 연속된 가용주소를 찾는다.
	if( dwLoadAddr == 0 )
	{	
		dwLoadAddr = find_free_linear_space( pPageDir, LOAD_MODULE_START_ADDR, pLDR->nTotalPage * 4096 );
		if( dwLoadAddr == 0 )
		{
			kdbg_printf( "load_pe_file() - free linear space not found!  ( %d pages)\n", pLDR->nTotalPage );
			goto BACK_1;
		}				
	}
	pLDR->dwLoadAddr  = dwLoadAddr;

	// BASE ADDRESS를 매핑한다.
	nR = mapping_for_section( pPageDir, pLDR, (char*)dwLoadAddr );
	if( nR < 0 )
	{
        kdbg_printf( "load_pe() : mapping_for_section failed!\n", pFileName );
        goto BACK_1;
	}

	// 이미지를 재배치 한다.
	nR = relocate_pe_image( pLDR, pTemp, (char*)dwLoadAddr );
	if( nR < 0 )
	{
		kdbg_printf( "load_pe() : relocate_pe_image failed!\n", pFileName );
		goto BACK_1;
	}

	// EntryPoint, Import Export Table, Debug info의 주소를 계산한다.
	dwEntryPoint = pLDR->pIO->AddressOfEntryPoint + dwLoadAddr;
	if( pLDR->pIO->dd_Import_dwVAddr != 0 )
		pLDR->pImp   = (MY_IMAGE_IMPORT_DESCRIPTOR*)( (DWORD)pLDR->pIO->dd_Import_dwVAddr + dwLoadAddr );
	if( pLDR->pIO->dd_Export_dwVAddr != 0 )
		pLDR->pExp   = (MY_IMAGE_EXPORT_DIRECTORY* )( (DWORD)pLDR->pIO->dd_Export_dwVAddr + dwLoadAddr );
	if( pLDR->pIO->dd_Debug_dwVAddr != 0 )
		dwDbgAddr = pLDR->pIO->dd_Debug_dwVAddr + dwLoadAddr;
	else
		dwDbgAddr = 0;	

	// Entry Point와 Imp, Exp 값을 출력한다.
	//kdbg_printf( "Entry=0x%08X, Exp=0x%08X, Imp=0x%08X, PreRelocDbg=0x%08X \n", 
	//	dwEntryPoint, (DWORD)pLDR->pExp, (DWORD)pLDR->pImp, dwDbgAddr );
	
BACK_1:
	// Temp 영역을 해제한다.
    nReleaseMapping( pPageDir, pTemp, lSize );

	return( dwEntryPoint );
}

