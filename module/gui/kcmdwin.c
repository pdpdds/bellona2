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

	// Ŭ���̾�Ʈ ��ǥ�� ������ ��ǥ�� ��ȯ�Ѵ�.
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

	// �� ������ �˻����� ������ ALT-F4�� ���� ���� ������ �����.
	if( pWin == NULL || pWin->pPrivate == NULL )
		return;

	kcmd_write( pWin, pS, KCMDWIN_TEXT_COLOR );

	// ȭ���� �����Ѵ�.
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

// ���� nY�� nX���� nH���� �����.
static int kcmd_erase_line( WinStt *pWin, int nX, int nY, int nH, UINT16 wColor )
{
	RectStt				r;
	KCmdWinPrivateStt	*pPrivate;
	
	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	kcmd_xy_to_rect( &r, pWin, nX, nY, nH, 1 );
	
	// �������� ��ǥ�� �����Ѵ�.
	client_to_win_pos( pWin, &r.nX, &r.nY );
	k_fill_rect( &pWin->gb, &r, wColor );

	return( 0 );
}	

// Ŀ���� �׸��ų� �����.
static int invert_cursor_pos( WinStt *pWin, int nX, int nY )
{
	RectStt				r;
	KCmdWinPrivateStt	*pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	r.nX = pPrivate->pFont->cH * nX + 1;
	r.nY = pPrivate->pFont->cV * nY + 1;
	r.nH = pPrivate->pFont->cH - 2;
	r.nV = pPrivate->pFont->cV - 2;

	// Ŀ�������� �����Ѵ�.
	client_to_win_pos( pWin, &r.nX, &r.nY );
	k_invert_rect( &pWin->gb, &r );
	
	return( 0 );
}

// Ŀ���� ���̰ų� �����.
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

	// Ŀ�� ������ �����Ѵ�.
	if( pPrivate->nXPos >= 0 && pPrivate->nYPos >= 0 )
		invert_cursor_pos( pWin, pPrivate->nXPos, pPrivate->nYPos );

	pPrivate->byShowCursor = byShow;

BACK:
	// Enable Interrupt	!!! ////////////////////////////////////////////////////
	_asm POPFD

	return( 0 );
}

// Kernel�ʿ��� ���� ȣ��ȴ�.
static void kcmd_direct_write_callback( WinStt *pWin, char *pS, int nX )
{
	RectStt		r;
	KCmdWinPrivateStt	*pPrivate;

	// �� ������ �˻����� ������ ALT-F4�� ���� ���� ������ �����.
	if( pWin == NULL || pWin->pPrivate == NULL )
		return;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	// Ŀ���� OFF �Ѵ�.
	kcmd_show_cursor( pWin, 0 );

	// line ��ü�� �����.
	kcmd_erase_line( pWin, nX, pPrivate->nYPos, pPrivate->nTotalCol-nX, KCMDWIN_BACK_COLOR );

	//kcmd_write( pWin, pS, KCMDWIN_TEXT_COLOR );
	kcmd_write_xy( pWin, nX, pPrivate->nYPos, pS, KCMDWIN_TEXT_COLOR );

	// Ŀ���� ON �Ѵ�.
	kcmd_show_cursor( pWin, 1 );

	// �ش� ���θ� �����Ѵ�.
	kcmd_xy_to_rect( &r, pWin, nX, pPrivate->nYPos, pPrivate->nTotalCol - nX, 1 );
	client_to_win_pos( pWin, &r.nX, &r.nY );
	call_win_message_handler( pWin, WMESG_PAINT, rect_xy_to_dword( &r), rect_hv_to_dword( &r) );
}	

// KCmdWin window�� ȭ���� �����Ѵ�. (���� ���������� �ʴ´�.)
static DWORD kcmd_make_screen( WinStt *pWin, int nY )
{
	RectStt				r;
	KCmdWinPrivateStt	*pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	if( nY >= pPrivate->nTotalLine )
		return( 0 );

	// ���� �κи� �����Ѵ�.
	kcmd_xy_to_rect( &r, pWin, 0, nY, pPrivate->nTotalCol, pPrivate->nTotalLine - nY );
	
	// �������� ��ǥ�� �����Ѵ�.
	client_to_win_pos( pWin, &r.nX, &r.nY );

	// Ŭ���̾�Ʈ ������ �����. 
	k_fill_rect( &pWin->gb, &r, KCMDWIN_BACK_COLOR );

	return( 0 );
}

// Kernel �ʿ��� Gui Console�� Ŀ�� ��ġ�� �����ϱ� ���� ȣ���Ѵ�.
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

// KCmdWin window�� ȭ���� �����Ѵ�. (���� ���������� �ʴ´�.)
// nY�� 0�̸� ȭ�� ��ü�� �����.
// nY�� nTotalLine�� ���ų� ũ�� ȭ���� ������ �ʴ´�.
static void kcmd_cls_callback( WinStt *pWin, int nY )
{
	RectStt r;
	KCmdWinPrivateStt *pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	if( nY >= pPrivate->nTotalLine )
		return;

	// Ŀ�� ��ġ�� (0,nY)�� �ű��.
	kcmd_cursor_callback( pWin, 0, nY );

	kcmd_show_cursor( pWin, 0 );
	
	// ȭ���� ��� �����.
	kcmd_make_screen( pWin, nY );
	kcmd_show_cursor( pWin, 0 );

	// ���� �κи� �����Ѵ�.
	kcmd_xy_to_rect( &r, pWin, 0, nY, pPrivate->nTotalCol, pPrivate->nTotalLine - nY );
	
	// �������� ��ǥ�� �����Ѵ�.
	client_to_win_pos( pWin, &r.nX, &r.nY );

	// Ŀ���� ��Ÿ����.
	kcmd_show_cursor( pWin, 1 );

	// ȭ���� �����Ѵ�.
	call_win_message_handler( pWin, WMESG_PAINT, rect_xy_to_dword( &r), rect_hv_to_dword( &r) );
}

// �� ������ ��ũ�� �Ѵ�.
static kcmd_scroll( WinStt *pWin )
{
	RectStt				rc, re;
	KCmdWinPrivateStt	*pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;
	
	//screen_to_win( &rc, &pWin->obj.r, &pWin->ct_r ); 
	get_client_rect( pWin, &rc );
	
	// ��ũ���� �Ŀ� ���� ������ ����Ѵ�.		
	memcpy( &re, &rc, sizeof( RectStt ) );
	re.nY += (pPrivate->nTotalLine-1) * pPrivate->nLineV;
	re.nV = pPrivate->nLineV;//rc.nY + rc.nV - re.nY;
	
	// GB ������ ��ũ���Ѵ�.
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

	// ���ڿ��� ����Ѵ�.
	kcmd_write_xy( pWin, pPrivate->nXPos, pPrivate->nYPos, pS, wColor );

	pPrivate->nXPos += strlen( pS );
	pS[0] = 0;
	
	return( 0 );
}

// Write Position, Cursor, Scroll���� ó���Ѵ�.
// Kernel���� ���� ȣ��ȴ�.
#define MAX_KCMD_STR	64
static int kcmd_write( WinStt *pWin, char *pS, UINT16 wColor )
{
	int					nI, nK;
	KCmdWinPrivateStt	*pPrivate;
	char				szT[ MAX_KCMD_STR +1 ];
	
	if( pS[0] == 0 )
		return( 0 );

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	// cursor�� ���ش�.
	kcmd_show_cursor( pWin, 0 );

	szT[0] = 0;
	for( nK = nI = 0; ; nI++ )
	{
		if( pS[nI] == 0 )
		{	// ���ڿ��� ���� �ٴٶ���.  ���ݱ��� �Էµ� ���ڿ��� �÷����Ѵ�.
			flushing_text( pWin, szT, wColor );
			nK = 0;
			break;
		}
		else if( pS[nI] == 13 )
			continue;		// �����Ѵ�.
		else if( pS[nI] == 10 )
		{	// ���ۿ� ����� �ؽ�Ʈ�� �÷����Ѵ�.
			flushing_text( pWin, szT, wColor );
			nK = 0;
			
			// ������ ���ο� ���� ���� ��ũ�� �Ѵ�.
			if( pPrivate->nYPos == pPrivate->nTotalLine-1 )
			{
				kcmd_scroll( pWin );
				pPrivate->nXPos = 0;
			}
			else
			{	// ���� �������� Ŀ���� �̵��Ѵ�.
				kcmd_show_cursor( pWin, 0 );
				pPrivate->nXPos = 0;
				pPrivate->nYPos++;
				kcmd_show_cursor( pWin, 0 );
			}
		}
		else
		{	// ���۷� �� ���ڸ� �����Ѵ�.
			szT[nK] = pS[nI];
			nK++;
			szT[nK] = 0;
			if( nK >= MAX_KCMD_STR )
			{	// ���۰� �� á���� �÷����Ѵ�.
				flushing_text( pWin, szT, wColor );
				nK = 0;
			}
		}	
	}

	// cursor�� �����ش�.
	kcmd_show_cursor( pWin, 1 );
	return( 0 );
}

// ���� �ֿܼ� ���۸��� �����͸� GUI �ֿܼ� �״�� �׷��ش�.
// gui_flushing()���� ȣ��ȴ�.
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

	// ȭ���� ���� �����.
	kcmd_make_screen( pWin, 0 );

	// nStartY���� nY������ ȭ�鿡 �׷��ָ� �ȴ�.
	nEndY = pVC->wCursorY;
	nStartY = nEndY - (pPrivate->nTotalLine - 1);
	if( nStartY < 0 )
		nStartY = 0;

	nTotalY = nEndY - nStartY;
	pVCBuff = &pVC->con_buff[ nStartY * 80 ];
	pFont   = pPrivate->pFont;
	for( nY = 0; nY < nTotalY; nY++ )
	{	// Ŭ���̾�Ʈ ������ ��ǥ�� ������ ���� ��ǥ�� ��ȯ�Ѵ�.
		nPosY = nY * pPrivate->nLineV;
		nPosX = 0;
		client_to_win_pos( pWin, &nPosX, &nPosY );

		for( nX = 0; nX < pPrivate->nTotalCol; nX++ )
		{	// ���۰� ���� + �Ӽ����� �Ǿ� �ֱ� ������ ���� ���ڸ� �׸��� �Լ��� ����Ѵ�.
			nR = drawchar_xy( &pWin->gb, nPosX, nPosY, pFont, pVCBuff->ch, KCMDWIN_TEXT_COLOR, 0 );
			if( nR < 0 )
				return;

			pVCBuff++;
			nPosX += pFont->cH;
		}
	}	

	// Ŀ���� �׷� �ش�.
	kcmd_show_cursor( pWin, 1 );

	// ȭ�� ��ü�� �ٽ� �׸���.
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
// kernel���� ���� ȣ���ϴ� gui ȭ�� ���� �Լ�.
static GuiConsoleFuncStt kcmd_ftbl = {
	NULL, 								  // pWin
	kcmd_write_callback,
	kcmd_direct_write_callback,
	kcmd_cursor_callback,
	kcmd_cls_callback,
	kcmd_flush_callback
};

// KCmdWin winow�� �ʱ�ȭ �Լ�.
int calc_KCmdWin_size( RectStt *pR, DWORD dwPredefStyleID )
{
	FontStt *pFont;

	// Window�� RECT�� ����Ѵ�.
	pFont = get_system_font( KCMDWIN_FONT );
	if( pFont != NULL )
	{
		WinStyleStt *pWS;
		GuiStt		*pGui;

		pGui = get_gui_stt();
		pWS  = find_predef_wstyle( dwPredefStyleID );

		// �ý����� �߾ӿ� ��ġ�ϵ��� �Ѵ�.
		pR->nH = ( pFont->cH * 80 ) + ( pWS->frame.nFrameWidth*2 );
		pR->nV = pFont->cV * 25 + ( pWS->frame.nFrameWidth*3 ) + pWS->frame.nTitleV;
		pR->nX = ( pGui->wall.obj.r.nH - pR->nH ) / 2;
		pR->nY = ( pGui->wall.obj.r.nV - pR->nV ) / 2;
	}

	return( 0 );
}

// Console Window�� �����Ѵ�.
static DWORD wmh_KCmdWin_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	RectStt				*pR;
	KCmdWinPrivateStt	*pPrivate;

	//Ÿ��Ʋ�� �����Ѵ�. 
	set_window_title( pWin, "Console" );

	// ���μ��� Alias�� �����Ѵ�.
	k_set_process_alias( k_get_current_process(), "Console" );

	// Win Private ������ �Ҵ��Ѵ�. 
	pWin->pPrivate = pPrivate = (KCmdWinPrivateStt*)kmalloc( sizeof( KCmdWinPrivateStt ) );
	if( pPrivate != NULL )
	{
		memset( pPrivate, 0, sizeof( KCmdWinPrivateStt ) );
		// ��Ʈ�� �ε��Ѵ�.
		pPrivate->pFont = get_system_font( KCMDWIN_FONT );
	}
	
	// Ŭ���̾�Ʈ ������ �����Ѵ�. (�Ʒ� �Լ����� client area ���� �����Ѵ�.)
	recalc_client_area( pWin );

	pR = get_client_rect( pWin, NULL );

	// ȭ�� ����(���μ�, Ŀ����ġ)�� �缳���Ѵ�.
	pPrivate->nLineV = pPrivate->pFont->cV + KCMDWIN_LINE_GAP;
	pPrivate->nTotalLine = pR->nV / pPrivate->nLineV;
	pPrivate->nXPos = 0;
	pPrivate->nYPos = pPrivate->nTotalLine-1;
	pPrivate->nTotalCol = 80;

	// ���ʷ� �����츦 �׷��д�.
	kcmd_make_screen( pWin, 0 );

	// TaskBar�� �������� ����Ѵ�.
	tb_add_icon( IDI_CMD_ICON, "KCMD", NULL, pWin );

	// �ΰ� ����� �ش�.
	kdbg_printf( "B2OS GUI Console.\n" );

	// redirect�� �����Ѵ�.
	kcmd_ftbl.pWin = pWin;

	set_gui_console_ftbl( &kcmd_ftbl );

	// Ŀ�� ǥ���Ѵ�.
	kcmd_show_cursor( pWin, 1 );

	// �ܼ� ��ü�� ���ʷ� �׷��ش�. (2004-03-25)
	kcmd_flush_callback( pWin, get_active_vconsole() );
	
	return( WMHRV_CONTINUE );
}

static DWORD wmh_KCmdWin_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	KCmdWinPrivateStt	*pPrivate;

	pPrivate = (KCmdWinPrivateStt*)pWin->pPrivate;

	// redirect�� �����Ѵ�.
	set_gui_console_ftbl( NULL );

	// Private ������ �����Ѵ�.
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

