#include <bellona2.h>
#include "gui.h"

static FontTableStt	ftable = {
	0, 
	{
	{ IDR_BF_BASE11,	NULL },
	{ IDR_BF_BASE12,	NULL },
	{ IDR_BF_BASE14,	NULL },
	{ IDR_BF_SYS12,		NULL },
	{ IDR_BF_SYS14,		NULL },
	{ IDR_BF_SIMPLE9,	NULL },
	{ 0, NULL },
	}
};
// 폰트 리소스를 로드한다. 
static FontStt *load_font( void *pVWinRes, int nID )
{
	WinResEntStt	*pRes;
	FontStt			*pFont;

	pRes = load_winres( pVWinRes, (UINT16)nID );
	if( pRes == NULL )
	{	// 리소스를 찾을 수 없다. 
		kdbg_printf( "Res %d loading failed!\n", nID );
		return( NULL );
	}

	pFont = (FontStt*)pRes->dwAddr;

	return( pFont );
}

// 시스템의 기본 폰트를 로드한다. 
int load_system_fonts()
{
	int nI;

	for( nI = 0; ftable.ent[nI].nID != 0; nI++ )
	{
		ftable.ent[nI].pFont = load_font( get_winres_stt(), ftable.ent[nI].nID );
		if( ftable.ent[nI].pFont != NULL )
		{
			kdbg_printf( "SYSTEM FONT (%d) = 0x%08X\n", ftable.ent[nI].nID, (DWORD)ftable.ent[nI].pFont );
			ftable.nTotal++;
		}
		else
			kdbg_printf( "SYSTEM FONT (%d) : failed!\n", ftable.ent[nI].nID );
	}	

	return( 0 );
}

// 시스템의 기본 폰트를 해제한다. (구조체만 초기화하면 된다.)
int unload_system_fonts()
{
	int		nI;
	
	// memset으로 구조체 전체를 clear하면 안된다.
	for( nI = 0; ftable.ent[nI].nID != 0; nI++ )
		ftable.ent[nI].pFont = NULL;

	ftable.nTotal = 0;
	
	return( 0 );
}

// 이미 로드된 시스템 폰트를 ID로 찾는다. 
FontStt *get_system_font( int nID )
{
	int nI;

	for( nI = 0; nI < ftable.nTotal; nI++ )
	{
		if( ftable.ent[nI].nID == nID )
			return( ftable.ent[nI].pFont );
	}	

	return( NULL );
}

// XY 좌표에 문자를 출력한다. 
int drawchar_xy( GraBuffStt *pGB, int nX, int nY, FontStt *pFont, char ch, UINT16 wColor, DWORD dwEffect )
{
	UINT16		*pAddr;
	DWORD		*pD, dwX;
	int			nI, nJ, nV, nH;
	
	pD = (DWORD*)( (DWORD)sizeof( FontStt )+ (DWORD)pFont );
	pD = (DWORD*)( (DWORD)pD + (DWORD)((int)pFont->cLinByte * (int)pFont->cV * ((int)ch-(int)pFont->wStartChar) ) );

	// 폰트의 cV가 GraBuff의 nV를 넘어서면 그릴 수 있는 만큼만 그린다.
	nV = (int)pFont->cV;
	if( pGB->pR->nV <= nY + nV )
		nV = pGB->pR->nV - nY;

	// GB의 X 범위를 넘었다.
	nH = (int)pFont->cH;
	if( pGB->pR->nH <= nX + nH )
		nH = pGB->pR->nH - nX;

	// 범위를 벗어났으므로 그릴 필요가 없다.
	if( nH <= 0 || nV <= 0 )
		return( 0 );

	pAddr = get_gra_buff_addr16( pGB, nX, nY );
	for( nI = 0; nI < nV; nI++ )
	{
		dwX = 1;
		dwX = (DWORD)( dwX << (pFont->cH-1) );
		
		for( nJ = 0; nJ < (int)pFont->cH; nJ++ )
		{
			if( pD[0] & dwX )
			{
				pAddr[nJ] = wColor;
				// 최적화 시킬 필요가 있다.  (일단은 작동하는지 부터 보자.)
				if( pGB->self_mask.pB != NULL )
					set_mask_bit( &pGB->self_mask, nX + nJ, nY + nI, 1 );
			}
			dwX = (DWORD)( dwX >> 1 );
		}
		
		pAddr = &pAddr[ pGB->pR->nH ];
		pD = (DWORD*)( (DWORD)pD + (DWORD)pFont->cLinByte );
	}		

	return( 0 );
}

// XY 좌표에 문자열을 출력한다.   (이전에 출력된 문자가 있으면 중첩된다.)
// 문자의 중첩 Draw를 막으려면 외부에서 이전에 Draw된 문자를 지워야 한다.
int drawtext_xy( struct GraBuffTag *pGB, int nX, int nY, FontStt *pFont, 
				 char *pStr, UINT16 wColor, DWORD dwEffect )
{
	int nR, nI;

	if( pFont == NULL )
	{	// 폰트가 로드되지 않았다.
		kdbg_printf( "draw_text_xy() : NULL font!\n" );
		return( -1 );
	}

	for( nI = 0; pStr[nI] != 0; nI++ )
	{	
		nR = drawchar_xy( pGB, nX + ( pFont->cH * nI ), nY, pFont, pStr[nI], wColor, dwEffect );
		if( nR < 0 )
			break;
	}	

	return( 0 );
}

