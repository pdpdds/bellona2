#include <bellona2.h>
#include "gui.h"

#define  _TB_CONTROL_TYPE_TB_ICON	  1
#define  _TB_CONTROL_TYPE_MENU_BTN	  2

static WinStt	*pTaskBar = NULL;


#define TB_MENU_ENT_ID_APPS			100
#define TB_MENU_ENT_ID_SYSINFO		101
#define TB_MENU_ENT_ID_QUIT			102

#define TB_MENU_ENT_ID_TETRIS		110
#define TB_MENU_ENT_ID_CLOCK		111

static MenuEntStt apps_ent[] = {
	{ {0}, TB_MENU_ENT_ID_TETRIS,	0, 0, "Tetris",		IDI_TETRIS,		NULL, NULL },
	{ {0}, TB_MENU_ENT_ID_CLOCK,	0, 0, "Clock",		IDI_CLOCK,		NULL, NULL },
	{ {0}, 0,						0, 0, {0},			0,				NULL, NULL }
};

static MenuStt apps_menu = {
	0,
	apps_ent
};
static MenuEntStt main_ent[] = {
	{ {0}, TB_MENU_ENT_ID_APPS,		0, 0, "Apps",		IDI_APPS,		NULL, &apps_menu },
	{ {0}, TB_MENU_ENT_ID_SYSINFO,	0, 0, "SysInfo",	IDI_SYSINFO,	NULL, NULL },
	{ {0}, MENU_ENT_ID_BREAK,		0, 0, {0},			0,				NULL, NULL },
	{ {0}, TB_MENU_ENT_ID_QUIT,		0, 0, "Quit",		IDI_QUIT,		NULL, NULL },
	{ {0}, 0,						0, 0, {0},			0,				NULL, NULL }
};

static MenuStt main_menu = {
	0,
	main_ent
};

// TaskBar�� ȭ���� ���� �����Ѵ�.
static DWORD taskbar_make_screen( WinStt *pWin )
{
	RectStt				r;
	int					nI;
	TaskBarPrivateStt	*pPrivate;

	pPrivate = (TaskBarPrivateStt*)pWin->pPrivate;

	// ����� �����.
	//screen_to_win( &r, pWin->gb.pR, &pWin->ct_r );
	get_client_rect( pWin, &r );
	
	k_fill_rect( &pWin->gb, &r, TASKBAR_COLOR );

	// ���ʿ� ������ �� �� �׸���.
	k_line_h( &pWin->gb, 0, 1, pWin->obj.r.nH, TASKBAR_UPLINE_COLOR );
	
	// menu ��ư, main menu�� �׸���.
	kdraw_button_gb( pPrivate->pMenuBtn );
	kflush_button_gb( &pWin->gb, pPrivate->pMenuBtn );

	// TIME BOX�� �׸���.
	k_3d_look( &pWin->gb,  &pPrivate->time_r, LOOK_3D_OUT, LOOK_3D_PRESSED, TASKBAR_MENU_LT_COLOR, TASKBAR_MENU_DK_COLOR );

	// ��ϵ� ��ư���� �׸���.
	for( nI = 0; nI < MAX_TB_ICON; nI++ )
	{
		if( pPrivate->tb_icon[nI].pBtn == NULL )
			continue;									  
		kdraw_button_gb( pPrivate->tb_icon[nI].pBtn );
		kflush_button_gb( &pWin->gb, pPrivate->tb_icon[nI].pBtn );
	}

	return( 0 );
}

// 1�� ���� ���� �ð��� �����Ѵ�.
static int update_current_time( DWORD dwParam )
{
	TTimeStt			t;
	FontStt				*pFont;
	char				szT[32];
	TaskBarPrivateStt	*pPrivate;
	int					nX, nY, nLength;

	read_cmos_time( &t );

	sprintf( szT, "%02d:%02d (%02d)", t.nHour, t.nMin, t.nSec );

	pPrivate = (TaskBarPrivateStt*)pTaskBar->pPrivate;

	// ��Ʈ�� ���ؼ� �߾� ��ǥ�� ���Ѵ�.
	pFont = get_system_font( IDR_BF_BASE11 ); 
	nLength = strlen( szT ) * pFont->cH;
	nX = pPrivate->time_r.nX + ( pPrivate->time_r.nH - nLength ) / 2;
	nY = pPrivate->time_r.nY + ( pPrivate->time_r.nV - pFont->cV ) / 2;

	// ���� ����� ���� �� �ð��� ����Ѵ�.
	k_fill_rect( &pTaskBar->gb, &pPrivate->time_r, TASKBAR_COLOR );
	drawtext_xy( 
		&pTaskBar->gb,													
		nX,
		nY,
		pFont,						
		szT, 
		RGB16( 0, 0, 0 ),
		0 );	 

	// �׷��� �ð��� ȭ�鿡 �����Ѵ�.
	call_win_message_handler( pTaskBar, WMESG_PAINT, rect_xy_to_dword( &pPrivate->time_r ), rect_hv_to_dword( &pPrivate->time_r) );

	return( 0 );
}

static DWORD wmh_taskbar_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	int					nR;
	RectStt 			client_r;
	TaskBarPrivateStt	*pPrivate;

	// This Window�� �����͸� ������ �д�.
	pTaskBar = pWin;

	//Ÿ��Ʋ�� �����Ѵ�. 
	set_window_title( pWin, "TaskBar" );

	get_client_rect( pWin, &client_r );

	// Private ������ �Ҵ��Ѵ�. 
	pPrivate = (TaskBarPrivateStt*)kmalloc( sizeof( TaskBarPrivateStt ) );
	pWin->pPrivate = pPrivate;
	if( pPrivate != NULL )
	{	// TaskBar�� Private Data�� �ʱ�ȭ �Ѵ�.
		memset( pPrivate, 0, sizeof( TaskBarPrivateStt ) );

		// Menu Button�� �����.
		pPrivate->pMenuBtn = kcreate_button( pWin,		 			 // Parent Win
									   0,								 // Image ID
									   "B2OS",							 // Button Text
									   CONTROL_ATTR_BORDER_ON_MOUSE,	 // Attribute
									   TASKBAR_COLOR,			 		 // Background Color
									   0,								 // Text Color
									   IDR_BF_BASE11,					 // Font ID
									   3,								 // nX
									   3,								 // nY
									   TASKBAR_MENU_H,					 // nH
									   client_r.nV - 6,					 // nV
									   WMESG_CONTROL,					 // Window Message
									   _TB_CONTROL_TYPE_MENU_BTN, 		 // Param A
									   0								 // Param B
									   );
		
		// Time (Ŭ���̾�Ʈ ���� ���� ��ǥ)
		pPrivate->time_r.nX = client_r.nH - TASKBAR_TIME_H - 2;
		pPrivate->time_r.nY = 3;
		pPrivate->time_r.nH = TASKBAR_TIME_H;
		pPrivate->time_r.nV = client_r.nV - 6;
		
		{// �޴��� �����Ѵ�.
			int		nMenuY, nMenuX, nMenuV;
			nMenuX = pPrivate->pMenuBtn->obj.r.nX;

			// �޴��� ���� ��ǥ�� ��� ���� Vertical Size�� ���ؾ� �Ѵ�.
			nMenuV = calc_menu_v( &main_menu );
			nMenuY = pWin->obj.r.nY - nMenuV;
			nR = create_menu( &main_menu, nMenuX, nMenuY, pWin->pWThread, pWin );
			if( nR == 0 )
				pPrivate->pMenu = &main_menu;
		}
	}

	// ���ʷ� �����츦 �׷��д�.
	taskbar_make_screen( pWin );

	// �ð��� �����ϱ� ���� Ÿ�̸� �ݹ��� �����Ѵ�.
	pPrivate->pTimeOutHandle = set_timer_callback( 1000, update_current_time, (DWORD)pWin );

	return( WMHRV_CONTINUE );
}

static DWORD wmh_taskbar_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	int					nI;
	TaskBarPrivateStt	*pPrivate;

	pPrivate = pWin->pPrivate;
	if( pPrivate != NULL )
	{
		// Ÿ�̸� �ݹ��� �����Ѵ�.
		kill_timer_callback( pPrivate->pTimeOutHandle );
		pPrivate->pTimeOutHandle = NULL;

		kclose_button( pPrivate->pMenuBtn );
		pPrivate->pMenuBtn = NULL;

		// TB Icon���� ������ �־�� �Ѵ�.
		for( nI = 0; nI < MAX_TB_ICON; nI++ )
		{
			if( pPrivate->tb_icon[nI].pBtn != NULL )
				kclose_button( pPrivate->tb_icon[nI].pBtn );
		}

		// �޴��� �����Ѵ�.
		close_menu( pPrivate->pMenu );

		// Private ������ �����Ѵ�.
		kfree( pPrivate );
		pWin->pPrivate = NULL;
	}

	return( 0 );
}
static DWORD wmh_taskbar_control( WinStt *pWin, DWORD dwWMesgID, DWORD dwType, DWORD dwTarget )
{
	if( dwType ==  _TB_CONTROL_TYPE_TB_ICON )
	{
		TBIconStt	*pTBIcon;
		pTBIcon = (TBIconStt*)dwTarget;
		if( pTBIcon->pWin->gb.dwState & WIN_STATE_MINIMIZED )
			pTBIcon->pWin->gb.dwState &= (DWORD)~(DWORD)WIN_STATE_MINIMIZED;
		// Top Window�� �ű��.
		set_top_window( pTBIcon->pWin );
	}
	else if( dwType ==  _TB_CONTROL_TYPE_MENU_BTN )
	{	// �޴� ��ư�� ������.
		MenuStt				*pMenu;
		TaskBarPrivateStt	*pPrivate;

		pPrivate = (TaskBarPrivateStt*)pWin->pPrivate;
		pMenu    = pPrivate->pMenu;
		// �̹� SHOW���� Ȯ���Ѵ�.
		if( is_win_show ( pMenu->pWin ) == 0 )
			kpost_message( pMenu->pWin, WMESG_SHOW, 1, 0 );	// HIDDEN�̸� SHOW�� �Ѵ�.
		else
			kpost_message( pMenu->pWin, WMESG_SHOW, 0, 0 );	// SHOW�̸� HIDDEN���� �Ѵ�.
		
		set_top_window( pMenu->pWin );
	}

	return( WMHRV_CONTINUE );
}

// Ÿ��Ʋ �� ��ư���κ��� ���ƿ� �͵�.
static DWORD wmh_taskbar_remake( WinStt *pWin, DWORD dwWMesgID, DWORD dwXY, DWORD dwHV )
{
	RectStt r;

	dword_to_rect_xy( dwXY, &r );
	dword_to_rect_hv( dwHV, &r );
	
	// Ÿ��Ʋ�� �׸���. 
	k_fill_rect( &pWin->gb, &r, TASKBAR_COLOR );
	
	return( WMHRV_CONTINUE );
}

WMFuncStt taskbar_marray[] = {
	{ WMESG_CREATE			, wmh_taskbar_create		},
	{ WMESG_DESTROY			, wmh_taskbar_destroy		},
	{ WMESG_CONTROL			, wmh_taskbar_control		},
	{ WMESG_REMAKE			, wmh_taskbar_remake 		},
	{ 0, NULL }
/*	
	{ WMESG_PAINT		   , wmh_taskbar_paint			},
	{ WMESG_MOUSE_MOVE_OUT , wmh_taskbar_mouse_move_out	},									 
	{ WMESG_MOUSE_MOVE     , wmh_taskbar_mouse_move		},
	{ WMESG_LBTN_DN		   , wmh_taskbar_lbtn_dn		},
	{ WMESG_LBTN_UP		   , wmh_taskbar_lbtn_up		},
	{ WMESG_CLOSE		   , wmh_taskbar_close			},
	{ WMESG_RBTN_DN		   , wmh_taskbar_rbtn_dn		},
	{ WMESG_RBTN_UP		   , wmh_taskbar_rbtn_up		},
	{ WMESG_MOUSE_MOVE_IN  , wmh_taskbar_mouse_move_in	},
	{ WMESG_MINIMIZE       , wmh_taskbar_minimize		},
	{ WMESG_MAXIMIZE       , wmh_taskbar_maximize		},
	{ WMESG_WIN_MOVE       , wmh_taskbar_win_move		},
*/	
};

// �ܺο��� Task Bar�� �������� �߰��� �� ����ϴ� �Լ�.
int tb_add_icon( int nIconID, char *pPopUpString, PopUpMenuStt *pMenu, WinStt *pWin )
{
	int					nI;
	ButtonStt			*pTB;
	TBIconStt			*pTBIcon;
	TaskBarPrivateStt	*pPrivate;

	if( pTaskBar == NULL )
		return( -1 );				// TaskBar�� ���� �ʱ�ȭ�� �ȵ�.

	pPrivate = (TaskBarPrivateStt*)pTaskBar->pPrivate;
	
	// Private �������� TBIcon ������ ���Ѵ�.
	for( nI = 0; ; nI++ )
	{
		if( nI >= MAX_TB_ICON )
			return( -1 );				// �������� ���̻� �߰��� �� ����.
		if( pPrivate->tb_icon[nI].pBtn == NULL )
		{
			pTBIcon = &pPrivate->tb_icon[nI];
			break;
		}
	}
	
	// ������ ��ư�� �����Ѵ�.
	pTB = kcreate_button( pTaskBar,					 				// Parent Win
						   nIconID,								 	// Image ID
						   NULL,							 		// Button Text
						   CONTROL_ATTR_BORDER_ON_MOUSE,	 		// Attribute
						   TASKBAR_COLOR,					 		// Background Color
						   0,								 		// Text Color
						   0,					 					// Font ID
						   (nI * TBICON_H) + TASKBAR_MENU_H + 10,	// nX
						   3,								 		// nY
						   TBICON_BUTTON_H,					 		// nH
						   TBICON_BUTTON_V, 				 		// nV
						   WMESG_CONTROL,					 		// Window Message
						   _TB_CONTROL_TYPE_TB_ICON,		 		// Param A
						   (DWORD)pTBIcon					 		// Param B
						   );

	// ��Ÿ ������ �����Ѵ�.
	pTBIcon->pBtn = pTB;
	pTBIcon->pPopUpMenu = pMenu;
	pTBIcon->pWin = pWin;
	strcpy( pTBIcon->szPopUpString, pPopUpString );

	// ȭ���� �ٽ� �����Ѵ�.
	taskbar_make_screen( pTaskBar );

	// ȭ���� �����Ѵ�.
	call_win_message_handler( pTaskBar, WMESG_PAINT, rect_xy_to_dword( &pTB->obj.r), rect_hv_to_dword( &pTB->obj.r) );

	return( nI );
}

int tb_add_icon_ex( ImageStt *pImg, char *pPopUpString, PopUpMenuStt *pMenu, WinStt *pWin )
{
	int 				nI;
	ButtonStt			*pTB;
	TBIconStt			*pTBIcon;
	TaskBarPrivateStt	*pPrivate;

	if( pTaskBar == NULL )
		return( -1 );				// TaskBar�� ���� �ʱ�ȭ�� �ȵ�.

	pPrivate = (TaskBarPrivateStt*)pTaskBar->pPrivate;
	
	// Private �������� TBIcon ������ ���Ѵ�.
	for( nI = 0; ; nI++ )
	{
		if( nI >= MAX_TB_ICON )
			return( -1 );				// �������� ���̻� �߰��� �� ����.
		if( pPrivate->tb_icon[nI].pBtn == NULL )
		{
			pTBIcon = &pPrivate->tb_icon[nI];
			break;
		}
	}
	
	// ������ ��ư�� �����Ѵ�.
	pTB = kcreate_button_ex( pTaskBar, 								// Parent Win
						   pImg, 									// Image ID
						   NULL,									// Button Text
						   CONTROL_ATTR_BORDER_ON_MOUSE,			// Attribute
						   TASKBAR_COLOR,							// Background Color
						   0,										// Text Color
						   0,										// Font ID
						   (nI * TBICON_H) + TASKBAR_MENU_H + 10,	// nX
						   3,										// nY
						   TBICON_BUTTON_H, 						// nH
						   TBICON_BUTTON_V, 						// nV
						   WMESG_CONTROL,							// Window Message
						   _TB_CONTROL_TYPE_TB_ICON,				// Param A
						   (DWORD)pTBIcon							// Param B
						   );

	// ��Ÿ ������ �����Ѵ�.
	pTBIcon->pBtn = pTB;
	pTBIcon->pPopUpMenu = pMenu;
	pTBIcon->pWin = pWin;
	strcpy( pTBIcon->szPopUpString, pPopUpString );

	// ȭ���� �ٽ� �����Ѵ�.
	taskbar_make_screen( pTaskBar );

	// ȭ���� �����Ѵ�.
	call_win_message_handler( pTaskBar, WMESG_PAINT, rect_xy_to_dword( &pTB->obj.r), rect_hv_to_dword( &pTB->obj.r) );

	return( nI );
}

int tb_del_icon( int nID )
{
	RectStt				r;
	int					nR;
	TBIconStt 			*pTBIcon;
	TaskBarPrivateStt	*pPrivate;

	if( pTaskBar == NULL || nID < 0 )
		return( -1 );				
	
	pPrivate = (TaskBarPrivateStt*)pTaskBar->pPrivate;
	pTBIcon = &pPrivate->tb_icon[nID];

	memcpy( &r, &pTBIcon->pBtn->obj.r, sizeof( RectStt ) );

	// ��ư�� �����Ѵ�.
	nR = kclose_button( pTBIcon->pBtn );

	pTBIcon->pBtn = NULL;

	// ȭ���� �ٽ� �����Ѵ�.
	taskbar_make_screen( pTaskBar );
	
	// ȭ���� �����Ѵ�.
	call_win_message_handler( pTaskBar, WMESG_PAINT, rect_xy_to_dword( &r), rect_hv_to_dword( &r) );

	return( 0 );
}

