#include <bellona2.h>
#include "gui.h"

static int remake_framew( WinStt *pWin )
{
	RectStt			r, rt;
	WinFrameStt		*pFrm;
	int				nClipV, nClipH;
	UINT16 			wBkColor, wLtGray, wMdGray, wDkGray, wWhite;

	pFrm = &pWin->pWStyle->frame;

	// 화면 전체를 지운다.
	wBkColor = get_sys_color( COLOR_INDEX_MENU_BACK );
	r.nY = r.nX = 0;
	r.nH = pWin->obj.r.nH;
	r.nV = pWin->obj.r.nV;	
	k_fill_rect( &pWin->gb, &r, wBkColor );

	wLtGray = RGB16( 145, 145, 145 );
	wMdGray = RGB16( 130, 130, 130 );
	wDkGray = RGB16(  40,  40,  40 );
	wWhite  = RGB16( 255, 255, 255 );

	// 테두리 2개를 친다.
	memcpy( &rt, &r, sizeof( rt ) );
	k_rect( &pWin->gb, &rt, LOOK_3D_IN, wDkGray );
	rt.nX += 1;
	rt.nY += 1;
	rt.nH -= 2;
	rt.nV -= 2;
	k_rect( &pWin->gb, &rt, LOOK_3D_IN, wWhite );
	rt.nX += 1;
	rt.nY += 1;
	rt.nH -= 2;
	rt.nV -= 2;
	k_rect( &pWin->gb, &rt, LOOK_3D_IN, wLtGray );

	// Vertical Rect of 4 Edge Clips.
	nClipV = 9;
	nClipH = 3;
	rt.nX = rt.nY = 0; 
	rt.nH = nClipH;
	rt.nV = nClipV;
	k_fill_rect( &pWin->gb, &rt, wMdGray );
	rt.nY = r.nV - nClipV; 
	k_fill_rect( &pWin->gb, &rt, wMdGray );
	rt.nX = r.nH - nClipH;
	k_fill_rect( &pWin->gb, &rt, wMdGray );
	rt.nY = 0;
	k_fill_rect( &pWin->gb, &rt, wMdGray );

	// Horizontal Rect of 4 Edge Clips.
	rt.nX = rt.nY = 0;
	rt.nH = nClipV;
	rt.nV = nClipH;
	k_fill_rect( &pWin->gb, &rt, wMdGray );
	rt.nY = r.nV - nClipH; 
	k_fill_rect( &pWin->gb, &rt, wMdGray );
	rt.nX = r.nH - nClipV;
	k_fill_rect( &pWin->gb, &rt, wMdGray );
	rt.nY = 0;
	k_fill_rect( &pWin->gb, &rt, wMdGray );
	
	// Up Left Edge Shadow
	k_line_v( &pWin->gb, nClipV-1,        0,        nClipH, wDkGray );
	k_line_h( &pWin->gb, nClipH-1, nClipH-1, nClipV-nClipH, wDkGray );
	k_line_v( &pWin->gb, nClipH-1, nClipH-1, nClipV-nClipH, wDkGray );
	k_line_h( &pWin->gb,        0, nClipV-1,        nClipH, wDkGray );

	// Up Right Edge Shadow
	k_line_h( &pWin->gb, r.nH - nClipV, nClipH-1, nClipV-nClipH, wDkGray );
	k_line_v( &pWin->gb,        r.nH-1,        0,        nClipV, wDkGray );
	k_line_h( &pWin->gb, r.nH - nClipH, nClipV-1,	   nClipH-1, wDkGray );

	// Down Left Edge Shadow
	k_line_v( &pWin->gb,      nClipH-1, r.nV - nClipV, nClipV-nClipH, wDkGray );
	k_line_h( &pWin->gb,	         0,        r.nV-1,        nClipV, wDkGray );
	k_line_v( &pWin->gb,	  nClipV-1,   r.nV-nClipH,		  nClipH, wDkGray );
	
	// Down Right Edge Shadow
	k_line_v( &pWin->gb,	     r.nH-1, r.nV-nClipV,   nClipV, wDkGray );
	k_line_h( &pWin->gb,  r.nH - nClipV,      r.nV-1, 	nClipV, wDkGray );

	// TitleV 영역에 선을 그린다.
	k_line_h( &pWin->gb,  nClipH, nClipH+pFrm->nTitleV+1, r.nH - nClipH*2, wMdGray );
	k_line_h( &pWin->gb,  nClipH, nClipH+pFrm->nTitleV+2, r.nH - nClipH*2, wWhite );
	

	return( 0 );
}

static DWORD wmh_framew_pre_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	WinFrameStt 		*pFrm;
	WinStyleStt 		*pWStyle;
	FramewPrivateStt 	*pPrivate;

	//kdbg_printf( "wmh_framew_create()\n" );

	pFrm	= &pWin->pWStyle->frame;
	pWStyle = pWin->pWStyle;

	// Private Data 영역을 할당한다. 
	pPrivate = (FramewPrivateStt*)kmalloc( sizeof( FramewPrivateStt ) );
	if( pPrivate != NULL )
		memset( pPrivate, 0, sizeof( FramewPrivateStt ) );
	pWin->pStylePrivate = pPrivate;

	// 클라이언트 영역을 계산한다. 
	recalc_client_area( pWin );

	// FrameW의 기본 틀을 그린다.
	remake_framew( pWin );

	return( WMHRV_CONTINUE );
}

static DWORD wmh_framew_pre_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	// Private Data 영역을 해제한다. 
	if( pWin->pStylePrivate != NULL )
	{
		kfree( pWin->pStylePrivate );
		pWin->pStylePrivate = NULL;
	}

	return( WMHRV_CONTINUE );
}

// 윈도우의 배경을 지워준다.
static DWORD wmh_framew_pre_remake( WinStt *pWin, DWORD dwWMesgID, DWORD dwXY, DWORD dwHV )
{
	RectStt r;

	dword_to_rect_xy( dwXY, &r );
	dword_to_rect_hv( dwHV, &r );
	
	if( pWin == NULL || pWin->pWStyle == NULL )
		return( WMHRV_ABORT );

	k_fill_rect( &pWin->gb, &r, pWin->pWStyle->wDefaultBkColor );
	
	return( WMHRV_CONTINUE );
}

static DWORD wmh_framew_post_paint( WinStt *pWin, DWORD dwWMesgID, DWORD dwXY, DWORD dwHV )
{
	RectStt r;

	if( dwXY == 0 && dwHV == 0 )
	{
		flush_gra_buff( pWin, NULL );
		return( 0 );
	}

	dword_to_rect_xy( dwXY, &r );
	dword_to_rect_hv( dwHV, &r );
	
	// GraBuff의 내용을 스크린에 그린다.
	flush_gra_buff( pWin, &r );

	return( 0 );
}

static int is_in_framew_edge_area( WinStt *pWin, int nX, int nY )
{
	RectStt r;
	int 	nMargin, nEdge;

	memcpy( &r, &pWin->obj.r, sizeof( RectStt ) );
	r.nX = r.nY = 0;
	nMargin = 3;
	nEdge	= 9;

	// DOWN
	if( r.nV - nMargin < nY && nY < r.nV )
	{
		// DOWN RIGHT
		if( r.nH - nEdge < nX && nX < r.nH )
			return( ECLIP_DOWN_RIGHT );

		// DOWN LEFT
		if( 0 < nX && nX < nEdge )
			return( ECLIP_DOWN_LEFT );

		return( ECLIP_DOWN_CENTER );
	}

	// UP
	if( 0 < nY && nY < nMargin )
	{
		// UP RIGHT
		if( r.nH - nEdge < nX && nX < r.nH )
			return( ECLIP_UP_RIGHT );
		
		// UP LEFT
		if( 0 < nX && nX < nEdge )
			return( ECLIP_UP_LEFT );
		
		return( ECLIP_UP_CENTER );
	}

	// LEFT
	if( 0 < nX && nX < nMargin )
	{
		// UP LEFT
		if( 0 < nY && nY < nEdge )
			return( ECLIP_UP_LEFT );

		// DOWN LEFT
		if( r.nV - nEdge < nY && nY < r.nV )
			return( ECLIP_DOWN_LEFT );		

		return( ECLIP_LEFT_CENTER );
	}

	// RIGHT
	if( r.nH - nMargin < nX && nX < r.nH )
	{
		// UP RIGHT
		if( 0 < nY && nY < nEdge )
			return( ECLIP_UP_RIGHT );

		// DOWN RIGHT
		if( r.nV - nEdge < nY && nY < r.nV )
			return( ECLIP_DOWN_RIGHT ); 	
	
		return( ECLIP_RIGHT_CENTER );
	}

	return( -1 );	
}

static DWORD wmh_framew_pre_lbtn_dn( WinStt *pWin, DWORD dwWMesgID, DWORD dwWX, DWORD dwWY )
{
	int nEdgeClip;
	
	// Top Window로 설정하고 돌아간다..
	if( is_top_window( pWin ) == 0 )
		set_top_window( pWin );

	// resize 영역안에 들어온 상태인가?
	nEdgeClip = is_in_framew_edge_area( pWin, (int)dwWX, (int)dwWY );
	if( nEdgeClip >= 0 )
	{
		enter_resize_mode( pWin, nEdgeClip );

		goto QUIT;		
	}	

	// 메뉴처럼 움직일 수 없는 속성이면 MOVE 상태로 들어가지 않는다.
	if( ( pWin->gb.dwAttr & WIN_ATTR_NONMOVABLE ) == 0 )
		enter_move_mode( pWin );

QUIT:	
	return( WMHRV_CONTINUE );
}

static int chk_edge_cursor( WinStt *pWin, DWORD dwWX, DWORD dwWY )
{
	int nEdgeClip, nR;

	nEdgeClip = is_in_framew_edge_area( pWin, (int)dwWX, (int)dwWY );
	if( nEdgeClip < 0 )
	{
		recover_resize_mouse_pointer( pWin );
		return( WMHRV_CONTINUE );
	}

	// 사이즈 변경 모드로 들어간다.
	nR = change_resize_mouse_pointer( pWin, nEdgeClip );	
	return( 0 );
}

static DWORD wmh_framew_post_mouse_move( WinStt *pWin, DWORD dwWMesgID, DWORD dwWX, DWORD dwWY )
{	
	// 마우스 포인터가 사이즈 변경 영역에 존재하면 변경한다.
	chk_edge_cursor( pWin, dwWX, dwWY );

	return( WMHRV_CONTINUE );	
}

static DWORD wmh_framew_post_mouse_move_out( WinStt *pWin, DWORD dwWMesgID, DWORD dwWX, DWORD dwWY )
{	
	// 마우스 포인터를 원상복구 한다.
	recover_resize_mouse_pointer( pWin );

	//chk_edge_cursor( pWin, dwWX, dwWY );
	
	return( WMHRV_CONTINUE );	
}

static GuiObjFuncStt wmfunc_framew[] = {
	{ WMESG_CREATE, 		(GUIOBJ_FUNC)wmh_framew_pre_create	 , NULL								 			},
	{ WMESG_DESTROY,		(GUIOBJ_FUNC)wmh_framew_pre_destroy  , NULL                              			}, 
	{ WMESG_PAINT,			NULL								 ,(GUIOBJ_FUNC)wmh_framew_post_paint 			},
	{ WMESG_REMAKE, 		(GUIOBJ_FUNC)wmh_framew_pre_remake	 , NULL								 			},
	{ WMESG_LBTN_DN,		(GUIOBJ_FUNC)wmh_framew_pre_lbtn_dn	 , NULL								 			},
	{ WMESG_MOUSE_MOVE,		NULL								 , (GUIOBJ_FUNC)wmh_framew_post_mouse_move 		},
	{ WMESG_MOUSE_MOVE_OUT,	NULL								 , (GUIOBJ_FUNC)wmh_framew_post_mouse_move_out	},
	{ 0, NULL }
};

static WinStyleStt framew_wstyle = {
	"framew",
	WSTYLE_FRAMEW,
	wmfunc_framew,
	{0},
	COLOR_MENU_BACK				// wDefaultBackColor
};

WinStyleStt *init_framew_win()
{
	WinStyleStt *pWS;

	pWS = &framew_wstyle;

	pWS->frame.nFrameWidth = 3;
	pWS->frame.nTitleV	   = 11;

	return( pWS );
}





