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

// Window�� Control�� �ڵ鸵 �Լ�.
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

// Predefined window style ����ü�� �ʱ�ȭ�� �� SIMPLE Ÿ���� �߰��Ѵ�. 
int init_predef_winstyle()
{
	SysWinStyleStt	*pSWS;

	pSWS = &sys_wstyle;
	memset( pSWS, 0, sizeof( SysWinStyleStt ) );

	// WSTYLE_SIMPLE�� �ʱ�ȭ�� �� �����͸� ��´�. 
	pSWS->ptr[pSWS->nTotal++] = init_simple_win();
	pSWS->ptr[pSWS->nTotal++] = init_flat_win();
	pSWS->ptr[pSWS->nTotal++] = init_framew_win();

	return( 0 );
}

// ������ ID�� �Ҵ��Ѵ�.
static DWORD kalloc_win_id()
{
	return( G_dwKWinID++ );
}

// ������ ��ü�� ������ �޽��� �ڵ鷯 �Լ�.
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
	
	// ����� ��Ʈ�ѵ��� ������� ���� ����̸� �Ǵ��Ѵ�.
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

	// �������ΰ�?
	if( pOwner->wType == GUI_OTYPE_WINDOW )
	{
		ProcessStt *pP;
		
		pWin = (WinStt*)pOwner;
		if( pWin->pWThread == NULL || pWin->pWThread->pThread == NULL )
			goto GO_CAT;
		// ���μ����� FG�� �����Ѵ�.	
		pP = pWin->pWThread->pThread->pProcess;
		set_fg_process( pP->pVConsole, pP );
		// VConsole�� Active�� �����Ѵ�.
		set_active_vconsole( pP->pVConsole );
	}
	
GO_CAT:
	pObj = find_cat_obj( pOwner, dwWinX, dwWinY );
	if(	pObj == NULL )
		return( WMHRV_CONTINUE );

	// ����̸� ���� ������ �� ��ǥ�� �����Ͽ� ȣ���Ѵ�.
	pOwner->pCat = pObj;

	// ���� ��ǥ�� ������ �� �����Ѵ�.
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

	// ����̸� ���� ������ �� ��ǥ�� �����Ͽ� ȣ���Ѵ�.
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

	// ����̸� ���� ������ �� ��ǥ�� �����Ͽ� ȣ���Ѵ�.
	pOwner->pCat = pObj;
	inner_pos( &pObj->r, dwWinX, dwWinY, &nObjX, &nObjY );
	nR = call_pre_guiobj_handler( pObj, dwWMesgID, nObjX, nObjY );
	
	return( nR );
}

static DWORD wmh_pre_common_mouse_move_out( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	int			nR;
	GuiObjStt	*pOldCat;

	// ���� ������� mouse_move_out �Լ��� ȣ���Ѵ�.
	if( pOwner->pCat == NULL )
		return( WMHRV_CONTINUE );

	pOldCat = pOwner->pCat;
	pOwner->pCat = NULL;
	nR = call_pre_guiobj_handler( pOldCat, dwWMesgID, 0, 0 );

	return( nR );
}

// dwWinX, dwWinY�� ������ ���� ��ǥ.
static DWORD wmh_pre_common_mouse_move( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwWinX, DWORD dwWinY )
{
	int			nR;
	GuiObjStt	*pObj;
	int			nObjX, nObjY;
	
	nR = 0;

	//kdbg_printf( "X=%d, Y=%d\n", dwWinX, dwWinY );

	pObj = find_cat_obj( pOwner, dwWinX, dwWinY );
	if(	pOwner->pCat == pObj )
	{	// ���� ����̿� ������ �׳� mouse_move �Լ��� ȣ���Ѵ�.
		// ��ǥ�� �����Ͽ� ȣ���Ѵ�.
		inner_pos( &pObj->r, dwWinX, dwWinY, &nObjX, &nObjY );
		nR = call_pre_guiobj_handler( pObj, WMESG_MOUSE_MOVE, nObjX, nObjY );
		return( WMHRV_CONTINUE );
	}

	// ���� ������� mouse_move_out �Լ��� ȣ���Ѵ�.
	if( pOwner->pCat != NULL )
	{	
		GuiObjStt	*pOldCat = pOwner->pCat;
		pOwner->pCat = NULL;	// ��� NULL�� ����� �ش�.
		nR = call_pre_guiobj_handler( pOldCat, WMESG_MOUSE_MOVE_OUT, 0, 0 );
	}

	// ����̸� ���� �����Ѵ�.
	pOwner->pCat = pObj;
	if( pObj != NULL )
	{	// ��ǥ�� �����Ͽ� ȣ���Ѵ�.
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

	// Transparent Window�� ���� ó���� �� �ش�.
	// ���� ��������� ���� �׷��� �Ѵ�.
	// ������ ��ǥ�� ���� ��ǥ�� �����Ѵ�.
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
	
	// Minimize ��Ų��.
	minimize_window( pWin );

	// �� ��������� ����ũ ��Ʈ�� �ٽ� ����Ѵ�.
	recalc_bit_mask();

	// �����찡 �ִ� �ڸ��� �����. 
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
		return( 0 );	// �̹� HIDE�� �����Ǿ� ����.
	}
	else if( ( pWin->gb.dwState & WIN_STATE_HIDDEN ) == 0  && dwShowFlag != 0 )
	{
		kdbg_printf( "Already Visible window\n" );
		return( 0 );	// �̹� SHOW�� �����Ǿ� ����.
	}

	if( dwShowFlag != 0 )
	{	// SHOW�� �����Ѵ�.
		pWin->gb.dwState &= (DWORD)~(DWORD)WIN_STATE_HIDDEN;

		// �� ��������� ����ũ ��Ʈ�� �ٽ� ����Ѵ�.
		recalc_bit_mask();

		// �����츦 �ٽ� �׸���.
		call_win_message_handler( pWin, WMESG_PAINT, 0, 0 );
	}
	else
	{	// HIDE�� �����Ѵ�. 
		pWin->gb.dwState |= (DWORD)WIN_STATE_HIDDEN;

		// �� ��������� ����ũ ��Ʈ�� �ٽ� ����Ѵ�.
		recalc_bit_mask();

		// �����찡 �ִ� �ڸ��� �����. 
		repaint_down_layer( pWin, &pWin->obj.r ); 
		repaint_upper_transparent( pWin, &pWin->obj.r ); 
	}

	return( 0 );
}

// �����츦 ���ο� ��ǥ�� �̵���Ų��.
static DWORD wmh_pre_common_win_move( GuiObjStt *pGuiObj, DWORD dwWMesgID, DWORD dwX, DWORD dwY )
{
	RectStt	old_r;
	WinStt *pWin;

	pWin = (WinStt*)pGuiObj;

	// �̹� ������ ��ġ�� �־����� Ȯ��.
	if( pWin->obj.r.nX == (int)dwX && pWin->obj.r.nY == (int)dwY )
		return( 0 );

	memcpy( &old_r, &pWin->obj.r, sizeof( RectStt ) );
	
	// ���ο� ��ǥ�� �����Ѵ�.
	pWin->obj.r.nX = (int)dwX;
	pWin->obj.r.nY = (int)dwY;

	// Ÿ��Ʋ�� �������� ������ Ŭ���̾�Ʈ ������ �ٽ� ����Ѵ�. 
	recalc_client_area( pWin );

	// �� ��������� ����ũ ��Ʈ�� �ٽ� ����Ѵ�.
	recalc_bit_mask();

	// ������ �����찡 �ִ� �ڸ��� �����. 
	repaint_down_layer( pWin, &old_r ); 
	repaint_upper_transparent( pWin, &old_r );

	// ��ü ������ �ڵ鷯�� ȣ���Ѵ�. 
	call_message_func( pWin, dwWMesgID, 0, 0 );

	// �����츦 �ٽ� �׸���.
	call_win_message_handler( pWin, WMESG_PAINT, 0, 0 );

	return( 0 );
}

static DWORD wmh_post_common_paint( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwXY, DWORD dwHV )
{
	WinStt	*pWin;
	RectStt win_r, r, *pR, winrect;

	pWin = (WinStt*)pOwner;
	
	if( dwXY == 0 && dwHV == 0 )
	{	// ����� ��� ���� ���׿���.  
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

	// ���� �������� ���ʿ� TRANSPARENT �����찡 ������ �ٽ� �׷��ش�.
	client_to_screen( &win_r, &pOwner->r, pR );
	repaint_upper_transparent( pWin, &win_r );
	
	return( WMHRV_CONTINUE );
}

static DWORD wmh_post_common_destroy( GuiObjStt *pOwner, DWORD dwWMesgID, DWORD dwWinRect, DWORD dwParamB )
{
	// �����츦 �ݴ´�.
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

	// 2004-01-24  ������ ������ ũ��� ��ǥ�� �Էµ��� �ʴ� ��쿡�� post create���� bit_mask�� �ٽ�
	// ó���ؾ� �Ѵ�.
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
		return( -1 );		// ���̻� �޽����� ����.
	
	// �޽��� �ϳ��� ������. 
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

// ���� �����쿡 ���� �޽����� ó���Ѵ�. 
static int dispatch_winmesg( WinStt *pWin )
{
	WMesgStt		wm;
	WMesgQStt		*pWMQ;	
	int 			nR, nTotal;
	
	pWMQ = &pWin->wmq;

	for( nTotal = 0;; )
	{	// ���� �޽����� �ϳ� ���´�.
		nR = pop_win_mesg( pWMQ, &wm );
		if( nR < 0 )
			break;			// �޽����� ���̻� ������ ���ư���.

		// �޽��� �ڵ鷯�� ã�� ȣ���Ѵ�.
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

// ������鿡 ���� �޽����� ó���Ѵ�.
static int winmesg_handling( WinThreadStt *pWThread )
{
	int			nTotal, nR;
	WinStt		*pWin, *pNext;

	nTotal = 0;
	// WinThread�� �����ϴ� ������鿡 ���� ������ �˻縦 �����Ѵ�.
	for( pWin = pWThread->pStart; pWin != NULL; pWin = pNext )
	{
		pNext = pWin->pNext;
		nR = dispatch_winmesg( pWin );
		if( nR > 0 )
			nTotal += nR;
	}

	return( nTotal );
}

// GUI �ý����� Ŀ�� ������ �޽��� �ڵ鷯.
static int kwin_mesg_thread( void *pParam )
{
	int				nTotal;
	WinThreadStt	*pWThread;

	pWThread = (WinThreadStt*)pParam;

	// �޽����� ���� ���� Ȯ���ϰ� SUSPEND�� ���� ������ �޽����� ���� ������
	// �׳� SUSPEND�� �� ������ �������� �������� ������.
	for( ;; )
	{	
		// �޽����� ���� �����찡 ������ ó���Ѵ�. 
		nTotal = winmesg_handling( pWThread );

		// ó���� �޽����� ������ Suspend�� ����.
		if( nTotal == 0 )
			ksuspend_thread( pWThread->pThread );
	}	

	return( 0 );
}

static char *pKWinEventName = "KWIN_MESG";
int kclose_win_thread( WinThreadStt *pWThread )
{
	// �̺�Ʈ�� �����Ѵ�.
	if( pWThread->pE != NULL )
		close_event( pWThread->pE );
	
	// �޽��� �����带 �����Ѵ�.
	// Event Wait ������ �����带 ������ �׿� ������. (�����ϵ��� �ϴ� ���� �� ������.)
	// ������ �������� ��쿡�� �����Ѵ�.
	if( pWThread->byThreadCreated != 0 && pWThread->pThread != NULL )
		kill_thread( pWThread->pThread );
			
	// WinThread�� �����Ѵ�.
	kfree( pWThread );
	
	return( 0 );
}

// ������ �����带 �����Ѵ�.
WinThreadStt *kcreate_win_thread( ThreadStt *pT )
{	
	WinThreadStt *pWThread;

	pWThread = (WinThreadStt*)kmalloc( sizeof( WinThreadStt ) );
	if( pWThread == NULL )
		return( NULL );

	memset( pWThread, 0, sizeof( WinThreadStt ) );

	// �̺�Ʈ�� �����Ѵ�.  ( close_event()�� call�� ��� �Ѵ�. )
	pWThread->pE = create_event( pKWinEventName );
	if( pWThread->pE == NULL )
	{	
		kdbg_printf( "create_event( KWIN_MESG ) failed!" );
		// �Ҵ��ߴ� ����ü�� �����Ѵ�.
		kfree( pWThread );
		return( NULL );
	}

	// ������ �޽����� ó���� �����带 �����Ѵ�. 
	// Owner Process, StackSize, Entry Function, Parameter
	if( pT == NULL )
	{
		pWThread->pThread = kcreate_thread( k_get_current_process(), 0, (DWORD)kwin_mesg_thread, (DWORD)pWThread, TS_READY_NORMAL );	
		if( pWThread->pThread == NULL )
		{	// ������ �Ҵ���� �̺�Ʈ ����ü�� �����Ѵ�.
			close_event( pWThread->pE );
			kfree( pWThread );
			return( NULL );		// �����带 ������ �� ����. 
		}
		pWThread->byThreadCreated = 1;		// �����尡 ���� ������.

		// ALIAS�� �����Ѵ�. 
		k_set_thread_alias( pWThread->pThread, "kwin_thread" );
		
		// ������ �����带 �ٷ� �����Ѵ�. 
		kernel_thread_switching( pWThread->pThread );
	}
	else
	{			  
		pWThread->pThread = pT;
	}	
	
	return( pWThread );
}

// Predefined Window Style�� ã�´�. 
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
	// ã�� �� ����. 
	return( NULL );
}

// �����츦 �����忡 �����Ѵ�. 
int append_win_to_thread( WinThreadStt *pWThread, WinStt *pWin )
{
	if( pWThread->nTotal == 0 )
	{	// ó�� �߰��Ǵ� ��. 
		pWThread->pStart = pWThread->pEnd = pWin;
		pWin->pPrev = pWin->pNext = NULL;
	}
	else
	{	// ť�� ���� �ڿ� �߰��Ѵ�. 
		pWThread->pEnd->pNext = pWin;
		pWin->pPrev		 = pWThread->pEnd;
		pWin->pNext		 = NULL;
		pWThread->pEnd		 = pWin;
	}
	pWThread->nTotal++;
	
	pWin->pWThread = pWThread;

	return( 0 );
}

// ������ ť���� �����Ѵ�.
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

	// 0���� �켱 �ʱ�ȭ �Ѵ�.
	memset( pGB, 0, sizeof( GraBuffStt ) );

	nSize = pR->nH * 2 * pR->nV;
	pGB->pW = (UINT16*)kmalloc( nSize );
	if( pGB->pW == NULL )
		return( -1 );

	pGB->pR = pR;

	return( 0 );
}

// �׷��� ���۸� �Ҵ��Ѵ�.
int alloc_gra_buff( WinStt *pWin )
{
	int			nR;

	nR = alloc_gra_buff_ex( &pWin->gb, &pWin->obj.r );

	return( nR );
}

// �׷��� ���۸� �Ҵ��Ѵ�.
int free_gra_buff_ex( GraBuffStt *pGB )
{
	// self mask�� �����Ѵ�.
	if( pGB->self_mask.pB != NULL )
	{
		kfree( pGB->self_mask.pB );
		pGB->self_mask.pB = NULL;
	}	

	// GraBuff�� ���۸� �����Ѵ�.
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


// ������ ����ü�� �����Ѵ�.
void free_win_Stt( WinStt *pWin )
{
	kfree( pWin );
}

// �����츦 �ݴ´�.
int kclose_window( WinStt *pWin )
{	
	RectStt		r;
	WinStt		*pUpperWin, *pLowerWin;

	memcpy( &r, &pWin->obj.r, sizeof( RectStt ) );
	pUpperWin = pWin->pPreLevel;
	pLowerWin = pWin->pNextLevel;

	// Focus�� ��� �־��ٸ� Invalidate ��Ų��.
	invalidate_mouse_owner( pWin );

	// �����츦 ��ũ������ �����Ѵ�.
	delete_win_from_scr( pWin );

	// �����츦 �����忡�� �����Ѵ�.
	delete_win_from_thread( pWin );	

	// ��Ʈ ����ũ�� �����Ѵ�.
	free_bit_mask( pWin );

	// �׷��� ���۸� �����Ѵ�.
	free_gra_buff( pWin );

	//������ ����ü�� �����Ѵ�.
	free_win_Stt( pWin );

	// �� ��������� ����ũ ��Ʈ�� �ٽ� ����Ѵ�.
	recalc_bit_mask();

	// ������ �����찡 �ִ� �ڸ��� �����. 
	repaint_down_layer( pUpperWin, &r ); 		  // pUpperWin, pLowerWin �� �� NULL
	repaint_upper_transparent( pLowerWin, &r );	  // �̾ �������.

	return( 0 );	
}

// �����츦 ��ũ���� ���� ��ũ�� �߰��Ѵ�.
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
	{	// TOP Level�� �߰��Ѵ�.
		pGui->pStartLevelWin->pPreLevel = pWin;
		pWin->pNextLevel				= pGui->pStartLevelWin;
		pWin->pPreLevel					= NULL;
		pGui->pStartLevelWin			= pWin;
	}		

	return( 0 );
}	

// �����츦 ��ũ���� ���� ��ũ���� �����Ѵ�.
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
 *  ���� ������ ��� pWall�� 0���� ������ ���� 0���� �����.
 *  CurWinBit = CurWinBit & pWall
**/
static int mask_from_wall_vect( MaskStt *pMask, MaskStt *pWall )
{
	int		nDiv, nMod, nH, nV;
	DWORD	dwM_LinesByte, dwW_LinesByte;
	BYTE	*pWallBuff, *pMaskBuff, byBit;

	if( pMask->pB == NULL || pWall->pB == NULL )
		return( -1 );

	// pMask->pR�� ����ũ�� ������ �ִ� �������� ���� ��ǥ.
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

			 // WALL�� �ش� ��Ʈ�� 0�ΰ�?
H_BITS:		 MOV DH, DL				  
			 AND DH, [EDI]
			 JNZ SHR_MASK_BIT		  
				 // WALL�� ��Ʈ�� 0�� ��쿡�� MASK�� ��Ʈ�� CLEAR�Ѵ�.
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

// pWall�� ��Ʈ�� ��� ���� �������� self_mask ��� 1�� ���� 0���� �����.
static int mask_to_wall_vect( MaskStt *pMask, MaskStt *pWall )
{
	int		nDiv, nMod, nH, nV;
	DWORD	dwM_LinesByte, dwW_LinesByte;
	BYTE	*pWallBuff, *pMaskBuff, byBit;

	if( pMask->pB == NULL || pWall->pB == NULL )
	{	// ����ũ ���۰� NULL�̴�.
		kdbg_printf( "mask_to_wall_vect() - NULL mask buffer!\n" );
		return( -1 );
	}

	// pMask->pR�� ����ũ�� ������ �ִ� �������� ���� ��ǥ.
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

		// MASK ��Ʈ�� '1'�̸� WALL �� ��Ʈ�� 0���� �����.
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

// ũ��� ������ ������ mask���� AND�� �׳� byte AND�ص� �ȴ�.
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

// ����ũ�� ��� ��Ʈ�� 0���� �Ǵ� 1���� �˻��Ѵ�.
static int check_mask_bits( MaskStt *pMask )
{
	BYTE	byModByte, byModMask;
	DWORD	dwCheck, dwLineBytes, dwMaskBuff;
	int		nBody, nTail, nH, nMod, nV, nR;

	if( pMask == NULL || pMask->pR == NULL || pMask->pB == NULL || pMask->dwSize == 0 )
		return( -10 );

	// Black���� �� ������ White�� �� ������ �����Ѵ�.
	if( pMask->pB[0] == 0 )
		dwCheck = 0;
	else if( (BYTE)pMask->pB[0] == (BYTE)0xFF )
		dwCheck =  0xFFFFffff;
	else
	{	// BLACK �Ǵ� WHITE ��� �ƴϴ�.
		pMask->byFlag = 0;		
		return( -1 );
	}

	// �� ������ align count�� ���Ѵ�.
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
			  AND   BL, byModMask	// �˻��� �ʿ䰡 ���� ��Ʈ���� '0'�� �ȴ�.
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

// �ý����� ��� �����쿡 ���� ��Ʈ ����ũ�� �����Ѵ�.
int recalc_bit_mask()
{
	int		nR;
	GuiStt	*pGui;
	WinStt	*pWin, *pWall;

	pGui = get_gui_stt();

	//kdbg_printf( "recalc_bit_mask()\n" );

	// ��� ȭ���� ��Ʈ ����ũ�� ��� 1�� �����Ѵ�.
	pWall = &pGui->wall; 
	memset( pWall->mask.pB, 0xFF, pWall->mask.dwSize );

	for( pWin = pGui->pStartLevelWin; pWin!= NULL; pWin = pWin->pNextLevel )
	{	
		if( pWin->gb.dwState & WIN_STATE_MINIMIZED || pWin->gb.dwState & WIN_STATE_HIDDEN )
		{	// minimize�Ǿ� ������ ����ũ�� ��� 0���� �����ϰ� ���� ������� �Ѿ��.
			//memset( pWin->mask.pB, 0x00, pWin->mask.dwSize );
			pWin->mask.byFlag = MASK_FLAG_BLACK;
			continue;
		}

		// �ʱ�ȭ���� ���� ������� ����ũ�� ������� �ʴ´�.
		if( ( pWin->gb.dwState & WIN_STATE_INITIALIZED ) == 0 )
			continue;
		
		// ���� �������� ��� ��Ʈ�� 1�� �����Ѵ�.
		memset( pWin->mask.pB, 0xFF, pWin->mask.dwSize );

		// pWall�� ��Ʈ ��� 0���� ������ ���� ������ 0���� �����.
		// �̹� ���� ������ �ٸ� �����쿡 ���� ������ �κ�.
		mask_from_wall_vect( &pWin->mask, &pWall->mask );

		pWin->mask.byFlag = 0;
		// pWall�� ��Ʈ�� ��� ���� ������ ������ �ش�Ǵ� �͵��� 0���� �����.
		if( pWin->gb.self_mask.pB == NULL )
		{
			if( ( pWin->gb.dwAttr & WIN_ATTR_TRANSPARENT ) == 0 )
				modify_bit_vect( &pWall->mask, &pWin->obj.r, 0 );
			
			// ����ũ�� BLACK���� WHITE���� �˻��� ����. (���� ����ũ�� �ִ� ��쿡�� �˻��� �ʿ䰡 ����.)
			nR = check_mask_bits( &pWin->mask );
			//kdbg_printf( "CHECK_MASK(%s) : %d\n", pWin->szTitle, nR );
		}
		else
		{	// self mask�� ������ �ڽ��� ������ mask�� and �Ѵ�.
			// bitmask�� ������ �����Ƿ� �׳� byte and �ص� �ȴ�.
			and_mask_byte_array( &pWin->mask, &pWin->gb.self_mask );

			// pWall�� ��Ʈ�� ��� ���� �������� self mask ��� 1�� ���� 0���� �����.
			// self_mask�� ���� ������ modify_bit_vect ��� ȣ��ȴ�. 
			if( ( pWin->gb.dwAttr & WIN_ATTR_TRANSPARENT ) == 0 )
				mask_to_wall_vect( &pWin->gb.self_mask, &pWall->mask );
		}
	}	

	return( 0 );
}

// ��Ʈ ����ũ�� �Ҵ��Ѵ�.
int internal_alloc_bit_mask( MaskStt *pMask, RectStt *pRect )
{
	// ����ũ�� �ҿ�� ����Ʈ ���� ���Ѵ�.  8�� ������ �ȴ�.
	pMask->dwLineBytes = (DWORD)( pRect->nH + 7 ) & (DWORD)0xFFFFFFF8;
	pMask->dwLineBytes = pMask->dwLineBytes >> 3;	

	// ����ũ ����Ʈ�� �Ҵ��Ѵ�.
	pMask->dwSize = pMask->dwLineBytes * pRect->nV;
	pMask->pB = (BYTE*)kmalloc( pMask->dwSize ); 
	if( pMask->pB == NULL )
		return( -1 );
	
	// ��Ʈ ����ũ�� 1�� �ʱ�ȭ �Ѵ�.
	memset(  pMask->pB, 0xFF, pMask->dwSize );
	
	// WinStt�� r ����ü�� �����Ѵ�.
	pMask->pR = pRect;  

	return( 0 );
}

// �������� ��Ʈ ����ũ�� �Ҵ��Ѵ�.
int alloc_bit_mask( WinStt *pWin )
{
	int nR;
	nR = internal_alloc_bit_mask( &pWin->mask, &pWin->obj.r );
	return( nR );
}

// ������ ��Ʈ ����ũ�� �����Ѵ�.
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

// ������ ����ü�� �Ҵ��ϰ� �ʱ�ȭ �Ѵ�.
static WinStt *alloc_win_stt()
{
	WinStt *pWin;

	pWin = (WinStt*)kmalloc( sizeof( WinStt ) );
	if( pWin == NULL )
		return( NULL );		// �޸𸮸� �Ҵ��� �� ����. 
	memset( pWin, 0, sizeof( WinStt ) );
	
	// OBJ�� �ʱ�ȭ �Ѵ�.
	init_gui_obj( NULL, &pWin->obj, GUI_OTYPE_WINDOW, wmfunc_common );

	return( pWin );
}

// �����츦 �����Ѵ�. 
WinStt *kcreate_window( WinThreadStt *pWThread, WinStt *pParentWin, DWORD dwPredefStyleID, 
/**/					RectStt *pRect, WMFuncStt *pWMArray, DWORD dwState, 
						DWORD dwWinAttr, DWORD dwMainIconID, DWORD dwCreateMesgParamA, DWORD dwCreateMesgParamB )
{
	WinStyleStt		*pWS;
	WinStt			*pWin;
	GuiStt			*pGui;

	if( pWThread == NULL )
		pWThread = get_kwin_thread();
	
	// predefined window style�� ã�´�. 
	pWS = find_predef_wstyle( dwPredefStyleID );
	if( pWS == NULL )
	{
		kdbg_printf( "Predefined window style %d not found!\n", dwPredefStyleID );
		return( NULL );		// ã�� �� ����. 
	}
	
	// ������ ����ü�� �Ҵ��Ѵ�. 
	pWin = alloc_win_stt();
	if( pWin == NULL )
		return( NULL );		// �Ҵ��� �� ����.

	// Parent Window�� �����Ѵ�.
	if( pParentWin != NULL )
		pWin->pParentWin = pParentWin;
	else
	{
		pGui = get_gui_stt();
		pWin->pParentWin = &pGui->wall;
	}

	// MainIconID�� �����Ѵ�.
	if( dwMainIconID == 0 )
		pWin->dwMainIconID = IDI_ABOUT_ICON;
	else
		pWin->dwMainIconID = dwMainIconID;

	// ID, Style, ��ǥ�� �����Ѵ�. 	
	pWin->dwID		= kalloc_win_id();
	pWin->pWStyle	= pWS;
	if( pRect != NULL )
		memcpy( &pWin->obj.r, pRect, sizeof( RectStt ) );

	// CLIENT ��ǥ�� �����Ѵ�.  (pWS�� pWin�� ������ �Ŀ� ȣ��Ǿ�� �Ѵ�.)
	recalc_client_area( pWin );
	
	pWin->pWMArray = pWMArray;
	
	// ��Ʈ ����ũ�� �Ҵ��Ѵ�.
	alloc_bit_mask( pWin );

	// �׷��� ���۸� �Ҵ��Ѵ�.
	alloc_gra_buff( pWin );

	// ������ �Ӽ�, ���¸� �����Ѵ�. (GB�� �Ҵ��� ���� �ʱ�ȭ�ؾ� �Ѵ�.)
	pWin->gb.dwAttr  = dwWinAttr;
	pWin->gb.dwState = dwState;

	// ������ �����忡 �߰��Ѵ�. 
	append_win_to_thread( pWThread, pWin );

	// ��ũ���� ���� ��ũ�� �߰��Ѵ�.
	append_win_to_scr( pWin );

	// �ٷ� CREATE �޽��� �ڵ鷯�� ȣ���Ѵ�. (POST�� ������ �ȵȴ�.)
	//call_win_message_handler( pWin, WMESG_CREATE, dwCreateMesgParamA, dwCreateMesgParamB );
	kpost_message( pWin, WMESG_CREATE, dwCreateMesgParamA, dwCreateMesgParamB );
	
	// WMESG_PAINT �޽����� ������. 
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

	// �������� PRE �޽��� �ڵ鷯�� ������ ȣ���Ѵ�.  (��� ������ ����)
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

	// �������� POST �޽��� �ڵ鷯�� ������ ȣ���Ѵ�.  (��� ������ ����)
	pObjFunc = find_post_guiobj_func( wmfunc_common, dwMesgID );
	if( pObjFunc == NULL )
		return( 0 );

	dwR = pObjFunc( &pWin->obj, dwMesgID, dwParamA, dwParamB );
	return( dwR );
}

// ��Ÿ�� ������� ���� �������� �޽��� �ڵ鷯�� ȣ���Ѵ�.
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

// �ش� �������� �޽��� �ڵ鷯�� ã�� ���� �����Ѵ�. 
int call_win_message_handler( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB )
{
	DWORD			dwR;

	// �������� PRE �޽��� �ڵ鷯�� ������ ȣ���Ѵ�.  (��� ������ ����)
	dwR = call_pre_window_func( pWin, dwMesgID, dwParamA, dwParamB );
	if( dwR == WMHRV_ABORT )
		return( 0 );

	// ��Ÿ�� ������� ���� �������� �޽��� �ڵ鷯�� ȣ���Ѵ�.
	dwR = (DWORD)kforward_message( pWin, dwMesgID, dwParamA, dwParamB );
	if( dwR == WMHRV_ABORT )
		return( 0 );

	// �������� POST �޽��� �ڵ鷯�� ������ ȣ���Ѵ�.
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

// �ش� ������� �޽����� ������ �Ѵ�. 
int kpost_message( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB )
{
	int			nR;
	WMesgQStt	*pWMQ;

	if( pWin->pWThread == NULL || pWin->pWThread->pE == NULL )
	{	// WinThread�� ����Ǿ� ���� ������ �׳� ����Ѵ�.
		//kdbg_printf( "kpost_message(%d): No WinThread\n", dwMesgID );
		return( -1 );
	}

	// �޽����� ���� ����?
	pWMQ = &pWin->wmq;
	if( pWMQ->nTotal >= MAX_WMESG_IN_Q )
	{
		//kdbg_printf( "post message : message q is full!\n" );
		return( -1 );		// ���̻� ť�� ������� ���� ����. 
	}

	// �޽��� ID�� �ķ����� 2��.
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

	// �̺�Ʈ�� ������. 
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

// ������ Ÿ��Ʋ�� �����Ѵ�. 
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
		// minimize�� ���� ��ŵ�Ѵ�.
		if( pWin->gb.dwState & WIN_STATE_MINIMIZED || pWin->gb.dwState & WIN_STATE_HIDDEN )
			continue;		
		
		// XY ��ǥ�� pWin�� ���ԵǴ��� �˻��Ѵ�. 
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

// XY�� Win���� ���ԵǴ��� Ȯ��.
int in_window_area( WinStt *pWin, int nX, int nY )
{
	int nR;

	nR = is_in_rect( &pWin->obj.r, nX, nY );

	return( nR );
}

// ���� ��ǥ�� �����Ѵ�. 
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

// Ŭ���̾�Ʈ ������ �ٽ� ����Ѵ�.
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
	
		// Ÿ��Ʋ �ٰ� �ִ� ���.
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
	
		// Ÿ��Ʋ �ٰ� �ִ� ���.
	if( pFrm->nTitleV > 0 )
	{
		pWin->client_r.nY += pFrm->nFrameWidth + pFrm->nTitleV; 	
		pWin->client_r.nV -= pFrm->nFrameWidth + pFrm->nTitleV;
	}

	return( 0 );
}

// Client ���� ��ǥ�� ������ ���� ��ǥ�� ��ȯ�Ѵ�.
int client_to_win_pos( WinStt *pWin, int *pX, int *pY )
{
	pX[0] += pWin->ct_r.nX - pWin->obj.r.nX;
	pY[0] += pWin->ct_r.nY - pWin->obj.r.nY;

	return( 0 );
}	

// RECT ��ǥ�� Client Rect���� Screen Rect �������� ��ȯ�Ѵ�.
// pBaseWin ������ pR�� Screen �������� ��ȯ�Ͽ� pResult�� �����Ѵ�.
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

// RECT ��ǥ�� Screen Rect���� Client Rect �������� ��ȯ�Ѵ�.
// Screen ������ pR�� pBaseWin �������� ��ȯ�Ѵ�.
int screen_to_win( RectStt *pResult, RectStt *pBaseWin, RectStt *pR )
{
	pResult->nX = pR->nX - pBaseWin->nX;
	pResult->nY = pR->nY - pBaseWin->nY;
	pResult->nH = pR->nH;
	pResult->nV = pR->nV;
	return( 0 );
}

/***********************************************************************************
// GraBuff�� ������ ��ũ���� �׸���.  (pR�� pGB ���� ���� ��ǥ)
// ���� C ����
int flush_gra_buff_c( GraBuffStt *pGB, RectStt *pR, MaskStt *pMask )
{
	RectStt		r, rt;
	int			nX, nY;
	UINT16		*pW, *pPixel;

	// ���콺 Ŀ���� �����.
	draw_mouse_pointer( 0 );

	if( pR == NULL )
	{
		rt.nH = pGB->pR->nH;
		rt.nV = pGB->pR->nV;
		rt.nX = 0;
		rt.nY = 0;
		pR = &rt;
	}

	// pGR->pR ���� ��ǥ�� pR�� Sscreen ��ǥ�� ��ȯ�Ѵ�.
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

	// ���콺 Ŀ���� �׸���.
	draw_mouse_pointer( 1 );

	return( 0 );
}

// flush_gra_buff()�� ����� ������ ��.
int flush_gra_buff( GraBuffStt *pGB, RectStt *pR, MaskStt *pMask )
{
	GuiStt		*pGui;
	RectStt		r, rt;
	UINT16		*pW, *pPixel;
	int			nH, nV, nIndex, nMod;
	BYTE		byBit, *pMaskBuff, *pSelfMaskBuff;
	DWORD		dwScreenAdder, dwPixelAdder, dwMaskAdder, dwSelfMaskAdder;

	// ���콺 Ŀ���� �����.
	draw_mouse_pointer( 0 );

	if( pR == NULL )
	{
		rt.nH = pGB->pR->nH;
		rt.nV = pGB->pR->nV;
		rt.nX = 0;
		rt.nY = 0;
		pR = &rt;
	}

	// pGR->pR ���� ��ǥ�� pR�� Sscreen ��ǥ�� ��ȯ�Ѵ�.
	client_to_screen( &r, pGB->pR, pR );

	pPixel = &pGB->pW[ pR->nX + ( pR->nY * pGB->pR->nH ) ];
	pW = get_video_mem_addr16( r.nX, r.nY	);
	
	// ���� ������ ó���� �� ���� �־�� �ϴ� ��.
	pGui = get_gui_stt();
	dwScreenAdder   = pGui->vmode.LinBytesPerScanLine;
	dwPixelAdder    = pGB->pR->nH * 2;
					
	// ���� ��Ʈ ����ũ�� ���Ѵ�.
	nMod = pR->nX % 8;
	byBit = 0x80;
	if( nMod > 0 )
		byBit = byBit >> nMod;

	// ����ũ�� ����
	if( pMask != NULL && pMask->pB != NULL )
	{
		dwMaskAdder = pMask->dwLineBytes;
		nIndex      = ( pR->nY * dwMaskAdder ) + ( pR->nX / 8 );
		pMaskBuff   = &pMask->pB[ nIndex ];
	}
	else
		pMaskBuff = NULL;
		
	// ���� ����ũ�� ���ۿ� ���� ����Ʈ ���� ���Ѵ�.
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

NEXT_LINE:// ���� ������ ó���Ѵ�.		
		PUSH  EBX
		PUSH  ECX
		PUSH  EDX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   AL,  byBit
		MOV   ECX, nH

NEXT_PIXEL:		
		// ����ũ ��Ʈ�� �˻��Ѵ�.
		CMP   EBX, 0
		JZ    CHK_SMASK
		      TEST [EBX], AL
		      JZ   PLUS_SIDI
		
CHK_SMASK:
		//  ���� ����ũ ��Ʈ�� �˻��Ѵ�.
		CMP   EDX, 0
		JZ    MOVE_PIXEL
		      TEST [EDX], AL
		      JNZ  MOVE_PIXEL
PLUS_SIDI:
		// �ȼ��� �������� �ʰ� ��ŵ�Ѵ�.
		ADD  ESI, 2
		ADD  EDI, 2
		JMP  ROL_MASK			  

MOVE_PIXEL:
		// �ȼ��� �����Ѵ�.
		MOVSW
			
ROL_MASK: // ����ũ ����Ʈ�� �������� �Űܾ� �ϴ°�?
        SHR   AL, 1
        CMP   AL, 0
        JNZ   LOOP1
              MOV  AL, 0x80
              CMP  EBX, 0      
              JZ   PLUS_SMB
				   INC EBX		// MASK BUFF�� �ε����� ������Ų��.
PLUS_SMB:     CMP  EDX, 0
			  JZ   LOOP1        
        		   INC EDX		// SELF MASK BUFF�� �ε����� ������Ų��.

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

	// ���콺 Ŀ���� �׸���.
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
	{	// ����ũ�� �ϳ��� ���� ���.=============================================
		
		get_align_count( (DWORD)pDestBuff, nH*2, 4, &nBody, &nTail );
		
		_asm {
		MOV   ESI, pSrcBuff
		MOV   EDI, pDestBuff

		MOV   ECX, nV

NEXT_LINE_A:
		// ���� ������ ó���Ѵ�.		
		PUSH  ECX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   ECX, nBody
		CMP   ECX, 0
		JZ    X_TAIL
			  REP   MOVSD		// 4����Ʈ�� �ű��.
		
X_TAIL:	MOV   ECX, nTail
		SHR   ECX, 1			// �������� �׻� 2�� ����� ���̴�.
		CMP   ECX, 0 
		JZ    X_NEXT
			  REP MOVSW			// 2����Ʈ�� �ű��.

X_NEXT:	POP   EDI
		POP   ESI
		POP   ECX
		
	    ADD   ESI, dwSrcLineAdder
		ADD   EDI, dwDestLineAdder
		LOOP  NEXT_LINE_A
		}
	}
	else if( pMaskBuff != NULL && pSelfMaskBuff != NULL )
	{	// ����ũ�� �� �� �ִ� ���.=============================================							 
		_asm {
		MOV   EBX, pMaskBuff
		MOV   EDX, pSelfMaskBuff
		MOV   ESI, pSrcBuff
		MOV   EDI, pDestBuff
		
		MOV   ECX, nV

NEXT_LINE_B:// ���� ������ ó���Ѵ�.		
		PUSH  EBX
		PUSH  ECX
		PUSH  EDX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   AL,  byBit
		MOV   ECX, nH

NEXT_PIXEL_B:		
		// ����ũ�� ���� ����ũ�� �˻��Ѵ�.
		TEST [EBX], AL
		JZ   PLUS_SIDI_B
		TEST [EDX], AL
		JNZ  MOVE_PIXEL_B

PLUS_SIDI_B:
		// �ȼ��� �������� �ʰ� ��ŵ�Ѵ�.
		ADD  ESI, 2
		ADD  EDI, 2
		JMP  ROL_MASK_B			  

MOVE_PIXEL_B:
		// �ȼ��� �����Ѵ�.
		MOVSW
			
ROL_MASK_B: 
		// ����ũ ����Ʈ�� �������� �Űܾ� �ϴ°�?
        SHR   AL, 1
        CMP   AL, 0
        JNZ   LOOP1_B
              MOV  AL, 0x80
			  INC EBX		// MASK BUFF�� �ε����� ������Ų��.
        	  INC EDX		// SELF MASK BUFF�� �ε����� ������Ų��.

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
	{	// ����ũ�� �ϳ��� �ִ� ���.=============================================							 
ONE_MASK:
		_asm {
		MOV   EBX, pMaskBuff
		MOV   ESI, pSrcBuff
		MOV   EDI, pDestBuff
		
		MOV   ECX, nV

NEXT_LINE_C:// ���� ������ ó���Ѵ�.		
		PUSH  EBX
		PUSH  ECX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   AL,  byBit
		MOV   ECX, nH

NEXT_PIXEL_C:		
		// ����ũ�� ���� ����ũ�� �˻��Ѵ�.
		TEST [EBX], AL
		JNZ  MOVE_PIXEL_C

		// �ȼ��� �������� �ʰ� ��ŵ�Ѵ�.
		ADD  ESI, 2
		ADD  EDI, 2
		JMP  ROL_MASK_C			  

MOVE_PIXEL_C:
		// �ȼ��� �����Ѵ�.
		MOVSW
			
ROL_MASK_C: 
		// ����ũ ����Ʈ�� �������� �Űܾ� �ϴ°�?
        SHR   AL, 1
        CMP   AL, 0
        JNZ   LOOP1_C
              MOV  AL, 0x80
			  INC EBX		// MASK BUFF�� �ε����� ������Ų��.
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
	{	// Self Mask �ϳ��� �ִ� ���.
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
	////  �������� ������ ���� �޸𸮿� �����Ѵ�.  ////
	/////////////////////////////////////////////////////
	_asm {
		PUSHFD
		PUSH  ESI
		PUSH  EDI
		CLD
	}

	if( pMaskBuff == NULL && pSelfMaskBuff == NULL )
	{	// ����ũ�� �ϳ��� ���� ���.=============================================
		
		get_align_count( (DWORD)pW, nH*2, 4, &nBody, &nTail );
		
		_asm {
		MOV   ESI, pPixel
		MOV   EDI, pW

		MOV   ECX, nV

NEXT_LINE_A:
		// ���� ������ ó���Ѵ�.		
		PUSH  ECX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   ECX, nBody
		CMP   ECX, 0
		JZ    X_TAIL
			  //REP   MOVSD		// 4����Ʈ�� �ű��.
		
	    // ������ ȿ��
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
		SHR   ECX, 1			// �������� �׻� 2�� ����� ���̴�.
		CMP   ECX, 0 
		JZ    X_NEXT
			  //REP MOVSW			// 2����Ʈ�� �ű��.

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
	{	// ����ũ�� �� �� �ִ� ���.=============================================							 
		_asm {
		MOV   EBX, pMaskBuff
		MOV   EDX, pSelfMaskBuff
		MOV   ESI, pPixel
		MOV   EDI, pW
		
		MOV   ECX, nV

NEXT_LINE_B:// ���� ������ ó���Ѵ�.		
		PUSH  EBX
		PUSH  ECX
		PUSH  EDX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   AL,  byBit
		MOV   ECX, nH

NEXT_PIXEL_B:		
		// ����ũ�� ���� ����ũ�� �˻��Ѵ�.
		TEST [EBX], AL
		JZ   PLUS_SIDI_B
		TEST [EDX], AL
		JNZ  MOVE_PIXEL_B

PLUS_SIDI_B:
		// �ȼ��� �������� �ʰ� ��ŵ�Ѵ�.
		ADD  ESI, 2
		ADD  EDI, 2
		JMP  ROL_MASK_B			  

MOVE_PIXEL_B:
		// �ȼ��� �����Ѵ�.
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
		// ����ũ ����Ʈ�� �������� �Űܾ� �ϴ°�?
        SHR   AL, 1
        CMP   AL, 0
        JNZ   LOOP1_B
              MOV  AL, 0x80
			  INC EBX		// MASK BUFF�� �ε����� ������Ų��.
        	  INC EDX		// SELF MASK BUFF�� �ε����� ������Ų��.

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
	{	// ����ũ�� �ϳ��� �ִ� ���.=============================================							 
ONE_MASK:
		_asm {
		MOV   EBX, pMaskBuff
		MOV   ESI, pPixel
		MOV   EDI, pW
		
		MOV   ECX, nV

NEXT_LINE_C:// ���� ������ ó���Ѵ�.		
		PUSH  EBX
		PUSH  ECX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   AL,  byBit
		MOV   ECX, nH

NEXT_PIXEL_C:		
		// ����ũ�� ���� ����ũ�� �˻��Ѵ�.
		TEST [EBX], AL
		JNZ  MOVE_PIXEL_C

		// �ȼ��� �������� �ʰ� ��ŵ�Ѵ�.
		ADD  ESI, 2
		ADD  EDI, 2
		JMP  ROL_MASK_C			  

MOVE_PIXEL_C:
		// �ȼ��� �����Ѵ�.
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
		// ����ũ ����Ʈ�� �������� �Űܾ� �ϴ°�?
        SHR   AL, 1
        CMP   AL, 0
        JNZ   LOOP1_C
              MOV  AL, 0x80
			  INC EBX		// MASK BUFF�� �ε����� ������Ų��.
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
	{	// Self Mask �ϳ��� �ִ� ���.
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

// flush_gra_buff()�� ����� 2�� ������ ��.
// ����ũ�� ���� ��쿡 ���� 3�� ���� ( MOVSW -> MOVSD + MOVSB )
// ����ũ ��ü�� 0, 1�� ���� �����Ͽ� WHITE_MASK, BLACK ����ũ�� �����Ͽ� ó��.
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

	// ������ ������ �༮�� �׸� �ʿ䰡 ����.
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

	// ���콺 Ŀ���� �����.
	draw_mouse_pointer( 0 );

	// pGB->pR ���� ��ǥ�� pR�� Sscreen ��ǥ�� ��ȯ�Ѵ�.
	client_to_screen( &r, pGB->pR, pR );

	pPixel = get_gra_buff_addr16( pGB, pR->nX, pR->nY );
	pW = get_video_mem_addr16( r.nX, r.nY	);
	
	// ���� ������ ó���� �� ���� �־�� �ϴ� ��.
	pGui = get_gui_stt();
	dwScreenAdder   = pGui->vmode.LinBytesPerScanLine;
	dwPixelAdder    = pGB->pR->nH * 2;
					
	// ���� ��Ʈ ����ũ�� ���Ѵ�.
	nMod = pR->nX % 8;
	byBit = 0x80;
	if( nMod > 0 )
		byBit = byBit >> nMod;

	// ����ũ�� ����
	if( pMask != NULL && pMask->pB != NULL )
	{
		dwMaskAdder = pMask->dwLineBytes;
		nIndex      = ( pR->nY * dwMaskAdder ) + ( pR->nX / 8 );
		pMaskBuff   = &pMask->pB[ nIndex ];
	}
	else
		pMaskBuff = NULL;
		
	// ���� ����ũ�� ���ۿ� ���� ����Ʈ ���� ���Ѵ�.
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

	// Top Level Window�̸� Mask ���� �׳� �׸���.
	if( pWin->mask.byFlag == MASK_FLAG_WHITE )
		pMaskBuff = NULL;		// ������ �κ��� �����Ƿ� �׳� �𸳵� �׸��� �ȴ�.
	else if( pWin == pGui->pStartLevelWin )
		pMaskBuff = NULL;

	// ������ �׸��� �Լ��� ȣ���Ѵ�.
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

	// ���콺 Ŀ���� �׸���.
	draw_mouse_pointer( 1 );

	return( 0 );
}
*/

// pR Rect�� ��ũ�� �ػ󵵸� ����� ��� �ڸ���.
static int clip_rect_by_screen( RectStt *pResult, RectStt *pR )
{
	GuiStt	*pGui;

	pGui = get_gui_stt();

	memcpy( pResult, pR, sizeof( RectStt ) );

	if( pResult->nX < 0 )
	{
		pResult->nH += pResult->nX;
		if( pResult->nH <= 0 )
			return( -1 );		// �׸� �� ����.
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

	// ������ ������ �༮�� �׸� �ʿ䰡 ����.
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

	// Client Rect�� pR�� Window Rect�� �����Ѵ�.
	client_to_screen( &r_scr_base, pGB->pR, pClientR );
	
	nR = clip_rect_by_screen( &r_clipped_win_base, &r_scr_base );
	if( nR < 0 )
		return( 0 );	// �׸� �ʿ䰡 ����.

	screen_to_win( pClientR, pGB->pR, &r_clipped_win_base );

	// ���콺 Ŀ���� �����.
	draw_mouse_pointer( 0 );

	pPixel = get_gra_buff_addr16( pGB, pClientR->nX, pClientR->nY );
	pW = get_video_mem_addr16( r_clipped_win_base.nX, r_clipped_win_base.nY	);
	
	// ���� ������ ó���� �� ���� �־�� �ϴ� ��.
	pGui = get_gui_stt();
	dwScreenAdder	= pGui->vmode.LinBytesPerScanLine;
	dwPixelAdder	= pGB->pR->nH * 2;
					
	// ���� ��Ʈ ����ũ�� ���Ѵ�.
	nMod = pClientR->nX % 8;
	byBit = 0x80;
	if( nMod > 0 )
		byBit = byBit >> nMod;

	// ����ũ�� ����
	if( pMask != NULL && pMask->pB != NULL )
	{
		dwMaskAdder = pMask->dwLineBytes;
		nIndex		= ( pClientR->nY * dwMaskAdder ) + ( pClientR->nX / 8 );
		pMaskBuff	= &pMask->pB[ nIndex ];
	}
	else
		pMaskBuff = NULL;
		
	// ���� ����ũ�� ���ۿ� ���� ����Ʈ ���� ���Ѵ�.
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

	// Top Level Window�̸� Mask ���� �׳� �׸���.
	if( pWin->mask.byFlag == MASK_FLAG_WHITE )
		pMaskBuff = NULL;		// ������ �κ��� �����Ƿ� �׳� �𸳵� �׸��� �ȴ�.
	else if( pWin == pGui->pStartLevelWin )
		pMaskBuff = NULL;

	// move rect�� on �����̸� ��� off �� �д�.(2004-03-25)
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

	// ������ �׸��� �Լ��� ȣ���Ѵ�.
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

	// move rect�� �ٽ� �׸���. (2004-03-25)
	if( nPrevMoveRectFlag != 0 )
		show_move_rect( 1 );

	// ���콺 Ŀ���� �׸���.
	draw_mouse_pointer( 1 );

	return( 0 );
}

int copy_gra_buff( GraBuffStt *pDestGB, GraBuffStt *pSrcGB, int nDestX, int nDestY )
{
	UINT16		*pDestBuff, *pSrcBuff;
	DWORD		dwSrcLineAddr, dwDestLineAddr;

	pDestBuff = get_gra_buff_addr16( pDestGB, nDestX, nDestY );
	pSrcBuff  = get_gra_buff_addr16( pSrcGB,  0, 0 );
	
	// ���� ������ ó���� �� ���� �־�� �ϴ� ��.
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
	
	// ���� ������ ó���� �� ���� �־�� �ϴ� ��.
	dwSrcLineAddr  = pSrcGB->pR->nH  * 2;
	dwDestLineAddr = pDestGB->pR->nH * 2;
					
	flushing_normal( pSrcBuff, pDestBuff, NULL, NULL, 0, 0, dwDestLineAddr, dwSrcLineAddr, pSrcR->nH, pSrcR->nV, 0 );

	return( 0 );
}


// �ش� �����츦 ���� ��ũ�� ���� �������� �ű��.
int set_top_window( WinStt *pWin )
{
	// �����츦 ���� ��ũ���� ������ �� �߰��ϸ� Top Level�� �߰��ȴ�.
	delete_win_from_scr( pWin );
	append_win_to_scr( pWin );

	// �� ��������� ����ũ ��Ʈ�� �ٽ� ����Ѵ�.
	recalc_bit_mask();

	// TRANSPARENT�̸� ���� ��������� �ٽ� �׸���.
	if( pWin->gb.dwAttr & WIN_ATTR_TRANSPARENT )
		repaint_down_layer( pWin, &pWin->obj.r ); 
	
	// �����츦 �ٽ� �׸���.
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

// pWin�� grabuff�� nScrollVPixel��ŭ Scroll�� �� wClearColor�� �����.
int scroll_gb( WinStt *pWin, RectStt *pR, int nScrollVPixel, RectStt *pClearR, UINT16 wClearColor )
{
	GraBuffStt	*pGB;
	int			nV, nBody, nTail;
	DWORD		dwDest, dwSrc, dwLineBytes;

	pGB = &pWin->gb;
	dwDest = (DWORD)get_gra_buff_addr16( pGB, pR->nX, pR->nY );
	dwSrc  = (DWORD)get_gra_buff_addr16( pGB, pR->nX, pR->nY + nScrollVPixel );
	nV     = pR->nV - nScrollVPixel;
	dwLineBytes = pGB->pR->nH * 2;  // GraBugg ��ü�� H Line ũ���̾�� �Ѵ�.  (pR->nH�� �ƴϴ�.)
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
		// ���� ������ ó���Ѵ�.		
		PUSH  ECX
		PUSH  ESI
		PUSH  EDI
		 
		MOV   ECX, nBody
		CMP   ECX, 0
		JZ    X_TAIL
			  REP   MOVSD		// 4����Ʈ�� �ű��.
		
X_TAIL:	MOV   ECX, nTail
		SHR   ECX, 1			// �������� �׻� 2�� ����� ���̴�.
		CMP   ECX, 0 
		JZ    X_NEXT
			  REP MOVSW			// 2����Ʈ�� �ű��.

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

	// ��ũ�ѵǰ� ���� ������ �����.
	k_fill_rect( pGB, pClearR, wClearColor );

	return( 0 );
}

// ������ pWin�� pParent���� �߰��Ѵ�.
int insert_win_to_scr( WinStt *pParent, WinStt *pWin )
{
	GuiStt	*pGui;
	
	pGui = get_gui_stt();

	if( pParent->pPreLevel == NULL )
	{
		if( pGui->pStartLevelWin != NULL )
			return( -1 );  // ����ó��.

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

// pre_common_minimize���� ȣ��ȴ�.
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
