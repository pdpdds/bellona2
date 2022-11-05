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

	// Ʋ�� ���� �׸���. 
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

	// 3D�� ���̵��� �Ѵ�. 
	screen_to_win( &r, pWin->gb.pR, &pWin->obj.r );
	k_3d_look( &pWin->gb, &r, LOOK_3D_IN, LOOK_3D_DEPRESSED, _3D_BORDER_LIGHT, _3D_BORDER_DK );	// ������ �ܰ�
	
	// �̰� �׸��� �ʴ� ���� �� ���� ���δ�.
	//r.nX = pFrm->nFrameWidth;
	//r.nY = pFrm->nFrameWidth*2 + pFrm->nTitleV;
	//r.nH = pWin->r.nH - pFrm->nFrameWidth * 2;
	//r.nV = pWin->r.nV - ( pFrm->nFrameWidth * 3 + pFrm->nTitleV );
	//k_3d_look( &pWin->gb, &r, LOOK_3D_IN, LOOK_3D_PRESSED, _3D_BORDER_LIGHT, _3D_BORDER_DK  );			// ������ ����

	// Ÿ��Ʋ�� �׸���. 
	k_fill_rect2( &pWin->gb, pTitleR, SIMPLEW_TITLE_COLOR );

	// Ÿ��Ʋ �������� �׸���. 
	kdraw_button_gb( pMin  );
	kdraw_button_gb( pMax  );
	kdraw_button_gb( pExit );
	kdraw_button_gb( pMain );
	kflush_button_gb( &pWin->gb, pMin  );
	kflush_button_gb( &pWin->gb, pMax  );
	kflush_button_gb( &pWin->gb, pExit );
	kflush_button_gb( &pWin->gb, pMain );

	// Window Title�� ����Ѵ�. 
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

// simplew style ������ ����.
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

	// Private Data ������ �Ҵ��Ѵ�. 
	pSimple = (SimpleWPrivateStt*)kmalloc( sizeof( SimpleWPrivateStt ) );
	if( pSimple != NULL )
		memset( pSimple, 0, sizeof( SimpleWPrivateStt ) );
	pWin->pStylePrivate = pSimple;

	// Ŭ���̾�Ʈ ������ ����Ѵ�. 
	recalc_client_area( pWin );

	// Ÿ��Ʋ ������ ����Ѵ�.
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

	// Exit ��ư�� �����Ѵ�.
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

	// Minimize ��ư�� �����Ѵ�.
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

	// Maximize ��ư�� �����Ѵ�.
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

	// Main Icon ��ư�� �����Ѵ�.
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
	// �ʱ� ȭ���� �׸���.
	simplew_make_screen( pWin );
	return( 0 );
}

static DWORD wmh_simplew_post_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	SimpleWPrivateStt	*pSimple;

	pSimple = (SimpleWPrivateStt*)pWin->pStylePrivate;
	if( pSimple != NULL )
	{
		// ��ư�� ���� �Ѵ�.	  
		pSimple->pExit = NULL;			//
		pSimple->pMin  = NULL;			// preloaded system icon�� �����ϸ� �ȵȴ�.
		pSimple->pMin  = NULL;			//
		kclose_button( pSimple->pMain );
		kclose_button( pSimple->pExit );
		kclose_button( pSimple->pMin  );
		kclose_button( pSimple->pMin  );
		pSimple->pMain = NULL;

		// Style Private Data ������ �����Ѵ�. 
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
	
	// GraBuff�� ������ ��ũ���� �׸���.
	flush_gra_buff( pWin, &r );

	return( 0 );
}

// XY ��ǥ�� TITLE ���� ���� ���ԵǴ°�?
static int is_in_simplew_title( WinStt *pWin, int nX, int nY )
{
	SimpleWPrivateStt	*pSimple;
	
	pSimple = (SimpleWPrivateStt*)pWin->pStylePrivate;

	return( is_in_rect( &pSimple->title_r, nX, nY ) );
}

// LBUTTON_DOWN	(���� ���콺 ��ư ����)
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

	// Top Window�� ����.
	if( is_top_window( pWin ) == 0 )
		set_top_window( pWin );

	get_client_rect( pWin, &r );
	r.nX += pWin->obj.r.nX;
	r.nY += pWin->obj.r.nY;

	// Ÿ��Ʋ �����ΰ�?
	if( is_in_simplew_title( pWin, (int)dwWX, (int)dwWY ) )
	{	
		enter_move_mode( pWin );
	}	
	else if( is_in_rect( &r, (int)dwWX, (int)dwWY ) )
	{	// �������� Ŭ���̾�Ʈ ����
		//call_message_func( pWin, dwWMesgID, dwWX-(DWORD)pFrm->nFrameWidth, dwWY-(DWORD)pFrm->nFrameWidth );	
		return( WMHRV_CONTINUE );
	}																	
	
	return( WMHRV_ABORT );
}

// Ÿ��Ʋ �� ��ư���κ��� ���ƿ� �͵�.
static DWORD wmh_simplew_pre_remake( WinStt *pWin, DWORD dwWMesgID, DWORD dwXY, DWORD dwHV )
{
	WinStyleStt 		*pWS;
	SimpleWPrivateStt	*pSimple;
	RectStt 			r, *pTitleR, *pR, rect;

	pR = &rect;
	dword_to_rect_xy( dwXY, pR );
	dword_to_rect_hv( dwHV, pR );
	
	// Ʋ�� ���� �׸���. 
	pWS  = pWin->pWStyle;
	pSimple = (SimpleWPrivateStt*)pWin->pStylePrivate;
	pTitleR = &pSimple->title_r;

	r.nX = pR->nX;
	r.nH = pR->nH;
	r.nY = pTitleR->nY;
	r.nV = pTitleR->nV;

	// Ÿ��Ʋ�� �׸���. 
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

// Simplew ��Ÿ�� �������� �ʱ�ȭ �Լ�.
WinStyleStt *init_simple_win()
{
	WinStyleStt *pWS;

	pWS = &simple_wstyle;

	pWS->frame.nFrameWidth = SIMPLEW_FRAME_WIDTH;
	pWS->frame.nTitleV     = SIMPLEW_TITLE_V;

	return( pWS );
}

