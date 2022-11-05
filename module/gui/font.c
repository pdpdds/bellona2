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
// ��Ʈ ���ҽ��� �ε��Ѵ�. 
static FontStt *load_font( void *pVWinRes, int nID )
{
	WinResEntStt	*pRes;
	FontStt			*pFont;

	pRes = load_winres( pVWinRes, (UINT16)nID );
	if( pRes == NULL )
	{	// ���ҽ��� ã�� �� ����. 
		kdbg_printf( "Res %d loading failed!\n", nID );
		return( NULL );
	}

	pFont = (FontStt*)pRes->dwAddr;

	return( pFont );
}

// �ý����� �⺻ ��Ʈ�� �ε��Ѵ�. 
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

// �ý����� �⺻ ��Ʈ�� �����Ѵ�. (����ü�� �ʱ�ȭ�ϸ� �ȴ�.)
int unload_system_fonts()
{
	int		nI;
	
	// memset���� ����ü ��ü�� clear�ϸ� �ȵȴ�.
	for( nI = 0; ftable.ent[nI].nID != 0; nI++ )
		ftable.ent[nI].pFont = NULL;

	ftable.nTotal = 0;
	
	return( 0 );
}

// �̹� �ε�� �ý��� ��Ʈ�� ID�� ã�´�. 
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

// XY ��ǥ�� ���ڸ� ����Ѵ�. 
int drawchar_xy( GraBuffStt *pGB, int nX, int nY, FontStt *pFont, char ch, UINT16 wColor, DWORD dwEffect )
{
	UINT16		*pAddr;
	DWORD		*pD, dwX;
	int			nI, nJ, nV, nH;
	
	pD = (DWORD*)( (DWORD)sizeof( FontStt )+ (DWORD)pFont );
	pD = (DWORD*)( (DWORD)pD + (DWORD)((int)pFont->cLinByte * (int)pFont->cV * ((int)ch-(int)pFont->wStartChar) ) );

	// ��Ʈ�� cV�� GraBuff�� nV�� �Ѿ�� �׸� �� �ִ� ��ŭ�� �׸���.
	nV = (int)pFont->cV;
	if( pGB->pR->nV <= nY + nV )
		nV = pGB->pR->nV - nY;

	// GB�� X ������ �Ѿ���.
	nH = (int)pFont->cH;
	if( pGB->pR->nH <= nX + nH )
		nH = pGB->pR->nH - nX;

	// ������ ������Ƿ� �׸� �ʿ䰡 ����.
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
				// ����ȭ ��ų �ʿ䰡 �ִ�.  (�ϴ��� �۵��ϴ��� ���� ����.)
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

// XY ��ǥ�� ���ڿ��� ����Ѵ�.   (������ ��µ� ���ڰ� ������ ��ø�ȴ�.)
// ������ ��ø Draw�� �������� �ܺο��� ������ Draw�� ���ڸ� ������ �Ѵ�.
int drawtext_xy( struct GraBuffTag *pGB, int nX, int nY, FontStt *pFont, 
				 char *pStr, UINT16 wColor, DWORD dwEffect )
{
	int nR, nI;

	if( pFont == NULL )
	{	// ��Ʈ�� �ε���� �ʾҴ�.
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

