#include "tetris.h"
#include "resource_b2.h"

static DWORD G_dwTID = 0;
static UINT16 G_wBack, G_wDkBack, G_wLtBack;
static WinResStt G_res;
static DWORD G_dwTbIcon;

ImageStt *G_pBkImg;

extern TTCfgStt tt_cfg;

static DWORD wm_tetris_create ( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB );
static DWORD wm_tetris_timer  ( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB );
static DWORD wm_tetris_close  ( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB );
static DWORD wm_tetris_destroy( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB );

static WMFuncStt tetris_wmesg_tbl[] = {
	{ WMESG_CREATE, 	(WMESG_FUNC)wm_tetris_create 	},
	{ WMESG_TIMER, 		(WMESG_FUNC)wm_tetris_timer		},
	{ WMESG_CLOSE,		(WMESG_FUNC)wm_tetris_close		},
	{ WMESG_DESTROY,	(WMESG_FUNC)wm_tetris_destroy	},
	{ 0, NULL }
};

static int kbd_handler( DWORD dwWinHandle )
{
	int nCh, nState;

	printf( "tetris win handle = 0x%X\n", dwWinHandle );

	for( ;; )
	{
		nCh = getch();

		switch( nCh )
		{
		case BK_LEFT :
			block_left( dwWinHandle );
			break;
		case BK_UP :
			block_rotate( dwWinHandle );
			break;
		case BK_RIGHT :
			block_right( dwWinHandle );
			break;
		case BK_DOWN :
			block_down( dwWinHandle, TT_LINE_V );
			break;
		case 0x0D :
			nState = get_state();
			if( nState == TT_STATE_PAUSED )
				set_state( TT_STATE_RUNNING );
			else if( nState == TT_STATE_GAME_OVER )
				block_start( dwWinHandle );
			break;
		case 'p' :
		case 'P' :
			block_pause( dwWinHandle );
			break;
		}
	}

	return( 0 );
}

int main( int argc, char *argv[] )
{
	RectStt			r;
	int				nR;
	ScrInfoStt		scr;
	WMesgStt		wmesg;
	int				nRValue;
	DWORD 			dwWThread, dwWinHandle;

	printf( "B2OS GUI Tetris v1.0\n" );

	if( is_gui_mode() == 0 )
	{
		printf( "error: system is not in gui mode.\n" );
		return( -2 );
	}

	// 프로세스 Alias를 설정한다.
	set_process_alias( get_cur_process_id(), "tetris" );

	// 현재 쓰레드를 Win Thread로 만든다.
	dwWThread = gx_create_win_thread( get_cur_thread_id() );
	if( dwWThread == 0 )
	{	// win thread를 생성 tlfvo.
		printf( "win thread = NULL!\n" );
		return( -1 );
	}

	nRValue = 0;
	gx_get_scr_info( &scr );

	// 윈도우를 화면 중앙에 생성한다.
	r.nX = (scr.nH - TT_WIN_H)/2;
	r.nY = (scr.nV - TT_WIN_V)/2;
	r.nH = TT_WIN_H;
	r.nV = TT_WIN_V;
	dwWinHandle = gx_create_window( dwWThread, WSTYLE_SIMPLE, &r, tetris_wmesg_tbl );
	if( dwWinHandle == 0 )
	{
		printf( "tetris main:gx_create_window failed!\n" );
		nRValue = -1;
		goto CLOSE_WIN_THREAD;
	}

	gx_set_win_text( dwWinHandle, "Tetris" );

	// 모듈의 리소스를 초기화한다.
	nR = gx_init_module_res( &G_res );
	if( nR < 0 )
		printf( "tetris init resource failed!\n" );
	else
		printf( "tetris resource : %d\n", G_res.nTotal );

	// 키 입력 처리용 쓰레드를 생성한다.
	G_dwTID = create_thread( (DWORD)kbd_handler, dwWinHandle, TS_READY_NORMAL, 0 );
	set_fg_tid( G_dwTID );
	set_thread_alias( G_dwTID, "tetris_kbd" );
	printf( "tetris: kbd thread id = %d\n", G_dwTID );
		
	// 들어온 메시지를 처리한다.
	for( ;; )
	{
		nR = gx_win_mesg_pumping( dwWThread, &dwWinHandle, &wmesg );
		if( nR < 0 )
			break;

		nR = gr_win_mesg_handling( dwWinHandle, &wmesg );
		if( wmesg.dwID == WMESG_DESTROY )
			break;
	}

CLOSE_WIN_THREAD:
	gx_close_win_thread( dwWThread );

	printf( "tetris: end\n" );
	
	return( nRValue );
}

// 그려둔 슬롯 전체를 스크린에 복사한다.
int flush_slot( DWORD dwWinHandle )
{
	gx_refresh_win( dwWinHandle, NULL );
	  
	return( 0 );
}

// 기본 화면을 만든다.
static DWORD wm_tetris_create( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB )
{
	int			nR;
	RectStt 	r, rx;
	
	printf( "tetris: wm_tetris_create()\n" );

	// 배경 비트맵을 로드한다.
	G_pBkImg = gx_load_bitmap_to_image16( "/c/tetris.bmp" );
	if( G_pBkImg == NULL )
		printf( "wm_tetris_create: bitmap(/c/tetris.bmp) failed!\n" );
	else
		printf( "tetris.bmp: (%d * %d)\n", G_pBkImg->nH, G_pBkImg->nV );

	// 윈도우를 그릴 색상을 tetris.
	G_wBack   = gx_get_sys_color( COLOR_INDEX_MENU_BACK );
	G_wDkBack = gx_get_sys_color( COLOR_INDEX_MENU_DK   );	
	G_wLtBack = gx_get_sys_color( COLOR_INDEX_MENU_LT   );

	// 윈도우의 크기를 tetris.
	nR = gx_get_client_rect( dwWinHandle, &r );
	if( nR < 0 )
	{
		printf( "tetris: wm_tetris_create: gx_get_client_rect failed!\n" );
		return( -1 );
	}
	if( r.nH <= 0 || r.nV <= 0 )
	{
		printf( "wm_tetris_create: invalid rect(nH=%d, nV=%d)\n", r.nH, r.nV );
		return( -1 );
	}
	printf( "wm_tetris_create: rect(nX = %d, nY = %d, nH = %d, nV = %d)\n", r.nX, r.nY, r.nH, r.nV );

	// 배경 색을 칠한다.
	nR = gx_fill_rect( dwWinHandle, &r, G_wBack );

	// 테두리를 그린다.
	nR = gx_3d_look( dwWinHandle, &r, LOOK_3D_IN, LOOK_3D_DEPRESSED, G_wLtBack, G_wDkBack );

	// 구조체를 초기화 한다.
	memcpy( &rx, &r, sizeof( r ) );
	r.nX += 3;
	r.nY += 3;
	r.nH -= 6;
	r.nV -= 6;
	init_tt_cfg( &tt_cfg, &rx );

	// GAME OVER 상태로 실행한다.
	set_state( TT_STATE_GAME_OVER );

	// 슬롯을 재구성한다.
	remake_slot( dwWinHandle );

	// 구성된 슬롯을 화면에 출력한다.
	flush_slot( dwWinHandle );

	// 타이머를 설정한다.
	nR = gx_register_gui_timer( dwWinHandle, TETRIS_GUI_TIMER_ID, 0, TT_TIMER_INTERVAL );

	// Task Basr에 아이콘을 등록한다.
	G_dwTbIcon= gx_tb_add_icon( &G_res, IDI_TETRIS_MAIN, "tetris", dwWinHandle );
	if( G_dwTbIcon == 0 )
		printf( "gx_tb_add_icon: failed!\n" );
	
	return( WMHRV_CONTINUE );
}

static DWORD wm_tetris_timer( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB )
{
	if( tt_cfg.nState != TT_STATE_RUNNING )
		return( 0 );		  
	
	// 일정 시간 간격으로 불러 준다.
	tt_cfg.nTimerCount++;
	if( TT_MAX_LEVEL - tt_cfg.nLevel < tt_cfg.nTimerCount )
	{
		slot_feed_timer( dwWinHandle );
		tt_cfg.nTimerCount = 0;
	}

	return( WMHRV_CONTINUE );
}

int fill_rect32( DWORD dwWinHandle, RectStt *pR, DWORD dwColor )
{
	int		nR;
	UINT16  wColor;

	wColor = gr_rgb32_to_rgb16( dwColor );

	nR = gx_fill_rect( dwWinHandle, pR, wColor );
	return( nR );  
}

int fill_rect32_or( DWORD dwWinHandle, RectStt *pR, DWORD dwColor )
{
	int 	nR;
	UINT16	wColor;

	wColor = gr_rgb32_to_rgb16( dwColor );

	nR = gx_fill_rect_ex( dwWinHandle, pR, wColor, 1 );
	return( nR );  
}

static DWORD wm_tetris_destroy( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB )
{
	return( WMHRV_CONTINUE );
}

static DWORD wm_tetris_close( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB )
{
	int nR, nExitCode;

	// 비트맵 이미지를 해제한다.
	if( G_pBkImg != NULL )
	{
		free( G_pBkImg );
		G_pBkImg = NULL;
	}
	
	// 타이머를 제거한다.
	nR = gx_unregister_gui_timer( dwWinHandle, TETRIS_GUI_TIMER_ID );

	// Task Bar Icon을 제거한다.
	gx_tb_del_icon( G_dwTbIcon );
	
	// 키보드 핸들러 Thread를 제거한다.
	remove_thread( G_dwTID );

	//printf( "waittid(%d) <- in\n", G_dwTID );

	// Thread가 완전히 종료되기를 기다린다.
	nR = waittid( G_dwTID, &nExitCode );

	//printf( "waittid(%d) -> out\n", G_dwTID );
		
	// Task Bar Icon을 제거한다.
	//gx_tb_del_icon( G_dwTbIcon );
	
	// DESTROY 메시지를 포스팅한다.
	gx_post_message( dwWinHandle, WMESG_DESTROY, 0, 0 );

	return( WMHRV_CONTINUE );
}

