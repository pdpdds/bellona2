#include <bellona2.h>
#include "gui.h"

static DWORD wmh_mbox_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	MBoxPrivateStt *pPrivate;

	pPrivate = (MBoxPrivateStt*)pWin->pPrivate;

	if( pWin->pPrivate != NULL )
	{
		// ��ư�� �����Ѵ�.  
		// preloaded sys icon�� �������� �ʵ��� ���� pImg�� NULL�� ������ �� ��ư�� �����Ѵ�.
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
		
		// Private ����ü�� �����Ѵ�.
		kfree( pWin->pPrivate );
		pWin->pPrivate = NULL;
	}

	return( WMHRV_CONTINUE );
}

// message box�� ȭ���� �����Ѵ�.
static DWORD mbox_make_screen( WinStt *pWin )
{
	RectStt			r;
	MBoxPrivateStt	*pPrivate;
	int				nX, nY, nH;

	// Ŭ���̾�Ʈ ������ �����. 
	//screen_to_win( &r, pWin->gb.pR, &pWin->ct_r );
	get_client_rect( pWin, &r );

	k_fill_rect( &pWin->gb, &r, MBOX_BACK_COLOR );

	// �������� �׵θ��� �׸���.
	k_3d_look( &pWin->gb, &r, LOOK_3D_IN, LOOK_3D_DEPRESSED, MBOX_LT_COLOR, MBOX_DK_COLOR );

	nX = 1;
	nY = MBOX_TITLE_V;
	nH = r.nH - (nX * 2);
	
	// TITLE UNDER LINE�� �ߴ´�.
	k_line_h( &pWin->gb, nX, nY, nH, MBOX_DK_COLOR );
	k_line_h( &pWin->gb, nX, nY+1,   nH, MBOX_LT_COLOR );

	// Main �������� �׸���.
	pPrivate = (MBoxPrivateStt*)pWin->pPrivate;
	copy_image16( &pWin->gb, pPrivate->pMBoxIcon, 1, 1, pPrivate->pMBoxIcon->nH, pPrivate->pMBoxIcon->nV, 0, 0 ); 

	// Exit ��ư�� �׸���. (���ʷ� �� ���� �׷� �־�� �Ѵ�.)
	kdraw_button_gb( pPrivate->pExit );
	kflush_button_gb( &pWin->gb, pPrivate->pExit );

	return( 0 );
}

// Ÿ��Ʋ�� �޽����� ����Ѵ�.
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

	// Ÿ��Ʋ�� ����Ѵ�.
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

	// �޽����� ����Ѵ�.
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

	// Parent�� ���� Modal �޽����� ������.
	kpost_message( pWin->pParentWin, WMESG_MODAL, pPrivate->dwMBoxID, dwConID );
	
	// �����ο��� DESTROY�� ������.
	kpost_message( pWin, WMESG_DESTROY, 0, 0 );

	return( WMHRV_CONTINUE );
}

static WMFuncStt mbox_marray[] = {
	{ WMESG_CREATE		   , wmh_mbox_create			},
	{ WMESG_DESTROY		   , wmh_mbox_destroy			},
	{ WMESG_CONTROL		   , wmh_mbox_control			},
	{ 0, NULL }
};

// ���� ���� ���Ѵ�.
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

// �޽��� �ڽ��� �����Ѵ�.
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
	{	// �̹� ��޹ڽ��� ������ �ִ�.
		kdbg_printf( "Window %X already has a modal box\n", (DWORD)pParent );
		return( NULL );
	}

	// MessageBox�� Vertical Size�� ����Ѵ�.
	pFont = get_system_font( MBOX_FONT );
	nLines = calc_lines( pStr );
	nV = MBOX_TITLE_V + ( pFont->cV * ( nLines + 1 ) ) + MBOX_MARGIN;
	if( dwStyle != 0 )
	{	// ��� ��ư�� �� ���� �ִ�.
		nV += MBOX_BUTTON_V;
	}
	nY = nV - MBOX_MARGIN - MBOX_BUTTON_V;

	// �����츦 �����Ѵ�. (Parent Window�� �߾�)
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
	// Create Message Handler���� �ٷ� ����ȴ�.
	pWin = kcreate_window( pParent->pWThread, pParent, WSTYLE_FLAT, &r, mbox_marray, 0, 0, IDI_MBOX, 0, 0 );
	if( pWin == NULL )
		return( NULL );

	// ���⼭ �̰� �� ��� �Ѵٴ�...
	recalc_client_area( pWin );

	// Ÿ��Ʋ�� �����Ѵ�. 
	set_window_title( pWin, "Notice" );
	
	// Win Private ������ �Ҵ��Ѵ�. 
	pPrivate = (MBoxPrivateStt*)kmalloc( sizeof( MBoxPrivateStt ) );
	pWin->pPrivate = pPrivate;
	if( pPrivate == NULL )
	{
		kdbg_printf( "message_box: private allocation failed!\n" );
	}

	memset( pPrivate, 0, sizeof( MBoxPrivateStt ) );

	// IDI_MBOX�� �ý��� ���ҽ��̹Ƿ� ������ �ʿ䰡 ����.
	pPrivate->pMBoxIcon = get_sys_icon( IDI_MBOX );
	
	// MessgeBox�� Exit ��ư�� �����Ѵ�.
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

	// �̹����� ������ �����Ѵ�.
	pExit->pImg = pImg;

	// �޽��� �ڽ� ��� �����, Exit ��ư, Title Line�� �׸���.
	mbox_make_screen( pWin ); 
	
	// Ÿ��Ʋ�� ������ ����Ѵ�.
	strcpy( pPrivate->szStr, pStr );
	strcpy( pWin->szTitle, pTitle );
	draw_mbox_str( pWin );

	// ��Ÿ���� �����Ѵ�.
	pPrivate->dwStyle = dwStyle;
	// MBOX ID�� �����Ѵ�.
	pPrivate->dwMBoxID = dwMBoxID;

	// ��ư�� ������ �����Ѵ�.
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

