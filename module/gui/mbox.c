#include <bellona2.h>
#include "gui.h"

static DWORD wmh_mbox_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	MBoxPrivateStt *pPrivate;

	pPrivate = (MBoxPrivateStt*)pWin->pPrivate;

	if( pWin->pPrivate != NULL )
	{
		// 버튼을 해제한다.  
		// preloaded sys icon은 해제되지 않도록 먼저 pImg를 NULL로 설정한 후 버튼을 제거한다.
		pPrivate->pExit->pImg = NULL;
		kclose_button( pPrivate->pExit );
		pPrivate->pExit = NULL;		

		if( pPrivate->pOk != NULL )
		{
			kclose_button( pPrivate->pOk );
			pPrivate->pOk = NULL;
		}
		if( pPrivate->pCancel != NULL )
		{
			kclose_button( pPrivate->pCancel );
			pPrivate->pCancel = NULL;
		}
		if( pPrivate->pYes != NULL )
		{
			kclose_button( pPrivate->pYes );
			pPrivate->pYes = NULL;
		}
		if( pPrivate->pNo != NULL )
		{
			kclose_button( pPrivate->pNo );
			pPrivate->pNo = NULL;
		}
		
		// Private 구조체를 해제한다.
		kfree( pWin->pPrivate );
		pWin->pPrivate = NULL;
	}

	return( WMHRV_CONTINUE );
}

// message box의 화면을 구성한다.
static DWORD mbox_make_screen( WinStt *pWin )
{
	RectStt			r;
	MBoxPrivateStt	*pPrivate;
	int				nX, nY, nH;

	// 클라이언트 영역을 지운다. 
	//screen_to_win( &r, pWin->gb.pR, &pWin->ct_r );
	get_client_rect( pWin, &r );

	k_fill_rect( &pWin->gb, &r, MBOX_BACK_COLOR );

	// 윈도우의 테두리를 그린다.
	k_3d_look( &pWin->gb, &r, LOOK_3D_IN, LOOK_3D_DEPRESSED, MBOX_LT_COLOR, MBOX_DK_COLOR );

	nX = 1;
	nY = MBOX_TITLE_V;
	nH = r.nH - (nX * 2);
	
	// TITLE UNDER LINE을 긋는다.
	k_line_h( &pWin->gb, nX, nY, nH, MBOX_DK_COLOR );
	k_line_h( &pWin->gb, nX, nY+1,   nH, MBOX_LT_COLOR );

	// Main 아이콘을 그린다.
	pPrivate = (MBoxPrivateStt*)pWin->pPrivate;
	copy_image16( &pWin->gb, pPrivate->pMBoxIcon, 1, 1, pPrivate->pMBoxIcon->nH, pPrivate->pMBoxIcon->nV, 0, 0 ); 

	// Exit 버튼을 그린다. (최초로 한 번은 그려 주어야 한다.)
	kdraw_button_gb( pPrivate->pExit );
	kflush_button_gb( &pWin->gb, pPrivate->pExit );

	return( 0 );
}

// 타이틀과 메시지를 출력한다.
static int draw_mbox_str( WinStt *pWin )
{
	FontStt			*pFont;
	int				nX, nY;
	MBoxPrivateStt	*pPrivate;

	pPrivate = (MBoxPrivateStt*)pWin->pPrivate;
	if( pPrivate == NULL )
	{
		kdbg_printf( "draw_mbox_str: pPrivate is NULL!\n" );
		return( -1 );
	}

	// 타이틀을 출력한다.
	nX = 20;
	nY = 4;
	pFont = get_system_font( MBOX_FONT );
	drawtext_xy( 
		&pWin->gb,													
		nX,
		nY,
		pFont,						
		pWin->szTitle,
		RGB16( 0, 0, 0 ),
		0 );													

	// 메시지를 출력한다.
	nY += 20;
	drawtext_xy( 
		&pWin->gb,													
		nX,
		nY,
		pFont,						
		pPrivate->szStr, 
		RGB16( 0, 0, 0 ),
		0 );													

	return( 0 );
}

static DWORD wmh_mbox_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( WMHRV_CONTINUE );
}

static DWORD wmh_mbox_control( WinStt *pWin, DWORD dwWMesgID, DWORD dwConID, DWORD dwParamB )
{
	MBoxPrivateStt  *pPrivate;

	pPrivate = (MBoxPrivateStt*)pWin->pPrivate;

	// Parent쪽 으로 Modal 메시지를 날린다.
	kpost_message( pWin->pParentWin, WMESG_MODAL, pPrivate->dwMBoxID, dwConID );
	
	// 스스로에게 DESTROY를 날린다.
	kpost_message( pWin, WMESG_DESTROY, 0, 0 );

	return( WMHRV_CONTINUE );
}

static WMFuncStt mbox_marray[] = {
	{ WMESG_CREATE		   , wmh_mbox_create			},
	{ WMESG_DESTROY		   , wmh_mbox_destroy			},
	{ WMESG_CONTROL		   , wmh_mbox_control			},
	{ 0, NULL }
};

// 라인 수를 구한다.
static int calc_lines( char *pS )
{
	int nI, nLine;

	if( pS == NULL || pS[0] == 0 )
		return( 0 );

	nLine = 1;
	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( pS[nI] == 13 )
			nLine++;
	}
	return( nLine );
}

// 메시지 박스를 생성한다.
WinStt *message_box( WinStt *pParent, DWORD dwMBoxID, char *pStr, char *pTitle, int nH, int nVert, DWORD dwStyle )
{
	WinStt			*pWin;
	ImageStt		*pImg;
	FontStt			*pFont;
	ButtonStt		*pExit;
	MBoxPrivateStt	*pPrivate;
	RectStt 		r, *pClientR;
	int				nX, nY, nV, nLines, nButtons, nButtonInterval;

	if( pParent->pModalWin != NULL )
	{	// 이미 모달박스를 가지고 있다.
		kdbg_printf( "Window %X already has a modal box\n", (DWORD)pParent );
		return( NULL );
	}

	// MessageBox의 Vertical Size를 계산한다.
	pFont = get_system_font( MBOX_FONT );
	nLines = calc_lines( pStr );
	nV = MBOX_TITLE_V + ( pFont->cV * ( nLines + 1 ) ) + MBOX_MARGIN;
	if( dwStyle != 0 )
	{	// 적어도 버튼이 한 개는 있다.
		nV += MBOX_BUTTON_V;
	}
	nY = nV - MBOX_MARGIN - MBOX_BUTTON_V;

	// 윈도우를 생성한다. (Parent Window의 중앙)
	r.nH = nH;
	r.nV = nV;
	r.nX = pParent->obj.r.nX + ( pParent->obj.r.nH / 2);
	r.nX -= r.nH / 2;
	if( r.nX < 0 )
		r.nX = 0;
	r.nY = pParent->obj.r.nY + ( pParent->obj.r.nV / 2);
	r.nY -= r.nV / 2;
	if( r.nY < 0 )
		r.nY = 0;
	// Create Message Handler까지 바로 수행된다.
	pWin = kcreate_window( pParent->pWThread, pParent, WSTYLE_FLAT, &r, mbox_marray, 0, 0, IDI_MBOX, 0, 0 );
	if( pWin == NULL )
		return( NULL );

	// 여기서 이걸 해 줘야 한다니...
	recalc_client_area( pWin );

	// 타이틀을 설정한다. 
	set_window_title( pWin, "Notice" );
	
	// Win Private 영역을 할당한다. 
	pPrivate = (MBoxPrivateStt*)kmalloc( sizeof( MBoxPrivateStt ) );
	pWin->pPrivate = pPrivate;
	if( pPrivate == NULL )
	{
		kdbg_printf( "message_box: private allocation failed!\n" );
	}

	memset( pPrivate, 0, sizeof( MBoxPrivateStt ) );

	// IDI_MBOX는 시스템 리소스이므로 해제할 필요가 없다.
	pPrivate->pMBoxIcon = get_sys_icon( IDI_MBOX );
	
	// MessgeBox의 Exit 버튼을 생성한다.
	pClientR = get_client_rect( pWin, NULL );
	pImg	 = get_sys_icon( IDI_EXIT );
	pExit	 = kcreate_button( pWin, 								// Parent Win
							   0,										// Image ID
							   NULL,									// Button Text
							   CONTROL_ATTR_BORDER, 					// Attribute
							   MBOX_BACK_COLOR, 						// Background Color
							   0,										// Text Color
							   IDR_BF_BASE11,							// Font ID
							   pClientR->nH - (pImg->nH + 3),			// nX
							   3,										// nY
							   pImg->nH,								// nH
							   pImg->nV,								// nV
							   WMESG_CONTROL,							// Window Message
							   MBOX_BUTTON_CANCEL,						// Param A
							   0										// Param B
							   );
											   
	pPrivate->pExit = pExit;

	// 이미지는 별도로 설정한다.
	pExit->pImg = pImg;

	// 메시지 박스 배경 지우기, Exit 버튼, Title Line을 그린다.
	mbox_make_screen( pWin ); 
	
	// 타이틀과 내용을 기록한다.
	strcpy( pPrivate->szStr, pStr );
	strcpy( pWin->szTitle, pTitle );
	draw_mbox_str( pWin );

	// 스타일을 설정한다.
	pPrivate->dwStyle = dwStyle;
	// MBOX ID를 설정한다.
	pPrivate->dwMBoxID = dwMBoxID;

	// 버튼의 개수를 생성한다.
	nButtons = 0;
	if( dwStyle & MBOX_BUTTON_OK     ) nButtons++;
	if( dwStyle & MBOX_BUTTON_YES    ) nButtons++;
	if( dwStyle & MBOX_BUTTON_NO     ) nButtons++;
	if( dwStyle & MBOX_BUTTON_CANCEL ) nButtons++;

	nButtonInterval = ( nH - ( nButtons * MBOX_BUTTON_H ) ) / ( nButtons + 1 );
	nX = nButtonInterval;
	if( dwStyle & MBOX_BUTTON_OK )
	{
		pPrivate->pOk = kcreate_button( pWin, 					 	 // Parent Win
									   0,								 // Image ID
									   "Ok",							 // Button Text
									   CONTROL_ATTR_BORDER, 			 // Attribute
									   MBOX_BTN_BORDER_COLOR,			 // Background Color
									   0,								 // Text Color
									   MBOX_FONT,						 // Font ID
									   nX,								 // nX
									   nY,								 // nY
									   MBOX_BUTTON_H,					 // nH
									   MBOX_BUTTON_V,					 // nV
									   WMESG_CONTROL,					 // Window Message
									   MBOX_BUTTON_OK,					 // Param A
									   0								 // Param B
									   );
		nX += MBOX_BUTTON_H + nButtonInterval;
		kdraw_button_gb( pPrivate->pOk );
		kflush_button_gb( &pWin->gb, pPrivate->pOk );
	}
	if( dwStyle & MBOX_BUTTON_YES )
	{
		pPrivate->pYes = kcreate_button( pWin,					 	 // Parent Win
									   0,								 // Image ID
									   "Yes",							 // Button Text
									   CONTROL_ATTR_BORDER, 			 // Attribute
									   MBOX_BTN_BORDER_COLOR,			 // Background Color
									   0,								 // Text Color
									   MBOX_FONT,						 // Font ID
									   nX,								 // nX
									   nY, 								 // nY
									   MBOX_BUTTON_H,					 // nH
									   MBOX_BUTTON_V,					 // nV
									   WMESG_CONTROL,					 // Window Message
									   MBOX_BUTTON_YES,					 // Param A
									   0								 // Param B
									   );
			nX += MBOX_BUTTON_H + nButtonInterval;
			kdraw_button_gb( pPrivate->pYes );
			kflush_button_gb( &pWin->gb, pPrivate->pYes );
	}
	if( dwStyle & MBOX_BUTTON_NO )
	{
		pPrivate->pNo = kcreate_button( pWin,						 // Parent Win
									   0,								 // Image ID
									   "No",							 // Button Text
									   CONTROL_ATTR_BORDER, 			 // Attribute
									   MBOX_BTN_BORDER_COLOR,			 // Background Color
									   0,								 // Text Color
									   MBOX_FONT,						 // Font ID
									   nX,								 // nX
									   nY,								 // nY
									   MBOX_BUTTON_H,					 // nH
									   MBOX_BUTTON_V,					 // nV
									   WMESG_CONTROL,					 // Window Message
									   MBOX_BUTTON_NO, 				 	// Param A
									   0								 // Param B
									   );
			nX += MBOX_BUTTON_H + nButtonInterval;
			kdraw_button_gb( pPrivate->pNo );
			kflush_button_gb( &pWin->gb, pPrivate->pNo );
	}
	if( dwStyle & MBOX_BUTTON_CANCEL )
	{
		pPrivate->pCancel = kcreate_button( pWin,					 // Parent Win
									   0,								 // Image ID
									   "Cancel",						 // Button Text
									   CONTROL_ATTR_BORDER, 			 // Attribute
									   MBOX_BTN_BORDER_COLOR,			 // Background Color
									   0,								 // Text Color
									   MBOX_FONT,						 // Font ID
									   nX,								 // nX
									   nY,								 // nY
									   MBOX_BUTTON_H,					 // nH
									   MBOX_BUTTON_V,					 // nV
									   WMESG_CONTROL,					 // Window Message
									   MBOX_BUTTON_CANCEL, 				 // Param A
									   0								 // Param B
									   );
			nX += MBOX_BUTTON_H + nButtonInterval;
			kdraw_button_gb( pPrivate->pCancel );
			kflush_button_gb( &pWin->gb, pPrivate->pCancel );
	}

	return( pWin );
}

