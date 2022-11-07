#include "bellona2.h"

// 모듈 pM에서 지정된 함수 pFuncName을 구한다.
DWORD find_function_address( ModuleStt *pM, char *pFuncName )
{
	int							nI;
	char						*pFName;
	MY_IMAGE_EXPORT_DIRECTORY	*pExp;
	DWORD						*pNameTbl, *pAddrTbl;

	pExp = pM->pExp;
	if( pExp == NULL )
		return( 0 );

	pNameTbl = (DWORD*)pExp->AddressOfNames;
	pAddrTbl = (DWORD*)pExp->AddressOfFunctions;
	for( nI = 0; nI < (int)pExp->NumberOfFunctions; nI++ )
	{
		pFName = (char*)pNameTbl[nI];

		if( strcmp( pFName, pFuncName ) == 0 )
			return( pAddrTbl[nI] );
	}

	return( 0 );
}

// init_sys_module_struct()에서 호출된다.
int realize_exp_table( MY_IMAGE_EXPORT_DIRECTORY *pExp, DWORD dwRelocBase )
{
	int							nI;
	DWORD						*pAddrTbl, *pNameTbl;

	if( pExp == NULL )
		return( 0 );

	pExp->AddressOfFunctions = (DWORD*)( (DWORD)pExp->AddressOfFunctions + dwRelocBase );
	pExp->AddressOfNames     = (DWORD*)( (DWORD)pExp->AddressOfNames + dwRelocBase );

	pAddrTbl = (DWORD*)pExp->AddressOfFunctions;
	pNameTbl = (DWORD*)pExp->AddressOfNames;

	for( nI = 0; nI < (int)pExp->NumberOfFunctions; nI++ )
	{
		pNameTbl[nI] = (DWORD)( (DWORD)pNameTbl[nI] + dwRelocBase );
		pAddrTbl[nI] = (DWORD)( (DWORD)pAddrTbl[nI] + dwRelocBase );
	}									 

	return( 0 );
}

// EXPORT TABLE을 출력한다.
int disp_export_table( MY_IMAGE_EXPORT_DIRECTORY *pExp )
{
	char						*pName;
	int							nI, nPage;
	DWORD						*pAddrTbl, *pNameTbl;

	if( pExp == NULL )
		return( -1 );

	pAddrTbl = (DWORD*)pExp->AddressOfFunctions;
	pNameTbl = (DWORD*)pExp->AddressOfNames;

	nPage = get_vertical_line_size() - 2;
	for( nI = 0; nI < (int)pExp->NumberOfFunctions; nI++ )
	{
		pName = (char*)pNameTbl[nI];
		kdbg_printf( "[%3d] 0x%08X %s\n", nI, pAddrTbl[nI], pName );

		if( nI > 0 && (nI % nPage ) == 0 )
			getchar();
	}									 

	return( 0 );
}