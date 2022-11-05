#include <bellona2.h>
#include "gui.h"

static DWORD wmh_flatw_pre_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	WinFrameStt			*pFrm;
	WinStyleStt			*pWStyle;
	FlatWPrivateStt		*pPrivate;

	//kdbg_printf( "wmh_flatw_create()\n" );

	pFrm    = &pWin->pWStyle->frame;
	pWStyle = pWin->pWStyle;

	// Private Data ������ �Ҵ��Ѵ�. 
	pPrivate = (FlatWPrivateStt*)kmalloc( sizeof( FlatWPrivateStt ) );
	if( pPrivate != NULL )
		memset( pPrivate, 0, sizeof( FlatWPrivateStt ) );
	pWin->pStylePrivate = pPrivate;

	// Ŭ���̾�Ʈ ������ ����Ѵ�. 
	recalc_client_area( pWin );

	return( WMHRV_CONTINUE );
}

static DWORD wmh_flatw_pre_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	// Private Data ������ �����Ѵ�. 
	if( pWin->pStylePrivate != NULL )
	{
		kfree( pWin->pStylePrivate );
		pWin->pStylePrivate = NULL;
	}

	return( WMHRV_CONTINUE );
}

static DWORD wmh_flatw_post_paint( WinStt *pWin, DWORD dwWMesgID, DWORD dwXY, DWORD dwHV )
{
	RectStt r;

	if( dwXY == 0 && dwHV == 0 )
	{
		flush_gra_buff( pWin, NULL );
		return( 0 );
	}

	dword_to_rect_xy( dwXY, &r );
	dword_to_rect_hv( dwHV, &r );
	
	// GraBuff�� ������ ��ũ���� �׸���.
	flush_gra_buff( pWin, &r );

	return( 0 );
}

// LBUTTON_DOWN	(���� ���콺 ��ư ����)
static DWORD wmh_flatw_pre_lbtn_dn( WinStt *pWin, DWORD dwWMesgID, DWORD dwWX, DWORD dwWY )
{
	// Top Window�� �����ϰ� ���ư���..
	if( is_top_window( pWin ) == 0 )
		set_top_window( pWin );

	// �޴�ó�� ������ �� ���� �Ӽ��̸� MOVE ���·� ���� �ʴ´�.
	if( ( pWin->gb.dwAttr & WIN_ATTR_NONMOVABLE ) == 0 )
		enter_move_mode( pWin );
	
	return( WMHRV_ABORT );
}

// �������� ����� �����ش�.
static DWORD wmh_flatw_pre_remake( WinStt *pWin, DWORD dwWMesgID, DWORD dwXY, DWORD dwHV )
{
	RectStt r;

	dword_to_rect_xy( dwXY, &r );
	dword_to_rect_hv( dwHV, &r );
	
	if( pWin == NULL || pWin->pWStyle == NULL )
		return( WMHRV_ABORT );

	k_fill_rect( &pWin->gb, &r, pWin->pWStyle->wDefaultBkColor );
	
	return( WMHRV_CONTINUE );
}

static GuiObjFuncStt wmfunc_flat[] = {
	{ WMESG_CREATE,			(GUIOBJ_FUNC)wmh_flatw_pre_create	, NULL			 					},
	{ WMESG_DESTROY,		(GUIOBJ_FUNC)wmh_flatw_pre_destroy	, NULL			 					},
	{ WMESG_PAINT,			NULL								, (GUIOBJ_FUNC)wmh_flatw_post_paint	},
	{ WMESG_LBTN_DN,		(GUIOBJ_FUNC)wmh_flatw_pre_lbtn_dn	, NULL		 						},
	{ WMESG_REMAKE,			(GUIOBJ_FUNC)wmh_flatw_pre_remake   , NULL                              },
	{ 0, NULL }
};

static WinStyleStt flat_wstyle = {
	"flatw",
	WSTYLE_FLAT,
	wmfunc_flat,
	{0},
	COLOR_MENU_BACK
};

WinStyleStt *init_flat_win()
{
	WinStyleStt *pWS;

	pWS = &flat_wstyle;

	pWS->frame.nFrameWidth = FLATW_FRAME_WIDTH;
	pWS->frame.nTitleV     = 0;

	return( pWS );
}
