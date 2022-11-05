#include "guiapp.h"

static ModuleStt *pModule = NULL;

int guiapp_main( ModuleStt *pCurModule, int argc, char* argv[] )
{
	pModule = pCurModule;
	printf( "B2OS GUI Application Library (c) 2003 by OHJJ\n" );

	return( 0 );
}

// 아래 함수를 직접 호출하지 말고 syscall_stub을 호출할 것.
static int _grxcall( int nTotalParam, DWORD *pParam )
{
    int nR;

	_asm {
		PUSHFD
		PUSH DS
		PUSH ES
		PUSH FS
		PUSH GS
		
		MOV  EAX, nTotalParam
        MOV  EBX, pParam
		int  54h
        MOV  nR, EAX

		POP  GS
		POP  FS
		POP  ES
		POP  DS
		POPFD
	}	

	return( nR );
}

static int _declspec( naked ) gx_call_stub( int nType, ... )
{
    static unsigned char    *pX;
    static int              nR, nI, nTotalParam, *pValue;

    _asm {
        MOV  EAX,    ESP
        MOV  pValue, EAX
        MOV  EAX,    [EAX]
        MOV  pX,     EAX
        PUSH ESI
        PUSH EDI
    }   
    pValue++;
    if( pX[0] != 0x83 || pX[1] != 0xC4 )
        goto ERR_RETURN;

    nTotalParam = ((int)pX[2] /4);

    nR = _grxcall( nTotalParam, pValue );

    _asm {
        POP EDI
        POP ESI
        MOV EAX, nR
        RETN
    } 

ERR_RETURN:
    _asm {
        POP EDI
        POP ESI
        MOV EAX, 0xFFFFFFFF
        RETN
    }   
}

//////////////////////////////////////////////////////////////////////////////////////
// image 구조체를 할당한다.
void *gr_alloc_image( int nBPP, int nH, int nV )
{
	ImageStt *pImg;
	int			nAllocSize, nPixelSize;

	if( nBPP == 8 )
		nPixelSize = 1;
	else if( nBPP == 16 )
		nPixelSize = 2;
	else if( nBPP == 32 )
		nPixelSize = 4;
	else
		return( NULL );

	if( nH <= 0 || nV <= 0 )
		return( NULL );

	nAllocSize = nH * nV * nPixelSize + sizeof( ImageStt );

	pImg = (ImageStt*)malloc( nAllocSize );
	memset( pImg, 0, sizeof( ImageStt ) );
	pImg->nBPP  = nBPP;
	pImg->nH    = nH;
	pImg->nV    = nV;
	pImg->nSize = nAllocSize;
	pImg->pMask = NULL;
	pImg->b.pW  = (UINT16*)( (DWORD)pImg + (DWORD)sizeof( ImageStt ) );
	
	return( (void*)pImg );
}

// 이미지의 버퍼주소를 구한다.
void *gr_get_image_buff( void *pVImg )
{
	ImageStt *pImg;

	pImg = (ImageStt*)pVImg;
	if( pImg == NULL )
		return( NULL );

	return( (void*)pImg->b.pW );
}

int gr_win_mesg_handling( DWORD dwWinHandle, WMesgStt *pWM )
{
	int				nR;
	WMESG_FUNC		pFunc;

	if( dwWinHandle == 0 )
		return( -1 );

	nR = gx_call_stub( GRXTYPE_PRE_WMESG_HANDLING, dwWinHandle, pWM->dwID, pWM->dwParamA, pWM->dwParamB );
	if( nR == WMHRV_ABORT )
		return( 0 );

	pFunc = (WMESG_FUNC)gx_call_stub( GRXTYPE_FIND_WMESG_FUNC, dwWinHandle, pWM->dwID );
	if( pFunc != NULL )
	{
		nR = pFunc( (WinStt*)dwWinHandle, pWM->dwID, pWM->dwParamA, pWM->dwParamB );
		if( nR == WMHRV_ABORT )
			return( 0 );
	}

	nR = gx_call_stub( GRXTYPE_POST_WMESG_HANDLING, dwWinHandle, pWM->dwID, pWM->dwParamA, pWM->dwParamB );

	return( 0 );
}

DWORD gr_rgb16_to_rgb32( UINT16 wColor )
{
	BYTE	r, g, b;
	DWORD 	dwColor;

	b = (BYTE)( (UINT16)wColor & (UINT16)31 );
	g = (BYTE)( (UINT16)(wColor >> 5 ) & (UINT16)61 );
	r = (BYTE)( (UINT16)(wColor >> 11) & (UINT16)31 );

	dwColor = RGB32( (UINT16)r << 3, (UINT16)g << 2, (UINT16)b << 3);

	return( dwColor );
}

UINT16 gr_rgb32_to_rgb16( DWORD dwColor )
{
	BYTE 	*pB;
	UINT16	wColor;

	pB = (BYTE*)&dwColor;
	wColor = RGB16( pB[0], pB[1], pB[2] ); // r, g, b
	
	return( wColor );
}
//===============================================================================================//

// 배경 윈도우를 찾는다.
DWORD gx_find_wall_window()
{
	DWORD dwHandle;

	dwHandle = gx_call_stub( GRXTYPE_FIND_WALL_WINDOW );

	return( dwHandle );
}

// Window의 GraBuff에 이미지를 복사한다.
int gx_copy_img_to_win( DWORD dwWinHandle, void *pImg, int nDestX, int nDestY, RectStt *pSrcRect )
{
	int nR;

	if( dwWinHandle == 0, nDestX < 0 || nDestY < 0 || pSrcRect == NULL )
		return( -1 );		// 잘못된 핸들.

	nR = gx_call_stub( GRXTYPE_COPY_IMG_TO_WIN, dwWinHandle, pImg, nDestX, nDestY, pSrcRect );

	return( nR );
}

// WIndow 영역을 Refresh 한다.
int gx_refresh_win( DWORD dwWinHandle, RectStt *pR )
{
	int nR;

	nR = gx_call_stub( GRXTYPE_REFRESH_WIN, dwWinHandle, pR );
	
	return( nR );
}


DWORD gx_create_window( DWORD dwWinThread, DWORD dwPredefStyleID, RectStt *pRect, WMFuncStt *pTbl )
{
	DWORD	dwHandle;

	dwHandle = gx_call_stub( GRXTYPE_CREATE_WINDOW, dwWinThread, dwPredefStyleID, pRect, pTbl );

	return( dwHandle );
}

DWORD gx_create_win_thread( DWORD dwThreadPtr )
{
	DWORD	dwHandle;

	dwHandle = gx_call_stub( GRXTYPE_CREATE_WIN_THREAD, dwThreadPtr );

	return( dwHandle );
}

int gx_close_win_thread( DWORD dwHandle )
{
	int nR;

	nR = gx_call_stub( GRXTYPE_CLOSE_WIN_THREAD, dwHandle );

	return( nR );
}

int gx_win_mesg_pumping( DWORD dwWThread, DWORD *pWinHandle, WMesgStt *pWM )
{
	int nR;

	nR = gx_call_stub( GRXTYPE_MESSAGE_PUMPING, dwWThread, pWinHandle, pWM );

	return( 0 );	
}

UINT16 gx_get_sys_color( int nIndex )
{
	UINT16 wColor;
	wColor = gx_call_stub( GRXTYPE_GET_SYS_COLOR, nIndex );
	return( wColor );
}

int gx_get_client_rect( DWORD dwWinHandle, RectStt *pR )
{
	int nR;
	nR = gx_call_stub( GRXTYPE_GET_CLIENT_RECT, dwWinHandle, pR );	
	return( nR );
}

int gx_get_window_rect( DWORD dwWinHandle, RectStt *pR )
{
	int nR;
	nR = gx_call_stub( GRXTYPE_GET_WINDOW_RECT, dwWinHandle, pR );	
	return( nR );
}

int gx_fill_rect( DWORD dwWinHandle, RectStt *pR, UINT16 wColor )
{
	int nR;
	nR = gx_call_stub( GRXTYPE_FILL_RECT, dwWinHandle, pR, wColor );
	return( nR );	
}

int gx_fill_rect_ex( DWORD dwWinHandle, RectStt *pR, UINT16 wColor, int nOrFlag )
{
	int nR;
	nR = gx_call_stub( GRXTYPE_FILL_RECT_EX, dwWinHandle, pR, wColor, nOrFlag );
	return( nR );	
}

int gx_3d_look( DWORD dwWinHandle, RectStt *pR, int nOuter, int nType, UINT16 wLightColor, UINT16 wDkColor )
{
	int nR;
	nR = gx_call_stub( GRXTYPE_3D_LOOK, dwWinHandle, pR, nOuter, nType, wLightColor, wDkColor );
	return( nR );
}

int gx_line( DWORD dwWinHandle, int nX1, int nY1, int nX2, int nY2, DWORD dwColor )
{
	int nR;
	nR = gx_call_stub( GRXTYPE_LINE, dwWinHandle, nX1, nY1, nX2, nY2, dwColor );
	return( nR );
}

DWORD gx_get_win_id( DWORD dwWinHandle )
{
	DWORD dwID;
	dwID = gx_call_stub( GRXTYPE_GET_WIN_ID, dwWinHandle );
	return( dwID );
}

int gx_register_gui_timer( DWORD dwWinHandle, DWORD dwTimerID, DWORD dwParamB, DWORD dwTick )
{
	int		nR;
	DWORD 	dwWinID;

	dwWinID = gx_get_win_id( dwWinHandle );
	if( dwWinID == 0 )
	{
		printf( "gx_register_gui_timer: win id not found!\n" );
		return( -1 );
	}

	nR = gx_call_stub( GRXTYPE_REGISTER_GUI_TIMER, dwWinID, dwTimerID, dwParamB, dwTick );
	
	return( nR );	
}

int gx_unregister_gui_timer( DWORD dwWinHandle, DWORD dwTimerID )
{
	int		nR;
	DWORD	dwWinID;
	
	dwWinID = gx_get_win_id( dwWinHandle );
	if( dwWinID == 0 )
	{
		printf( "gx_unregister_gui_timer: win id not found!\n" );
		return( -1 );
	}
	
	nR = gx_call_stub( GRXTYPE_UNREGISTER_GUI_TIMER, dwWinID, dwTimerID );
	
	return( nR );	
}

int gx_drawtext_xy( DWORD dwWinHandle, int nX, int nY, DWORD dwFontID, char *pStr, unsigned short wColor, DWORD dwEffect )
{
	int nR;

	nR = gx_call_stub( GRXTYPE_DRAWTEXT_XY, dwWinHandle, nX, nY, dwFontID, pStr, wColor, dwEffect );	
	
	return( nR );
}

int gx_set_win_text( DWORD dwWinHandle, char *pS )
{
	int nR;
	nR = gx_call_stub( GRXTYPE_SET_WIN_TEXT, dwWinHandle, pS );
	return( nR );
}

int gx_close_button( DWORD dwHandle )
{
	int nR;
	nR = gx_call_stub( GRXTYPE_CLOSE_BUTTON, dwHandle );
	return( nR );
}

DWORD gx_create_button( DWORD		dwWinHandle,
						WinResStt	*pWR, 
						int 		nImgID,
						char		*pText, 	
						DWORD 		dwAttr,
						UINT16		wBackColor,
						UINT16		wTextColor,
						int			nFontID,
						int			nX,
						int			nY,
						int			nH,
						int			nV,
						DWORD 		dwWinMesg,
						DWORD 		dwParamA,
						DWORD 		dwParamB	   
						)
{/*`````````````````````````````````````````````````*/
	DWORD 		dwButtonHandle;

	if( dwWinHandle == 0 )
		return( 0 );
	
	dwButtonHandle = gx_call_stub( GRXTYPE_CREATE_BUTTON,
		dwWinHandle, pWR, nImgID, pText, dwAttr, wBackColor, wTextColor, 
		nFontID, nX, nY, nH, nV, dwWinMesg, dwParamA, dwParamB );

	return( dwButtonHandle );	
}

int gx_init_module_res( WinResStt *pWinRes )
{
	int nR;
	
	memset( pWinRes, 0, sizeof( WinResStt ) );

	nR = gx_call_stub( GRXTYPE_INIT_MODULE_RES, pWinRes );

	return( nR );
}

DWORD gx_tb_add_icon( WinResStt *pWR, int nIconID, char *pPopUpString, DWORD dwWinHandle )
{
	DWORD dwHandle;

	dwHandle = gx_call_stub( GRXTYPE_TB_ADD_ICON, pWR, nIconID, pPopUpString, dwWinHandle );
	
	return( dwHandle );
}

int gx_tb_del_icon( DWORD dwHandle )
{
	int nR;
	
	nR = gx_call_stub( GRXTYPE_TB_DEL_ICON, dwHandle );
	
	return( nR );
}

int gx_post_message( DWORD dwWinHandle, DWORD dwMesg, DWORD dwParamA, DWORD dwParamB )
{
	int nR;
	
	nR = gx_call_stub( GRXTYPE_POST_MESSAGE, dwWinHandle, dwMesg, dwParamA, dwParamB );

	return( nR );
}

int gx_get_scr_info( ScrInfoStt *pScr )
{
	int nR;

	nR = gx_call_stub( GRXTYPE_GET_SCR_INFO, pScr );	

	return( nR );
}

int gx_draw_line( DWORD dwWinHandle, int nX, int nY, int nX2, int nY2, UINT16 wColor, int nOrFlag )
{
	int nR;
	nR = gx_call_stub( GRXTYPE_DRAW_LINE, dwWinHandle, nX, nY, nX2, nY2, wColor, nOrFlag );
	return( nR );
}

ImageStt *gx_load_bitmap_to_image16( char *pFileName )
{
	ImageStt *pImg;
	pImg = (ImageStt*)gx_call_stub( GRXTYPE_LOAD_BITMAP16, pFileName );
	return( pImg );
}

int gx_copy_image16( DWORD dwWinHandle, void *pVImg, int nX, int nY, int nH, int nV, int nSrcX, int nSrcY )
{
	int nR;
	nR = gx_call_stub( GRXTYPE_COPY_IMAGE16, dwWinHandle, pVImg, nX, nY, nH, nV, nSrcX, nSrcY );
	return( nR );
}



