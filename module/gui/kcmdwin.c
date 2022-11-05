#include <bellona2.h>
#include "gui.h"

static int kcmd_write( WinStt *pWin, char *pS, UINT16 wColor );

static int kcmd_write_xy( WinStt *pWin, int nCol, int nLine, char *pS, UINT16 wColor )
{
	FontStt				*pFont;
	int					nX, nY;
	KCmdWinPrivateStt	*pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;
	pFont = pPrivate->pFont;

	nX = nCol * pFont->cH;
	nY = nLine * pPrivate->nLineV;

	// 클라이언트 좌표를 윈도우 좌표로 변환한다.
	client_to_win_pos( pWin, &nX, &nY );
	nY += KCMDWIN_LINE_GAP / 2;
		
	drawtext_xy( 
		&pWin->gb,													
		nX,
		nY,
		pFont, 
		pS, 
		wColor,
		0 );
	
	return( 0 );
}

static void kcmd_write_callback( WinStt *pWin, char *pS )
{
	RectStt		r;

	// 이 조건을 검사하지 않으면 ALT-F4를 누른 직후 문제가 생긴다.
	if( pWin == NULL || pWin->pPrivate == NULL )
		return;

	kcmd_write( pWin, pS, KCMDWIN_TEXT_COLOR );

	// 화면을 갱신한다.
	screen_to_win( &r, &pWin->obj.r, &pWin->ct_r );
	get_client_rect( pWin, &r );
	call_win_message_handler( pWin, WMESG_PAINT, rect_xy_to_dword( &r), rect_hv_to_dword( &r) );
}	

static int kcmd_xy_to_rect( RectStt *pR, WinStt *pWin, int nX, int nY, int nCol, int nLine )
{
	KCmdWinPrivateStt	*pPrivate;
	
	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;
	
	pR->nX = pPrivate->pFont->cH * nX;
	pR->nY = pPrivate->pFont->cV * nY;
	pR->nH = pPrivate->pFont->cH * nCol;
	pR->nV = pPrivate->pFont->cV * nLine;

	return( 0 );
}

// 라인 nY의 nX부터 nH까지 지운다.
static int kcmd_erase_line( WinStt *pWin, int nX, int nY, int nH, UINT16 wColor )
{
	RectStt				r;
	KCmdWinPrivateStt	*pPrivate;
	
	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	kcmd_xy_to_rect( &r, pWin, nX, nY, nH, 1 );
	
	// 윈도우의 좌표로 변경한다.
	client_to_win_pos( pWin, &r.nX, &r.nY );
	k_fill_rect( &pWin->gb, &r, wColor );

	return( 0 );
}	

// 커서를 그리거나 지운다.
static int invert_cursor_pos( WinStt *pWin, int nX, int nY )
{
	RectStt				r;
	KCmdWinPrivateStt	*pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	r.nX = pPrivate->pFont->cH * nX + 1;
	r.nY = pPrivate->pFont->cV * nY + 1;
	r.nH = pPrivate->pFont->cH - 2;
	r.nV = pPrivate->pFont->cV - 2;

	// 커서영역을 반전한다.
	client_to_win_pos( pWin, &r.nX, &r.nY );
	k_invert_rect( &pWin->gb, &r );
	
	return( 0 );
}

// 커서를 보이거나 숨긴다.
static int kcmd_show_cursor( WinStt *pWin, BYTE byShow )
{
	KCmdWinPrivateStt	*pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	_asm {	// Disable Interrupt ///////////////////////////////////////////////
		PUSHFD
		CLI
	}

	if( ( pPrivate->byShowCursor == 0 && byShow == 0 ) || 
		( pPrivate->byShowCursor != 0 && byShow != 0 ) )
		goto BACK;

	// 커서 영역을 반전한다.
	if( pPrivate->nXPos >= 0 && pPrivate->nYPos >= 0 )
		invert_cursor_pos( pWin, pPrivate->nXPos, pPrivate->nYPos );

	pPrivate->byShowCursor = byShow;

BACK:
	// Enable Interrupt	!!! ////////////////////////////////////////////////////
	_asm POPFD

	return( 0 );
}

// Kernel쪽에서 직접 호출된다.
static void kcmd_direct_write_callback( WinStt *pWin, char *pS, int nX )
{
	RectStt		r;
	KCmdWinPrivateStt	*pPrivate;

	// 이 조건을 검사하지 않으면 ALT-F4를 누른 직후 문제가 생긴다.
	if( pWin == NULL || pWin->pPrivate == NULL )
		return;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	// 커서를 OFF 한다.
	kcmd_show_cursor( pWin, 0 );

	// line 자체를 지운다.
	kcmd_erase_line( pWin, nX, pPrivate->nYPos, pPrivate->nTotalCol-nX, KCMDWIN_BACK_COLOR );

	//kcmd_write( pWin, pS, KCMDWIN_TEXT_COLOR );
	kcmd_write_xy( pWin, nX, pPrivate->nYPos, pS, KCMDWIN_TEXT_COLOR );

	// 커서를 ON 한다.
	kcmd_show_cursor( pWin, 1 );

	// 해당 라인만 갱신한다.
	kcmd_xy_to_rect( &r, pWin, nX, pPrivate->nYPos, pPrivate->nTotalCol - nX, 1 );
	client_to_win_pos( pWin, &r.nX, &r.nY );
	call_win_message_handler( pWin, WMESG_PAINT, rect_xy_to_dword( &r), rect_hv_to_dword( &r) );
}	

// KCmdWin window의 화면을 구성한다. (실제 보여지지는 않는다.)
static DWORD kcmd_make_screen( WinStt *pWin, int nY )
{
	RectStt				r;
	KCmdWinPrivateStt	*pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	if( nY >= pPrivate->nTotalLine )
		return( 0 );

	// 지운 부분만 갱신한다.
	kcmd_xy_to_rect( &r, pWin, 0, nY, pPrivate->nTotalCol, pPrivate->nTotalLine - nY );
	
	// 윈도우의 좌표로 변경한다.
	client_to_win_pos( pWin, &r.nX, &r.nY );

	// 클라이언트 영역을 지운다. 
	k_fill_rect( &pWin->gb, &r, KCMDWIN_BACK_COLOR );

	return( 0 );
}

// Kernel 쪽에서 Gui Console의 커서 위치를 설정하기 위해 호출한다.
static void kcmd_cursor_callback( WinStt *pWin, int nX, int nY )
{
	KCmdWinPrivateStt	*pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	if( nY > pPrivate->nTotalLine-1 )
		nY = pPrivate->nTotalLine-1;
		
	kcmd_show_cursor( pWin, 0 );

	pPrivate->nXPos = nX;
	pPrivate->nYPos = nY;

	kcmd_show_cursor( pWin, 1 );
}

// KCmdWin window의 화면을 구성한다. (실제 보여지지는 않는다.)
// nY가 0이면 화면 전체를 지운다.
// nY가 nTotalLine과 같거나 크면 화면을 지우지 않는다.
static void kcmd_cls_callback( WinStt *pWin, int nY )
{
	RectStt r;
	KCmdWinPrivateStt *pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	if( nY >= pPrivate->nTotalLine )
		return;

	// 커서 위치를 (0,nY)로 옮긴다.
	kcmd_cursor_callback( pWin, 0, nY );

	kcmd_show_cursor( pWin, 0 );
	
	// 화면을 모두 지운다.
	kcmd_make_screen( pWin, nY );
	kcmd_show_cursor( pWin, 0 );

	// 지운 부분만 갱신한다.
	kcmd_xy_to_rect( &r, pWin, 0, nY, pPrivate->nTotalCol, pPrivate->nTotalLine - nY );
	
	// 윈도우의 좌표로 변경한다.
	client_to_win_pos( pWin, &r.nX, &r.nY );

	// 커서를 나타낸다.
	kcmd_show_cursor( pWin, 1 );

	// 화면을 갱신한다.
	call_win_message_handler( pWin, WMESG_PAINT, rect_xy_to_dword( &r), rect_hv_to_dword( &r) );
}

// 한 라인을 스크롤 한다.
static kcmd_scroll( WinStt *pWin )
{
	RectStt				rc, re;
	KCmdWinPrivateStt	*pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;
	
	//screen_to_win( &rc, &pWin->obj.r, &pWin->ct_r ); 
	get_client_rect( pWin, &rc );
	
	// 스크롤한 후에 지울 영역을 계산한다.		
	memcpy( &re, &rc, sizeof( RectStt ) );
	re.nY += (pPrivate->nTotalLine-1) * pPrivate->nLineV;
	re.nV = pPrivate->nLineV;//rc.nY + rc.nV - re.nY;
	
	// GB 영역을 스크롤한다.
	scroll_gb( pWin, &rc, pPrivate->nLineV, &re, KCMDWIN_BACK_COLOR );
	
	pPrivate->nYPos++;
	if( pPrivate->nYPos >= pPrivate->nTotalLine )
		pPrivate->nYPos = pPrivate->nTotalLine -1;

	return( 0 );
}

static int flushing_text( WinStt *pWin, char *pS, UINT16 wColor )
{	
	KCmdWinPrivateStt	*pPrivate;
	
	if( pS[0] == 0 )
		return( 0 );

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	// 문자열을 출력한다.
	kcmd_write_xy( pWin, pPrivate->nXPos, pPrivate->nYPos, pS, wColor );

	pPrivate->nXPos += strlen( pS );
	pS[0] = 0;
	
	return( 0 );
}

// Write Position, Cursor, Scroll등을 처리한다.
// Kernel에서 직접 호출된다.
#define MAX_KCMD_STR	64
static int kcmd_write( WinStt *pWin, char *pS, UINT16 wColor )
{
	int					nI, nK;
	KCmdWinPrivateStt	*pPrivate;
	char				szT[ MAX_KCMD_STR +1 ];
	
	if( pS[0] == 0 )
		return( 0 );

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	// cursor를 없앤다.
	kcmd_show_cursor( pWin, 0 );

	szT[0] = 0;
	for( nK = nI = 0; ; nI++ )
	{
		if( pS[nI] == 0 )
		{	// 문자열의 끝에 다다랐다.  지금까지 입력된 문자열을 플러싱한다.
			flushing_text( pWin, szT, wColor );
			nK = 0;
			break;
		}
		else if( pS[nI] == 13 )
			continue;		// 무시한다.
		else if( pS[nI] == 10 )
		{	// 버퍼에 저장된 텍스트를 플러싱한다.
			flushing_text( pWin, szT, wColor );
			nK = 0;
			
			// 마지막 라인에 있을 때만 스크롤 한다.
			if( pPrivate->nYPos == pPrivate->nTotalLine-1 )
			{
				kcmd_scroll( pWin );
				pPrivate->nXPos = 0;
			}
			else
			{	// 다음 라인으로 커서를 이동한다.
				kcmd_show_cursor( pWin, 0 );
				pPrivate->nXPos = 0;
				pPrivate->nYPos++;
				kcmd_show_cursor( pWin, 0 );
			}
		}
		else
		{	// 버퍼로 한 문자를 복사한다.
			szT[nK] = pS[nI];
			nK++;
			szT[nK] = 0;
			if( nK >= MAX_KCMD_STR )
			{	// 버퍼가 다 찼으며 플러싱한다.
				flushing_text( pWin, szT, wColor );
				nK = 0;
			}
		}	
	}

	// cursor를 보여준다.
	kcmd_show_cursor( pWin, 1 );
	return( 0 );
}

// 가상 콘솔에 버퍼링된 데이터를 GUI 콘솔에 그대로 그려준다.
// gui_flushing()에서 호출된다.
static void kcmd_flush_callback( WinStt *pWin, VConsoleStt *pVC )
{
	RectStt				r;
	FontStt				*pFont;
	KCmdWinPrivateStt	*pPrivate;
	VConCharStt			*pVCBuff;
	int					nR, nX, nY, nEndY, nStartY, nTotalY, nPosX, nPosY;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;
	if( pVC == NULL )
	{
		kdbg_printf( "kcmd_flush_callback: pVC = NULL\n" );
		return;
	}

	kcmd_show_cursor( pWin, 0 );

	// 화면을 몽땅 지운다.
	kcmd_make_screen( pWin, 0 );

	// nStartY부터 nY까지만 화면에 그려주면 된다.
	nEndY = pVC->wCursorY;
	nStartY = nEndY - (pPrivate->nTotalLine - 1);
	if( nStartY < 0 )
		nStartY = 0;

	nTotalY = nEndY - nStartY;
	pVCBuff = &pVC->con_buff[ nStartY * 80 ];
	pFont   = pPrivate->pFont;
	for( nY = 0; nY < nTotalY; nY++ )
	{	// 클라이언트 영역의 좌표를 윈도우 기준 좌표로 변환한다.
		nPosY = nY * pPrivate->nLineV;
		nPosX = 0;
		client_to_win_pos( pWin, &nPosX, &nPosY );

		for( nX = 0; nX < pPrivate->nTotalCol; nX++ )
		{	// 버퍼가 문자 + 속성으로 되어 있기 때문에 직접 문자를 그리는 함수를 사용한다.
			nR = drawchar_xy( &pWin->gb, nPosX, nPosY, pFont, pVCBuff->ch, KCMDWIN_TEXT_COLOR, 0 );
			if( nR < 0 )
				return;

			pVCBuff++;
			nPosX += pFont->cH;
		}
	}	

	// 커서를 그려 준다.
	kcmd_show_cursor( pWin, 1 );

	// 화면 전체를 다시 그린다.
	//screen_to_win( &r, &pWin->obj.r, &pWin->ct_r );
	get_client_rect( pWin, &r );
	
	call_win_message_handler( pWin, WMESG_PAINT, rect_xy_to_dword( &r), rect_hv_to_dword( &r) );
}

// vconsole.h
/*
typedef struct GuiConsoleFuncTag {
	void					*pWin;
	GUI_WRITE				pWrite;
	GUI_DIRECT_WRITE		pDirectWrite;
	GUI_CURSOR_XY			pCursorXY;
	GUI_CLS					pCls;
	GUI_FLUSHING			pFlushing;
} GuiConsoleFuncStt;
*/
// kernel에서 직접 호출하는 gui 화면 관련 함수.
static GuiConsoleFuncStt kcmd_ftbl = {
	NULL, 								  // pWin
	kcmd_write_callback,
	kcmd_direct_write_callback,
	kcmd_cursor_callback,
	kcmd_cls_callback,
	kcmd_flush_callback
};

// KCmdWin winow의 초기화 함수.
int calc_KCmdWin_size( RectStt *pR, DWORD dwPredefStyleID )
{
	FontStt *pFont;

	// Window의 RECT를 계산한다.
	pFont = get_system_font( KCMDWIN_FONT );
	if( pFont != NULL )
	{
		WinStyleStt *pWS;
		GuiStt		*pGui;

		pGui = get_gui_stt();
		pWS  = find_predef_wstyle( dwPredefStyleID );

		// 시스템의 중앙에 위치하도록 한다.
		pR->nH = ( pFont->cH * 80 ) + ( pWS->frame.nFrameWidth*2 );
		pR->nV = pFont->cV * 25 + ( pWS->frame.nFrameWidth*3 ) + pWS->frame.nTitleV;
		pR->nX = ( pGui->wall.obj.r.nH - pR->nH ) / 2;
		pR->nY = ( pGui->wall.obj.r.nV - pR->nV ) / 2;
	}

	return( 0 );
}

// Console Window를 생성한다.
static DWORD wmh_KCmdWin_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	RectStt				*pR;
	KCmdWinPrivateStt	*pPrivate;

	//타이틀을 설정한다. 
	set_window_title( pWin, "Console" );

	// 프로세스 Alias를 설정한다.
	k_set_process_alias( k_get_current_process(), "Console" );

	// Win Private 영역을 할당한다. 
	pWin->pPrivate = pPrivate = (KCmdWinPrivateStt*)kmalloc( sizeof( KCmdWinPrivateStt ) );
	if( pPrivate != NULL )
	{
		memset( pPrivate, 0, sizeof( KCmdWinPrivateStt ) );
		// 폰트를 로드한다.
		pPrivate->pFont = get_system_font( KCMDWIN_FONT );
	}
	
	// 클라이언트 영역을 재계산한다. (아래 함수에서 client area 값을 참조한다.)
	recalc_client_area( pWin );

	pR = get_client_rect( pWin, NULL );

	// 화면 정보(라인수, 커서위치)를 재설정한다.
	pPrivate->nLineV = pPrivate->pFont->cV + KCMDWIN_LINE_GAP;
	pPrivate->nTotalLine = pR->nV / pPrivate->nLineV;
	pPrivate->nXPos = 0;
	pPrivate->nYPos = pPrivate->nTotalLine-1;
	pPrivate->nTotalCol = 80;

	// 최초로 윈도우를 그려둔다.
	kcmd_make_screen( pWin, 0 );

	// TaskBar에 아이콘을 등록한다.
	tb_add_icon( IDI_CMD_ICON, "KCMD", NULL, pWin );

	// 로고를 출력해 준다.
	kdbg_printf( "B2OS GUI Console.\n" );

	// redirect를 설정한다.
	kcmd_ftbl.pWin = pWin;

	set_gui_console_ftbl( &kcmd_ftbl );

	// 커서 표시한다.
	kcmd_show_cursor( pWin, 1 );

	// 콘솔 전체를 최초로 그려준다. (2004-03-25)
	kcmd_flush_callback( pWin, get_active_vconsole() );
	
	return( WMHRV_CONTINUE );
}

static DWORD wmh_KCmdWin_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	KCmdWinPrivateStt	*pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	// redirect를 설정한다.
	set_gui_console_ftbl( NULL );

	// Private 영역을 해제한다.
	if( pWin->pPrivate != NULL )
	{
		kfree( pWin->pPrivate );
		pWin->pPrivate = NULL;
	}
	
	kdbg_printf( "nullify gui_redirect\n" );

	return( WMHRV_CONTINUE );
}

static DWORD wmh_KCmdWin_lbtn_dn( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( WMHRV_CONTINUE );
}

static DWORD wmh_KCmdWin_lbtn_up( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{

	return( WMHRV_CONTINUE );
}

static DWORD wmh_KCmdWin_rbtn_dn( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{

	return( WMHRV_CONTINUE );
}

static DWORD wmh_KCmdWin_rbtn_up( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{

	return( WMHRV_CONTINUE );
}

static DWORD wmh_KCmdWin_mouse_move_out( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( 0 );
}	

static DWORD wmh_KCmdWin_mouse_move_in( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( 0 );
}	
static DWORD wmh_KCmdWin_mouse_move( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( 0 );
}				

static DWORD wmh_KCmdWin_minimize( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( 0 );
}

static DWORD wmh_KCmdWin_maximize( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( 0 );
}

static DWORD wmh_KCmdWin_win_move( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( 0 );
}

WMFuncStt KCmdWin_marray[] = {
	{ WMESG_LBTN_DN		   , wmh_KCmdWin_lbtn_dn		},
	{ WMESG_LBTN_UP		   , wmh_KCmdWin_lbtn_up		},
	{ WMESG_RBTN_DN		   , wmh_KCmdWin_rbtn_dn		},
	{ WMESG_RBTN_UP		   , wmh_KCmdWin_rbtn_up		},
	{ WMESG_MOUSE_MOVE_OUT , wmh_KCmdWin_mouse_move_out	},									 
	{ WMESG_MOUSE_MOVE_IN  , wmh_KCmdWin_mouse_move_in	},
	{ WMESG_MINIMIZE       , wmh_KCmdWin_minimize		},
	{ WMESG_MAXIMIZE       , wmh_KCmdWin_maximize		},
	{ WMESG_MOUSE_MOVE     , wmh_KCmdWin_mouse_move		},
	{ WMESG_WIN_MOVE       , wmh_KCmdWin_win_move		},
	{ WMESG_CREATE		   , wmh_KCmdWin_create			},
	{ WMESG_DESTROY		   , wmh_KCmdWin_destroy		},
	{ 0, NULL }
};

