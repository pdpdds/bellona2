#include <bellona2.h>
#include "gui.h"

/************************************************
			1
  +-------------------+
  |			6		  |
  |					  |
  |-------------------|
  |			4    	  |
 2|					  |3
  |					  |
  +-------------------+
			5
*************************************************/
static DWORD simplew_make_screen( WinStt *pWin )
{
	RectStt				r;
	WinStyleStt			*pWS;
	WinFrameStt			*pFrm;
	SimpleWPrivateStt	*pSimple;
	RectStt				*pTitleR;
	ButtonStt			*pMain, *pMin, *pMax, *pExit;

	// 틀을 먼저 그린다. 
	pWS  = pWin->pWStyle;
	pFrm = &pWS->frame;
	pSimple = (SimpleWPrivateStt*)pWin->pStylePrivate;

	pExit = pSimple->pExit;
	pMain = pSimple->pMain;
	pMax  = pSimple->pMax;
	pMin  = pSimple->pMin;
	pTitleR = &pSimple->title_r;

	// (1)
	r.nX = 0;
	r.nY = 0;
	r.nH = pWin->obj.r.nH;
	r.nV = pFrm->nFrameWidth;
	k_fill_rect( &pWin->gb, &r, SIMPLEW_FRAME_COLOR );
		
	// (2)
	r.nX = 0;
	r.nY = pFrm->nFrameWidth;
	r.nH = pFrm->nFrameWidth;
	r.nV = pWin->obj.r.nV - pFrm->nFrameWidth;
	k_fill_rect( &pWin->gb, &r, SIMPLEW_FRAME_COLOR );

	// (3)
	r.nX = pWin->obj.r.nH - pFrm->nFrameWidth;
	r.nY = pFrm->nFrameWidth;
	r.nH = pFrm->nFrameWidth;
	r.nV = pWin->obj.r.nV - pFrm->nFrameWidth;
	k_fill_rect( &pWin->gb, &r, SIMPLEW_FRAME_COLOR );

	// (4)
	r.nX = pFrm->nFrameWidth;
	r.nY = pFrm->nFrameWidth + pFrm->nTitleV;
	r.nH = pWin->obj.r.nH - pFrm->nFrameWidth * 2;
	r.nV = pFrm->nFrameWidth;
	k_fill_rect( &pWin->gb, &r, SIMPLEW_FRAME_COLOR );

	// (5)
	r.nX = pFrm->nFrameWidth;
	r.nY = pWin->obj.r.nV - pFrm->nFrameWidth;
	r.nH = pWin->obj.r.nH - pFrm->nFrameWidth * 2;
	r.nV = pFrm->nFrameWidth;
	k_fill_rect( &pWin->gb, &r, SIMPLEW_FRAME_COLOR );

	// 3D로 보이도록 한다. 
	screen_to_win( &r, pWin->gb.pR, &pWin->obj.r );
	k_3d_look( &pWin->gb, &r, LOOK_3D_IN, LOOK_3D_DEPRESSED, _3D_BORDER_LIGHT, _3D_BORDER_DK );	// 프레임 외곽
	
	// 이건 그리지 않는 편이 더 나아 보인다.
	//r.nX = pFrm->nFrameWidth;
	//r.nY = pFrm->nFrameWidth*2 + pFrm->nTitleV;
	//r.nH = pWin->r.nH - pFrm->nFrameWidth * 2;
	//r.nV = pWin->r.nV - ( pFrm->nFrameWidth * 3 + pFrm->nTitleV );
	//k_3d_look( &pWin->gb, &r, LOOK_3D_IN, LOOK_3D_PRESSED, _3D_BORDER_LIGHT, _3D_BORDER_DK  );			// 프레임 내부

	// 타이틀을 그린다. 
	k_fill_rect2( &pWin->gb, pTitleR, SIMPLEW_TITLE_COLOR );

	// 타이틀 아이콘을 그린다. 
	kdraw_button_gb( pMin  );
	kdraw_button_gb( pMax  );
	kdraw_button_gb( pExit );
	kdraw_button_gb( pMain );
	kflush_button_gb( &pWin->gb, pMin  );
	kflush_button_gb( &pWin->gb, pMax  );
	kflush_button_gb( &pWin->gb, pExit );
	kflush_button_gb( &pWin->gb, pMain );

	// Window Title을 출력한다. 
	drawtext_xy( 
		&pWin->gb,											// WIN
		pFrm->nFrameWidth + pMain->pImg->nH + 10,			// X
		pFrm->nFrameWidth + 1,								// Y
		get_system_font( IDR_BF_BASE14 ),					// Font
		pWin->szTitle,										// String
		SIMPLEW_TITLE_TEXT_COLOR,
		0 );												// Effect	
	
	return( 0 );
}

// simplew style 윈도우 생성.
static DWORD wmh_simplew_pre_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	WinStyleStt				*pWS;
	WinFrameStt				*pFrm;
	int 					nX, nY;
	SimpleWPrivateStt		*pSimple;
	int						nInterval;
	RectStt					*pTitleR;
	ImageStt				*pExitImg, *pMinImg, *pMaxImg;

	pFrm = &pWin->pWStyle->frame;
	pWS	 = pWin->pWStyle;

	// Private Data 영역을 할당한다. 
	pSimple = (SimpleWPrivateStt*)kmalloc( sizeof( SimpleWPrivateStt ) );
	if( pSimple != NULL )
		memset( pSimple, 0, sizeof( SimpleWPrivateStt ) );
	pWin->pStylePrivate = pSimple;

	// 클라이언트 영역을 계산한다. 
	recalc_client_area( pWin );

	// 타이틀 영역을 계산한다.
	pTitleR = &pSimple->title_r;
	pTitleR->nX	= pFrm->nFrameWidth;
	pTitleR->nY	= pFrm->nFrameWidth;
	pTitleR->nH	= pWin->obj.r.nH - pFrm->nFrameWidth * 2;
	pTitleR->nV	= pFrm->nTitleV;

	pExitImg = get_sys_icon( IDI_EXIT     );
	pMinImg	 = get_sys_icon( IDI_MINIMIZE );
	pMaxImg	 = get_sys_icon( IDI_MAXIMIZE );

	nInterval = 0;
	nX = pTitleR->nH - pExitImg->nH - nInterval;
	nY = pFrm->nFrameWidth + nInterval;	  	

	// Exit 버튼을 생성한다.
	pSimple->pExit = kcreate_button( pWin,						// Parent Win
								   0,							    // Image ID
								   NULL,							// Button Text
								   CONTROL_ATTR_BORDER_ON_MOUSE,	// Attribute
								   SIMPLEW_TITLE_COLOR,			 	// Background Color
								   0,								// Text Color
								   0,						 		// Font ID
								   nX,								// nX
								   nY,								// nY
								   pExitImg->nH,					// nH
								   pExitImg->nV,					// nV
								   WMESG_CLOSE,					 	// Window Message
								   0,					 			// Param A
								   0								// Param B
								   );
	pSimple->pExit->pImg = pExitImg;
	nX -= ( pExitImg->nH + nInterval );

	// Minimize 버튼을 생성한다.
	pSimple->pMin = kcreate_button( pWin,						// Parent Win
								   0,								// Image ID
								   NULL,							// Button Text
								   CONTROL_ATTR_BORDER_ON_MOUSE,	// Attribute
								   SIMPLEW_TITLE_COLOR, 			// Background Color
								   0,								// Text Color
								   0,								// Font ID
								   nX,								// nX
								   nY,								// nY
								   pMinImg->nH,						// nH
								   pMinImg->nV,						// nV
								   WMESG_MINIMIZE, 					// Window Message
								   0,								// Param A
								   0								// Param B
								   );
	pSimple->pMin->pImg = pMinImg;
	nX -= ( pMinImg->nH + nInterval );

	// Maximize 버튼을 생성한다.
	pSimple->pMax = kcreate_button( pWin,						// Parent Win
								   0,								// Image ID
								   NULL,							// Button Text
								   CONTROL_ATTR_BORDER_ON_MOUSE,	// Attribute
								   SIMPLEW_TITLE_COLOR, 			// Background Color
								   0,								// Text Color
								   0,								// Font ID
								   nX,								// nX
								   nY,								// nY
								   pMaxImg->nH, 					// nH
								   pMaxImg->nV, 					// nV
								   WMESG_MAXIMIZE,					// Window Message
								   0,								// Param A
								   0								// Param B
								   );
	pSimple->pMax->pImg = pMaxImg;

	// Main Icon 버튼을 생성한다.
	pSimple->pMain = kcreate_button( pWin,						// Parent Win
								   pWin->dwMainIconID,				// Image ID
								   NULL,							// Button Text
								   CONTROL_ATTR_BORDER_ON_MOUSE,	// Attribute
								   SIMPLEW_TITLE_COLOR, 			// Background Color
								   0,								// Text Color
								   0,								// Font ID
								   pFrm->nFrameWidth + nInterval,	// nX
								   nY,								// nY
								   SIMPLEW_MAIN_ICON_H,				// nH
								   SIMPLEW_MAIN_ICON_V,				// nV
								   WMESG_MAXIMIZE,					// Window Message
								   0,								// Param A
								   0								// Param B
								   );

	return( WMHRV_CONTINUE );
}

static DWORD wmh_simplew_post_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	// 초기 화면을 그린다.
	simplew_make_screen( pWin );
	return( 0 );
}

static DWORD wmh_simplew_post_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	SimpleWPrivateStt	*pSimple;

	pSimple = (SimpleWPrivateStt*)pWin->pStylePrivate;
	if( pSimple != NULL )
	{
		// 버튼을 해제 한다.	  
		pSimple->pExit = NULL;			//
		pSimple->pMin  = NULL;			// preloaded system icon은 해제하면 안된다.
		pSimple->pMin  = NULL;			//
		kclose_button( pSimple->pMain );
		kclose_button( pSimple->pExit );
		kclose_button( pSimple->pMin  );
		kclose_button( pSimple->pMin  );
		pSimple->pMain = NULL;

		// Style Private Data 영역을 해제한다. 
		kfree( pSimple );
		pWin->pStylePrivate = NULL;
	}

	return( 0 );
}

static DWORD wmh_simplew_post_paint( WinStt *pWin, DWORD dwWMesgID, DWORD dwXY, DWORD dwHV )
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

// XY 좌표가 TITLE 영역 내에 포함되는가?
static int is_in_simplew_title( WinStt *pWin, int nX, int nY )
{
	SimpleWPrivateStt	*pSimple;
	
	pSimple = (SimpleWPrivateStt*)pWin->pStylePrivate;

	return( is_in_rect( &pSimple->title_r, nX, nY ) );
}

// LBUTTON_DOWN	(왼쪽 마우스 버튼 눌림)
static DWORD wmh_simplew_pre_lbtn_dn( WinStt *pWin, DWORD dwWMesgID, DWORD dwWX, DWORD dwWY )
{
	RectStt				r;
	WinFrameStt			*pFrm;
	SimpleWPrivateStt	*pSimple;
	int					nRefreshTitleFlag;
	ButtonStt			*pMain, *pMax, *pMin, *pExit;

	pFrm    = &pWin->pWStyle->frame;
	pSimple = (SimpleWPrivateStt*)pWin->pStylePrivate;

	pExit = pSimple->pExit;
	pMain = pSimple->pMain;
	pMax  = pSimple->pMax;
	pMin  = pSimple->pMin;

	nRefreshTitleFlag = 0;

	// Top Window로 설정.
	if( is_top_window( pWin ) == 0 )
		set_top_window( pWin );

	get_client_rect( pWin, &r );
	r.nX += pWin->obj.r.nX;
	r.nY += pWin->obj.r.nY;

	// 타이틀 영역인가?
	if( is_in_simplew_title( pWin, (int)dwWX, (int)dwWY ) )
	{	
		enter_move_mode( pWin );
	}	
	else if( is_in_rect( &r, (int)dwWX, (int)dwWY ) )
	{	// 윈도우의 클라이언트 영역
		//call_message_func( pWin, dwWMesgID, dwWX-(DWORD)pFrm->nFrameWidth, dwWY-(DWORD)pFrm->nFrameWidth );	
		return( WMHRV_CONTINUE );
	}																	
	
	return( WMHRV_ABORT );
}

// 타이틀 바 버튼으로부터 날아온 것들.
static DWORD wmh_simplew_pre_remake( WinStt *pWin, DWORD dwWMesgID, DWORD dwXY, DWORD dwHV )
{
	WinStyleStt 		*pWS;
	SimpleWPrivateStt	*pSimple;
	RectStt 			r, *pTitleR, *pR, rect;

	pR = &rect;
	dword_to_rect_xy( dwXY, pR );
	dword_to_rect_hv( dwHV, pR );
	
	// 틀을 먼저 그린다. 
	pWS  = pWin->pWStyle;
	pSimple = (SimpleWPrivateStt*)pWin->pStylePrivate;
	pTitleR = &pSimple->title_r;

	r.nX = pR->nX;
	r.nH = pR->nH;
	r.nY = pTitleR->nY;
	r.nV = pTitleR->nV;

	// 타이틀을 그린다. 
	k_fill_rect2( &pWin->gb, &r, SIMPLEW_TITLE_COLOR );

	return( WMHRV_CONTINUE );
}

static GuiObjFuncStt wmfunc_simplew[] = {
	{ WMESG_CREATE,			(GUIOBJ_FUNC)wmh_simplew_pre_create	,	(GUIOBJ_FUNC)wmh_simplew_post_create  	},
	{ WMESG_DESTROY,		NULL								,	(GUIOBJ_FUNC)wmh_simplew_post_destroy 	},
	{ WMESG_PAINT,			NULL								,	(GUIOBJ_FUNC)wmh_simplew_post_paint	  	},
	{ WMESG_LBTN_DN,		(GUIOBJ_FUNC)wmh_simplew_pre_lbtn_dn,	NULL 					 				},
	{ WMESG_REMAKE, 		(GUIOBJ_FUNC)wmh_simplew_pre_remake ,   NULL 									},
	{ 0, NULL }
};

static WinStyleStt simple_wstyle = {
	"simplew",
	WSTYLE_SIMPLE,
	wmfunc_simplew,
	{0},
	COLOR_MENU_BACK	
};

// Simplew 스타일 윈도우의 초기화 함수.
WinStyleStt *init_simple_win()
{
	WinStyleStt *pWS;

	pWS = &simple_wstyle;

	pWS->frame.nFrameWidth = SIMPLEW_FRAME_WIDTH;
	pWS->frame.nTitleV     = SIMPLEW_TITLE_V;

	return( pWS );
}

