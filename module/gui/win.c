#include <bellona2.h>
#include "gui.h"

static int minimize_window( WinStt *pWin );
static int delete_win_from_scr( WinStt *pWin );

static SysWinStyleStt sys_wstyle;
static DWORD G_dwKWinID = 1;

typedef struct {
	DWORD		dwID;
	char		*pStr;
} WMesgStrStt;

static WMesgStrStt	wmesg_str[] = {
	{ WMESG_CREATE			,  "CREATE"			},
	{ WMESG_CLOSE			,  "CLOSE"			},
	{ WMESG_DESTROY			,  "DESTROY"		},
	{ WMESG_PAINT			,  "PAINT"			},
	{ WMESG_LBTN_DN			,  "LBTN_DN"		},
	{ WMESG_LBTN_UP			,  "LBTN_UP"		},
	{ WMESG_RBTN_DN			,  "RBTN_DN"		},
	{ WMESG_RBTN_UP			,  "RBTN_UP"		},
	{ WMESG_MOUSE_MOVE_OUT	,  "M_MOVE_OUT"		},
	{ WMESG_MOUSE_MOVE_IN	,  "M_MOVE_IN"		},
	{ WMESG_MINIMIZE		,  "MINIMIZE"		},
	{ WMESG_MAXIMIZE		,  "MAXIMIZE"		},
	{ WMESG_MOUSE_MOVE		,  "M_MOVE"			},
	{ WMESG_WIN_MOVE		,  "WIN_MOVE"		},
	{ WMESG_CONTROL			,  "CONTROL"		},
	{ WMESG_REMAKE 			,  "REMAKE"			},
	{ 0, NULL }
};

int disp_wmesg_list()
{
	int	nI;

	kdbg_printf( "Message List : Mesg Value, MesgString\n" );
	for( nI = 0; wmesg_str[nI].pStr != NULL; nI++ )
		kdbg_printf( "  %4d %s\n", wmesg_str[nI].dwID, wmesg_str[nI].pStr );
	return( 0 );
}

// Window나 Control의 핸들링 함수.
static GUIOBJ_FUNC find_pre_guiobj_func( GuiObjFuncStt *pArray, DWORD dwID )
{
	int nI;

	if( pArray == NULL )
		return( NULL );

	for( nI = 0; pArray[nI].dwID != 0; nI++ )
	{
		if( dwID == pArray[nI].dwID )
			return( pArray[nI].pPreFunc );
	}
	return( NULL );
}
static GUIOBJ_FUNC find_post_guiobj_func( GuiObjFuncStt *pArray, DWORD dwID )
{
	int nI;

	if( pArray == NULL )
		return( NULL );

	for( nI = 0; pArray[nI].dwID != 0; nI++ )
	{
		if( dwID == pArray[nI].dwID )
			return( pArray[nI].pPostFunc );
	}
	return( NULL );
}

char *get_wmesg_str( DWORD dwID )
{
	int nI;

	for( nI = 0; wmesg_str[nI].pStr != NULL; nI++ )
	{
		if( wmesg_str[nI].dwID == dwID )
			return( wmesg_str[ nI].pStr );
	}	

	return( "WMESG_UNKNOWN" );
}

DWORD get_wmesg_value( char *pMesgStr )
{
	int nI;

	for( nI = 0; wmesg_str[nI].pStr != NULL; nI++ )
	{
		if( strcmpi( wmesg_str[nI].pStr, pMesgStr ) == 0 )
			return( wmesg_str[ nI].dwID );
	}	

	return( 0 );
}

// Predefined window style 구조체를 초기화한 후 SIMPLE 타입을 추가한다. 
int init_predef_winstyle()
{
	SysWinStyleStt	*pSWS;

	pSWS = &sys_wstyle;
	memset( pSWS, 0, sizeof( SysWinStyleStt ) );

	// WSTYLE_SIMPLE을 초기화한 후 포인터를 얻는다. 
	pSWS->ptr[pSWS->nTotal++] = init_simple_win();
	pSWS->ptr[pSWS->nTotal++] = init_flat_win();
	pSWS->ptr[pSWS->nTotal++] = init_framew_win();

	return( 0 );
}

// 윈도우 ID를 할당한다.
static DWORD kalloc_win_id()
{
	return( G_dwKWinID++ );
}

// 윈도우 객체의 윈도우 메시지 핸들러 함수.
WMESG_FUNC find_r3_wmesg_func( WMFuncStt *pArray, DWORD dwID )
{
	int nI;

	if( pArray == NULL || (DWORD)pArray < (DWORD) 0x80000000 )
		return( NULL );

	for( nI = 0; pArray[nI].pFunc != NULL; nI++ )
	{
		if( dwID == pArray[nI].dwID )
			return( pArray[nI].pFunc );
	}
	return( NULL );
}

WMESG_FUNC find_r0_wmesg_func( WMFuncStt *pArray, DWORD dwID )
{
	int nI;

	if( pArray == NULL || (DWORD)pArray >= (DWORD) 0x80000000 )
		return( NULL );
	
	for( nI = 0; pArray[nI].pFunc != NULL; nI++ )
	{
		if( dwID == pArray[nI].dwID )
			return( pArray[nI].pFunc );
	}
	return( NULL );
}

int call_message_func( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	DWORD		dwR;
	WMESG_FUNC	pFunc;

	if( pWin == NULL )
		return( -1 );

	pFunc = find_r0_wmesg_func( pWin->pWMArray, dwWMesgID );
	if( pFunc == NULL )
		return( -1 );

	dwR = pFunc( pWin, dwWMesgID, dwParamA, dwParamB );
	
	return( dwR );
}

int call_pre_guiobj_handler( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	DWORD		dwR;
	GUIOBJ_FUNC	pFunc;

	if( pOwner == NULL )
		return( -1 );

	pFunc = find_pre_guiobj_func( pOwner->pFuncArray, dwWMesgID );
	if( pFunc == NULL )
		return( -1 );

	dwR = pFunc( pOwner, dwWMesgID, dwParamA, dwParamB );
	
	return( dwR );
}

int call_post_guiobj_handler( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	DWORD		dwR;
	GUIOBJ_FUNC pFunc;

	if( pOwner == NULL )
		return( -1 );

	pFunc = find_post_guiobj_func( pOwner->pFuncArray, dwWMesgID );
	if( pFunc == NULL )
		return( -1 );

	dwR = pFunc( pOwner, dwWMesgID, dwParamA, dwParamB );
	
	return( dwR );
}

static GuiObjStt *find_cat_obj( GuiObjStt *pOwner, DWORD dwX, DWORD dwY )
{
	GuiObjStt	*pObj;
	
	// 연결된 컨트롤들을 대상으로 새로 고양이를 판단한다.
	for( pObj = pOwner->pStart; ; pObj = pObj->pNext )
	{
		if( pObj == NULL || pObj->wMagic != GUI_OBJ_MAGIC )
			break;

		if( is_in_rect( &pObj->r, dwX, dwY ) )
		{
			return( pObj );
		}
	}
	return( NULL );
}

static int disp_gui_obj( GuiObjStt *pOwner )
{
	GuiObjStt *pObj;

	if( pOwner->wType == GUI_OTYPE_WINDOW )
	{
		WinStt *pWin;
		pWin = (WinStt*)pOwner;
		kdbg_printf( "Owner: %s\n", pWin->szTitle );
	}
	else
		kdbg_printf( "Owner: UNKNOWN\n" );

	for( pObj = pOwner->pStart; pObj != NULL; pObj = pObj->pNext )
	{
		kdbg_printf( "GuiObj: 0x%X (%d, %d, %d, %d)\n", pObj, pObj->r.nX, pObj->r.nY, pObj->r.nH, pObj->r.nV );
	}	

	return( 0 );
}

static DWORD wmh_pre_common_lbtn_dn( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwWinX, DWORD dwWinY )
{
	int			nR;
	GuiObjStt	*pObj;
	WinStt		*pWin;
	int			nObjX, nObjY;

	// 윈도우인가?
	if( pOwner->wType == GUI_OTYPE_WINDOW )
	{
		ProcessStt *pP;
		
		pWin = (WinStt*)pOwner;
		if( pWin->pWThread == NULL || pWin->pWThread->pThread == NULL )
			goto GO_CAT;
		// 프로세스를 FG로 설정한다.	
		pP = pWin->pWThread->pThread->pProcess;
		set_fg_process( pP->pVConsole, pP );
		// VConsole을 Active로 설정한다.
		set_active_vconsole( pP->pVConsole );
	}
	
GO_CAT:
	pObj = find_cat_obj( pOwner, dwWinX, dwWinY );
	if(	pObj == NULL )
		return( WMHRV_CONTINUE );

	// 고양이를 새로 설정한 후 좌표를 변경하여 호출한다.
	pOwner->pCat = pObj;

	// 내부 좌표로 변경한 후 전달한다.
	inner_pos( &pObj->r, dwWinX, dwWinY, &nObjX, &nObjY );
	nR = call_pre_guiobj_handler( pObj, dwWMesgID, nObjX, nObjY );

	return( nR );
}

static DWORD wmh_pre_common_lbtn_up( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwWinX, DWORD dwWinY )
{
	int			nR;
	GuiObjStt	*pObj;
	int			nObjX, nObjY;

	pObj = find_cat_obj( pOwner, dwWinX, dwWinY );
	if(	pObj == NULL )
		return( 0 );

	// 고양이를 새로 설정한 후 좌표를 변경하여 호출한다.
	pOwner->pCat = pObj;
	inner_pos( &pObj->r, dwWinX, dwWinY, &nObjX, &nObjY );
	nR = call_pre_guiobj_handler( pObj, dwWMesgID, nObjX, nObjY );

	return( nR );
}

static DWORD wmh_pre_common_mouse_move_in( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwWinX, DWORD dwWinY )
{
	int			nR;
	GuiObjStt	*pObj;
	int			nObjX, nObjY;

	pObj = find_cat_obj( pOwner, dwWinX, dwWinY );
	if(	pObj == NULL )
		return( WMHRV_CONTINUE );

	// 고양이를 새로 설정한 후 좌표를 변경하여 호출한다.
	pOwner->pCat = pObj;
	inner_pos( &pObj->r, dwWinX, dwWinY, &nObjX, &nObjY );
	nR = call_pre_guiobj_handler( pObj, dwWMesgID, nObjX, nObjY );
	
	return( nR );
}

static DWORD wmh_pre_common_mouse_move_out( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	int			nR;
	GuiObjStt	*pOldCat;

	// 이전 고양이의 mouse_move_out 함수를 호출한다.
	if( pOwner->pCat == NULL )
		return( WMHRV_CONTINUE );

	pOldCat = pOwner->pCat;
	pOwner->pCat = NULL;
	nR = call_pre_guiobj_handler( pOldCat, dwWMesgID, 0, 0 );

	return( nR );
}

// dwWinX, dwWinY는 윈도우 내의 좌표.
static DWORD wmh_pre_common_mouse_move( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwWinX, DWORD dwWinY )
{
	int			nR;
	GuiObjStt	*pObj;
	int			nObjX, nObjY;
	
	nR = 0;

	//kdbg_printf( "X=%d, Y=%d\n", dwWinX, dwWinY );

	pObj = find_cat_obj( pOwner, dwWinX, dwWinY );
	if(	pOwner->pCat == pObj )
	{	// 이전 고양이와 같으면 그냥 mouse_move 함수를 호출한다.
		// 좌표를 변경하여 호출한다.
		inner_pos( &pObj->r, dwWinX, dwWinY, &nObjX, &nObjY );
		nR = call_pre_guiobj_handler( pObj, WMESG_MOUSE_MOVE, nObjX, nObjY );
		return( WMHRV_CONTINUE );
	}

	// 이전 고양이의 mouse_move_out 함수를 호출한다.
	if( pOwner->pCat != NULL )
	{	
		GuiObjStt	*pOldCat = pOwner->pCat;
		pOwner->pCat = NULL;	// 잠깐 NULL로 만들어 준다.
		nR = call_pre_guiobj_handler( pOldCat, WMESG_MOUSE_MOVE_OUT, 0, 0 );
	}

	// 고양이를 새로 설정한다.
	pOwner->pCat = pObj;
	if( pObj != NULL )
	{	// 좌표를 변경하여 호출한다.
		inner_pos( &pObj->r, dwWinX, dwWinY, &nObjX, &nObjY );
		nR = call_pre_guiobj_handler( pObj, WMESG_MOUSE_MOVE_IN, nObjX, nObjY );
		return( WMHRV_ABORT );
	}

	return( WMHRV_CONTINUE );
}

static DWORD wmh_pre_common_paint( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwXY, DWORD dwHV )
{
	WinStt	*pWin;
	RectStt	r, *pR, winrect;

	pWin = (WinStt*)pOwner;

	if( dwXY == 0 && dwHV == 0 )
		pR = NULL;
	else
	{
		dword_to_rect_xy( dwXY, &winrect );
		dword_to_rect_hv( dwHV, &winrect );
		pR   = &winrect;
	}

	// Transparent Window에 대한 처리를 해 준다.
	// 하위 윈도우들을 먼저 그려야 한다.
	// 윈도우 좌표를 절대 좌표로 변경한다.
	if( pR != NULL && (pWin->gb.dwAttr & WIN_ATTR_TRANSPARENT) )
	{
		client_to_screen( &r, &pOwner->r, pR );

		//kdbg_printf( "wmh_pre_common_paint: (%d, %d, %d, %d)\n", pR->nX, pR->nY, pR->nH, pR->nV );

		repaint_down_layer( pWin, &r );
	}
	
	return( WMHRV_CONTINUE );
}

static DWORD wmh_pre_common_minimize( GuiObjStt *pObj, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	WinStt *pWin;

	pWin = (WinStt*)pObj;

	call_message_func( pWin, dwWMesgID, dwParamA, dwParamB );
	
	// Minimize 시킨다.
	minimize_window( pWin );

	// 각 윈도우들의 마스크 비트를 다시 계산한다.
	recalc_bit_mask();

	// 윈도우가 있던 자리를 지운다. 
	repaint_down_layer( pWin, &pWin->obj.r ); 
	repaint_upper_transparent( pWin, &pWin->obj.r ); 

	return( 0 );
}

static DWORD wmh_pre_common_show( GuiObjStt *pObj, DWORD dwWMesgID, DWORD dwShowFlag, DWORD dwParamB )
{
	DWORD	dwR;
	WinStt	*pWin;


	kdbg_printf( "wmh_pre_common_show: dwShowFlag = %d\n", dwShowFlag );

	pWin = (WinStt*)pObj;

	dwR = call_message_func( pWin, dwWMesgID, dwShowFlag, dwParamB );
	if( dwR == WMHRV_ABORT )
		return( dwR );
	
	if( ( pWin->gb.dwState & WIN_STATE_HIDDEN ) && dwShowFlag == 0 )
	{
		kdbg_printf( "Already Hidden window\n" );
		return( 0 );	// 이미 HIDE로 설정되어 있음.
	}
	else if( ( pWin->gb.dwState & WIN_STATE_HIDDEN ) == 0  && dwShowFlag != 0 )
	{
		kdbg_printf( "Already Visible window\n" );
		return( 0 );	// 이미 SHOW로 설정되어 있음.
	}

	if( dwShowFlag != 0 )
	{	// SHOW로 설정한다.
		pWin->gb.dwState &= (DWORD)~(DWORD)WIN_STATE_HIDDEN;

		// 각 윈도우들의 마스크 비트를 다시 계산한다.
		recalc_bit_mask();

		// 윈도우를 다시 그린다.
		call_win_message_handler( pWin, WMESG_PAINT, 0, 0 );
	}
	else
	{	// HIDE로 설정한다. 
		pWin->gb.dwState |= (DWORD)WIN_STATE_HIDDEN;

		// 각 윈도우들의 마스크 비트를 다시 계산한다.
		recalc_bit_mask();

		// 윈도우가 있던 자리를 지운다. 
		repaint_down_layer( pWin, &pWin->obj.r ); 
		repaint_upper_transparent( pWin, &pWin->obj.r ); 
	}

	return( 0 );
}

// 윈도우를 새로운 좌표로 이동시킨다.
static DWORD wmh_pre_common_win_move( GuiObjStt *pGuiObj, DWORD dwWMesgID, DWORD dwX, DWORD dwY )
{
	RectStt	old_r;
	WinStt *pWin;

	pWin = (WinStt*)pGuiObj;

	// 이미 동일한 위치에 있었는지 확인.
	if( pWin->obj.r.nX == (int)dwX && pWin->obj.r.nY == (int)dwY )
		return( 0 );

	memcpy( &old_r, &pWin->obj.r, sizeof( RectStt ) );
	
	// 새로운 좌표로 설정한다.
	pWin->obj.r.nX = (int)dwX;
	pWin->obj.r.nY = (int)dwY;

	// 타이틀바 프레임을 제외한 클라이언트 영역을 다시 계산한다. 
	recalc_client_area( pWin );

	// 각 윈도우들의 마스크 비트를 다시 계산한다.
	recalc_bit_mask();

	// 기존에 윈도우가 있던 자리를 지운다. 
	repaint_down_layer( pWin, &old_r ); 
	repaint_upper_transparent( pWin, &old_r );

	// 객체 윈도우 핸들러를 호출한다. 
	call_message_func( pWin, dwWMesgID, 0, 0 );

	// 윈도우를 다시 그린다.
	call_win_message_handler( pWin, WMESG_PAINT, 0, 0 );

	return( 0 );
}

static DWORD wmh_post_common_paint( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwXY, DWORD dwHV )
{
	WinStt	*pWin;
	RectStt win_r, r, *pR, winrect;

	pWin = (WinStt*)pOwner;
	
	if( dwXY == 0 && dwHV == 0 )
	{	// 상당히 잡기 힘든 버그였다.  
		pR = &pWin->obj.r;
		screen_to_win( &r, pR, pR );
		pR = &r;
	}
	else
	{
		dword_to_rect_xy( dwXY, &winrect );
		dword_to_rect_hv( dwHV, &winrect );
		pR	 = &winrect;
	}

	// 현재 윈도우의 위쪽에 TRANSPARENT 윈도우가 있으면 다시 그려준다.
	client_to_screen( &win_r, &pOwner->r, pR );
	repaint_upper_transparent( pWin, &win_r );
	
	return( WMHRV_CONTINUE );
}

static DWORD wmh_post_common_destroy( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwWinRect, DWORD dwParamB )
{
	// 윈도우를 닫는다.
	kclose_window( (WinStt*)pOwner );	
	
	return( WMHRV_CONTINUE );
}

static DWORD wmh_post_common_create( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwWinRect, DWORD dwParamB )
{
	WinStt *pWin;

	if( pOwner->wType == GUI_OTYPE_WINDOW )
	{
		pWin = (WinStt*)pOwner;
		pWin->gb.dwState |= WIN_STATE_INITIALIZED;
	}

	// 2004-01-24  윈도우 생성시 크기와 좌표가 입력되지 않는 경우에는 post create에서 bit_mask를 다시
	// 처리해야 한다.
	recalc_bit_mask();
	
	return( WMHRV_CONTINUE );
}

static GuiObjFuncStt wmfunc_common[] = {
	{ WMESG_CREATE			,	NULL							, wmh_post_common_create	},
	{ WMESG_MOUSE_MOVE_OUT	,	wmh_pre_common_mouse_move_out	, NULL						},
	{ WMESG_MOUSE_MOVE_IN	,	wmh_pre_common_mouse_move_in	, NULL						},
	{ WMESG_MOUSE_MOVE		,	wmh_pre_common_mouse_move		, NULL						},
	{ WMESG_LBTN_DN			, 	wmh_pre_common_lbtn_dn			, NULL						},
	{ WMESG_LBTN_UP			, 	wmh_pre_common_lbtn_up			, NULL						},
	{ WMESG_PAINT			,	wmh_pre_common_paint			, wmh_post_common_paint 	},
	{ WMESG_MINIMIZE		,	wmh_pre_common_minimize			, NULL						},
	{ WMESG_WIN_MOVE		,	wmh_pre_common_win_move			, NULL						},
	{ WMESG_SHOW			,	wmh_pre_common_show				, NULL						},
	{ WMESG_DESTROY			,	NULL							, wmh_post_common_destroy	},
	{ 0, NULL, NULL }
};

int pop_win_mesg( WMesgQStt *pWMQ, WMesgStt *pWM )
{
	if( pWMQ->nTotal == 0 )
		return( -1 );		// 더이상 메시지가 없다.
	
	// 메시지 하나를 꺼낸다. 
	_asm {
		PUSHFD
		CLI
	}
	memcpy( pWM, &pWMQ->q[ pWMQ->nStart ], sizeof( WMesgStt ) );;
	pWMQ->nTotal--;
	pWMQ->nStart++;
	if( pWMQ->nStart >= MAX_WMESG_IN_Q )
		pWMQ->nStart = 0;
	_asm POPFD
		
	return( 0 );
}

// 개별 윈도우에 들어온 메시지를 처리한다. 
static int dispatch_winmesg( WinStt *pWin )
{
	WMesgStt		wm;
	WMesgQStt		*pWMQ;	
	int 			nR, nTotal;
	
	pWMQ = &pWin->wmq;

	for( nTotal = 0;; )
	{	// 들어온 메시지를 하나 얻어온다.
		nR = pop_win_mesg( pWMQ, &wm );
		if( nR < 0 )
			break;			// 메시지가 더이상 없으며 돌아간다.

		// 메시지 핸들러를 찾아 호출한다.
		if( wm.dwID >MAX_WMESG_FUNC )
		{
			kdbg_printf( " Invalid WMESG(%d)\n", wm.dwID );
			continue;
		}

		call_win_message_handler( pWin, wm.dwID, wm.dwParamA, wm.dwParamB );
		nTotal++;
	}
	
	return( nTotal );
}

// 윈도우들에 들어온 메시지를 처리한다.
static int winmesg_handling( WinThreadStt *pWThread )
{
	int			nTotal, nR;
	WinStt		*pWin, *pNext;

	nTotal = 0;
	// WinThread가 관리하는 윈도우들에 대해 일일이 검사를 수행한다.
	for( pWin = pWThread->pStart; pWin != NULL; pWin = pNext )
	{
		pNext = pWin->pNext;
		nR = dispatch_winmesg( pWin );
		if( nR > 0 )
			nTotal += nR;
	}

	return( nTotal );
}

// GUI 시스템의 커널 윈도우 메시지 핸들러.
static int kwin_mesg_thread( void *pParam )
{
	int				nTotal;
	WinThreadStt	*pWThread;

	pWThread = (WinThreadStt*)pParam;

	// 메시지가 없는 것을 확인하고 SUSPEND로 들어가기 직전에 메시지가 들어와 버리면
	// 그냥 SUSPEND로 들어가 버리는 잠재적인 문제점이 존재함.
	for( ;; )
	{	
		// 메시지를 받은 윈도우가 있으면 처리한다. 
		nTotal = winmesg_handling( pWThread );

		// 처리한 메시지가 없으면 Suspend로 들어간다.
		if( nTotal == 0 )
			ksuspend_thread( pWThread->pThread );
	}	

	return( 0 );
}

static char *pKWinEventName = "KWIN_MESG";
int kclose_win_thread( WinThreadStt *pWThread )
{
	// 이벤트를 해제한다.
	if( pWThread->pE != NULL )
		close_event( pWThread->pE );
	
	// 메시지 쓰레드를 제거한다.
	// Event Wait 상태의 쓰레드를 강제로 죽여 버린다. (자진하도록 하는 것이 더 낫겠지.)
	// 생성된 쓰레드일 경우에만 제거한다.
	if( pWThread->byThreadCreated != 0 && pWThread->pThread != NULL )
		kill_thread( pWThread->pThread );
			
	// WinThread를 해제한다.
	kfree( pWThread );
	
	return( 0 );
}

// 윈도우 쓰레드를 생성한다.
WinThreadStt *kcreate_win_thread( ThreadStt *pT )
{	
	WinThreadStt *pWThread;

	pWThread = (WinThreadStt*)kmalloc( sizeof( WinThreadStt ) );
	if( pWThread == NULL )
		return( NULL );

	memset( pWThread, 0, sizeof( WinThreadStt ) );

	// 이벤트를 생성한다.  ( close_event()를 call해 줘야 한다. )
	pWThread->pE = create_event( pKWinEventName );
	if( pWThread->pE == NULL )
	{	
		kdbg_printf( "create_event( KWIN_MESG ) failed!" );
		// 할당했던 구조체를 해제한다.
		kfree( pWThread );
		return( NULL );
	}

	// 윈도우 메시지를 처리할 쓰레드를 생성한다. 
	// Owner Process, StackSize, Entry Function, Parameter
	if( pT == NULL )
	{
		pWThread->pThread = kcreate_thread( k_get_current_process(), 0, (DWORD)kwin_mesg_thread, (DWORD)pWThread, TS_READY_NORMAL );	
		if( pWThread->pThread == NULL )
		{	// 위에서 할당받은 이벤트 구조체를 해제한다.
			close_event( pWThread->pE );
			kfree( pWThread );
			return( NULL );		// 쓰레드를 생성할 수 없다. 
		}
		pWThread->byThreadCreated = 1;		// 쓰레드가 새로 생성됨.

		// ALIAS를 설정한다. 
		k_set_thread_alias( pWThread->pThread, "kwin_thread" );
		
		// 생성된 쓰레드를 바로 실행한다. 
		kernel_thread_switching( pWThread->pThread );
	}
	else
	{			  
		pWThread->pThread = pT;
	}	
	
	return( pWThread );
}

// Predefined Window Style을 찾는다. 
WinStyleStt *find_predef_wstyle( DWORD dwID )
{
	int nI;
	SysWinStyleStt	*pSWS;

	pSWS = &sys_wstyle;
	for( nI = 0; nI < MAX_PREDEF_WIN_STYLE; nI++ )
	{
		if( pSWS->ptr[nI] != NULL && pSWS->ptr[nI]->dwID == dwID )
			return( pSWS->ptr[nI] );
	}
	// 찾을 수 없다. 
	return( NULL );
}

// 윈도우를 쓰레드에 연결한다. 
int append_win_to_thread( WinThreadStt *pWThread, WinStt *pWin )
{
	if( pWThread->nTotal == 0 )
	{	// 처음 추가되는 것. 
		pWThread->pStart = pWThread->pEnd = pWin;
		pWin->pPrev = pWin->pNext = NULL;
	}
	else
	{	// 큐의 가장 뒤에 추가한다. 
		pWThread->pEnd->pNext = pWin;
		pWin->pPrev		 = pWThread->pEnd;
		pWin->pNext		 = NULL;
		pWThread->pEnd		 = pWin;
	}
	pWThread->nTotal++;
	
	pWin->pWThread = pWThread;

	return( 0 );
}

// 윈도우 큐에서 제거한다.
static int delete_win_from_thread( WinStt *pWin )
{
	WinThreadStt *pWThread;
	
	pWThread = pWin->pWThread;
	if( pWThread == NULL )
	    return( -1 );
	
	if( pWThread->nTotal <= 0 )
	    return( -1 );
	    
	if( pWin->pPrev == NULL )
	    pWThread->pStart = pWin->pNext;
	else
	    pWin->pPrev->pNext = pWin->pNext;
	    
	if( pWin->pNext == NULL )
	    pWThread->pEnd = pWin->pPrev;
	else
	    pWin->pNext->pPrev = pWin->pPrev;        	
	
	pWin->pWThread = NULL;
	
	return( 0 );
}

int alloc_gra_buff_ex( GraBuffStt *pGB, RectStt *pR )
{
	int 	nSize;

	// 0으로 우선 초기화 한다.
	memset( pGB, 0, sizeof( GraBuffStt ) );

	nSize = pR->nH * 2 * pR->nV;
	pGB->pW = (UINT16*)kmalloc( nSize );
	if( pGB->pW == NULL )
		return( -1 );

	pGB->pR = pR;

	return( 0 );
}

// 그래픽 버퍼를 할당한다.
int alloc_gra_buff( WinStt *pWin )
{
	int			nR;

	nR = alloc_gra_buff_ex( &pWin->gb, &pWin->obj.r );

	return( nR );
}

// 그래픽 버퍼를 할당한다.
int free_gra_buff_ex( GraBuffStt *pGB )
{
	// self mask를 해제한다.
	if( pGB->self_mask.pB != NULL )
	{
		kfree( pGB->self_mask.pB );
		pGB->self_mask.pB = NULL;
	}	

	// GraBuff의 버퍼를 해제한다.
	if( pGB->pW != NULL )
	{
		kfree( pGB->pW );
		pGB->pW = NULL;
	}

	return( 0 );
}

int free_gra_buff( WinStt *pWin )
{
	int nR;
	nR = free_gra_buff_ex( &pWin->gb );
	return( nR );
}


// 윈도우 구조체를 해제한다.
void free_win_Stt( WinStt *pWin )
{
	kfree( pWin );
}

// 윈도우를 닫는다.
int kclose_window( WinStt *pWin )
{	
	RectStt		r;
	WinStt		*pUpperWin, *pLowerWin;

	memcpy( &r, &pWin->obj.r, sizeof( RectStt ) );
	pUpperWin = pWin->pPreLevel;
	pLowerWin = pWin->pNextLevel;

	// Focus를 쥐고 있었다면 Invalidate 시킨다.
	invalidate_mouse_owner( pWin );

	// 윈도우를 스크린에서 제거한다.
	delete_win_from_scr( pWin );

	// 윈도우를 쓰레드에서 제거한다.
	delete_win_from_thread( pWin );	

	// 비트 마스크를 제거한다.
	free_bit_mask( pWin );

	// 그래픽 버퍼를 해제한다.
	free_gra_buff( pWin );

	//윈도우 구조체를 해제한다.
	free_win_Stt( pWin );

	// 각 윈도우들의 마스크 비트를 다시 계산한다.
	recalc_bit_mask();

	// 기존에 윈도우가 있던 자리를 지운다. 
	repaint_down_layer( pUpperWin, &r ); 		  // pUpperWin, pLowerWin 둘 다 NULL
	repaint_upper_transparent( pLowerWin, &r );	  // 이어도 상관없다.

	return( 0 );	
}

// 윈도우를 스크린의 레벨 링크에 추가한다.
int append_win_to_scr( WinStt *pWin )
{
	GuiStt *pGui;

	pGui = get_gui_stt();

	if( pGui->pStartLevelWin == NULL )
	{
		pGui->pStartLevelWin = pWin;
		pGui->pEndLevelWin   = pWin;
		pWin->pPreLevel      = NULL;
		pWin->pNextLevel     = NULL;
	}
	else
	{	// TOP Level에 추가한다.
		pGui->pStartLevelWin->pPreLevel = pWin;
		pWin->pNextLevel				= pGui->pStartLevelWin;
		pWin->pPreLevel					= NULL;
		pGui->pStartLevelWin			= pWin;
	}		

	return( 0 );
}	

// 윈도우를 스크린의 레벨 링크에서 제거한다.
static int delete_win_from_scr( WinStt *pWin )
{
	GuiStt	*pGui;

	//if( pWin == NULL )
	//{
	//	kdbg_printf( "delete_win_from_scr: pWin is NULL!\n" );
	//	return( -1 );
	//}
	//else
	//	kdbg_printf( "delete_win_from_scr: pWin = 0x%X\n", (DWORD)pWin );
		

	pGui = get_gui_stt();

	if( pWin->pPreLevel == NULL )
		pGui->pStartLevelWin = pWin->pNextLevel;
	else
		pWin->pPreLevel->pNextLevel = pWin->pNextLevel;

	if( pWin->pNextLevel == NULL )
		pGui->pEndLevelWin = pWin->pPreLevel;
	else
		pWin->pNextLevel->pPreLevel = pWin->pPreLevel;

	pWin->pPreLevel = pWin->pNextLevel = NULL;

	return( 0 );
}

/**
 *  현재 윈도우 가운데 pWall에 0으로 설정된 것은 0으로 만든다.
 *  CurWinBit = CurWinBit & pWall
**/
static int mask_from_wall_vect( MaskStt *pMask, MaskStt *pWall )
{
	int		nDiv, nMod, nH, nV;
	DWORD	dwM_LinesByte, dwW_LinesByte;
	BYTE	*pWallBuff, *pMaskBuff, byBit;

	if( pMask->pB == NULL || pWall->pB == NULL )
		return( -1 );

	// pMask->pR은 마스크를 가지고 있는 윈도우의 현재 좌표.
	nDiv = pMask->pR->nX / 8;
	nMod = pMask->pR->nX % 8;

	pMaskBuff = pMask->pB;
	pWallBuff = &pWall->pB[ nDiv + pMask->pR->nY * pWall->dwLineBytes ];
	byBit = (BYTE)0x80;
	if( nMod > 0 ) 
		byBit = (BYTE)( byBit >> nMod );
	nH = pMask->pR->nH;
	nV = pMask->pR->nV;
	dwM_LinesByte = pMask->dwLineBytes; 
	dwW_LinesByte = pWall->dwLineBytes;

	_asm {
		MOV  ESI, pMaskBuff
		MOV	 EDI, pWallBuff
		MOV  ECX, nV
		MOV  DL,  byBit
		MOV  BL,  0x80

V_LINES:
		PUSH EBX
		PUSH ECX
		PUSH EDX
		PUSH ESI
		PUSH EDI
			 
		     MOV ECX,nH 

			 // WALL측 해당 비트가 0인가?
H_BITS:		 MOV DH, DL				  
			 AND DH, [EDI]
			 JNZ SHR_MASK_BIT		  
				 // WALL측 비트가 0인 경우에만 MASK의 비트를 CLEAR한다.
				 MOV DH, BL					  
				 NOT DH
				 AND [ESI], DH
SHR_MASK_BIT:
			 SHR DL, 1
			 OR  DL, DL
			 JNZ SHR_M_MASK
			     MOV DL,0x80 
				 INC EDI

SHR_M_MASK:  SHR BL, 1
			 OR  BL,BL
			 JNZ H_LOOP
			     MOV BL, 0X80
				 INC ESI

H_LOOP:		 LOOP  H_BITS;
		
		POP  EDI
		POP  ESI
		POP  EDX
		POP  ECX
		POP  EBX

		ADD  ESI, dwM_LinesByte
		ADD  EDI, dwW_LinesByte;
		LOOP V_LINES
	}	

	return( 0 );
}		

// pWall의 비트들 가운데 현재 윈도우의 self_mask 가운데 1인 것을 0으로 만든다.
static int mask_to_wall_vect( MaskStt *pMask, MaskStt *pWall )
{
	int		nDiv, nMod, nH, nV;
	DWORD	dwM_LinesByte, dwW_LinesByte;
	BYTE	*pWallBuff, *pMaskBuff, byBit;

	if( pMask->pB == NULL || pWall->pB == NULL )
	{	// 마스크 버퍼가 NULL이다.
		kdbg_printf( "mask_to_wall_vect() - NULL mask buffer!\n" );
		return( -1 );
	}

	// pMask->pR은 마스크를 가지고 있는 윈도우의 현재 좌표.
	nDiv = pMask->pR->nX / 8;
	nMod = pMask->pR->nX % 8;

	pMaskBuff = pMask->pB;
	pWallBuff = &pWall->pB[ nDiv + pMask->pR->nY * pWall->dwLineBytes ];
	byBit = (BYTE)0x80;
	if( nMod > 0 ) 
		byBit = (BYTE)( byBit >> nMod );
	nH = pMask->pR->nH;
	nV = pMask->pR->nV;
	dwM_LinesByte = pMask->dwLineBytes; 
	dwW_LinesByte = pWall->dwLineBytes;

	_asm {
		MOV  ESI, pMaskBuff
		MOV	 EDI, pWallBuff
		MOV  ECX, nV
		MOV  DL,  byBit
		MOV  BL,  0x80

V_LINES:
		PUSH EBX
		PUSH ECX
		PUSH EDX
		PUSH ESI
		PUSH EDI
			 
	     MOV ECX,nH 

		// MASK 비트가 '1'이면 WALL 측 비트를 0으로 만든다.
H_BITS:	MOV DH, BL
		AND DH, [ESI]
		JZ  SHR_W_MASK
		    MOV DH, DL
			NOT DH
			AND [EDI], DH

SHR_W_MASK:	 SHR DL, 1
			 OR  DL, DL
			 JNZ SHR_M_MASK
			     MOV DL,0x80 
				 INC EDI

SHR_M_MASK:  SHR BL, 1
			 OR  BL,BL
			 JNZ H_LOOP
			     MOV BL, 0x80
				 INC ESI

H_LOOP:		 LOOP  H_BITS;
		
		POP  EDI
		POP  ESI
		POP  EDX
		POP  ECX
		POP  EBX

		ADD  ESI, dwM_LinesByte
		ADD  EDI, dwW_LinesByte;
		LOOP V_LINES
	}	

	return( 0 );
}		

// 크기와 구조가 동일한 mask간의 AND는 그냥 byte AND해도 된다.
static int and_mask_byte_array( MaskStt *pDest, MaskStt *pSrc )
{
	int nI;

	if( pDest->pB == NULL || pSrc->pB == NULL )
		return( -1 );

	for( nI = 0; nI < (int)pDest->dwSize; nI++ )
		pDest->pB[nI] &= pSrc->pB[nI];

	return( 0 );
}

void get_align_count( DWORD dwAddr, DWORD dwSize, int nAlignSize, 
							int *pBody, int *pTail )
{
	pBody[0] = dwSize / nAlignSize;
	pTail[0] = dwSize - ( pBody[0] * nAlignSize );
}

// 마스크의 모든 비트가 0인지 또는 1인지 검사한다.
static int check_mask_bits( MaskStt *pMask )
{
	BYTE	byModByte, byModMask;
	DWORD	dwCheck, dwLineBytes, dwMaskBuff;
	int		nBody, nTail, nH, nMod, nV, nR;

	if( pMask == NULL || pMask->pR == NULL || pMask->pB == NULL || pMask->dwSize == 0 )
		return( -10 );

	// Black으로 갈 것인지 White로 갈 것인지 결정한다.
	if( pMask->pB[0] == 0 )
		dwCheck = 0;
	else if( (BYTE)pMask->pB[0] == (BYTE)0xFF )
		dwCheck =  0xFFFFffff;
	else
	{	// BLACK 또는 WHITE 모두 아니다.
		pMask->byFlag = 0;		
		return( -1 );
	}

	// 한 라인의 align count를 구한다.
	nH = pMask->pR->nH/8;
	nV = pMask->pR->nV;
	nMod = pMask->pR->nH % 8;
	if( nMod != 0 )
	{
		nH--;

		byModMask = 0xFF;
		byModMask = byModMask >> nMod;
		byModMask = (BYTE)~byModMask;
		if( dwCheck == 0 )
			byModByte = 0;
		else
			byModByte = byModMask;
	}
	get_align_count( (DWORD)pMask->pB, nH, 4, &nBody, &nTail );
	
	dwLineBytes = pMask->dwLineBytes;
	dwMaskBuff  = (DWORD)pMask->pB;
	_asm{
		PUSH EDI
		PUSHFD
		CLD

		MOV	  ECX, nV
		MOV   EDI, dwMaskBuff
		MOV   EAX, dwCheck
		
NEXT_LINE:
		PUSH  ECX
		PUSH  EDI

		// BODY
		MOV   ECX, nBody
		CMP   ECX, 0
		JZ    CHK_TAIL
			  REPE  SCASD
			  CMP   ECX, 0
			  MOV   DWORD PTR nR, 0xFFFFFFFF-1
			  JNZ   X_BACK

CHK_TAIL:// TAIL
		MOV   ECX, nTail
		CMP   ECX, 0
		JZ    CHK_MOD
			  REPE  SCASB
			  CMP   ECX, 0
			  MOV   DWORD PTR nR, 0xFFFFFFFF-2
			  JNZ   X_BACK

CHK_MOD:// MOD
		CMP   DWORD PTR nMod, 0
		JZ    VVV
			  MOV   BL, [EDI]
			  AND   BL, byModMask	// 검사할 필요가 없는 비트들은 '0'이 된다.
			  CMP   BL, byModByte
			  MOV   DWORD PTR nR, 0xFFFFFFFF-3
	    	  JNZ   X_BACK		
		
VVV:	POP  EDI
		POP  ECX
		
		ADD  EDI, dwLineBytes
		LOOP NEXT_LINE

		POPFD
		POP EDI
	}

	if( dwCheck == 0 )
	{
		pMask->byFlag = MASK_FLAG_BLACK;
		nR = 0;
	}
	else
	{
		pMask->byFlag = MASK_FLAG_WHITE;
		nR = 1;
	}

	return( nR );

	_asm {
X_BACK:
		POP  EDI
		POP  ECX
		POPFD
		POP  EDI
	}

	return( nR );
}

// 시스템의 모든 윈도우에 대한 비트 마스크를 갱신한다.
int recalc_bit_mask()
{
	int		nR;
	GuiStt	*pGui;
	WinStt	*pWin, *pWall;

	pGui = get_gui_stt();

	//kdbg_printf( "recalc_bit_mask()\n" );

	// 배경 화면의 비트 마스크를 모두 1로 설정한다.
	pWall = &pGui->wall; 
	memset( pWall->mask.pB, 0xFF, pWall->mask.dwSize );

	for( pWin = pGui->pStartLevelWin; pWin!= NULL; pWin = pWin->pNextLevel )
	{	
		if( pWin->gb.dwState & WIN_STATE_MINIMIZED || pWin->gb.dwState & WIN_STATE_HIDDEN )
		{	// minimize되어 있으면 마스크를 모두 0으로 설정하고 다음 윈도우로 넘어간다.
			//memset( pWin->mask.pB, 0x00, pWin->mask.dwSize );
			pWin->mask.byFlag = MASK_FLAG_BLACK;
			continue;
		}

		// 초기화되지 않은 윈도우는 마스크를 고려하지 않는다.
		if( ( pWin->gb.dwState & WIN_STATE_INITIALIZED ) == 0 )
			continue;
		
		// 현재 윈도우의 모든 비트를 1로 설정한다.
		memset( pWin->mask.pB, 0xFF, pWin->mask.dwSize );

		// pWall의 비트 가운데 0으로 설정된 것이 있으면 0으로 만든다.
		// 이미 보다 위쪽의 다른 윈도우에 의해 가려진 부분.
		mask_from_wall_vect( &pWin->mask, &pWall->mask );

		pWin->mask.byFlag = 0;
		// pWall의 비트들 가운데 현재 윈도우 영역에 해당되는 것들을 0으로 만든다.
		if( pWin->gb.self_mask.pB == NULL )
		{
			if( ( pWin->gb.dwAttr & WIN_ATTR_TRANSPARENT ) == 0 )
				modify_bit_vect( &pWall->mask, &pWin->obj.r, 0 );
			
			// 마스크가 BLACK인지 WHITE인지 검사해 본다. (셀프 마스크가 있는 경우에는 검사할 필요가 없다.)
			nR = check_mask_bits( &pWin->mask );
			//kdbg_printf( "CHECK_MASK(%s) : %d\n", pWin->szTitle, nR );
		}
		else
		{	// self mask가 있으면 자신의 윈도우 mask와 and 한다.
			// bitmask의 구조가 같으므로 그냥 byte and 해도 된다.
			and_mask_byte_array( &pWin->mask, &pWin->gb.self_mask );

			// pWall의 비트들 가운데 현재 윈도우의 self mask 가운데 1인 것을 0으로 만든다.
			// self_mask가 있을 때에는 modify_bit_vect 대신 호출된다. 
			if( ( pWin->gb.dwAttr & WIN_ATTR_TRANSPARENT ) == 0 )
				mask_to_wall_vect( &pWin->gb.self_mask, &pWall->mask );
		}
	}	

	return( 0 );
}

// 비트 마스크를 할당한다.
int internal_alloc_bit_mask( MaskStt *pMask, RectStt *pRect )
{
	// 마스크에 소요될 바이트 수를 구한다.  8로 나누면 된다.
	pMask->dwLineBytes = (DWORD)( pRect->nH + 7 ) & (DWORD)0xFFFFFFF8;
	pMask->dwLineBytes = pMask->dwLineBytes >> 3;	

	// 마스크 바이트를 할당한다.
	pMask->dwSize = pMask->dwLineBytes * pRect->nV;
	pMask->pB = (BYTE*)kmalloc( pMask->dwSize ); 
	if( pMask->pB == NULL )
		return( -1 );
	
	// 비트 마스크를 1로 초기화 한다.
	memset(  pMask->pB, 0xFF, pMask->dwSize );
	
	// WinStt와 r 구조체를 공유한다.
	pMask->pR = pRect;  

	return( 0 );
}

// 윈도우의 비트 마스크를 할당한다.
int alloc_bit_mask( WinStt *pWin )
{
	int nR;
	nR = internal_alloc_bit_mask( &pWin->mask, &pWin->obj.r );
	return( nR );
}

// 윈도우 비트 마스크를 해제한다.
int free_bit_mask( WinStt *pWin )
{
	if( pWin == NULL )
		return( -1 );

	if( pWin->mask.pB != NULL )
	{
		kfree( pWin->mask.pB );
		pWin->mask.pB = NULL;
	}						 

	return( 0 );
}

// 윈도우 구조체를 할당하고 초기화 한다.
static WinStt *alloc_win_stt()
{
	WinStt *pWin;

	pWin = (WinStt*)kmalloc( sizeof( WinStt ) );
	if( pWin == NULL )
		return( NULL );		// 메모리를 할당할 수 없다. 
	memset( pWin, 0, sizeof( WinStt ) );
	
	// OBJ를 초기화 한다.
	init_gui_obj( NULL, &pWin->obj, GUI_OTYPE_WINDOW, wmfunc_common );

	return( pWin );
}

// 윈도우를 생성한다. 
WinStt *kcreate_window( WinThreadStt *pWThread, WinStt *pParentWin, DWORD dwPredefStyleID, 
/**/					RectStt *pRect, WMFuncStt *pWMArray, DWORD dwState, 
						DWORD dwWinAttr, DWORD dwMainIconID, DWORD dwCreateMesgParamA, DWORD dwCreateMesgParamB )
{
	WinStyleStt		*pWS;
	WinStt			*pWin;
	GuiStt			*pGui;

	if( pWThread == NULL )
		pWThread = get_kwin_thread();
	
	// predefined window style을 찾는다. 
	pWS = find_predef_wstyle( dwPredefStyleID );
	if( pWS == NULL )
	{
		kdbg_printf( "Predefined window style %d not found!\n", dwPredefStyleID );
		return( NULL );		// 찾을 수 없다. 
	}
	
	// 윈도우 구조체를 할당한다. 
	pWin = alloc_win_stt();
	if( pWin == NULL )
		return( NULL );		// 할당할 수 없다.

	// Parent Window를 설정한다.
	if( pParentWin != NULL )
		pWin->pParentWin = pParentWin;
	else
	{
		pGui = get_gui_stt();
		pWin->pParentWin = &pGui->wall;
	}

	// MainIconID를 설정한다.
	if( dwMainIconID == 0 )
		pWin->dwMainIconID = IDI_ABOUT_ICON;
	else
		pWin->dwMainIconID = dwMainIconID;

	// ID, Style, 좌표를 설정한다. 	
	pWin->dwID		= kalloc_win_id();
	pWin->pWStyle	= pWS;
	if( pRect != NULL )
		memcpy( &pWin->obj.r, pRect, sizeof( RectStt ) );

	// CLIENT 좌표를 설정한다.  (pWS가 pWin에 설정된 후에 호출되어야 한다.)
	recalc_client_area( pWin );
	
	pWin->pWMArray = pWMArray;
	
	// 비트 마스크를 할당한다.
	alloc_bit_mask( pWin );

	// 그래픽 버퍼를 할당한다.
	alloc_gra_buff( pWin );

	// 윈도우 속성, 상태를 저장한다. (GB를 할당한 다음 초기화해야 한다.)
	pWin->gb.dwAttr  = dwWinAttr;
	pWin->gb.dwState = dwState;

	// 윈도우 쓰레드에 추가한다. 
	append_win_to_thread( pWThread, pWin );

	// 스크린의 레벨 링크에 추가한다.
	append_win_to_scr( pWin );

	// 바로 CREATE 메시지 핸들러를 호출한다. (POST로 날리면 안된다.)
	//call_win_message_handler( pWin, WMESG_CREATE, dwCreateMesgParamA, dwCreateMesgParamB );
	kpost_message( pWin, WMESG_CREATE, dwCreateMesgParamA, dwCreateMesgParamB );
	
	// WMESG_PAINT 메시지를 보낸다. 
	kpost_message( pWin, WMESG_PAINT, 0, 0 );

	return( pWin );
}

DWORD call_pre_style_func( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB )
{
	DWORD 			dwR;
	WinStyleStt		*pWS;
	GUIOBJ_FUNC 	pFunc;
	
	pWS = pWin->pWStyle;
	if( pWS == NULL )
		return( 0 );
	
	pFunc = find_pre_guiobj_func( pWS->pWMArray, dwMesgID );
	if( pFunc == NULL )
		return( 0 );
	dwR = pFunc( (GuiObjStt*)pWin, dwMesgID, dwParamA, dwParamB );

	return( dwR );
}

DWORD call_post_style_func( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB )
{
	DWORD			dwR;
	WinStyleStt 	*pWS;
	GUIOBJ_FUNC 	pFunc;
	
	pWS = pWin->pWStyle;
	if( pWS == NULL )
		return( 0 );
	
	pFunc = find_post_guiobj_func( pWS->pWMArray, dwMesgID );
	if( pFunc == NULL )
		return( 0 );
	dwR = pFunc( (GuiObjStt*)pWin, dwMesgID, dwParamA, dwParamB );

	return( dwR );
}

DWORD call_pre_window_func( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB )
{
	DWORD			dwR;
	GUIOBJ_FUNC 	pObjFunc;

	// 윈도우의 PRE 메시지 핸들러가 있으면 호출한다.  (모든 윈도우 공통)
	pObjFunc = find_pre_guiobj_func( wmfunc_common, dwMesgID );
	if( pObjFunc == NULL )
		return( 0 );

	dwR = pObjFunc( &pWin->obj, dwMesgID, dwParamA, dwParamB );
	return( dwR );
}
 
DWORD call_post_window_func( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB )
{
	DWORD			dwR;
	GUIOBJ_FUNC 	pObjFunc;

	// 윈도우의 POST 메시지 핸들러가 있으면 호출한다.  (모든 윈도우 공통)
	pObjFunc = find_post_guiobj_func( wmfunc_common, dwMesgID );
	if( pObjFunc == NULL )
		return( 0 );

	dwR = pObjFunc( &pWin->obj, dwMesgID, dwParamA, dwParamB );
	return( dwR );
}

// 스타일 윈도우와 실제 윈도우의 메시지 핸들러를 호출한다.
int kforward_message( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB )
{
	DWORD			dwR;

	dwR = call_pre_style_func( pWin, dwMesgID, dwParamA, dwParamB );
	if( dwR == WMHRV_ABORT )
		return( 0 );

	dwR = call_message_func( pWin, dwMesgID, dwParamA, dwParamB );
	if( dwR == WMHRV_ABORT )
		return( 0 );
	
	dwR = call_post_style_func( pWin, dwMesgID, dwParamA, dwParamB );
		
	return( 0 );
}

// 해당 윈도우의 메시지 핸들러를 찾아 직접 수행한다. 
int call_win_message_handler( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB )
{
	DWORD			dwR;

	// 윈도우의 PRE 메시지 핸들러가 있으면 호출한다.  (모든 윈도우 공통)
	dwR = call_pre_window_func( pWin, dwMesgID, dwParamA, dwParamB );
	if( dwR == WMHRV_ABORT )
		return( 0 );

	// 스타일 윈도우와 실제 윈도우의 메시지 핸들러를 호출한다.
	dwR = (DWORD)kforward_message( pWin, dwMesgID, dwParamA, dwParamB );
	if( dwR == WMHRV_ABORT )
		return( 0 );

	// 윈도우의 POST 메시지 핸들러가 있으면 호출한다.
	dwR = call_post_window_func( pWin, dwMesgID, dwParamA, dwParamB );
	if( dwR == WMHRV_ABORT )
		return( 0 );

	return( (int)dwR );
}

int ksend_message( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB )
{
	int nR;
	//nR = call_win_message_handler( pWin, dwMesgID, dwParamA, dwParamB );
	nR = kpost_message( pWin, dwMesgID, dwParamA, dwParamB );
	return( nR );
}

// 해당 윈도우로 메시지를 포스팅 한다. 
int kpost_message( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB )
{
	int			nR;
	WMesgQStt	*pWMQ;

	if( pWin->pWThread == NULL || pWin->pWThread->pE == NULL )
	{	// WinThread가 연결되어 있지 않으면 그냥 폐기한다.
		//kdbg_printf( "kpost_message(%d): No WinThread\n", dwMesgID );
		return( -1 );
	}

	// 메시지를 넣을 공간?
	pWMQ = &pWin->wmq;
	if( pWMQ->nTotal >= MAX_WMESG_IN_Q )
	{
		//kdbg_printf( "post message : message q is full!\n" );
		return( -1 );		// 더이상 큐에 집어넣을 수가 없다. 
	}

	// 메시지 ID와 파러메터 2개.
	_asm {
		PUSHFD
		CLI
	}
	pWMQ->q[ pWMQ->nEnd ].dwID     = dwMesgID;
	pWMQ->q[ pWMQ->nEnd ].dwParamA = dwParamA;
	pWMQ->q[ pWMQ->nEnd ].dwParamB = dwParamB;

	pWMQ->nTotal++;
	pWMQ->nEnd++;
	if( pWMQ->nEnd >= MAX_WMESG_IN_Q )
		pWMQ->nEnd = 0;

	// 이벤트를 보낸다. 
	if( pWin != NULL && pWin->pWThread != NULL )
		kresume_thread( pWin->pWThread->pThread );
	else
	{
		kdbg_printf( "kpost_message: critical error!\n" );
	}

	_asm POPFD

	nR = chk_mesgmon( pWin->dwID, dwMesgID );
	if( nR > 0 )
		kdbg_printf( "[MESGMON] WinID=%d %s(%d) Counter=%d\n", pWin->dwID, 
					get_wmesg_str(dwMesgID), dwMesgID, nR );
		
	return( 0 );
}

// 윈도우 타이틀을 설정한다. 
int set_window_title( WinStt *pWin, char *pTitle )
{
	strcpy( pWin->szTitle, pTitle );
	return( 0 );
}

WinStt *find_window_by_pos( int nX, int nY )
{
	WinStt	*pWin;
	GuiStt	*pGui;

	pGui = get_gui_stt();

	for( pWin = pGui->pStartLevelWin; pWin != NULL; pWin = pWin->pNextLevel )
	{	
		// minimize된 것은 스킵한다.
		if( pWin->gb.dwState & WIN_STATE_MINIMIZED || pWin->gb.dwState & WIN_STATE_HIDDEN )
			continue;		
		
		// XY 좌표가 pWin에 포함되는지 검사한다. 
		if( in_window_area( pWin, nX, nY ) )
		{
			return( pWin );
		}	
	}		

	return( NULL );
}

WinStt *find_window_by_id( DWORD dwID )
{
	WinStt	*pWin;
	GuiStt	*pGui;

	pGui = get_gui_stt();

	for( pWin = pGui->pStartLevelWin; pWin != NULL; pWin = pWin->pNextLevel )
	{	
		if( pWin->dwID == dwID )
			return( pWin );
	}		

	return( NULL );
}

// XY가 Win내에 포함되는지 확인.
int in_window_area( WinStt *pWin, int nX, int nY )
{
	int nR;

	nR = is_in_rect( &pWin->obj.r, nX, nY );

	return( nR );
}

// 내부 좌표로 변경한다. 
void inner_pos( RectStt *pR, int nX, int nY, int *pNewX, int *pNewY )
{
	pNewX[0] = nX - pR->nX;
	pNewY[0] = nY - pR->nY;
}

RectStt *get_client_rect( WinStt *pWin, RectStt *pR )
{
	if( pWin == NULL )
	{
		kdbg_printf( "get_client_rect: pWin = NULL!\n" );
		return( NULL );
	}

	if( pR == NULL )
		pR = &pWin->client_r;
	else
		memcpy( pR, &pWin->client_r, sizeof( RectStt ) );	

	return( pR );
}

// 클라이언트 영역을 다시 계산한다.
int recalc_client_area( WinStt *pWin )
{
	WinFrameStt		*pFrm;

	if( pWin == NULL || pWin->pWStyle == NULL )
	{
		kdbg_printf( "recalc_client_area: pWin = NULL or pWin->pWStyle = NULL\n" );
		return( -1 );
	}

	pFrm = &pWin->pWStyle->frame;
	pWin->ct_r.nX = pWin->obj.r.nX + pFrm->nFrameWidth;
	pWin->ct_r.nY = pWin->obj.r.nY + pFrm->nFrameWidth;
	pWin->ct_r.nH = pWin->obj.r.nH - (pFrm->nFrameWidth * 2);
	pWin->ct_r.nV = pWin->obj.r.nV - (pFrm->nFrameWidth * 2);
	
		// 타이틀 바가 있는 경우.
	if( pFrm->nTitleV > 0 )
	{
		pWin->ct_r.nY += pFrm->nFrameWidth + pFrm->nTitleV;		
		pWin->ct_r.nV -= pFrm->nFrameWidth + pFrm->nTitleV;
	}

	////////////  New Client Area  //////////////////////////
	pWin->client_r.nX = pFrm->nFrameWidth;
	pWin->client_r.nY = pFrm->nFrameWidth;
	pWin->client_r.nH = pWin->obj.r.nH - (pFrm->nFrameWidth * 2);
	pWin->client_r.nV = pWin->obj.r.nV - (pFrm->nFrameWidth * 2);
	
		// 타이틀 바가 있는 경우.
	if( pFrm->nTitleV > 0 )
	{
		pWin->client_r.nY += pFrm->nFrameWidth + pFrm->nTitleV; 	
		pWin->client_r.nV -= pFrm->nFrameWidth + pFrm->nTitleV;
	}

	return( 0 );
}

// Client 내의 좌표를 윈도우 내의 좌표로 변환한다.
int client_to_win_pos( WinStt *pWin, int *pX, int *pY )
{
	pX[0] += pWin->ct_r.nX - pWin->obj.r.nX;
	pY[0] += pWin->ct_r.nY - pWin->obj.r.nY;

	return( 0 );
}	

// RECT 좌표를 Client Rect에서 Screen Rect 기준으로 변환한다.
// pBaseWin 기준의 pR을 Screen 기준으로 변환하여 pResult에 저장한다.
int client_to_screen( RectStt *pResult, RectStt *pBaseWin, RectStt *pR )
{
	pResult->nX = pR-> nX + pBaseWin->nX;
	pResult->nY = pR-> nY + pBaseWin->nY;
	
	if( pR->nH <= pBaseWin->nH - pR->nX )
		pResult->nH = pR->nH;
	else 
		pResult->nH = pBaseWin->nH - pR->nX;
	if( pR->nV <= pBaseWin->nV - pR->nY )
		pResult->nV = pR->nV;
	else
		pResult->nV = pBaseWin->nV - pR->nY ;
	return( 0 );
}

// RECT 좌표를 Screen Rect에서 Client Rect 기준으로 변환한다.
// Screen 기준의 pR을 pBaseWin 기중으로 변환한다.
int screen_to_win( RectStt *pResult, RectStt *pBaseWin, RectStt *pR )
{
	pResult->nX = pR->nX - pBaseWin->nX;
	pResult->nY = pR->nY - pBaseWin->nY;
	pResult->nH = pR->nH;
	pResult->nV = pR->nV;
	return( 0 );
}

/***********************************************************************************
// GraBuff의 내용을 스크린에 그린다.  (pR은 pGB 영역 내의 좌표)
// 느린 C 버전
int flush_gra_buff_c( GraBuffStt *pGB, RectStt *pR, MaskStt *pMask )
{
	RectStt		r, rt;
	int			nX, nY;
	UINT16		*pW, *pPixel;

	// 마우스 커서를 지운다.
	draw_mouse_pointer( 0 );

	if( pR == NULL )
	{
		rt.nH = pGB->pR->nH;
		rt.nV = pGB->pR->nV;
		rt.nX = 0;
		rt.nY = 0;
		pR = &rt;
	}

	// pGR->pR 내의 좌표인 pR을 Sscreen 좌표로 변환한다.
	client_to_screen( &r, pGB->pR, pR );

	pPixel = &pGB->pW[ pR->nX + ( pR->nY * pGB->pR->nH ) ];
	pW = get_video_mem_addr16( r.nX, r.nY	);
						 	
	for( nY = 0; nY < r.nV; nY++ )
	{
		for( nX = 0; nX < r.nH; nX++ )
		{
			if( get_mask_bit( pMask, pR->nX + nX, pR->nY + nY ) != 0 && 
				get_mask_bit( &pGB->self_mask, pR->nX + nX, pR->nY + nY ) != 0 )
			{
				pW[nX] = pPixel[nX];
			}
		}
		
		pW = get_next_screen_line( pW );
		pPixel = &pPixel[ pGB->pR->nH ];
	}

	// 마우스 커서를 그린다.
	draw_mouse_pointer( 1 );

	return( 0 );
}

// flush_gra_buff()를 어셈블리 가속한 것.
int flush_gra_buff( GraBuffStt *pGB, RectStt *pR, MaskStt *pMask )
{
	GuiStt		*pGui;
	RectStt		r, rt;
	UINT16		*pW, *pPixel;
	int			nH, nV, nIndex, nMod;
	BYTE		byBit, *pMaskBuff, *pSelfMaskBuff;
	DWORD		dwScreenAdder, dwPixelAdder, dwMaskAdder, dwSelfMaskAdder;

	// 마우스 커서를 지운다.
	draw_mouse_pointer( 0 );

	if( pR == NULL )
	{
		rt.nH = pGB->pR->nH;
		rt.nV = pGB->pR->nV;
		rt.nX = 0;
		rt.nY = 0;
		pR = &rt;
	}

	// pGR->pR 내의 좌표인 pR을 Sscreen 좌표로 변환한다.
	client_to_screen( &r, pGB->pR, pR );

	pPixel = &pGB->pW[ pR->nX + ( pR->nY * pGB->pR->nH ) ];
	pW = get_video_mem_addr16( r.nX, r.nY	);
	
	// 다음 라인을 처리할 때 더해 주어야 하는 값.
	pGui = get_gui_stt();
	dwScreenAdder   = pGui->vmode.LinBytesPerScanLine;
	dwPixelAdder    = pGB->pR->nH * 2;
					
	// 시작 비트 마스크를 구한다.
	nMod = pR->nX % 8;
	byBit = 0x80;
	if( nMod > 0 )
		byBit = byBit >> nMod;

	// 마스크의 버퍼
	if( pMask != NULL && pMask->pB != NULL )
	{
		dwMaskAdder = pMask->dwLineBytes;
		nIndex      = ( pR->nY * dwMaskAdder ) + ( pR->nX / 8 );
		pMaskBuff   = &pMask->pB[ nIndex ];
	}
	else
		pMaskBuff = NULL;
		
	// 셀프 마스크의 버퍼와 시작 바이트 값을 구한다.
	if( pGB->self_mask.pB != NULL )
	{
		dwSelfMaskAdder = pGB->self_mask.dwLineBytes;
		nIndex          = ( pR->nY * dwSelfMaskAdder ) + ( pR->nX / 8 );
		pSelfMaskBuff   = &pGB->self_mask.pB[ nIndex ];
	}
	else
		pSelfMaskBuff = NULL;
		
	nH = r.nH;
	nV = r.nV;	
	_asm {
		PUSHFD
		PUSH  ESI
		PUSH  EDI
		
		CLD
		MOV   EBX, pMaskBuff
		MOV   EDX, pSelfMaskBuff
		MOV   ESI, pPixel
		MOV   EDI, pW
		
		MOV   ECX, nV

NEXT_LINE:// 다음 라인을 처리한다.		
		PUSH  EBX
		PUSH  ECX
		PUSH  EDX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   AL,  byBit
		MOV   ECX, nH

NEXT_PIXEL:		
		// 마스크 비트를 검사한다.
		CMP   EBX, 0
		JZ    CHK_SMASK
		      TEST [EBX], AL
		      JZ   PLUS_SIDI
		
CHK_SMASK:
		//  셀프 마스크 비트를 검사한다.
		CMP   EDX, 0
		JZ    MOVE_PIXEL
		      TEST [EDX], AL
		      JNZ  MOVE_PIXEL
PLUS_SIDI:
		// 픽셀을 복사히지 않고 스킵한다.
		ADD  ESI, 2
		ADD  EDI, 2
		JMP  ROL_MASK			  

MOVE_PIXEL:
		// 픽셀을 복사한다.
		MOVSW
			
ROL_MASK: // 마스크 바이트를 다음으로 옮겨야 하는가?
        SHR   AL, 1
        CMP   AL, 0
        JNZ   LOOP1
              MOV  AL, 0x80
              CMP  EBX, 0      
              JZ   PLUS_SMB
				   INC EBX		// MASK BUFF의 인덱스를 증가시킨다.
PLUS_SMB:     CMP  EDX, 0
			  JZ   LOOP1        
        		   INC EDX		// SELF MASK BUFF의 인덱스를 증가시킨다.

LOOP1:  LOOP NEXT_PIXEL      
		
		POP   EDI
		POP   ESI
		POP   EDX
		POP   ECX
		POP   EBX
		
		CMP   EBX, 0
		JZ    AEX_MMM
			  ADD   EBX, dwMaskAdder
AEX_MMM:
		CMP   EDX, 0
		JZ    AEX_PXX
	          ADD   EDX, dwSelfMaskAdder
AEX_PXX:
	    ADD   ESI, dwPixelAdder
		ADD   EDI, dwScreenAdder
		LOOP  NEXT_LINE
		
		POP   EDI
		POP   ESI		
		POPFD
	}

	// 마우스 커서를 그린다.
	draw_mouse_pointer( 1 );

	return( 0 );
}

****************************************************************************************/
void flushing_normal(  UINT16 *pSrcBuff, UINT16 *pDestBuff, BYTE *pMaskBuff, BYTE *pSelfMaskBuff, DWORD dwMaskAdder, 
		DWORD dwSelfMaskAdder, DWORD dwDestLineAdder, DWORD dwSrcLineAdder, int nH, int nV, BYTE byBit )
{
	int nBody, nTail;

	_asm {
		PUSHFD
		PUSH  ESI
		PUSH  EDI
		CLD
	}

	if( pMaskBuff == NULL && pSelfMaskBuff == NULL )
	{	// 마스크가 하나도 없는 경우.=============================================
		
		get_align_count( (DWORD)pDestBuff, nH*2, 4, &nBody, &nTail );
		
		_asm {
		MOV   ESI, pSrcBuff
		MOV   EDI, pDestBuff

		MOV   ECX, nV

NEXT_LINE_A:
		// 다음 라인을 처리한다.		
		PUSH  ECX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   ECX, nBody
		CMP   ECX, 0
		JZ    X_TAIL
			  REP   MOVSD		// 4바이트씩 옮긴다.
		
X_TAIL:	MOV   ECX, nTail
		SHR   ECX, 1			// 나머지는 항상 2의 배수일 것이다.
		CMP   ECX, 0 
		JZ    X_NEXT
			  REP MOVSW			// 2바이트씩 옮긴다.

X_NEXT:	POP   EDI
		POP   ESI
		POP   ECX
		
	    ADD   ESI, dwSrcLineAdder
		ADD   EDI, dwDestLineAdder
		LOOP  NEXT_LINE_A
		}
	}
	else if( pMaskBuff != NULL && pSelfMaskBuff != NULL )
	{	// 마스크가 둘 다 있는 경우.=============================================							 
		_asm {
		MOV   EBX, pMaskBuff
		MOV   EDX, pSelfMaskBuff
		MOV   ESI, pSrcBuff
		MOV   EDI, pDestBuff
		
		MOV   ECX, nV

NEXT_LINE_B:// 다음 라인을 처리한다.		
		PUSH  EBX
		PUSH  ECX
		PUSH  EDX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   AL,  byBit
		MOV   ECX, nH

NEXT_PIXEL_B:		
		// 마스크와 셀프 마스크를 검사한다.
		TEST [EBX], AL
		JZ   PLUS_SIDI_B
		TEST [EDX], AL
		JNZ  MOVE_PIXEL_B

PLUS_SIDI_B:
		// 픽셀을 복사히지 않고 스킵한다.
		ADD  ESI, 2
		ADD  EDI, 2
		JMP  ROL_MASK_B			  

MOVE_PIXEL_B:
		// 픽셀을 복사한다.
		MOVSW
			
ROL_MASK_B: 
		// 마스크 바이트를 다음으로 옮겨야 하는가?
        SHR   AL, 1
        CMP   AL, 0
        JNZ   LOOP1_B
              MOV  AL, 0x80
			  INC EBX		// MASK BUFF의 인덱스를 증가시킨다.
        	  INC EDX		// SELF MASK BUFF의 인덱스를 증가시킨다.

LOOP1_B:
		LOOP NEXT_PIXEL_B      
		
		POP   EDI
		POP   ESI
		POP   EDX
		POP   ECX
		POP   EBX
		
	    ADD   EBX, dwMaskAdder
        ADD   EDX, dwSelfMaskAdder
	    ADD   ESI, dwSrcLineAdder
		ADD   EDI, dwDestLineAdder

		LOOP  NEXT_LINE_B
		}
	}
	else if( pMaskBuff != NULL )
	{	// 마스크가 하나만 있는 경우.=============================================							 
ONE_MASK:
		_asm {
		MOV   EBX, pMaskBuff
		MOV   ESI, pSrcBuff
		MOV   EDI, pDestBuff
		
		MOV   ECX, nV

NEXT_LINE_C:// 다음 라인을 처리한다.		
		PUSH  EBX
		PUSH  ECX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   AL,  byBit
		MOV   ECX, nH

NEXT_PIXEL_C:		
		// 마스크와 셀프 마스크를 검사한다.
		TEST [EBX], AL
		JNZ  MOVE_PIXEL_C

		// 픽셀을 복사히지 않고 스킵한다.
		ADD  ESI, 2
		ADD  EDI, 2
		JMP  ROL_MASK_C			  

MOVE_PIXEL_C:
		// 픽셀을 복사한다.
		MOVSW
			
ROL_MASK_C: 
		// 마스크 바이트를 다음으로 옮겨야 하는가?
        SHR   AL, 1
        CMP   AL, 0
        JNZ   LOOP1_C
              MOV  AL, 0x80
			  INC EBX		// MASK BUFF의 인덱스를 증가시킨다.
LOOP1_C:
		LOOP NEXT_PIXEL_C      
		
		POP   EDI
		POP   ESI
		POP   ECX
		POP   EBX
		
	    ADD   EBX, dwMaskAdder
	    ADD   ESI, dwSrcLineAdder
		ADD   EDI, dwDestLineAdder

		LOOP  NEXT_LINE_C
		}
	}
	else
	{	// Self Mask 하나만 있는 경우.
		pMaskBuff   = pSelfMaskBuff;
		dwMaskAdder = dwSelfMaskAdder;
		goto ONE_MASK;
	}
	
	_asm{
		POP   EDI
		POP   ESI		
		POPFD
	}
}

void flushing_transparent(  UINT16 *pPixel, UINT16 *pW, BYTE *pMaskBuff, BYTE *pSelfMaskBuff, DWORD dwMaskAdder, 
		DWORD dwSelfMaskAdder, DWORD dwScreenAdder, DWORD dwPixelAdder, int nH, int nV, BYTE byBit )
{
	int nBody, nTail;

	/////////////////////////////////////////////////////
	////  윈도우의 내용을 비디오 메모리에 복사한다.  ////
	/////////////////////////////////////////////////////
	_asm {
		PUSHFD
		PUSH  ESI
		PUSH  EDI
		CLD
	}

	if( pMaskBuff == NULL && pSelfMaskBuff == NULL )
	{	// 마스크가 하나도 없는 경우.=============================================
		
		get_align_count( (DWORD)pW, nH*2, 4, &nBody, &nTail );
		
		_asm {
		MOV   ESI, pPixel
		MOV   EDI, pW

		MOV   ECX, nV

NEXT_LINE_A:
		// 다음 라인을 처리한다.		
		PUSH  ECX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   ECX, nBody
		CMP   ECX, 0
		JZ    X_TAIL
			  //REP   MOVSD		// 4바이트씩 옮긴다.
		
	    // 반투명 효과
		TRANSP_A:
			  LODSD
			  SHR  EAX, 1
			  AND  EAX,	0x7BEF7BEF
			  MOV  EBX, [EDI]
			  SHR  EBX, 1
			  AND  EBX,	0x7BEF7BEF
			  ADD  EAX, EBX
			  STOSD
			  LOOP TRANSP_A
		 

X_TAIL:	MOV   ECX, nTail
		SHR   ECX, 1			// 나머지는 항상 2의 배수일 것이다.
		CMP   ECX, 0 
		JZ    X_NEXT
			  //REP MOVSW			// 2바이트씩 옮긴다.

		TRANSP_B:
			  LODSW
			  SHR  AX, 1
			  AND  AX,	0x7BEF
			  MOV  BX, [EDI]
			  SHR  BX, 1
			  AND  BX,	0x7BEF
			  ADD  AX, BX
			  STOSW
			  LOOP TRANSP_B

X_NEXT:	POP   EDI
		POP   ESI
		POP   ECX
		
	    ADD   ESI, dwPixelAdder
		ADD   EDI, dwScreenAdder
		LOOP  NEXT_LINE_A
		}
	}
	else if( pMaskBuff != NULL && pSelfMaskBuff != NULL )
	{	// 마스크가 둘 다 있는 경우.=============================================							 
		_asm {
		MOV   EBX, pMaskBuff
		MOV   EDX, pSelfMaskBuff
		MOV   ESI, pPixel
		MOV   EDI, pW
		
		MOV   ECX, nV

NEXT_LINE_B:// 다음 라인을 처리한다.		
		PUSH  EBX
		PUSH  ECX
		PUSH  EDX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   AL,  byBit
		MOV   ECX, nH

NEXT_PIXEL_B:		
		// 마스크와 셀프 마스크를 검사한다.
		TEST [EBX], AL
		JZ   PLUS_SIDI_B
		TEST [EDX], AL
		JNZ  MOVE_PIXEL_B

PLUS_SIDI_B:
		// 픽셀을 복사히지 않고 스킵한다.
		ADD  ESI, 2
		ADD  EDI, 2
		JMP  ROL_MASK_B			  

MOVE_PIXEL_B:
		// 픽셀을 복사한다.
		//MOVSW

		PUSH EAX
		PUSH EBX
		LODSW
		SHR  AX, 1
		AND  AX, 0x7BEF
		MOV  BX, [EDI]
		SHR  BX, 1
		AND  BX, 0x7BEF
		ADD  AX, BX
		STOSW
		POP  EBX
		POP  EAX
			
ROL_MASK_B: 
		// 마스크 바이트를 다음으로 옮겨야 하는가?
        SHR   AL, 1
        CMP   AL, 0
        JNZ   LOOP1_B
              MOV  AL, 0x80
			  INC EBX		// MASK BUFF의 인덱스를 증가시킨다.
        	  INC EDX		// SELF MASK BUFF의 인덱스를 증가시킨다.

LOOP1_B:
		LOOP NEXT_PIXEL_B      
		
		POP   EDI
		POP   ESI
		POP   EDX
		POP   ECX
		POP   EBX
		
	    ADD   EBX, dwMaskAdder
        ADD   EDX, dwSelfMaskAdder
	    ADD   ESI, dwPixelAdder
		ADD   EDI, dwScreenAdder

		LOOP  NEXT_LINE_B
		}
	}
	else if( pMaskBuff != NULL )
	{	// 마스크가 하나만 있는 경우.=============================================							 
ONE_MASK:
		_asm {
		MOV   EBX, pMaskBuff
		MOV   ESI, pPixel
		MOV   EDI, pW
		
		MOV   ECX, nV

NEXT_LINE_C:// 다음 라인을 처리한다.		
		PUSH  EBX
		PUSH  ECX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   AL,  byBit
		MOV   ECX, nH

NEXT_PIXEL_C:		
		// 마스크와 셀프 마스크를 검사한다.
		TEST [EBX], AL
		JNZ  MOVE_PIXEL_C

		// 픽셀을 복사히지 않고 스킵한다.
		ADD  ESI, 2
		ADD  EDI, 2
		JMP  ROL_MASK_C			  

MOVE_PIXEL_C:
		// 픽셀을 복사한다.
		//MOVSW

		PUSH EAX
		PUSH EBX
		LODSW
		SHR  AX, 1
		AND  AX, 0x7BEF
		MOV  BX, [EDI]
		SHR  BX, 1
		AND  BX, 0x7BEF
		ADD  AX, BX
		STOSW
		POP  EBX
		POP  EAX
			
ROL_MASK_C: 
		// 마스크 바이트를 다음으로 옮겨야 하는가?
        SHR   AL, 1
        CMP   AL, 0
        JNZ   LOOP1_C
              MOV  AL, 0x80
			  INC EBX		// MASK BUFF의 인덱스를 증가시킨다.
LOOP1_C:
		LOOP NEXT_PIXEL_C      
		
		POP   EDI
		POP   ESI
		POP   ECX
		POP   EBX
		
	    ADD   EBX, dwMaskAdder
	    ADD   ESI, dwPixelAdder
		ADD   EDI, dwScreenAdder

		LOOP  NEXT_LINE_C
		}
	}
	else
	{	// Self Mask 하나만 있는 경우.
		pMaskBuff   = pSelfMaskBuff;
		dwMaskAdder = dwSelfMaskAdder;
		goto ONE_MASK;
	}
	
	_asm{
		POP   EDI
		POP   ESI		
		POPFD
	}
}

// flush_gra_buff()를 어셈블리 2차 가속한 것.
// 마스크가 없는 경우에 대한 3차 가속 ( MOVSW -> MOVSD + MOVSB )
// 마스크 전체가 0, 1인 것을 구분하여 WHITE_MASK, BLACK 마스크로 구분하여 처리.
/*
int org_flush_gra_buff( WinStt *pWin, RectStt *pR )
{
	GraBuffStt *pGB;
	GuiStt		*pGui;
	RectStt		r, rt;
	MaskStt		*pMask;
	UINT16		*pW, *pPixel;
	int 		nH, nV, nIndex, nMod;
	BYTE		byBit, *pMaskBuff, *pSelfMaskBuff;
	DWORD		dwScreenAdder, dwPixelAdder, dwMaskAdder, dwSelfMaskAdder;

	pGB   = &pWin->gb;
	pMask = &pWin->mask;

	// 완전히 가려진 녀석은 그릴 필요가 없다.
	if( pWin->mask.byFlag == MASK_FLAG_BLACK )
		return( 0 );	
	
	if( pR == NULL )
	{
		rt.nH = pGB->pR->nH;
		rt.nV = pGB->pR->nV;
		rt.nX = 0;
		rt.nY = 0;
		pR = &rt;
	}

	// 마우스 커서를 지운다.
	draw_mouse_pointer( 0 );

	// pGB->pR 내의 좌표인 pR을 Sscreen 좌표로 변환한다.
	client_to_screen( &r, pGB->pR, pR );

	pPixel = get_gra_buff_addr16( pGB, pR->nX, pR->nY );
	pW = get_video_mem_addr16( r.nX, r.nY	);
	
	// 다음 라인을 처리할 때 더해 주어야 하는 값.
	pGui = get_gui_stt();
	dwScreenAdder   = pGui->vmode.LinBytesPerScanLine;
	dwPixelAdder    = pGB->pR->nH * 2;
					
	// 시작 비트 마스크를 구한다.
	nMod = pR->nX % 8;
	byBit = 0x80;
	if( nMod > 0 )
		byBit = byBit >> nMod;

	// 마스크의 버퍼
	if( pMask != NULL && pMask->pB != NULL )
	{
		dwMaskAdder = pMask->dwLineBytes;
		nIndex      = ( pR->nY * dwMaskAdder ) + ( pR->nX / 8 );
		pMaskBuff   = &pMask->pB[ nIndex ];
	}
	else
		pMaskBuff = NULL;
		
	// 셀프 마스크의 버퍼와 시작 바이트 값을 구한다.
	if( pGB->self_mask.pB != NULL )
	{
		dwSelfMaskAdder = pGB->self_mask.dwLineBytes;
		nIndex          = ( pR->nY * dwSelfMaskAdder ) + ( pR->nX / 8 );
		pSelfMaskBuff   = &pGB->self_mask.pB[ nIndex ];
	}
	else
		pSelfMaskBuff = NULL;
		
	nH = r.nH;
	nV = r.nV;	

	// Top Level Window이면 Mask 없이 그냥 그린다.
	if( pWin->mask.byFlag == MASK_FLAG_WHITE )
		pMaskBuff = NULL;		// 가려진 부분이 없으므로 그냥 디립따 그리면 된다.
	else if( pWin == pGui->pStartLevelWin )
		pMaskBuff = NULL;

	// 실제로 그리는 함수를 호출한다.
	if( pWin->gb.dwAttr & WIN_ATTR_TRANSPARENT )
	{
		flushing_transparent( pPixel, pW, pMaskBuff, pSelfMaskBuff, dwMaskAdder, 
			dwSelfMaskAdder, dwScreenAdder, dwPixelAdder, nH, nV, byBit );
	}
	else
	{
		flushing_normal( pPixel, pW, pMaskBuff, pSelfMaskBuff, dwMaskAdder, 
			dwSelfMaskAdder, dwScreenAdder, dwPixelAdder, nH, nV, byBit );
	}

	// 마우스 커서를 그린다.
	draw_mouse_pointer( 1 );

	return( 0 );
}
*/

// pR Rect가 스크린 해상도를 벗어나는 경우 자른다.
static int clip_rect_by_screen( RectStt *pResult, RectStt *pR )
{
	GuiStt	*pGui;

	pGui = get_gui_stt();

	memcpy( pResult, pR, sizeof( RectStt ) );

	if( pResult->nX < 0 )
	{
		pResult->nH += pResult->nX;
		if( pResult->nH <= 0 )
			return( -1 );		// 그릴 수 없다.
		pResult->nX = 0;
	}
	else if( pResult->nX+pResult->nH >= (int)pGui->vmode.wX )
	{
		pResult->nH = (int)pGui->vmode.wX - pResult->nX;
		if( pResult->nH <= 0 )
			return( -1 );
	}

	if( pResult->nY < 0 )
	{
		pResult->nV += pResult->nY;
		if( pResult->nV <= 0 )
			return( -1 );
		pResult->nY = 0;
	}
	else if( pResult->nY+pResult->nV >= (int)pGui->vmode.wY )
	{
		pResult->nV = (int)pGui->vmode.wY - pResult->nY;
		if( pResult->nV <= 0 )
			return( -1 );
	}
	
	return( 0 );
}


int flush_gra_buff( WinStt *pWin, RectStt *pClientR )
{
	GraBuffStt *pGB;
	GuiStt		*pGui;
	RectStt 	r_clipped_win_base, r_client, r_scr_base;
	MaskStt 	*pMask;
	UINT16		*pW, *pPixel;
	int 		nR, nH, nV, nIndex, nMod, nPrevMoveRectFlag;
	BYTE		byBit, *pMaskBuff, *pSelfMaskBuff;
	DWORD		dwScreenAdder, dwPixelAdder, dwMaskAdder, dwSelfMaskAdder;

	pGB   = &pWin->gb;
	pMask = &pWin->mask;

	// 완전히 가려진 녀석은 그릴 필요가 없다.
	if( pWin->mask.byFlag == MASK_FLAG_BLACK )
		return( 0 );	
	
	if( pClientR == NULL )
	{
		r_client.nH = pGB->pR->nH;
		r_client.nV = pGB->pR->nV;
		r_client.nX = 0;
		r_client.nY = 0;
	}
	else
		memcpy( &r_client, pClientR, sizeof( RectStt ) );

	pClientR = &r_client;

	// Client Rect인 pR을 Window Rect로 변경한다.
	client_to_screen( &r_scr_base, pGB->pR, pClientR );
	
	nR = clip_rect_by_screen( &r_clipped_win_base, &r_scr_base );
	if( nR < 0 )
		return( 0 );	// 그릴 필요가 없다.

	screen_to_win( pClientR, pGB->pR, &r_clipped_win_base );

	// 마우스 커서를 지운다.
	draw_mouse_pointer( 0 );

	pPixel = get_gra_buff_addr16( pGB, pClientR->nX, pClientR->nY );
	pW = get_video_mem_addr16( r_clipped_win_base.nX, r_clipped_win_base.nY	);
	
	// 다음 라인을 처리할 때 더해 주어야 하는 값.
	pGui = get_gui_stt();
	dwScreenAdder	= pGui->vmode.LinBytesPerScanLine;
	dwPixelAdder	= pGB->pR->nH * 2;
					
	// 시작 비트 마스크를 구한다.
	nMod = pClientR->nX % 8;
	byBit = 0x80;
	if( nMod > 0 )
		byBit = byBit >> nMod;

	// 마스크의 버퍼
	if( pMask != NULL && pMask->pB != NULL )
	{
		dwMaskAdder = pMask->dwLineBytes;
		nIndex		= ( pClientR->nY * dwMaskAdder ) + ( pClientR->nX / 8 );
		pMaskBuff	= &pMask->pB[ nIndex ];
	}
	else
		pMaskBuff = NULL;
		
	// 셀프 마스크의 버퍼와 시작 바이트 값을 구한다.
	if( pGB->self_mask.pB != NULL )
	{
		dwSelfMaskAdder = pGB->self_mask.dwLineBytes;
		nIndex			= ( pClientR->nY * dwSelfMaskAdder ) + ( pClientR->nX / 8 );
		pSelfMaskBuff	= &pGB->self_mask.pB[ nIndex ];
	}
	else
		pSelfMaskBuff = NULL;
		
	nH = r_clipped_win_base.nH;
	nV = r_clipped_win_base.nV;	

	// Top Level Window이면 Mask 없이 그냥 그린다.
	if( pWin->mask.byFlag == MASK_FLAG_WHITE )
		pMaskBuff = NULL;		// 가려진 부분이 없으므로 그냥 디립따 그리면 된다.
	else if( pWin == pGui->pStartLevelWin )
		pMaskBuff = NULL;

	// move rect가 on 상태이면 잠시 off 해 둔다.(2004-03-25)
	nPrevMoveRectFlag = 0;
	if( pGui->nShowMoveFlag != 0 )
	{
		RectStt r;
 		nR = get_overlapped_rect( &r, &pGui->mr, &r_clipped_win_base );
		if( nR != 0 )
		{
			nPrevMoveRectFlag = 1;
			show_move_rect( 0 );
		}
	}

	// 실제로 그리는 함수를 호출한다.
	if( pWin->gb.dwAttr & WIN_ATTR_TRANSPARENT )
	{
		flushing_transparent( pPixel, pW, pMaskBuff, pSelfMaskBuff, dwMaskAdder, 
			dwSelfMaskAdder, dwScreenAdder, dwPixelAdder, nH, nV, byBit );
	}
	else
	{
		flushing_normal( pPixel, pW, pMaskBuff, pSelfMaskBuff, dwMaskAdder, 
			dwSelfMaskAdder, dwScreenAdder, dwPixelAdder, nH, nV, byBit );
	}

	// move rect를 다시 그린다. (2004-03-25)
	if( nPrevMoveRectFlag != 0 )
		show_move_rect( 1 );

	// 마우스 커서를 그린다.
	draw_mouse_pointer( 1 );

	return( 0 );
}

int copy_gra_buff( GraBuffStt *pDestGB, GraBuffStt *pSrcGB, int nDestX, int nDestY )
{
	UINT16		*pDestBuff, *pSrcBuff;
	DWORD		dwSrcLineAddr, dwDestLineAddr;

	pDestBuff = get_gra_buff_addr16( pDestGB, nDestX, nDestY );
	pSrcBuff  = get_gra_buff_addr16( pSrcGB,  0, 0 );
	
	// 다음 라인을 처리할 때 더해 주어야 하는 값.
	dwSrcLineAddr  = pSrcGB->pR->nH  * 2;
	dwDestLineAddr = pDestGB->pR->nH * 2;
					
	flushing_normal( pSrcBuff, pDestBuff, NULL, NULL, 0, 0, dwDestLineAddr, dwSrcLineAddr, pSrcGB->pR->nH, pSrcGB->pR->nV, 0 );

	return( 0 );
}

int copy_gra_buff_ex( GraBuffStt *pDestGB, int nDestX, int nDestY, GraBuffStt *pSrcGB, RectStt *pSrcR )
{
	UINT16		*pDestBuff, *pSrcBuff;
	DWORD		dwSrcLineAddr, dwDestLineAddr;
	
	pDestBuff = get_gra_buff_addr16( pDestGB, nDestX, nDestY );
	pSrcBuff  = get_gra_buff_addr16( pSrcGB,  pSrcR->nX, pSrcR->nY );
	
	// 다음 라인을 처리할 때 더해 주어야 하는 값.
	dwSrcLineAddr  = pSrcGB->pR->nH  * 2;
	dwDestLineAddr = pDestGB->pR->nH * 2;
					
	flushing_normal( pSrcBuff, pDestBuff, NULL, NULL, 0, 0, dwDestLineAddr, dwSrcLineAddr, pSrcR->nH, pSrcR->nV, 0 );

	return( 0 );
}


// 해당 윈도우를 레벨 링크의 가장 위쪽으로 옮긴다.
int set_top_window( WinStt *pWin )
{
	// 윈도우를 레벨 링크에서 제거한 후 추가하면 Top Level에 추가된다.
	delete_win_from_scr( pWin );
	append_win_to_scr( pWin );

	// 각 윈도우들의 마스크 비트를 다시 계산한다.
	recalc_bit_mask();

	// TRANSPARENT이면 하위 윈도우들을 다시 그린다.
	if( pWin->gb.dwAttr & WIN_ATTR_TRANSPARENT )
		repaint_down_layer( pWin, &pWin->obj.r ); 
	
	// 윈도우를 다시 그린다.
	call_win_message_handler( pWin, WMESG_PAINT, 0, 0 );

	return( 0 );
}

int is_top_window( WinStt *pWin )
{
	GuiStt *pGui;

	pGui = get_gui_stt();
	if( pGui->pStartLevelWin == pWin )
		return( 1 );
	
	return( 0 );
}

// pWin의 grabuff를 nScrollVPixel만큼 Scroll한 후 wClearColor로 지운다.
int scroll_gb( WinStt *pWin, RectStt *pR, int nScrollVPixel, RectStt *pClearR, UINT16 wClearColor )
{
	GraBuffStt	*pGB;
	int			nV, nBody, nTail;
	DWORD		dwDest, dwSrc, dwLineBytes;

	pGB = &pWin->gb;
	dwDest = (DWORD)get_gra_buff_addr16( pGB, pR->nX, pR->nY );
	dwSrc  = (DWORD)get_gra_buff_addr16( pGB, pR->nX, pR->nY + nScrollVPixel );
	nV     = pR->nV - nScrollVPixel;
	dwLineBytes = pGB->pR->nH * 2;  // GraBugg 자체의 H Line 크기이어야 한다.  (pR->nH가 아니다.)
	get_align_count( dwDest, pR->nH * 2, 4, &nBody, &nTail );
		
	_asm {
		PUSHFD
		CLD
		PUSH  ESI
		PUSH  EDI

		MOV   ESI, dwSrc
		MOV   EDI, dwDest

		MOV   ECX, nV

NEXT_LINE_A:
		// 다음 라인을 처리한다.		
		PUSH  ECX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   ECX, nBody
		CMP   ECX, 0
		JZ    X_TAIL
			  REP   MOVSD		// 4바이트씩 옮긴다.
		
X_TAIL:	MOV   ECX, nTail
		SHR   ECX, 1			// 나머지는 항상 2의 배수일 것이다.
		CMP   ECX, 0 
		JZ    X_NEXT
			  REP MOVSW			// 2바이트씩 옮긴다.

X_NEXT:	POP   EDI
		POP   ESI
		POP   ECX
		
	    ADD   ESI, dwLineBytes
		ADD   EDI, dwLineBytes
		LOOP  NEXT_LINE_A

		POP   EDI
		POP   ESI		
		POPFD
	}

	// 스크롤되고 남은 영역을 지운다.
	k_fill_rect( pGB, pClearR, wClearColor );

	return( 0 );
}

// 윈도우 pWin을 pParent위에 추가한다.
int insert_win_to_scr( WinStt *pParent, WinStt *pWin )
{
	GuiStt	*pGui;
	
	pGui = get_gui_stt();

	if( pParent->pPreLevel == NULL )
	{
		if( pGui->pStartLevelWin != NULL )
			return( -1 );  // 에러처리.

		pGui->pStartLevelWin = pWin;
		pWin->pPreLevel = NULL;
	}
	else
	{
		pParent->pPreLevel->pNextLevel = pWin;
		pWin->pPreLevel = pParent->pPreLevel;
	}

	pWin->pNextLevel   = pParent;
	pParent->pPreLevel = pWin;

	return( 0 );
}

// pre_common_minimize에서 호출된다.
static int minimize_window( WinStt *pWin )
{
	pWin->gb.dwState |= WIN_STATE_MINIMIZED;

	kdbg_printf( "Minimize %s Window\n", pWin->szTitle );

	return( 0 );
}

int is_win_show( WinStt *pWin )
{
	if( pWin == NULL )
		return( 0 );

	if( pWin->gb.dwState & WIN_STATE_HIDDEN )
		return( 0 );

	return( 1 );
}
