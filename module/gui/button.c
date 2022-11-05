#include <bellona2.h>
#include "gui.h"

static DWORD btn_common_lbtn_dn( GuiObjStt *pObj, DWORD dwWMesgID, DWORD dwA, DWORD dwB )
{
	ButtonStt	*pBtn;
	WinStt		*pWin;

	if( pObj->wType != GUI_OTYPE_BUTTON )
		return( -1 );

	pBtn = (ButtonStt*)pObj;
	pWin = (WinStt*)pObj->pOwner;
	if( pWin == NULL || pWin->obj.wMagic != GUI_OBJ_MAGIC || pWin->obj.wType != GUI_OTYPE_WINDOW )
		return( WMHRV_CONTINUE );

	// 버튼이 위치한 영역의 배경을 Owner Window로 하여금 지우게 한다.
	call_win_message_handler( pWin, WMESG_REMAKE, rect_xy_to_dword( &pBtn->obj.r ), rect_hv_to_dword( &pBtn->obj.r ) );
	
	pBtn->dwState |= BTN_STATE_PRESSED;
	kdraw_button_gb( pBtn );

	// 버튼을 그린다.
	kflush_button_gb( &pWin->gb, pBtn );

	// 버튼 영역만 다시 그리도록 한다.
	call_win_message_handler( pWin, WMESG_PAINT, rect_xy_to_dword( &pBtn->obj.r), rect_hv_to_dword( &pBtn->obj.r) );

	return( WMHRV_ABORT );
}

static DWORD btn_common_lbtn_up( GuiObjStt *pObj, DWORD dwWMesgID, DWORD dwA, DWORD dwB )
{
	ButtonStt	*pBtn;
	WinStt		*pWin;

	if( pObj->wType != GUI_OTYPE_BUTTON )
		return( WMHRV_CONTINUE );

	pBtn = (ButtonStt*)pObj;
	pWin = (WinStt*)pObj->pOwner;
	if( pWin == NULL || pWin->obj.wMagic != GUI_OBJ_MAGIC || pWin->obj.wType != GUI_OTYPE_WINDOW )
		return( WMHRV_CONTINUE );

	// 버튼이 위치한 영역의 배경을 Owner Window로 하여금 지우게 한다.
	call_win_message_handler( pWin, WMESG_REMAKE, rect_xy_to_dword( &pBtn->obj.r ), rect_hv_to_dword( &pBtn->obj.r ) );
	
	pBtn->dwState &= (UINT16)~(UINT16)BTN_STATE_PRESSED;
	kdraw_button_gb( pBtn );

	// 버튼을 그린다.
	kflush_button_gb( &pWin->gb, pBtn );

	// 버튼만 다시 그리도록 한다.
	call_win_message_handler( pWin, WMESG_PAINT, rect_xy_to_dword( &pBtn->obj.r), rect_hv_to_dword( &pBtn->obj.r) );

	// 눌린 메시지를 전달한다.
	ksend_message( pWin, pBtn->dwWinMesg, pBtn->dwParamA, pBtn->dwParamB );

	return( WMHRV_ABORT );
}

static DWORD btn_common_mouse_move_in( GuiObjStt *pObj, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	ButtonStt	*pBtn;
	WinStt		*pWin;

	if( pObj->wType != GUI_OTYPE_BUTTON )
		return( WMHRV_CONTINUE );

	pBtn = (ButtonStt*)pObj;
	pWin = (WinStt*)pObj->pOwner;
	if( pWin == NULL || pWin->obj.wMagic != GUI_OBJ_MAGIC || pWin->obj.wType != GUI_OTYPE_WINDOW )
		return( WMHRV_CONTINUE );

	// 버튼이 위치한 영역의 배경을 Owner Window로 하여금 지우게 한다.
	call_win_message_handler( pWin, WMESG_REMAKE, rect_xy_to_dword( &pBtn->obj.r ), rect_hv_to_dword( &pBtn->obj.r ) );
	
	pBtn->dwState |= CONTROL_STATE_ON_MOUSE;
	kdraw_button_gb( pBtn );

	// 버튼을 그린다.
	kflush_button_gb( &pWin->gb, pBtn );

	// 버튼만 다시 그리도록 한다.
	call_win_message_handler( pWin, WMESG_PAINT, rect_xy_to_dword( &pBtn->obj.r), rect_hv_to_dword( &pBtn->obj.r) );

	return( WMHRV_ABORT );
}

static DWORD btn_common_mouse_move_out( GuiObjStt *pObj, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB )
{
	ButtonStt	*pBtn;
	WinStt		*pWin;

	if( pObj->wType != GUI_OTYPE_BUTTON )
		return( WMHRV_CONTINUE );

	pBtn = (ButtonStt*)pObj;
	pWin = (WinStt*)pObj->pOwner;
	if( pWin == NULL || pWin->obj.wMagic != GUI_OBJ_MAGIC || pWin->obj.wType != GUI_OTYPE_WINDOW )
		return( WMHRV_CONTINUE );

	// 버튼이 위치한 영역의 배경을 Owner Window로 하여금 지우게 한다.
	call_win_message_handler( pWin, WMESG_REMAKE, rect_xy_to_dword( &pBtn->obj.r ), rect_hv_to_dword( &pBtn->obj.r ) );
	
	pBtn->dwState &= (DWORD)~(DWORD)( CONTROL_STATE_ON_MOUSE | BTN_STATE_PRESSED );
	kdraw_button_gb( pBtn );

	// 버튼을 그린다.
	kflush_button_gb( &pWin->gb, pBtn );

	// 버튼만 다시 그리도록 한다.
	call_win_message_handler( pWin, WMESG_PAINT, rect_xy_to_dword( &pBtn->obj.r), rect_hv_to_dword( &pBtn->obj.r) );

	return( WMHRV_ABORT );
}

static GuiObjFuncStt btn_mfunc_common[] = {
	{ WMESG_MOUSE_MOVE_OUT,	btn_common_mouse_move_out	 },
	{ WMESG_MOUSE_MOVE_IN,	btn_common_mouse_move_in	 },
	{ WMESG_LBTN_DN, 		btn_common_lbtn_dn			 },
	{ WMESG_LBTN_UP, 		btn_common_lbtn_up			 },
	{ 0, NULL }
};

ButtonStt *kcreate_button_ex( WinStt 	*pParentWin, 
			        	      ImageStt  *pImg,
						      char 		*pText,		
						      DWORD		dwAttr,
						      UINT16	wBackColor,
						      UINT16	wTextColor,
						      int		nFontID,
						      int		nX,
						      int 		nY,
						      int		nH,
						      int      	nV,
						      DWORD		dwWinMesg,
						      DWORD		dwParamA,
						      DWORD		dwParamB	   
						      )
{
	ButtonStt *pBtn;
	
	// 버튼 구조체를 할당하고 초기화한다.
	pBtn = (ButtonStt*)kmalloc( sizeof( ButtonStt ) );
	if( pBtn == NULL )
		return( NULL );
	
	memset( pBtn, 0, sizeof( ButtonStt ) );
	init_gui_obj( &pParentWin->obj, &pBtn->obj, GUI_OTYPE_BUTTON, btn_mfunc_common );

	pBtn->pParentWin 	= pParentWin;
	pBtn->dwAttr		= dwAttr;
	pBtn->dwWinMesg		= dwWinMesg;
    pBtn->dwParamA		= dwParamA;
	pBtn->dwParamB		= dwParamB;
	pBtn->wTextColor	= wTextColor;
	pBtn->wBkColor		= wBackColor;
	pBtn->obj.r.nX 		= nX;
	pBtn->obj.r.nY 		= nY;
	pBtn->obj.r.nH 		= nH;
	pBtn->obj.r.nV 		= nV;	
	pBtn->pImg 			= pImg;
	if( nFontID > 0 )
		pBtn->pFont	= get_system_font( nFontID );
	if( pText != NULL )
		strcpy( pBtn->szText, pText );

	// grabuff를 할당한다.
	alloc_gra_buff_ex( &pBtn->gb, &pBtn->obj.r );

	return( pBtn );
}

ButtonStt *kcreate_button( WinStt	*pParentWin, 
							  int		nImgID,
							  char		*pText, 	
							  DWORD 	dwAttr,
							  UINT16	wBackColor,
							  UINT16	wTextColor,
							  int		nFontID,
							  int		nX,
							  int		nY,
							  int		nH,
							  int		nV,
							  DWORD 	dwWinMesg,
							  DWORD 	dwParamA,
							  DWORD 	dwParamB	   
							  )
{
	ButtonStt 	*pBtn;
	ImageStt	*pImg;

	if( nImgID > 0 )
		pImg = load_icon_image16( get_winres_stt(), nImgID );
	else
		pImg = NULL;
	
	pBtn = kcreate_button_ex( pParentWin, pImg, pText, dwAttr, wBackColor, wTextColor, 
							nFontID, nX, nY, nH, nV, dwWinMesg, dwParamA, dwParamB );

	if( nImgID > 0 )
		pBtn->nImgID = nImgID;
	
	return( pBtn );	
}

// 버튼을 해제한다.
int kclose_button( ButtonStt *pBtn )
{
	RectStt r;
	WinStt *pWin;
	
	if( pBtn == NULL )
		return( -1 );

	pWin = pBtn->pParentWin;
	memcpy( &r, &pBtn->obj.r, sizeof( RectStt ) );

	close_gui_obj( &pBtn->obj );

	free_gra_buff_ex( &pBtn->gb );

	// 이미지를 직접 로드한 경우에만 해제한다.
	if( pBtn->nImgID > 0 && pBtn->pImg != NULL )
		free_image16( pBtn->pImg );

	kfree( pBtn );

	// Owner Window로 REMAKE 메시지를 날려야 한다.
	if( pWin != NULL )
	{
		kpost_message( pWin, WMESG_REMAKE, rect_xy_to_dword( &r ), rect_hv_to_dword( &r ) );
		kpost_message( pWin, WMESG_PAINT, rect_xy_to_dword( &r ), rect_hv_to_dword( &r ) );
	}
	
	return( 0 );
}

// 버튼을 상위 GB에 그린다.
int kflush_button_gb( GraBuffStt *pGB, ButtonStt *pButton )
{
	int			nR;
	GraBuffStt 	*pSrcGB;

	if( pButton == NULL )
	{
		kdbg_printf( "kflush_button_gb: pButton is NULL!\n" );
		return( -1 );
	}

	pSrcGB = &pButton->gb;
	if( pGB == NULL || pGB->pW == NULL || pButton->gb.pW == NULL )
	{
		kdbg_printf( "kflush_button_gb: invalid gb!\n" );
		return( -1 );
	}

	// GB를 복사한다.
	nR = copy_gra_buff( pGB, pSrcGB, pButton->obj.r.nX, pButton->obj.r.nY );

	return( nR );
}

// 버튼을 그린다.
int kdraw_button_gb( ButtonStt *pButton )
{
	GraBuffStt		*pGB;
	WinStt			*pOwner;
	RectStt 		*pR, r;
	int 			nR, nX, nY, nLength;

	//pButton->obj.r은 parent window의 GB내에서의 상대 좌표.
	memcpy( &r, &pButton->obj.r, sizeof( RectStt ) );
	pR = &r;
	r.nX = r.nY = 0;

	pGB = &pButton->gb;

	// Owner Window의 영역을 복사해 온다. (지우는 효과)
	//k_fill_rect( pGB, pR, pButton->wBkColor );
	pOwner = (WinStt*)pButton->obj.pOwner;
	nR = copy_gra_buff_ex( pGB, 0, 0, &pOwner->gb, &pButton->obj.r  );

	// 버튼의 테두리를 그린다.
	if( pButton->dwState & BTN_STATE_PRESSED )
		k_3d_look( pGB, pR, LOOK_3D_IN, LOOK_3D_PRESSED, _3D_BORDER_LIGHT, _3D_BORDER_DK  );
	else
	{
		if( pButton->dwAttr & CONTROL_ATTR_BORDER )
		{
			if( pButton->dwState & CONTROL_STATE_ON_MOUSE )
				k_3d_look( pGB, pR, LOOK_3D_IN, LOOK_3D_DEPRESSED, _3D_BORDER_LIGHT, _3D_BORDER_DK	);
			else
				k_rect( pGB, pR, LOOK_3D_IN, _3D_BORDER_DIMM );
		}
		else if( pButton->dwAttr & CONTROL_ATTR_BORDER_ON_MOUSE )
		{	// 마우스가 올라간 경우에만 BORDER를 그린다.
			if( pButton->dwState & CONTROL_STATE_ON_MOUSE )
				k_3d_look( pGB, pR, LOOK_3D_IN, LOOK_3D_DEPRESSED, _3D_BORDER_LIGHT, _3D_BORDER_DK	);
		}		
	}
		
	if( pButton->szText[0] != 0 )
	{	// 버튼의 글자를 정중앙에 그린다.
		nLength = strlen( pButton->szText ) * pButton->pFont->cH;
		nX = pR->nX + ( pR->nH - nLength ) / 2;
		nY = pR->nY + ( pR->nV - pButton->pFont->cV ) / 2;

		drawtext_xy( 
			pGB,													
			nX,
			nY,
			pButton->pFont, 					
			pButton->szText, 
			pButton->wTextColor,
			0 );			  // Effect ??
	} 

	// 이미지가 설정되어 있으면 그린다.
	if( pButton->pImg != NULL )
	{	
		// 이미지를 버튼의 중앙에 그린다
		nX = pR->nX + (( pR->nH - pButton->pImg->nH ) / 2);
		nY = pR->nY + (( pR->nV - pButton->pImg->nV ) / 2);

		if( nX < 0 )
		{
			kdbg_printf( "pR->nX(%d) + (( pR->nH(%d) - pButton->pImg->nH(%d)\n", pR->nX,pR->nH,pButton->pImg->nH );
		}
		
		copy_image16( pGB, pButton->pImg, nX, nY, pButton->pImg->nH, pButton->pImg->nV, 0, 0 );
	} 

	return( 0 );
}





 
