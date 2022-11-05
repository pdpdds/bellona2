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

	// ���μ��� Alias�� �����Ѵ�.
	set_process_alias( get_cur_process_id(), "tetris" );

	// ���� �����带 Win Thread�� �����.
	dwWThread = gx_create_win_thread( get_cur_thread_id() );
	if( dwWThread == 0 )
	{	// win thread�� ���� tlfvo.
		printf( "win thread = NULL!\n" );
		return( -1 );
	}

	nRValue = 0;
	gx_get_scr_info( &scr );

	// �����츦 ȭ�� �߾ӿ� �����Ѵ�.
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

	// ����� ���ҽ��� �ʱ�ȭ�Ѵ�.
	nR = gx_init_module_res( &G_res );
	if( nR < 0 )
		printf( "tetris init resource failed!\n" );
	else
		printf( "tetris resource : %d\n", G_res.nTotal );

	// Ű �Է� ó���� �����带 �����Ѵ�.
	G_dwTID = create_thread( (DWORD)kbd_handler, dwWinHandle, TS_READY_NORMAL, 0 );
	set_fg_tid( G_dwTID );
	set_thread_alias( G_dwTID, "tetris_kbd" );
	printf( "tetris: kbd thread id = %d\n", G_dwTID );
		
	// ���� �޽����� ó���Ѵ�.
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

// �׷��� ���� ��ü�� ��ũ���� �����Ѵ�.
int flush_slot( DWORD dwWinHandle )
{
	gx_refresh_win( dwWinHandle, NULL );
	  
	return( 0 );
}

// �⺻ ȭ���� �����.
static DWORD wm_tetris_create( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB )
{
	int			nR;
	RectStt 	r, rx;
	
	printf( "tetris: wm_tetris_create()\n" );

	// ��� ��Ʈ���� �ε��Ѵ�.
	G_pBkImg = gx_load_bitmap_to_image16( "/c/tetris.bmp" );
	if( G_pBkImg == NULL )
		printf( "wm_tetris_create: bitmap(/c/tetris.bmp) failed!\n" );
	else
		printf( "tetris.bmp: (%d * %d)\n", G_pBkImg->nH, G_pBkImg->nV );

	// �����츦 �׸� ������ tetris.
	G_wBack   = gx_get_sys_color( COLOR_INDEX_MENU_BACK );
	G_wDkBack = gx_get_sys_color( COLOR_INDEX_MENU_DK   );	
	G_wLtBack = gx_get_sys_color( COLOR_INDEX_MENU_LT   );

	// �������� ũ�⸦ tetris.
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

	// ��� ���� ĥ�Ѵ�.
	nR = gx_fill_rect( dwWinHandle, &r, G_wBack );

	// �׵θ��� �׸���.
	nR = gx_3d_look( dwWinHandle, &r, LOOK_3D_IN, LOOK_3D_DEPRESSED, G_wLtBack, G_wDkBack );

	// ����ü�� �ʱ�ȭ �Ѵ�.
	memcpy( &rx, &r, sizeof( r ) );
	r.nX += 3;
	r.nY += 3;
	r.nH -= 6;
	r.nV -= 6;
	init_tt_cfg( &tt_cfg, &rx );

	// GAME OVER ���·� �����Ѵ�.
	set_state( TT_STATE_GAME_OVER );

	// ������ �籸���Ѵ�.
	remake_slot( dwWinHandle );

	// ������ ������ ȭ�鿡 ����Ѵ�.
	flush_slot( dwWinHandle );

	// Ÿ�̸Ӹ� �����Ѵ�.
	nR = gx_register_gui_timer( dwWinHandle, TETRIS_GUI_TIMER_ID, 0, TT_TIMER_INTERVAL );

	// Task Basr�� �������� ����Ѵ�.
	G_dwTbIcon= gx_tb_add_icon( &G_res, IDI_TETRIS_MAIN, "tetris", dwWinHandle );
	if( G_dwTbIcon == 0 )
		printf( "gx_tb_add_icon: failed!\n" );
	
	return( WMHRV_CONTINUE );
}

static DWORD wm_tetris_timer( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB )
{
	if( tt_cfg.nState != TT_STATE_RUNNING )
		return( 0 );		  
	
	// ���� �ð� �������� �ҷ� �ش�.
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

	// ��Ʈ�� �̹����� �����Ѵ�.
	if( G_pBkImg != NULL )
	{
		free( G_pBkImg );
		G_pBkImg = NULL;
	}
	
	// Ÿ�̸Ӹ� �����Ѵ�.
	nR = gx_unregister_gui_timer( dwWinHandle, TETRIS_GUI_TIMER_ID );

	// Task Bar Icon�� �����Ѵ�.
	gx_tb_del_icon( G_dwTbIcon );
	
	// Ű���� �ڵ鷯 Thread�� �����Ѵ�.
	remove_thread( G_dwTID );

	//printf( "waittid(%d) <- in\n", G_dwTID );

	// Thread�� ������ ����Ǳ⸦ ��ٸ���.
	nR = waittid( G_dwTID, &nExitCode );

	//printf( "waittid(%d) -> out\n", G_dwTID );
		
	// Task Bar Icon�� �����Ѵ�.
	//gx_tb_del_icon( G_dwTbIcon );
	
	// DESTROY �޽����� �������Ѵ�.
	gx_post_message( dwWinHandle, WMESG_DESTROY, 0, 0 );

	return( WMHRV_CONTINUE );
}

