#ifndef _RTEST_
	#include <bellona2.h>
	#include "gui.h"
#endif

#ifdef _RTEST_
	#include "stdafx.h"
	#include <windows.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include "..\..\pefile.h"
	#include "winres.h"
	#include "mpointer.h"

	#define kdbg_printf printf
#endif

typedef struct {
	int			nID;
	ImageStt	*pIconImg;
} SysIconStt;

static SysIconStt sys_icon[] = {
	{ IDI_MBOX,			NULL },
	{ IDI_EXIT,			NULL },
	{ IDI_MINIMIZE,		NULL },
	{ IDI_MAXIMIZE,		NULL },
	{ IDI_MORE,			NULL },
	{ -1, NULL }
};

// �ý��ۿ��� ����� �������� �ε��Ѵ�.
int preload_sys_icon()
{
	int nI;

	for( nI = 0; sys_icon[nI].nID >= 0; nI++ )
		sys_icon[nI].pIconImg = load_icon_image16( NULL, sys_icon[nI].nID );

	return( 0 );
}

// �ý��ۿ��� ����� �������� �ε��Ѵ�.
int release_preload_sys_icon()
{
	int nI;

	for( nI = 0; sys_icon[nI].nID >= 0; nI++ )
	{
		if( sys_icon[nI].pIconImg != NULL )
		{
			free_image16( sys_icon[nI].pIconImg );
			sys_icon[nI].pIconImg = NULL;
		}
	}

	return( 0 );
}

ImageStt *get_sys_icon( int nID )
{
	int nI;

	for( nI = 0; sys_icon[nI].nID >= 0; nI++ )
	{
		if( nID == sys_icon[nI].nID && sys_icon[nI].pIconImg != NULL )
			return( sys_icon[nI].pIconImg );
	}

	return( NULL );
}

// �����ڵ� ���ڿ��� ASCII ���ڿ��� �����Ѵ�. 
static int unistr_to_ascii( char *pS, int nSize, UniStrStt *pUni )
{
	int nI;

	for( nI = 0; nI < nSize && nI < (int)pUni->wSize; nI++ )
		pS[nI] = (char)pUni->ustr[nI];
	pS[nI] = 0;
	return( 0 );
}

// ����� ���� ���ҽ��� �̸��� ���Ѵ�. 
static int add_userdef_res_name( WinResStt *pWR, UniStrStt *pUni, unsigned short int wType )
{
	int nR;

	nR = unistr_to_ascii( pWR->resname[ pWR->nTotalResName ].szName, sizeof( pWR->resname[ pWR->nTotalResName ].szName ), pUni );
	pWR->resname[ pWR->nTotalResName ].wType = wType;

	pWR->nTotalResName++;

	return( 0 );
}

// ����� ���� ���ҽ� �̸��� ���Ѵ�. 
char *get_res_name( unsigned short int wId, WinResStt *pWR )
{
	int nI;

	for( nI = 0; nI < pWR->nTotalResName; nI++ )
	{
		if( pWR->resname[nI].wType == wId )
			return( pWR->resname[nI].szName );
	}
	
	return( NULL );
}

// ���� ���ҽ� ��Ʈ���� ó���Ѵ�. 
static int resource_ent( WORD wTypeID, MY_IMAGE_RESOURCE_DIRECTORY *pRD, DWORD dwRes, DWORD dwRelocBase, WinResStt *pWR )
{
	char								*pB;
	MY_IMAGE_RESOURCE_DIRECTORY			*pRDir;
	MY_IMAGE_RESOURCE_DATA_ENTRY		*pData;
	DWORD								dwOffset;
	int									nI, nTotal;
	WinResEntStt						*pWinResEnt;
	MY_IMAGE_RESOURCE_DIRECTORY_ENTRY	*pEnt, *pEntX;
	WORD								wNameID, wLangID;

	// NAME ID�� ���Ѵ�.
	pB      = (char*)pRD;
	pEntX   = (MY_IMAGE_RESOURCE_DIRECTORY_ENTRY*)&pB[ sizeof( MY_IMAGE_RESOURCE_DIRECTORY ) ];
	
	nTotal = (int)pRD->NumberOfNamedEntries + pRD->NumberOfIdEntries;
	for( nI = 0; nI < nTotal; nI++ )
	{  	
		wNameID = pEntX->Id;

		// LANG ID�� ���Ѵ�.
		pRDir = (MY_IMAGE_RESOURCE_DIRECTORY*)( dwRes + (DWORD)pEntX->OffsetToDirectory );
		pB = (char*)pRDir;
		pEnt = (MY_IMAGE_RESOURCE_DIRECTORY_ENTRY*)&pB[ sizeof( MY_IMAGE_RESOURCE_DIRECTORY ) ];
		wLangID = pEnt->Id;

		// DATA OFFSET�� Size�� ���Ѵ�.
		pData = (MY_IMAGE_RESOURCE_DATA_ENTRY*)( dwRes + (DWORD)pEnt->OffsetToData );
	
		// DATA ������(RVA)
		dwOffset = pData->OffsetToData;

		// ���ҽ� ��Ʈ���� �߰��Ѵ�.
		//kdbg_printf( "[%d] Type=%2d, ResID=%-4d, Offset=0x%08X, Size=%d\n", nI,
		//	wTypeID, wNameID, pData->OffsetToData, pData->Size );

		pWinResEnt = &pWR->ent[ pWR->nTotal ];
		pWinResEnt->dwAddr = (DWORD)( pData->OffsetToData + dwRelocBase );
		pWinResEnt->dwSize = (DWORD)pData->Size;
		pWinResEnt->wID    = (unsigned short int)wNameID;
		pWinResEnt->wType  = (unsigned short int)wTypeID;
		pWR->nTotal++;

		pEntX++;
	}
	return( nI );
}

// �ڽ��� ���ο� ���Ե� ���ҽ��� �ν��Ͽ� ���� ������ ���·� �����Ѵ�. 
int win_resource( DWORD dwModule, WinResStt *pWR )
{
	MY_IMAGE_OPTIONAL_HEADER			*pIO;
	MY_IMAGE_DOS_HEADER					*pDos;
	UniStrStt							*pUni;
	MY_IMAGE_RESOURCE_DIRECTORY_ENTRY	*pEnt;	 
	MY_IMAGE_RESOURCE_DIRECTORY			*pRes, *pRD;
	int									nI, nR, nIndex, nTotal;

	pDos = (MY_IMAGE_DOS_HEADER*)dwModule;
	if( pDos->e_magic != MY_IMAGE_DOS_SIGNATURE )
	{
		kdbg_printf( "win_resource: invalid module handle(0x%X)\n", dwModule );
		return( -1 );	
	}
	
	pIO  = (MY_IMAGE_OPTIONAL_HEADER*)( (DWORD)pDos->e_lfanew + dwModule + (DWORD)sizeof( MY_IMAGE_FILE_HEADER ) );
	if( pIO->dd_Resource_dwVAddr == 0 )
	{
		kdbg_printf( "win_res: dd_Resource_dwVAdd = 0\n" );
		return( -1 );
	}
	
	memset( pWR, 0, sizeof( WinResStt ) );
	pRes = (MY_IMAGE_RESOURCE_DIRECTORY*)( pIO->dd_Resource_dwVAddr + dwModule );
	pEnt = (MY_IMAGE_RESOURCE_DIRECTORY_ENTRY*)( (DWORD)pRes + (DWORD)sizeof( MY_IMAGE_RESOURCE_DIRECTORY ) );

	// ��Ʈ���� ������ ���Ѵ�.
	nTotal = (int)pRes->NumberOfNamedEntries + pRes->NumberOfIdEntries;
	for( nI = 0; nI < nTotal; nI++ )
	{	
		// �̸����� ���ǵ� ���ҽ����� Ȯ���Ѵ�.
		if( pEnt->Id > 32 )
		{
			pUni = (UniStrStt*)( (DWORD)pRes + (DWORD)pEnt->Id );
			add_userdef_res_name( pWR, pUni, pEnt->Id );
		}	

		nIndex = pWR->nTotal;
		// TYPE ��Ʈ���� �߰��Ѵ�. (BITMAP, ICON, VERSION ���...)
		pRD = (MY_IMAGE_RESOURCE_DIRECTORY*)( (DWORD)pRes + (DWORD)pEnt->OffsetToDirectory );
		nR = resource_ent( pEnt->Id, pRD, (DWORD)pRes, (DWORD)dwModule, pWR );
		if( nR < 0 )
			break;
		else if( nR > 0 )  // �ϳ��� ó���� ���� �ִ�.
		{		   
			if( pEnt->Id == 1 )				// CURSOR
				pWR->nCursorIndex = nIndex;
			else if( pEnt->Id == 3 )		// ICON
				pWR->nIconIndex = nIndex;
			else if( pEnt->Id == 12 )		// GROUP CURSOR
			{
				pWR->nGroupCursorIndex = nIndex;
				pWR->nTotalCursor =  pWR->nTotal - nIndex;
			}
			else if( pEnt->Id == 14 )		// GROUP ICON
			{
				pWR->nGroupIconIndex = nIndex;
				pWR->nTotalIcon =  pWR->nTotal - nIndex;
			}
		}
		  		  
		pEnt++;
	}

	// ICON Resource ID�� �����Ѵ�. 
	for( nI = 0; nI < pWR->nTotalIcon; nI++ )
	{
		pWR->ent[ nI + pWR->nIconIndex ].wID = pWR->ent[ nI + pWR->nGroupIconIndex ].wID;
		pWR->ent[ nI + pWR->nIconIndex ].nPainIndex = nI + pWR->nGroupIconIndex;
	}
	// CURSOR Resource ID�� �����Ѵ�. 
	for( nI = 0; nI < pWR->nTotalCursor; nI++ )
	{
		pWR->ent[ nI + pWR->nCursorIndex ].wID = pWR->ent[ nI + pWR->nGroupCursorIndex ].wID;
		pWR->ent[ nI + pWR->nCursorIndex ].nPainIndex = nI + pWR->nGroupCursorIndex;
	}

	return( 0 );
}

// WinRes ��Ʈ���� ���Ѵ�. 
WinResEntStt *load_winres( WinResStt *pWR, unsigned short int wResID )
{
	int nI;

	if( pWR == NULL )
		pWR = get_winres_stt();

	for( nI = 0; nI < pWR->nTotal; nI++ )
	{
		if( pWR->ent[nI].wID == wResID )
			return( &pWR->ent[nI] );
	}

	return( NULL );
}

// CURSOR ���ҽ��� �ε��Ѵ�. 
int load_cursor( WinResStt *pWR, void *pVoidBC, unsigned short int wID )
{
	BCursorStt			*pBC;
	unsigned short int	*pHot;
	WinResEntStt		*pWinRes, *pGrpRes;

	pBC = (BCursorStt*)pVoidBC;

	pWinRes = load_winres( pWR, wID );
	if( pWinRes == NULL )
	{	// ���ҽ��� ã�� �� ����. 
		kdbg_printf( "Res %d loading failed!\n", wID );
		return( -1 );
	}

	// Ŀ���� �ؼ��Ѵ�. 
	memset( pBC, 0, sizeof( BCursorStt ) );
	pGrpRes = &pWR->ent[ pWinRes->nPainIndex ];
	pBC->pIDir = (ICONDIR*)pGrpRes->dwAddr;
	pBC->pBit  = (BITMAPINFO*)( pWinRes->dwAddr + 4 );
	pHot = (unsigned short int*)pWinRes->dwAddr;
	pBC->wHotX = pHot[0];
	pBC->wHotY = pHot[1];

	kdbg_printf( "CURSOR Hot(%d,%d) ICONDIR( 0x%08X ), BITMAP( 0x%08X )\n", 
		pBC->wHotX, pBC->wHotY, pBC->pIDir, pBC->pBit );
 
	return( 0 );
}


ResObjStt *kalloc_res_obj( DWORD dwType )
{
	ResObjStt *pRO;

	pRO = kmalloc( sizeof( ResObjStt ) );
	if( pRO == NULL )
		return( NULL );

	memset( pRO, 0, sizeof( ResObjStt ) );

	pRO->dwType = dwType;
	
	return( pRO );
}

