#include <bellona2.h>
#include "gui.h"
#include "resource.h"

// ����ȭ�� �����츦 ã�´�.
static int gxh_find_wall_window()
{
	WinStt *pWin;

	pWin = get_wall_win();
	
	return( (int)pWin );
}

// �̹����� ������ ���ۿ� �����Ѵ�.
int gxh_copy_img_to_win( WinStt *pWin, ImageStt *pImg, int nDestX, int nDestY, RectStt *pSrcRect )
{
	int nR;

	if( pWin == NULL || pWin->gb.pW == NULL || pImg == NULL || nDestX < 0 || nDestY < 0 || pSrcRect == NULL )
	{
		kdbg_printf( "gxh_copy_img_to_win: imvalid parameter\n" );
		return( -1 );
	}

	nR = copy_image16( &pWin->gb, pImg, nDestX, nDestY, pSrcRect->nH, pSrcRect->nV, pSrcRect->nX, pSrcRect->nY );

	kdbg_printf( "gxh_copy_img_to_win: %d\n", nR );

	return( 0 );
}	

// ������ �������� �ش� ������ �������� �Ѵ�.
int gxh_refresh_win( WinStt *pWin, RectStt *pR )
{
	int nR;

	if( pWin == NULL )
		return( -1 );

	if( pWin == get_wall_win() )
		nR = flush_wall( pR );
	else
		nR = flush_gra_buff( pWin, pR );

	return( nR );
}

static DWORD gxh_create_window( DWORD dwWinThread, DWORD dwPredefStyleID, RectStt *pRect, WMFuncStt *pTbl )
{
	WinStt	*pWin;

	pWin = kcreate_window( (WinThreadStt*)dwWinThread, 
		(WinStt*)NULL, 
		dwPredefStyleID, 
		pRect, 
		pTbl,
		0, 0, 0, 0, 0 );

	return( (DWORD)pWin );
}

static DWORD gxh_create_win_thread( DWORD dwTID )
{
	ThreadStt 		*pT;
	WinThreadStt 	*pWT;

	if( dwTID != 0 )
	{	// ������ Thread ID�� �̿��Ͽ� Win Thread�� �����.
		pT = find_thread_by_id( dwTID );
		if( pT == NULL )
			return( 0 );
	}
	else
		pT = NULL;			// ���ο� Thread�� �����Ͽ� Win Thread�� �����.

	pWT = kcreate_win_thread( pT );

	return( (DWORD)pWT );
}

static int gxh_close_win_thread( DWORD dwWinThread )
{
	int nR;
	
	nR = kclose_win_thread( (WinThreadStt*)dwWinThread );

	return( nR );
}

static int gxh_message_pumping( DWORD dwWThread, DWORD *pWinHandle, WMesgStt *pWM )
{
	WMesgStt		wm;
 	WinThreadStt 	*pWThread;
	WinStt			*pWin, *pNext;
	int 			nR, nTotal;

	if( dwWThread == 0 || pWinHandle == NULL || pWM == NULL )
		return( -1 );		// ����.
	
	pWThread = (WinThreadStt*)dwWThread;

	for( ;; )
	{	
		nTotal = 0;
		// �޽����� ���� �����찡 ������ ó���Ѵ�. 
		for( pWin = pWThread->pStart; pWin != NULL; pWin = pNext )
		{
			pNext = pWin->pNext;

			// ���� �޽����� �ϳ� ���´�.
			nR = pop_win_mesg( &pWin->wmq, &wm );
			if( nR == 0 )
			{
				WMESG_FUNC	pUserFunc;

				if( wm.dwID >MAX_WMESG_FUNC )
				{
					kdbg_printf( " Invalid WMESG(%d)\n", wm.dwID );
					continue;
				}

				// User Message Handler�� ������ �׳� ���������� ó���Ѵ�.
				pUserFunc = find_r3_wmesg_func( pWin->pWMArray, wm.dwID );
				if( pUserFunc != NULL )
				{	
					pWinHandle[0] = (DWORD)pWin;
					memcpy( pWM, &wm, sizeof( WMesgStt ) );
					return( 0 );
				}
				else// �޽��� �ڵ鷯�� ���� ȣ���Ѵ�.
					call_win_message_handler( pWin, wm.dwID, wm.dwParamA, wm.dwParamB );
				nTotal++;
			}
		}

		if( nTotal == 0 )
			ksuspend_thread( pWThread->pThread );
  	}	

	// ���⼭ ���ϵǴ� ���� ����.
	return( 0 );
}

static int gxh_pre_wmesg_handling( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB )
{
	DWORD			dwR;
	
	// �������� PRE �޽��� �ڵ鷯�� ������ ȣ���Ѵ�.  (��� ������ ����)
	dwR = call_pre_window_func( pWin, dwMesgID, dwParamA, dwParamB );
	if( dwR == WMHRV_ABORT )
		return( WMHRV_ABORT );

	// ��Ÿ�� �������� PRE �Լ��� ȣ���Ѵ�.
	dwR = call_pre_style_func( pWin, dwMesgID, dwParamA, dwParamB );
	if( dwR == WMHRV_ABORT )
		return( WMHRV_ABORT );

	return( 0 );
}

static int gxh_post_wmesg_handling( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB )
{
	DWORD dwR;
	
	dwR = call_post_style_func( pWin, dwMesgID, dwParamA, dwParamB );
	if( dwR == WMHRV_ABORT )
		return( WMHRV_ABORT );

	// �������� POST �޽��� �ڵ鷯�� ������ ȣ���Ѵ�.
	dwR = call_post_window_func( pWin, dwMesgID, dwParamA, dwParamB );

	return( 0 );
}

static WMESG_FUNC gxh_find_wmesg_func( WinStt *pWin, DWORD dwMesgID )
{
	WMESG_FUNC	pFunc;
	pFunc = find_r3_wmesg_func( pWin->pWMArray, dwMesgID );
	return( pFunc );
}

static UINT16 gxh_get_sys_color( int nIndex )
{
	UINT16 wColor;
	wColor = get_sys_color( nIndex );
	return( wColor );
}

static int gxh_get_client_rect( WinStt *pW, RectStt *pR )
{
	if( pW == NULL || pR == NULL )
		return( -1 );

	//screen_to_win( pR, &pW->obj.r, &pW->ct_r );
	get_client_rect( pW, pR );
	
	return( 0 );
}

static int gxh_get_window_rect( WinStt *pW, RectStt *pR )
{
	if( pW == NULL || pR == NULL )
		return( -1 );
	
	memcpy( pR, &pW->obj.r, sizeof( RectStt ) );
	
	return( 0 );
}

static int gxh_fill_rect( WinStt *pWin, RectStt *pR, UINT16 wColor )
{
	int 	nR;
	nR = k_fill_rect( &pWin->gb, pR, wColor );
	return( nR );
}

static int gxh_fill_rect_ex( WinStt *pWin, RectStt *pR, UINT16 wColor, int nOrFlag )
{
	int 	nR;
	nR = k_fill_rect_or( &pWin->gb, pR, wColor );
	return( nR );
}

static int gxh_3d_look( WinStt *pWin, RectStt *pR, int nOuter, int nType, UINT16 wLightColor, UINT16 wDkColor )
{
	int nR;
	nR = k_3d_look( &pWin->gb, pR, nOuter, nType, wLightColor, wDkColor );
	return( nR );
}

static int gxh_line( WinStt *pWin, int nX1, int nY1, int nX2, int nY2, DWORD dwColor )
{
	int nR;
	nR = k_bresenhem_line( &pWin->gb, nX1, nY1, nX2, nY2, dwColor );
	return( nR );
}

static DWORD gxh_get_win_id( DWORD dwWinHandle )
{
	WinStt *pWin;
	pWin = (WinStt*)dwWinHandle;
	return( pWin->dwID );
}

static int gxh_register_gui_timer( DWORD dwWinID, DWORD dwTimerID, DWORD dwParamB, DWORD dwTick )
{
	GuiTimerStt *pGT;

	pGT = register_gui_timer( dwWinID, dwTimerID, dwParamB, dwTick );
	if( pGT == NULL )
		return( -1 );

	return( 0 );
}

static int gxh_unregister_gui_timer( DWORD dwWinID, DWORD dwTimerID )
{
	int				nR;
	GuiTimerStt 	*pGT;

	pGT = find_gui_timer( dwWinID, dwTimerID );
	if( pGT == NULL )
	{
		kdbg_printf( "gxh_unregister_gui_timer: dwWinID(%d), dwTimerID(%d) not found!\n", dwWinID, dwTimerID );
		return( -1 );
	}

	nR = unregister_gui_timer( pGT );
	
	return( nR );
}

static int gxh_drawtext_xy( DWORD dwWinHandle, int nX, int nY, DWORD dwFontID, char *pStr, unsigned short wColor, DWORD dwEffect )
{
	int		nR;
	WinStt	*pWin;
	FontStt	*pFont;
	
	pWin = (WinStt*)dwWinHandle;

	pFont = get_system_font( dwFontID );
	if( pFont == NULL )
	{
		kdbg_printf( "gx_drawtext_xt: FontID(%d) not found!\n", dwFontID );
		return( -1 );
	}	

	nR = drawtext_xy( &pWin->gb, nX, nY,  pFont, pStr, wColor, dwEffect );

	return( nR );	
}

static int gxh_set_win_text( WinStt *pWin, char *pS )
{
	if( pWin == NULL || pS == NULL )
		return( -1 );

	strcpy( pWin->szTitle, pS );

	return( 0 );
}

// GUI ����� gui_init_mod_res�� �����Ѵ�.
static int gxh_init_module_res( WinResStt *pWinRes )
{
	int 						nR;
	ProcessStt					*pP;
	
	pP = k_get_current_process();
	if( pP == NULL || pP->pModule == NULL )
	{
		kdbg_printf( "gxh_init_module_res: cur process or module is NULL!\n" );
		return( -1 );
	}
	
	// �Լ��� ȣ���Ѵ�. 
	nR = win_resource( (DWORD)pP->pModule->dwLoadAddr,  pWinRes );

	return( nR );
}

static int gxh_close_button( ButtonStt *pBtn )
{
	int nR;

	nR = kclose_button( pBtn );

	return( nR );
}

static DWORD gxh_create_button( WinStt	*pWin,
						WinResStt	*pWR, 
						int 		nImgID,
						char		*pText, 	
						DWORD		dwAttr,
						UINT16		wBackColor,
						UINT16		wTextColor,
						int 		nFontID,
						int 		nX,
						int 		nY,
						int 		nH,
						int 		nV,
						DWORD		dwWinMesg,
						DWORD		dwParamA,
						DWORD		dwParamB	   
						)
{/*```````````````````````````````````````````````````*/
	ButtonStt	*pBtn;
	ImageStt	*pImg;
	
	if( nImgID > 0 )
		pImg = load_icon_image16( pWR, nImgID );
	else
		pImg = NULL;
	
	pBtn = kcreate_button_ex( pWin, pImg, pText, dwAttr, wBackColor, wTextColor, 
							nFontID, nX, nY, nH, nV, dwWinMesg, dwParamA, dwParamB );
	
	if( nImgID > 0 )
		pBtn->nImgID = nImgID;

	// ���ʷ� ��ư�� �׸� �� Owner Window�� GB�� Flush �Ѵ�.
	kdraw_button_gb( pBtn );
	kflush_button_gb( &pWin->gb, pBtn );
	
	return( (DWORD)pBtn ); 
}

DWORD gxh_tb_add_icon( WinResStt *pWR, int nIconID, char *pPopUpString, DWORD dwWinHandle )
{
	int			nID;
	ResObjStt	*pRO;
	ImageStt	*pImg, *pKernImg;
	
	if( nIconID > 0 )
		pImg = load_icon_image16( pWR, nIconID );
	else
		pImg = NULL;

	// pImg�� �׳� �����ϸ� �ȵǰ� Ŀ�� ������ �޸𸮸� �Ҵ��Ͽ� ������ �� �����ؾ� �Ѵ�.
	pKernImg = kdup_image16( pImg );
	if( pKernImg == NULL )
		return( 0 );		// �̹����� ������ �� ����.

	nID = tb_add_icon_ex( pKernImg, pPopUpString, NULL, (WinStt*)dwWinHandle );

	// ���ҽ� ������Ʈ�� �Ҵ��Ѵ�.
	pRO = kalloc_res_obj( ROTYPE_TBICON );
	if( pRO == NULL )
	{
		kdbg_printf( "gxh_tb_add_icon: kalloc_res_obj() failed!\n" );
		return( 0 );
	}

	// ���߿� �����Ϸ��� ������ �ξ�� �Ѵ�.
	pRO->dwPtr  = (DWORD)pKernImg;
	pRO->dwData = (DWORD)nID;

	return( (DWORD)pRO );	
}

static int gxh_tb_del_icon( ResObjStt	*pRO )
{
	int 		nID, nR;
	ImageStt	*pKernImg;

	if( pRO->dwType != ROTYPE_TBICON )
	{
		kdbg_printf( "gxh_tb_del_icon: invalid res obj!\n" );
		return( -1 );
	}

	pKernImg = (ImageStt*)pRO->dwPtr;
	nID = (int)pRO->dwData;

	nR = tb_del_icon( nID );

	// ������ �̹����̹Ƿ� �׳� �����ϸ� �ȴ�.
	kfree( pKernImg );  

	// res obj�� �����Ѵ�.
	kfree( pRO );
		
	return( 0 );
}

static int gxh_post_message( WinStt *pWin, DWORD dwMesg, DWORD dwParamA, DWORD dwParamB )
{
	int nR;

	nR = kpost_message( pWin, dwMesg, dwParamA, dwParamB );

	return( nR );
}

static int gxh_get_scr_info( ScrInfoStt *pScr )
{
	GuiStt *pGui;

	if( pScr == NULL )
		return( -1 );

	pGui = get_gui_stt();
	memset( pScr, 0, sizeof( ScrInfoStt ) );
	pScr->nBPP = (int)pGui->vmode.byBPP;
	pScr->nH   = (int)pGui->vmode.wX;
	pScr->nV   = (int)pGui->vmode.wY;

	return( 0 );
}

static int gxh_draw_line( WinStt *pWin, int nX, int nY, int nX2, int nY2, UINT16 wColor, int nOrFlag )
{
	int nR;
	nR = k_bresenhem_line16( &pWin->gb, nX, nY, nX2, nY2, wColor, nOrFlag );
	return( nR );
}

static ImageStt *gxh_load_bitmap16( char *pFileName )
{
	ImageStt *pImg;

	// ����� �޸𸮿� ��Ʈ�� ������ �ε��Ѵ�.
	pImg = load_bitmap_file_to_image16_ex( pFileName, 1 );

	return( pImg );
}

static int gxh_copy_image16( WinStt *pWin, void *pVImg, int nX, int nY, int nH, int nV, int nSrcX, int nSrcY )
{
	int nR;

	nR = copy_image16( &pWin->gb, pVImg, nX, nY, nH, nV, nSrcX, nSrcY );

	return( nR );	
}

#pragma data_seg( "data2" )
static SCallTblStt grxcall_tbl[] = {
	{ GRXTYPE_FIND_WALL_WINDOW,		(DWORD)gxh_find_wall_window  		},
	{ GRXTYPE_COPY_IMG_TO_WIN,		(DWORD)gxh_copy_img_to_win			},
	{ GRXTYPE_REFRESH_WIN,			(DWORD)gxh_refresh_win				},
	{ GRXTYPE_CREATE_WINDOW,		(DWORD)gxh_create_window			},
	{ GRXTYPE_CREATE_WIN_THREAD,	(DWORD)gxh_create_win_thread		},
	{ GRXTYPE_CLOSE_WIN_THREAD,		(DWORD)gxh_close_win_thread			},
	{ GRXTYPE_MESSAGE_PUMPING, 		(DWORD)gxh_message_pumping			},
	{ GRXTYPE_PRE_WMESG_HANDLING,	(DWORD)gxh_pre_wmesg_handling		},
	{ GRXTYPE_POST_WMESG_HANDLING,	(DWORD)gxh_post_wmesg_handling		},
	{ GRXTYPE_FIND_WMESG_FUNC, 		(DWORD)gxh_find_wmesg_func			},
	{ GRXTYPE_GET_SYS_COLOR,		(DWORD)gxh_get_sys_color 			},
	{ GRXTYPE_GET_CLIENT_RECT,		(DWORD)gxh_get_client_rect			},
	{ GRXTYPE_GET_WINDOW_RECT,		(DWORD)gxh_get_window_rect			},
	{ GRXTYPE_FILL_RECT,			(DWORD)gxh_fill_rect				},
	{ GRXTYPE_FILL_RECT_EX,			(DWORD)gxh_fill_rect_ex				},
	{ GRXTYPE_3D_LOOK,				(DWORD)gxh_3d_look					},
	{ GRXTYPE_LINE,					(DWORD)gxh_line						},
	{ GRXTYPE_GET_WIN_ID,			(DWORD)gxh_get_win_id				},
	{ GRXTYPE_REGISTER_GUI_TIMER,	(DWORD)gxh_register_gui_timer		},
	{ GRXTYPE_UNREGISTER_GUI_TIMER,	(DWORD)gxh_unregister_gui_timer		},
	{ GRXTYPE_DRAWTEXT_XY,			(DWORD)gxh_drawtext_xy				},
	{ GRXTYPE_SET_WIN_TEXT,			(DWORD)gxh_set_win_text				},
	{ GRXTYPE_INIT_MODULE_RES,		(DWORD)gxh_init_module_res			},
	{ GRXTYPE_CREATE_BUTTON,		(DWORD)gxh_create_button			},
	{ GRXTYPE_TB_ADD_ICON,			(DWORD)gxh_tb_add_icon              },
	{ GRXTYPE_TB_DEL_ICON,			(DWORD)gxh_tb_del_icon				},
	{ GRXTYPE_CLOSE_BUTTON,			(DWORD)gxh_close_button             },
	{ GRXTYPE_POST_MESSAGE, 		(DWORD)gxh_post_message				},
	{ GRXTYPE_GET_SCR_INFO,			(DWORD)gxh_get_scr_info				},
	{ GRXTYPE_DRAW_LINE,			(DWORD)gxh_draw_line				},
	{ GRXTYPE_LOAD_BITMAP16,		(DWORD)gxh_load_bitmap16			},
	{ GRXTYPE_COPY_IMAGE16, 		(DWORD)gxh_copy_image16				},

	{ 0, 0 }
};
static SC_FUNC grxcall_addr_tbl[ TOTAL_SYSTEM_CALL ];
#pragma data_seg()

void set_grx_syscall()
{
	set_grxcall_tbl( grxcall_tbl, grxcall_addr_tbl );
}

void reset_grx_syscall()
{
	set_grxcall_tbl( NULL, NULL );

}

