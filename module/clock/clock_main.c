#include "clock.h"
#include "resource.h"

extern long sin_value( int nAngle );
extern long cos_value( int nAngle );

static DWORD wm_clock_timer  ( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB );
static DWORD wm_clock_close  ( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB );
static DWORD wm_clock_remake ( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB );
static DWORD wm_clock_create ( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB );
static DWORD wm_clock_destroy( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB );

static WMFuncStt clock_wmesg_tbl[] = {
	{ WMESG_CREATE, 	(WMESG_FUNC)wm_clock_create 	},
	{ WMESG_TIMER, 		(WMESG_FUNC)wm_clock_timer		},
	{ WMESG_CLOSE,		(WMESG_FUNC)wm_clock_close  	},
	{ WMESG_DESTROY,	(WMESG_FUNC)wm_clock_destroy  	},
	{ 0, NULL }
};

static WinResStt G_res;
static DWORD G_dwTbIcon;

int main( int argc, char *argv[] )
{
	RectStt			r;
	int				nR;
	ScrInfoStt		scr;
	WMesgStt		wmesg;
	int				nRValue;
	DWORD 			dwWThread, dwWinHandle;

	printf( "B2OS GUI Clock v1.1\n" );

	if( is_gui_mode() == 0 )
	{
		printf( "error: system is not in gui mode.\n" );
		return( -2 );
	}

	// 프로세스 Alias를 설정한다.
	set_process_alias( get_cur_process_id(), "clock" );
	
	// 현재 쓰레드를 Win Thread로 만든다.
	dwWThread = gx_create_win_thread( get_cur_thread_id() );
	if( dwWThread == 0 )
	{	// win thread 생성 실패.
		printf( "win thread = NULL!\n" );
		return( -1 );
	}

	nRValue = 0;
	gx_get_scr_info( &scr );

	// 화면 중앙에 윈도우를 생성한다.
	r.nX = (scr.nH - CLOCK_H)/2;
	r.nY = (scr.nV - CLOCK_V)/2;
	r.nH = CLOCK_H;
	r.nV = CLOCK_V;
	dwWinHandle = gx_create_window( dwWThread, WSTYLE_FLAT, &r, clock_wmesg_tbl );
	if( dwWinHandle == 0 )
	{
		printf( "clock main:gx_create_window failed!\n" );
		nRValue = -1;
		goto CLOSE_WIN_THREAD;
	}

	// 모듈의 리소스를 초기화한다.
	nR = gx_init_module_res( &G_res );
	if( nR < 0 )
		printf( "clock init resource failed!\n" );
	else
		printf( "clock resource : %d\n", G_res.nTotal );

	// 들어온 메시지를 처리한다.
	for( ;; )
	{
		nR = gx_win_mesg_pumping( dwWThread, &dwWinHandle, &wmesg );
		if( nR < 0 )
			break;
		nR = gr_win_mesg_handling( dwWinHandle, &wmesg );
		if( wmesg.dwID == WMESG_DESTROY )
		{
			printf( "clock: WMESG_DESTROY!\n" );
			break;
		}
	}

CLOSE_WIN_THREAD:
	gx_close_win_thread( dwWThread );

	printf( "clock: end.\n" );
	
	return( nRValue );
}

static void clock_angle_to_pos( int nAngle, DWORD dwCenterX, DWORD dwCenterY, DWORD dwRadius, DWORD *pX, DWORD *pY )
{
	int 	nAngleValue;

	if( nAngle <= 90 )
		nAngleValue = nAngle;
	else if( nAngle < 180 )
	{
		nAngleValue = nAngle - 90;
		nAngleValue = 90 - nAngleValue;
	}
	else if( nAngle <= 270 )
		nAngleValue = nAngle - 180;
	else 
	{
		nAngleValue = nAngle - 270;
		nAngleValue = 90 - nAngleValue;
	}

	pX[0] = ( dwRadius * cos_value( nAngleValue ) ) / 100000;
	pY[0] = ( dwRadius * sin_value( nAngleValue ) ) / 100000;
		
	if( nAngle <= 90 )
	{
		pX[0] = dwCenterX + pX[0];
		pY[0] = dwCenterY - pY[0];
	}
	else if( nAngle <= 180 )
	{
		pX[0] = dwCenterX - pX[0];		//
		pY[0] = dwCenterY - pY[0];		//
	}
	else if( nAngle <= 270 )
	{
		pX[0] = dwCenterX - pX[0];
		pY[0] = dwCenterY + pY[0];
	}
	else
	{
		pX[0] = dwCenterX + pX[0];		//
		pY[0] = dwCenterY + pY[0];		//
	}
}

static int clock_draw_gage( DWORD dwWinHandle, DWORD dwCenterX, DWORD dwCenterY, DWORD dwRadius, UINT16 wColor )
{
	DWORD	x1, y1, x2, y2, dwColor; 
	int 	nAngle, nChimLen;

	// 컬러를 16 -> 32로 변환한다.
	dwColor = gr_rgb16_to_rgb32( wColor );

	for( nAngle = 0; nAngle <= 360; nAngle += 30 )
	{
		if( ( nAngle % 90 ) == 0 )
			nChimLen = 5;
		else
			nChimLen = 3;

		clock_angle_to_pos( nAngle, dwCenterX, dwCenterY, dwRadius-nChimLen, &x1, &y1 );
		clock_angle_to_pos( nAngle, dwCenterX, dwCenterY, dwRadius, &x2, &y2 );

		if( nAngle == 90 || nAngle == 270 )
		{
			gx_line( dwWinHandle, x1-1, y1, x2-1, y2, dwColor ); 
			gx_line( dwWinHandle, x1,   y1, x2,	  y2, dwColor );	  
			gx_line( dwWinHandle, x1+1, y1, x2+1, y2, dwColor ); 
		}
		else if( nAngle == 0 || nAngle == 180 )
		{
			gx_line( dwWinHandle, x1, y1-1, x2, y2-1, dwColor ); 
			gx_line( dwWinHandle, x1, y1,   x2, y2,   dwColor );	  
			gx_line( dwWinHandle, x1, y1+1, x2, y2+1, dwColor ); 
		}
		else
		{
			gx_line( dwWinHandle, x1, y1, x2, y2, dwColor );	  
		}
	}
	return( 0 );
}

static DWORD  G_dwExitBtn, G_dwMinBtn;
static UINT16 G_wBack, G_wDkBack, G_wLtBack, G_wMdBack;

// 기본 화면을 만든다.
static DWORD wm_clock_create( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB )
{
	RectStt 	r;
	int			nR, nX, nH;
	
	printf( "clock: wm_clock_create()\n" );

	// 윈도우를 그릴 색상을 구한다.
	G_wBack   = gx_get_sys_color( COLOR_INDEX_MENU_BACK );
	G_wDkBack = gx_get_sys_color( COLOR_INDEX_MENU_DK   );	
	G_wLtBack = gx_get_sys_color( COLOR_INDEX_MENU_LT   );
	G_wMdBack = RGB16( 170,  140,  160 );

	// 윈도우의 크기를 구한다.
	nR = gx_get_client_rect( dwWinHandle, &r );
	if( nR < 0 )
	{
		printf( "clock: wm_clock_create: gx_get_client_rect failed!\n" );
		return( -1 );
	}
	if( r.nH <= 0 || r.nV <= 0 )
	{
		printf( "clock: wm_clock_create: invalid rect(nH=%d, nV=%d)\n", r.nH, r.nV );
		return( -1 );
	}
	printf( "clock: wm_clock_create: rect(nX = %d, nY = %d, nH = %d, nV = %d)\n", r.nX, r.nY, r.nH, r.nV );

	// 배경 색을 칠한다.
	nR = gx_fill_rect( dwWinHandle, &r, G_wBack );

	// 테두리를 그린다.
	nR = gx_3d_look( dwWinHandle, &r, LOOK_3D_IN, LOOK_3D_DEPRESSED, G_wLtBack, G_wDkBack );

	// 12분할 점을 그린다.
	nR = clock_draw_gage( dwWinHandle, CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS, G_wDkBack );

	// 타이머를 설정한다.
	nR = gx_register_gui_timer( dwWinHandle, CLOCK_GUI_TIMER_ID, 0, 450 );

	// 최초로 바늘들을 그린다.
	wm_clock_timer( dwWinHandle, 0, 0, 0 );

	// 버튼을 생성한다.
	nH = 9;
	nX = CLOCK_H-5-nH;
	G_dwMinBtn = gx_create_button( dwWinHandle, &G_res, IDI_CLK_MIN, "", 
						CONTROL_ATTR_BORDER_ON_MOUSE, G_wBack, 0, 0, nX, 3, nH, 9, WMESG_MINIMIZE, 0, 0);

	nX = nX-3-nH;
	G_dwExitBtn = gx_create_button( dwWinHandle, &G_res, IDI_CLK_EXIT, "", 
						CONTROL_ATTR_BORDER_ON_MOUSE, G_wBack, 0, 0, nX, 3, nH, 9, WMESG_CLOSE, 0, 0);

	// 라인을 그린다.
 	nH = CLOCK_H-8;
	nX = 4;
	gx_draw_line( dwWinHandle, nX,  13, nX+nH,  13, G_wMdBack, 0 );
	gx_draw_line( dwWinHandle, nX,  14, nX+nH,  14, G_wLtBack, 0 );

	// Task Basr에 아이콘을 등록한다.
	G_dwTbIcon= gx_tb_add_icon( &G_res, IDI_CLK_MAIN, "Clock", dwWinHandle );

	// 타이틀을 그린다.
	gx_drawtext_xy( dwWinHandle, 6, 1, IDR_BF_SIMPLE9, "clock", G_wDkBack, 0 );
	
	return( WMHRV_CONTINUE );
}

static int reverse_angle( int nAngle )
{
	nAngle += 180;
	if( nAngle >= 360 )
		nAngle -= 360;
	return( nAngle );
}

int draw_compas( DWORD dwWinHandle, DWORD dwCenterX, DWORD dwCenterY, DWORD dwRadius, int nHour, int nMin, int nSec, UINT16 wColor )
{
	DWORD	x1, y1, x2, y2, dwColor;
	int		nHourAngle, nMinAngle, nSecAngle, nHourArmLen, nMinArmLen, nSecArmLen;

	// 컬러를 16 -> 32로 변환한다.
	dwColor = gr_rgb16_to_rgb32( wColor );

	// 24 -> 12로 변환.
	if( nHour > 12 )
		nHour -= 12;
	else if( nHour == 0 )
		nHour = 12;

	// hour -> angle
	nHourAngle = 360 - (nHour*30) + 90;
	if( nHourAngle >= 360 )
		nHourAngle -= 360;
	// min -> angle
	nMinAngle = 360 - (nMin*6) + 90;
	if( nMinAngle >= 360 )
		nMinAngle -= 360;
	// min -> angle
	nSecAngle = 360 - (nSec*6) + 90;
	if( nSecAngle >= 360 )
		nSecAngle -= 360;
	// 분침의 이동비율을 시침에 적용.
	nHourAngle -= ( ( (nMin*100) / 60 ) * 30 ) / 100;
	if( nHourAngle < 0 )
		nHourAngle += 360;

	nHourArmLen = dwRadius - 13;
	nMinArmLen  = dwRadius - 10;
	nSecArmLen  = dwRadius - 7;

	// 시침을 그린다.
	clock_angle_to_pos( nHourAngle, dwCenterX, dwCenterY, nHourArmLen, &x1, &y1 );
	gx_line( dwWinHandle, dwCenterX, dwCenterY, x1, y1, dwColor ); 
	// 분침을 그린다.
	clock_angle_to_pos( nMinAngle, dwCenterX, dwCenterY, nMinArmLen, &x1, &y1 );
	gx_line( dwWinHandle, dwCenterX, dwCenterY, x1, y1, dwColor ); 
	// 초침을 그린다.
	clock_angle_to_pos( nSecAngle, dwCenterX, dwCenterY, nSecArmLen, &x1, &y1 );
	clock_angle_to_pos( reverse_angle( nSecAngle ), dwCenterX, dwCenterY, 7, &x2, &y2 );
	gx_line( dwWinHandle, x1, y1, x2, y2, dwColor ); 

	return( 0 );
}

static int nC = 0;
static TTimeStt prev_t;
static DWORD wm_clock_timer( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB )
{
	TTimeStt	t;

	get_time( &t );
	if( t.nSec == prev_t.nSec )
		goto BACK;

	draw_compas( dwWinHandle, CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS, prev_t.nHour, prev_t.nMin, prev_t.nSec, G_wBack);
	//printf( "clock: wm_clock_timer (%d) %02d:%02d %02d\n", nC++, t.nHour, t.nMin, t.nSec );
	draw_compas( dwWinHandle, CLOCK_CENTER_X, CLOCK_CENTER_Y, CLOCK_RADIUS, t.nHour, t.nMin, t.nSec, G_wDkBack);

	// 화면을 다시 그린다.
	gx_refresh_win( dwWinHandle, NULL );

	memcpy( &prev_t, &t, sizeof( t ) );

BACK:	
	return( WMHRV_CONTINUE );
}

static DWORD wm_clock_close( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB )
{
	int nR;
	
	// 타이머를 제거한다.
	nR = gx_unregister_gui_timer( dwWinHandle, CLOCK_GUI_TIMER_ID );
		
	// Task Bar Icon을 제거한다.
	gx_tb_del_icon( G_dwTbIcon );

	// MIN, EXIT 버튼을 제거한다.
	gx_close_button( G_dwMinBtn );  
	gx_close_button( G_dwExitBtn );
	G_dwMinBtn = G_dwExitBtn = 0;

	// DESTROY 메시지를 포스팅한다.
	gx_post_message( dwWinHandle, WMESG_DESTROY, 0, 0 );
	
	return( WMHRV_CONTINUE );
}

// 아무것도 하는 일은 없지만 DESTROY 핸들러를 만들어 주어야 메시지가 핸들러로 전달된다.
static DWORD wm_clock_destroy( DWORD dwWinHandle, DWORD dwWinMesg, DWORD dwParamA, DWORD dwParamB )
{
	return( WMHRV_CONTINUE );
}


