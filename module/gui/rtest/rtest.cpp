// RTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>
#include "..\Winres.h"

WinResStt winres;

typedef struct {
	char	*pStr;
	WORD	wType;
} RTStrEntStt;

static RTStrEntStt rt_str[] = {
	{ "CURSOR"       , 1 },
	{ "BITMAP"       , 2 },
	{ "ICON"         , 3 },
	{ "MENU"         , 4 },
	{ "DIALOG"       , 5 },
	{ "STRING"       , 6 },
	{ "FONTDIR"      , 7 },
	{ "FONT"         , 8 },
	{ "ACCELERATOR"  , 9 },
	{ "RCDATA"       , 10 },  
	{ "MESSAGETABLE" , 11 }, 
	{ "GROUP_CURSOR" , 12 }, 
	{ "GROUP_ICON"   , 14 }, 
	{ "VERSION"		 , 16 },	
	{ "DLGINCLUDE"   , 17 },	
	{ "PLUGPLAY"	 , 19 },	
	{ "VXD"			 , 20 },	
	{ "ANICURSOR"	 , 21 },
	{ "ANIICON"		 , 22 },	
	{ "HTML"		 , 23 },	
	{ NULL, 0 }
}; 

static char *get_restype_str( WORD wId, WinResStt *pWR )
{
	int		nI;
	char	*pName;

	for( nI = 0; rt_str[nI].pStr != NULL; nI++ )
	{
		if( rt_str[nI].wType == wId )
			return( rt_str[nI].pStr );
	}					
	
	// 사용자 정의 리소스 이름을 구해 본다. 
	pName = get_res_name( wId, pWR );
	if( pName == NULL )
		pName = "UNKNOWN";

	return( pName );
}			   

int main(int argc, char* argv[])
{
	WinResStt		*pWR;
	WinResEntStt	*pEnt, *pPairEnt;
	int				nR, nI;
	HMODULE			hModule;

	hModule = GetModuleHandle( NULL );
	if( hModule == NULL )
	{
		printf( "module handle is NULL!\n" );
		return( 0 );
	}

	// 리소스를 구한다. 
	nR = win_resource( (DWORD)hModule, &winres );
	if( nR < 0 )
	{
		printf( "No resource!\n" );
		return( 0 );
	}

	// 구한 결과를 출력한다.
	pWR = &winres;
	for( nI = 0; nI < pWR->nTotal; nI++ )
	{
		pEnt = &pWR->ent[nI];
		printf( "Type= %-10s (%2d), ResID=%-4d, Offset=0x%08X, Size=%d\n", 
			get_restype_str( pEnt->wType, &winres ), pEnt->wType, pEnt->wID, pEnt->dwAddr, pEnt->dwSize );

		// 아이콘일 경우
		if( pEnt->wType == 3 )
		{
			ICONDIR *pID;
			BITMAPINFO	*pBit;

			pPairEnt = &pWR->ent[ pEnt->nPainIndex ];
			pID = (ICONDIR*)pPairEnt->dwAddr;
			pBit = (BITMAPINFO*)pEnt->dwAddr;
		}  	// 커서일 경우
		else if( pEnt->wType == 1 )
		{
			unsigned short int	*pW, wX, wY;
			ICONDIR		*pID;
			BITMAPINFO	*pBit;

			pPairEnt = &pWR->ent[ pEnt->nPainIndex ];
			pID = (ICONDIR*)pPairEnt->dwAddr;
			pW  = (unsigned short int*)pEnt->dwAddr;
			wX = pW[0];	// Hot spot
			wY = pW[1];
			pBit = (BITMAPINFO*)( pEnt->dwAddr + 4 );
		} // 비트맵
		else if( pEnt->wType == 2 )
		{
			BYTE *pX;
			BITMAPINFO	*pBit;
			pBit = (BITMAPINFO*)pEnt->dwAddr;
			pX = (BYTE*)( &pBit->bmiColors[256] );
		}
	}
		 
	return( 0 );
}
