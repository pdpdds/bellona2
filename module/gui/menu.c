#include <bellona2.h>
#include "gui.h"

static int close_menu_ent( MenuEntStt *pEnt );
static int draw_menu_ent( GraBuffStt *pGB, MenuStt *pMenu, MenuEntStt *pMenuEnt );
static int create_menu_ent( WinStt *pWin, MenuEntStt *pEnt, RectStt *pR );

static DWORD wmh_menu_destroy( WinStt *pWin, DWORD dwWMesgID, DWORD dwMenuPtr, DWORD dwParamB )
{
	if( pWin->pPrivate != NULL )
	{
		kfree( pWin->pPrivate );
		pWin->pPrivate = NULL;
	}

	return( 0 );
}	

// �޴��� ������ ĥ�ϰ� �׵θ��� �׸���.
static int menu_make_screen( WinStt *pWin, MenuStt *pMenu )
{
	RectStt		r;
	int			nI;
	
	// Ŭ���̾�Ʈ ������ �����. 
	//screen_to_win( &r, pWin->gb.pR, &pWin->ct_r );
	get_client_rect( pWin, &r );
	
	k_fill_rect( &pWin->gb, &r, MENU_BACK_COLOR );

	// �������� �׵θ��� �׸���.
	k_3d_look( &pWin->gb, &r, LOOK_3D_IN, LOOK_3D_DEPRESSED, MENU_LT_COLOR, MENU_DK_COLOR );

	// �޴� ��Ʈ������ �׸���.
	for( nI = 0; nI < pMenu->nTotalEnt; nI++ )
		draw_menu_ent( &pWin->gb, pMenu, &pMenu->pEnt[nI] );

	return( 0 );
}

static DWORD wmh_menu_create( WinStt *pWin, DWORD dwWMesgID, DWORD dwMenuPtr, DWORD dwParamB )
{
	RectStt				r;
	int					nI, nV;
	MenuWinPrivateStt	*pPrivate;

	pWin->pPrivate = pPrivate = (MenuWinPrivateStt*)kmalloc( sizeof( MenuWinPrivateStt ) );
	if( pPrivate != NULL )
	{
		memset( pPrivate, 0, sizeof( MenuWinPrivateStt ) );
		pPrivate->pMenu = (MenuStt*)dwMenuPtr;
	}

	// �޴� ��Ʈ������ �����Ѵ�.
	r.nX = 0;
	r.nY = MENU_MARGIN_V;
	r.nH = pWin->obj.r.nH;
	for( nI = 0; nI < pPrivate->pMenu->nTotalEnt; nI++ )
	{	
		if( pPrivate->pMenu->pEnt[nI].wID == MENU_ENT_ID_BREAK )
			nV = MENU_BREAK_V;
		else 
			nV = MENU_ENT_V;
		r.nV = nV;
		// �������� �����Ǿ� ������ �ε��ϰ� Rect�� ����(��ġ�� ��´�.)�Ѵ�.
		create_menu_ent( pWin, &pPrivate->pMenu->pEnt[ nI ], &r );
		r.nY += nV;
	}

	// ���ʷ� �޴��� �׷��д�.
	menu_make_screen( pWin, (MenuStt*)dwMenuPtr );

	return( 0 );
}

static DWORD wmh_menu_show( WinStt *pWin, DWORD dwWMesgID, DWORD dwShowFlag, DWORD dwParamB )
{
	int					nI;
	MenuStt				*pMenu;
	MenuWinPrivateStt	*pPrivate;
	WinStt				*pMouseOwnerWin;
	
	pMouseOwnerWin = get_mouse_owner_win();

	if( dwShowFlag == 0 )
	{
		if( pWin == pMouseOwnerWin )
			return( WMHRV_ABORT );

		// SubMenu ��� SHow�� �Ǿ� �ִ� ���� HIdden���� �����Ѵ�.
		pPrivate = (MenuWinPrivateStt*)pWin->pPrivate;
		pMenu = (MenuStt*)pPrivate->pMenu;
		for( nI = 0; nI < pMenu->nTotalEnt; nI++ )
		{
			if( pMenu->pEnt[nI].pSubMenu != NULL )
			{
				if( is_win_show( pMenu->pEnt[nI].pSubMenu->pWin ) != 0 )
					kpost_message( pMenu->pEnt[nI].pSubMenu->pWin, WMESG_SHOW, 0, 0 );
			}
		}
	}

	return( WMHRV_CONTINUE );
}

static WMFuncStt menu_marray[] = {
	{ WMESG_CREATE		   , wmh_menu_create			},
	{ WMESG_DESTROY		   , wmh_menu_destroy			},
	{ WMESG_SHOW		   , wmh_menu_show				},
	{ 0, NULL }
};


int calc_menu_v( MenuStt *pMenu )
{
	UINT16	wID;
	int		nI, nV;

	// ��Ʈ���� ������ ���Ѵ�.
	for( nV = nI = 0; ; nI++ )
	{
		wID = pMenu->pEnt[nI].wID;
		if( wID == 0 )
			break;
		else if( wID == MENU_ENT_ID_BREAK )
			nV += MENU_BREAK_V;
		else
			nV += MENU_ENT_V;
	}
	nV += MENU_MARGIN_V*2;

	return( nV );
}

int create_menu( MenuStt *pMenu, int nX, int nY, WinThreadStt *pWThread, WinStt *pParentWin )
{
	RectStt		r;
	FontStt		*pFont;
	DWORD		dwParamA;
	int			nI, nMaxStr, nT;

	// �޴� ��Ʈ���� ������ ���Ѵ�.
	pMenu->nTotalEnt = 0;
	nMaxStr = 0;
	for( nI = 0; ; nI++ )
	{
		if( pMenu->pEnt[nI].wID == 0 )
			break;
		pMenu->nTotalEnt++;

		// ���� �� ���ڿ��� ���Ѵ�.
		nT = strlen( pMenu->pEnt[nI].szStr );
		if( nT > nMaxStr )
			nMaxStr = nT;
	}

	// ��Ʈ�� �ּҸ� ���Ѵ�.
	pFont = get_system_font( MENU_BASE_FONT );

	// �����츦 �����Ѵ�.
	r.nX = nX;
	r.nY = nY;
	// ���� ������ �ֱ� ���� MARGIN_H�� 3�� ��.
	r.nH = (MENU_MARGIN_H*3) + (pFont->cH * MENU_MARGIN_H) + MENU_ICON_H; 
	r.nV = calc_menu_v( pMenu );
	dwParamA = (DWORD)pMenu;
	// HIDE �� ���·� �޴��� �����Ѵ�.
	pMenu->pWin = kcreate_window( pWThread, pParentWin, WSTYLE_FLAT, &r, menu_marray,
						WIN_STATE_HIDDEN, WIN_ATTR_NONMOVABLE, 0, dwParamA, 0 );

	return( 0 );
}

// �޴��� �����Ѵ�.
int close_menu( MenuStt *pMenu )
{
	int	nI;

	// �� �޴� ��Ʈ������ �ı��Ѵ�.
	for( nI = 0; nI < pMenu->nTotalEnt; nI++ )
		close_menu_ent( &pMenu->pEnt[nI] );

	// �����츦 �ݴ´�.
	kpost_message( pMenu->pWin, WMESG_DESTROY, 0, 0 );

	return( 0 );
}

static DWORD menuent_common_lbtn_dn( GuiObjStt *pObj, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( 0 );
}

static DWORD menuent_common_lbtn_up( GuiObjStt *pObj, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	return( 0 );
}

// wmh_pre_common_mouse_move���� ���콺�� �������� ��Ʈ���� out�� ���� ȣ���� �� ���콺��
// ���� ��Ʈ���� in�� ȣ���Ѵ�.
// ���콺�� �޴� ��Ʈ���� SubMenu������ ������ ���� �ش� �޴� ��Ʈ���� out�� ȣ��Ǿ�
// pSubMenu�� hide ��Ų��.  �� �� �޴��� hide�Ǹ� �ȵȴ�.  �� ��쿡 out���� posting��
// WMESG_SHOW(0)�� pSubMenu�� Window���� �����ϵ��� ó���Ѵ�.  ���콺�� ��� �ִ� �����찡
// Menu Window�� ������ Hide �޽����� �����ϰ� WMHRV_ABORT�� �����Ѵ�.  (�����찡 Hide���� �ʴ´�.)

// �޴� ��Ʈ���� ���콺�� ������ ������ ��.
static DWORD menuent_common_mouse_move_in( GuiObjStt *pObj, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	int					nI;
	WinStt				*pWin;
	MenuEntStt			*pEnt;
	MenuStt				*pMenu;
	int					nX, nY;
	MenuWinPrivateStt	*pPrivate;

	pEnt = (MenuEntStt*)pObj;
	pWin = (WinStt*)pObj->pOwner;
	pPrivate = (MenuWinPrivateStt*)pWin->pPrivate;
	if( pWin == NULL || pWin->obj.wMagic != GUI_OBJ_MAGIC || pWin->obj.wType != GUI_OTYPE_WINDOW )
		return( WMHRV_CONTINUE );

	// �޴� ��Ʈ���� �ٽ� �׸���.
	draw_menu_ent( &pWin->gb, pPrivate->pMenu, (MenuEntStt*)pObj );
	call_win_message_handler( pWin, WMESG_PAINT, rect_xy_to_dword( &pObj->r), rect_hv_to_dword( &pObj->r) );

	// SubMenu ��� SHow�� �Ǿ� �ִ� ���� HIdden���� �����Ѵ�.
	pMenu = (MenuStt*)pPrivate->pMenu;
	for( nI = 0; nI < pMenu->nTotalEnt; nI++ )
	{
		if( pMenu->pEnt[nI].pSubMenu == NULL )
			continue;

		if( pEnt == &pMenu->pEnt[nI] )
		{	// Show �Ѵ�.
			// �޴� �������� ��ġ�� �ٽ� ��´�.
			nX = pWin->obj.r.nX + pObj->r.nH;//pWin->obj.r.nH;
			nY = pEnt->obj.r.nY + pWin->obj.r.nY;

			// �������� ��ġ�� �����Ѵ�.
			call_win_message_handler( pEnt->pSubMenu->pWin, WMESG_WIN_MOVE, (DWORD)nX, (WORD)nY );

			// �����츦 ��Ÿ����.
			kpost_message( pEnt->pSubMenu->pWin, WMESG_SHOW, 1, 0 );
			set_top_window( pEnt->pSubMenu->pWin );
		}
		else
		{	// Show�Ǿ� ������ Hidden���� �Ѵ�.
			if( is_win_show( pMenu->pEnt[nI].pSubMenu->pWin ) != 0 )
				kpost_message( pMenu->pEnt[nI].pSubMenu->pWin, WMESG_SHOW, 0, 0 );
		}
	}

	return( 0 );
}

// �޴� ��Ʈ������ ���콺�� ���� ������ ��
static DWORD menuent_common_mouse_move_out( GuiObjStt *pObj, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	WinStt				*pWin;
	MenuEntStt			*pEnt;
	MenuWinPrivateStt	*pPrivate;

	pEnt = (MenuEntStt*)pObj;
	pWin = (WinStt*)pObj->pOwner;
	pPrivate = (MenuWinPrivateStt*)pWin->pPrivate;
	if( pWin == NULL || pWin->obj.wMagic != GUI_OBJ_MAGIC || pWin->obj.wType != GUI_OTYPE_WINDOW )
		return( WMHRV_CONTINUE );

	// �޴� ��Ʈ���� �ٽ� �׸���.
	draw_menu_ent( &pWin->gb, pPrivate->pMenu, (MenuEntStt*)pObj );
	call_win_message_handler( pWin, WMESG_PAINT, rect_xy_to_dword( &pObj->r), rect_hv_to_dword( &pObj->r) );

	// Sub Menu�� ������ �����.
	if( pEnt->pSubMenu != NULL )
	{
		if( is_win_show( pEnt->pSubMenu->pWin ) != 0 )
			kpost_message( pEnt->pSubMenu->pWin, WMESG_SHOW, 0, 0 );
	}

	return( 0 );
}

static GuiObjFuncStt menuent_mfunc_common[] = {
	{ WMESG_MOUSE_MOVE_OUT,	menuent_common_mouse_move_out	 },
	{ WMESG_MOUSE_MOVE_IN,	menuent_common_mouse_move_in	 },
	{ WMESG_LBTN_DN, 		menuent_common_lbtn_dn			 },
	{ WMESG_LBTN_UP, 		menuent_common_lbtn_up			 },
	{ 0, NULL }
};

// �޴� ��Ʈ���� �����Ѵ�.
static int create_menu_ent( WinStt *pParentWin, MenuEntStt *pEnt, RectStt *pR )
{
	init_gui_obj( &pParentWin->obj, &pEnt->obj, GUI_OTYPE_MENUENT, menuent_mfunc_common );

	// �������� �����Ǿ� ������ �ε��Ѵ�.
	if( pEnt->dwIconID != 0 )
		pEnt->pImg = load_icon_image16( get_winres_stt(), pEnt->dwIconID );

	// Rect�� �����Ѵ�.
	memcpy( &pEnt->obj.r, pR, sizeof( RectStt ) );

	// Sub Menu�� ������ Recursive�ϰ� ȣ���Ѵ�.
	if( pEnt->pSubMenu != NULL )
	{
		int	nR;
		nR = create_menu( pEnt->pSubMenu, 0, 0, pParentWin->pWThread, pParentWin->pParentWin );
	}	

	return( 0 );
}

// �޴� ��Ʈ���� �����Ѵ�.
static int close_menu_ent( MenuEntStt *pEnt )
{
	if( pEnt->pImg != NULL )
	{
		free_image16( pEnt->pImg );
		pEnt->pImg = NULL;
	}

	return( 0 );
}

// �޴� ��Ʈ���� �׸���.
static int draw_menu_ent( GraBuffStt *pGB, MenuStt *pMenu, MenuEntStt *pMenuEnt )
{
	RectStt		*pR, r;
	FontStt		*pFont;
	int			nX, nY;
	ImageStt	*pMoreImg;
	UINT16		wTextColor;

	pR = &pMenuEnt->obj.r;

	memcpy( &r, pR, sizeof( RectStt ) );
	pR = &r;
	pR->nH -= 4;
	pR->nX += 2;

	if( pMenuEnt->wID == MENU_ENT_ID_BREAK )
	{
		nX = pR->nX+1;
		nY = pR->nY + pR->nV / 2;
		k_line_h( pGB, nX, nY-1,   pR->nH-2, MENU_DK_COLOR );
		k_line_h( pGB, nX, nY, pR->nH-2, MENU_LT_COLOR );

		return( 0 );
	}
	
	// ������ ĥ�Ѵ�.
	if( pMenu->pWin->obj.pCat == &pMenuEnt->obj )
	{
		k_fill_rect( pGB, pR, MENU_DK_BACK_COLOR );
		wTextColor = MENU_LT_TEXT_COLOR;
	}
	else
	{
		k_fill_rect( pGB, pR, MENU_BACK_COLOR );	
		wTextColor = MENU_DK_TEXT_COLOR;
	}

	// BORDER�� �����Ǿ� ������ �׸���.
	if( pMenuEnt->wAttr & CONTROL_ATTR_BORDER )
	{
		if( pMenuEnt->wState & BTN_STATE_PRESSED )
			k_3d_look( pGB, pR, LOOK_3D_IN, LOOK_3D_PRESSED, _3D_BORDER_LIGHT, _3D_BORDER_DK  );
		else
		{
			if( pMenuEnt->wState & CONTROL_ATTR_BORDER )
				k_3d_look( pGB, pR, LOOK_3D_IN, LOOK_3D_DEPRESSED, _3D_BORDER_LIGHT, _3D_BORDER_DK  );
			else
				k_rect( pGB, pR, LOOK_3D_IN, MENU_BACK_COLOR );
		}
	}


	// �̹����� �����Ǿ� ������ �׸���.
	nX = MENU_MARGIN_H + 2;
	if( pMenuEnt->pImg != NULL )
	{	
		nY = pR->nY + ( pR->nV - pMenuEnt->pImg->nV ) / 2;
		copy_image16( pGB, pMenuEnt->pImg, nX, nY, pMenuEnt->pImg->nH, pMenuEnt->pImg->nV, 0, 0 );

		nX += pMenuEnt->pImg->nH + 2;
	} 

	// �޴� ��Ʈ���� ��Ʈ���� ����Ѵ�.
	pFont = get_system_font( IDR_BF_BASE11 ); 
	if( pMenuEnt->szStr[0] != 0 )
	{	
		nY = pR->nY + ( pR->nV - pFont->cV ) / 2;

		drawtext_xy( 
			pGB,													
			nX,
			nY,
			pFont,						
			pMenuEnt->szStr, 
			wTextColor,
			0 );			  // Effect ??
	} 

	// SubMenu�������� ������ More Image�� �׸���.
	if( pMenuEnt->pSubMenu != NULL )
	{
		pMoreImg = get_sys_icon( IDI_MORE );
		nX = pR->nX + pR->nH - pMoreImg->nH - 2;  
		nY = pR->nY + ( pR->nV - pMoreImg->nV ) / 2;
		copy_image16( pGB, pMoreImg, nX, nY, pMoreImg->nH, pMoreImg->nV, 0, 0 );
	}

	return( 0 );
}
