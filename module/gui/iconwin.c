#include <bellona2.h>
#include "gui.h"

// icon window�� Ÿ��Ʋ�� �������� �ε��Ͽ� GraBuff�� �׸���.
int initialize_iconwin( WinStt *pWin, char *pTitle, DWORD dwIconID )
{
	RectStt				r;
	FontStt				*pFont;
	IconWinPrivateStt	*pPrivate;
	int					nX, nY, nLength;

	pPrivate = (IconWinPrivateStt*)pWin->pPrivate;

	// ����� �����.
	//screen_to_win( &r, pWin->gb.pR, &pWin->ct_r );
	get_client_rect( pWin, &r );
	
	k_fill_rect( &pWin->gb, &r, RGB16( 255, 255, 255) );

	// ������ Ÿ��Ʋ�� �����Ѵ�.
	if( pTitle != NULL )
		set_window_title( pWin, pTitle );

	// icon�� �ε��Ѵ�.
	pPrivate->pImg = load_icon_image16(	NULL, (int)dwIconID );

	// �������� �׸���.
	if( pPrivate->pImg != NULL )
	{
		nX = ( r.nH - pPrivate->pImg->nH) / 2;
		nY = 3;
		copy_image16( &pWin->gb, pPrivate->pImg, nX, nY, pPrivate->pImg->nH, pPrivate->pImg->nV, 0, 0 );
	}
	else
		kdbg_printf( "Icon %d loading failed!\n", dwIconID );

	// Ÿ��Ʋ�� �߾ӿ� �׸���.
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

	// Private ������ �Ҵ��Ѵ�. 
	pPrivate = (IconWinPrivateStt*)kmalloc( sizeof( IconWinPrivateStt ) );
	pWin->pPrivate = pPrivate;
	if( pPrivate != NULL )
	{	// iconwin�� Private Data�� �ʱ�ȭ �Ѵ�.
		memset( pPrivate, 0, sizeof( IconWinPrivateStt ) );
	}	

	// �����ܰ� ���ڸ� ������ �ϹǷ� GraBuff�� ���� mask�� �Ҵ��Ѵ�.
	// close, destroy�ÿ� �������� �ʾƵ� free_gra_buff()������ �����ȴ�.
	internal_alloc_bit_mask( &pWin->gb.self_mask, &pWin->obj.r );
	
	// self_mask�� �ϴ� 0���� �ʱ�ȭ �Ѵ�.  ���߿� �׷��� ��Ʈ�� 1�� ����. (���??!!!)
	memset( pWin->gb.self_mask.pB, 0, pWin->gb.self_mask.dwSize );

	// �ʱ�ȭ ��ƾ�� ȣ���Ѵ�.
	initialize_iconwin( pWin, (char*)dwTitlePtr, dwIconID );

	return( WMHRV_CONTINUE );
}

// Ŭ���̾�Ʈ ������ �׸���. 
static DWORD wmh_iconwin_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	IconWinPrivateStt *pPrivate;

	pPrivate = (IconWinPrivateStt*)pWin->pPrivate;
	if( pPrivate != NULL )
	{	// �������� �����Ѵ�.
		if( pPrivate->pImg != NULL )
			free_image16( pPrivate->pImg );
		// Private ������ �����Ѵ�.
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


