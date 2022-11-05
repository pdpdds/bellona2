#include <bellona2.h>
#include "gui.h"

// icon window의 타이틀과 아이콘을 로드하여 GraBuff에 그린다.
int initialize_iconwin( WinStt *pWin, char *pTitle, DWORD dwIconID )
{
	RectStt				r;
	FontStt				*pFont;
	IconWinPrivateStt	*pPrivate;
	int					nX, nY, nLength;

	pPrivate = (IconWinPrivateStt*)pWin->pPrivate;

	// 배경을 지운다.
	//screen_to_win( &r, pWin->gb.pR, &pWin->ct_r );
	get_client_rect( pWin, &r );
	
	k_fill_rect( &pWin->gb, &r, RGB16( 255, 255, 255) );

	// 윈도우 타이틀을 설정한다.
	if( pTitle != NULL )
		set_window_title( pWin, pTitle );

	// icon을 로드한다.
	pPrivate->pImg = load_icon_image16(	NULL, (int)dwIconID );

	// 아이콘을 그린다.
	if( pPrivate->pImg != NULL )
	{
		nX = ( r.nH - pPrivate->pImg->nH) / 2;
		nY = 3;
		copy_image16( &pWin->gb, pPrivate->pImg, nX, nY, pPrivate->pImg->nH, pPrivate->pImg->nV, 0, 0 );
	}
	else
		kdbg_printf( "Icon %d loading failed!\n", dwIconID );

	// 타이틀을 중앙에 그린다.
	pFont = get_system_font( IDR_BF_BASE11 );
	nY += pPrivate->pImg->nV + 2;
	nLength = strlen( pWin->szTitle ) * pFont->cH;
	nX = ( pWin->obj.r.nH - nLength ) / 2;
	drawtext_xy( 
		&pWin->gb,													
		nX,
		nY,
		pFont,						
		pWin->szTitle,							// String
		RGB16( 0, 0, 0 ),
		0 );													

	return( 0 );
}

static DWORD wmh_iconwin_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwTitlePtr, DWORD dwIconID )
{
	IconWinPrivateStt	*pPrivate;

	//kdbg_printf( "wmh_iconwin_create()\n" );

	// Private 영역을 할당한다. 
	pPrivate = (IconWinPrivateStt*)kmalloc( sizeof( IconWinPrivateStt ) );
	pWin->pPrivate = pPrivate;
	if( pPrivate != NULL )
	{	// iconwin의 Private Data를 초기화 한다.
		memset( pPrivate, 0, sizeof( IconWinPrivateStt ) );
	}	

	// 아이콘과 글자만 보여야 하므로 GraBuff에 속한 mask를 할당한다.
	// close, destroy시에 해제하지 않아도 free_gra_buff()내에서 해제된다.
	internal_alloc_bit_mask( &pWin->gb.self_mask, &pWin->obj.r );
	
	// self_mask를 일단 0으로 초기화 한다.  나중에 그려진 비트만 1로 설정. (어디서??!!!)
	memset( pWin->gb.self_mask.pB, 0, pWin->gb.self_mask.dwSize );

	// 초기화 루틴을 호출한다.
	initialize_iconwin( pWin, (char*)dwTitlePtr, dwIconID );

	return( WMHRV_CONTINUE );
}

// 클라이언트 영역을 그린다. 
static DWORD wmh_iconwin_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	IconWinPrivateStt *pPrivate;

	pPrivate = (IconWinPrivateStt*)pWin->pPrivate;
	if( pPrivate != NULL )
	{	// 아이콘을 해제한다.
		if( pPrivate->pImg != NULL )
			free_image16( pPrivate->pImg );
		// Private 영역을 해제한다.
		kfree( pPrivate );
		pWin->pPrivate = NULL;
	}

	return( 0 );
}

WMFuncStt iconwin_marray[] = {
	{ WMESG_CREATE		   , wmh_iconwin_create			},
	{ WMESG_DESTROY		   , wmh_iconwin_destroy		},
	{ 0, NULL }
};


