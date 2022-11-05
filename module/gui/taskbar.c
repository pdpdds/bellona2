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

// TaskBar의 화면을 새로 구성한다.
static DWORD taskbar_make_screen( WinStt *pWin )
{
	RectStt				r;
	int					nI;
	TaskBarPrivateStt	*pPrivate;

	pPrivate = (TaskBarPrivateStt*)pWin->pPrivate;

	// 배경을 지운다.
	//screen_to_win( &r, pWin->gb.pR, &pWin->ct_r );
	get_client_rect( pWin, &r );
	
	k_fill_rect( &pWin->gb, &r, TASKBAR_COLOR );

	// 위쪽에 라인을 한 줄 그린다.
	k_line_h( &pWin->gb, 0, 1, pWin->obj.r.nH, TASKBAR_UPLINE_COLOR );
	
	// menu 버튼, main menu를 그린다.
	kdraw_button_gb( pPrivate->pMenuBtn );
	kflush_button_gb( &pWin->gb, pPrivate->pMenuBtn );

	// TIME BOX를 그린다.
	k_3d_look( &pWin->gb,  &pPrivate->time_r, LOOK_3D_OUT, LOOK_3D_PRESSED, TASKBAR_MENU_LT_COLOR, TASKBAR_MENU_DK_COLOR );

	// 등록된 버튼들을 그린다.
	for( nI = 0; nI < MAX_TB_ICON; nI++ )
	{
		if( pPrivate->tb_icon[nI].pBtn == NULL )
			continue;									  
		kdraw_button_gb( pPrivate->tb_icon[nI].pBtn );
		kflush_button_gb( &pWin->gb, pPrivate->tb_icon[nI].pBtn );
	}

	return( 0 );
}

// 1초 마다 현재 시간을 갱신한다.
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

	// 폰트를 구해서 중앙 좌표를 구한다.
	pFont = get_system_font( IDR_BF_BASE11 ); 
	nLength = strlen( szT ) * pFont->cH;
	nX = pPrivate->time_r.nX + ( pPrivate->time_r.nH - nLength ) / 2;
	nY = pPrivate->time_r.nY + ( pPrivate->time_r.nV - pFont->cV ) / 2;

	// 먼저 배경을 지운 후 시간을 기록한다.
	k_fill_rect( &pTaskBar->gb, &pPrivate->time_r, TASKBAR_COLOR );
	drawtext_xy( 
		&pTaskBar->gb,													
		nX,
		nY,
		pFont,						
		szT, 
		RGB16( 0, 0, 0 ),
		0 );	 

	// 그려진 시간을 화면에 복사한다.
	call_win_message_handler( pTaskBar, WMESG_PAINT, rect_xy_to_dword( &pPrivate->time_r ), rect_hv_to_dword( &pPrivate->time_r) );

	return( 0 );
}

static DWORD wmh_taskbar_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	int					nR;
	RectStt 			client_r;
	TaskBarPrivateStt	*pPrivate;

	// This Window의 포인터를 저장해 둔다.
	pTaskBar = pWin;

	//타이틀을 설정한다. 
	set_window_title( pWin, "TaskBar" );

	get_client_rect( pWin, &client_r );

	// Private 영역을 할당한다. 
	pPrivate = (TaskBarPrivateStt*)kmalloc( sizeof( TaskBarPrivateStt ) );
	pWin->pPrivate = pPrivate;
	if( pPrivate != NULL )
	{	// TaskBar의 Private Data를 초기화 한다.
		memset( pPrivate, 0, sizeof( TaskBarPrivateStt ) );

		// Menu Button을 만든다.
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
		
		// Time (클라이언트 영역 내의 좌표)
		pPrivate->time_r.nX = client_r.nH - TASKBAR_TIME_H - 2;
		pPrivate->time_r.nY = 3;
		pPrivate->time_r.nH = TASKBAR_TIME_H;
		pPrivate->time_r.nV = client_r.nV - 6;
		
		{// 메뉴를 생성한다.
			int		nMenuY, nMenuX, nMenuV;
			nMenuX = pPrivate->pMenuBtn->obj.r.nX;

			// 메뉴의 생성 좌표를 잡기 위해 Vertical Size를 구해야 한다.
			nMenuV = calc_menu_v( &main_menu );
			nMenuY = pWin->obj.r.nY - nMenuV;
			nR = create_menu( &main_menu, nMenuX, nMenuY, pWin->pWThread, pWin );
			if( nR == 0 )
				pPrivate->pMenu = &main_menu;
		}
	}

	// 최초로 윈도우를 그려둔다.
	taskbar_make_screen( pWin );

	// 시간을 갱신하기 위한 타이머 콜백을 설정한다.
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
		// 타이머 콜백을 제거한다.
		kill_timer_callback( pPrivate->pTimeOutHandle );
		pPrivate->pTimeOutHandle = NULL;

		kclose_button( pPrivate->pMenuBtn );
		pPrivate->pMenuBtn = NULL;

		// TB Icon들을 제거해 주어야 한다.
		for( nI = 0; nI < MAX_TB_ICON; nI++ )
		{
			if( pPrivate->tb_icon[nI].pBtn != NULL )
				kclose_button( pPrivate->tb_icon[nI].pBtn );
		}

		// 메뉴를 해제한다.
		close_menu( pPrivate->pMenu );

		// Private 영역을 해제한다.
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
		// Top Window로 옮긴다.
		set_top_window( pTBIcon->pWin );
	}
	else if( dwType ==  _TB_CONTROL_TYPE_MENU_BTN )
	{	// 메뉴 버튼을 눌렀다.
		MenuStt				*pMenu;
		TaskBarPrivateStt	*pPrivate;

		pPrivate = (TaskBarPrivateStt*)pWin->pPrivate;
		pMenu    = pPrivate->pMenu;
		// 이미 SHOW인지 확인한다.
		if( is_win_show ( pMenu->pWin ) == 0 )
			kpost_message( pMenu->pWin, WMESG_SHOW, 1, 0 );	// HIDDEN이면 SHOW로 한다.
		else
			kpost_message( pMenu->pWin, WMESG_SHOW, 0, 0 );	// SHOW이면 HIDDEN으로 한다.
		
		set_top_window( pMenu->pWin );
	}

	return( WMHRV_CONTINUE );
}

// 타이틀 바 버튼으로부터 날아온 것들.
static DWORD wmh_taskbar_remake( WinStt *pWin, DWORD dwWMesgID, DWORD dwXY, DWORD dwHV )
{
	RectStt r;

	dword_to_rect_xy( dwXY, &r );
	dword_to_rect_hv( dwHV, &r );
	
	// 타이틀을 그린다. 
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

// 외부에서 Task Bar에 아이콘을 추가할 때 사용하는 함수.
int tb_add_icon( int nIconID, char *pPopUpString, PopUpMenuStt *pMenu, WinStt *pWin )
{
	int					nI;
	ButtonStt			*pTB;
	TBIconStt			*pTBIcon;
	TaskBarPrivateStt	*pPrivate;

	if( pTaskBar == NULL )
		return( -1 );				// TaskBar가 아직 초기화가 안됨.

	pPrivate = (TaskBarPrivateStt*)pTaskBar->pPrivate;
	
	// Private 영역에서 TBIcon 슬롯을 구한다.
	for( nI = 0; ; nI++ )
	{
		if( nI >= MAX_TB_ICON )
			return( -1 );				// 아이콘을 더이상 추가할 수 없다.
		if( pPrivate->tb_icon[nI].pBtn == NULL )
		{
			pTBIcon = &pPrivate->tb_icon[nI];
			break;
		}
	}
	
	// 아이콘 버튼을 생성한다.
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

	// 기타 정보를 설정한다.
	pTBIcon->pBtn = pTB;
	pTBIcon->pPopUpMenu = pMenu;
	pTBIcon->pWin = pWin;
	strcpy( pTBIcon->szPopUpString, pPopUpString );

	// 화면을 다시 구성한다.
	taskbar_make_screen( pTaskBar );

	// 화면을 갱신한다.
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
		return( -1 );				// TaskBar가 아직 초기화가 안됨.

	pPrivate = (TaskBarPrivateStt*)pTaskBar->pPrivate;
	
	// Private 영역에서 TBIcon 슬롯을 구한다.
	for( nI = 0; ; nI++ )
	{
		if( nI >= MAX_TB_ICON )
			return( -1 );				// 아이콘을 더이상 추가할 수 없다.
		if( pPrivate->tb_icon[nI].pBtn == NULL )
		{
			pTBIcon = &pPrivate->tb_icon[nI];
			break;
		}
	}
	
	// 아이콘 버튼을 생성한다.
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

	// 기타 정보를 설정한다.
	pTBIcon->pBtn = pTB;
	pTBIcon->pPopUpMenu = pMenu;
	pTBIcon->pWin = pWin;
	strcpy( pTBIcon->szPopUpString, pPopUpString );

	// 화면을 다시 구성한다.
	taskbar_make_screen( pTaskBar );

	// 화면을 갱신한다.
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

	// 버튼을 해제한다.
	nR = kclose_button( pTBIcon->pBtn );

	pTBIcon->pBtn = NULL;

	// 화면을 다시 구성한다.
	taskbar_make_screen( pTaskBar );
	
	// 화면을 갱신한다.
	call_win_message_handler( pTaskBar, WMESG_PAINT, rect_xy_to_dword( &r), rect_hv_to_dword( &r) );

	return( 0 );
}

