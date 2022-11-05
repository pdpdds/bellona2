#ifndef GRX_HEADER_JJ
#define GRX_HEADER_JJ

#include "grxdef.h"

extern BELL_EXPORT DWORD	gx_find_wall_window				();
extern BELL_EXPORT DWORD	gx_create_win_thread			( DWORD dwThreadPtr );
extern BELL_EXPORT DWORD	gx_create_window				( DWORD dwWinThread, DWORD dwPredefStyleID, RectStt *pRect, WMFuncStt *pWMesgTbl );
extern BELL_EXPORT DWORD	gr_rgb16_to_rgb32				( UINT16 wColor );

extern BELL_EXPORT void		*gr_get_image_buff				( void *pVImg );
extern BELL_EXPORT void		*gr_alloc_image					( int nBPP, int nH, int nV );

extern BELL_EXPORT UINT16	gx_get_sys_color				( int nIndex );
extern BELL_EXPORT UINT16 	gr_rgb32_to_rgb16				( DWORD dwColor );

extern BELL_EXPORT int 		gr_win_mesg_handling			( DWORD dwWinHandle, WMesgStt *pWM );;
extern BELL_EXPORT int		gx_copy_img_to_win				( DWORD dwWinHandle, void *pImg, int nDestX, int nDestY, struct RectTag *pSrcRect );
extern BELL_EXPORT int		gx_close_win_thread 			( DWORD dwThreadPtr );
extern BELL_EXPORT int		gx_refresh_win					( DWORD dwWinHandle, RectStt *pR );
extern BELL_EXPORT int 		gx_win_mesg_pumping				( DWORD dwWThread, DWORD *pWinHandle, WMesgStt *pWM );
extern BELL_EXPORT int		gx_get_client_rect				( DWORD dwWinHandle, RectStt *pR );
extern BELL_EXPORT int		gx_get_window_rect				( DWORD dwWinHandle, RectStt *pR );
extern BELL_EXPORT int 		gx_fill_rect					( DWORD dwWinHandle, RectStt *pR, UINT16 wColor );
extern BELL_EXPORT int 		gx_fill_rect_ex					( DWORD dwWinHandle, RectStt *pR, UINT16 wColor, int nOrFlag );
extern BELL_EXPORT int 		gx_3d_look						( DWORD dwWinHandle, RectStt *pR, int nOuter, int nType, UINT16 wLightColor, UINT16 wDkColor );
extern BELL_EXPORT int 		gx_line							( DWORD dwWinHandle, int nX1, int nY1, int nX2, int nY2, DWORD dwColor );
extern BELL_EXPORT int 		gx_register_gui_timer			( DWORD dwWinHandle, DWORD dwTimerID, DWORD dwParamB, DWORD dwTick );
extern BELL_EXPORT int 		gx_unregister_gui_timer			( DWORD dwWinHandle, DWORD dwTimerID );
extern BELL_EXPORT int 		gx_drawtext_xy					( DWORD dwWinHandle, int nX, int nY, DWORD dwFontID, char *pStr, unsigned short wColor, DWORD dwEffect );
extern BELL_EXPORT int 		gx_set_win_text					( DWORD dwWinHandle, char *pS );
extern BELL_EXPORT int 		gx_init_module_res				( WinResStt *pWinRes );
extern BELL_EXPORT int 		gx_tb_del_icon					( DWORD dwHandle );
extern BELL_EXPORT int 		gx_close_button					( DWORD dwHandle );
extern BELL_EXPORT int 		gx_post_message					( DWORD dwWinHandle, DWORD dwMesg, DWORD dwParamA, DWORD dwParamB );
extern BELL_EXPORT int 		gx_get_scr_info					( ScrInfoStt *pScr );
extern BELL_EXPORT int 		gx_draw_line					( DWORD dwWinHandle, int nX, int nY, int nX2, int nY2, UINT16 wColor, int nOrFlag );
extern BELL_EXPORT int 		gx_copy_image16					( DWORD dwWinHandle, void *pVImg, int nX, int nY, int nH, int nV, int nSrcX, int nSrcY );
	
extern BELL_EXPORT DWORD	gx_tb_add_icon					( WinResStt *pWR, int nIconID, char *pPopUpString, DWORD dwWinHandle );
extern BELL_EXPORT DWORD 	gx_create_button				( DWORD dwWinHandle,
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
);


extern BELL_EXPORT struct ImageTag *gx_load_bitmap_to_image16		( char *pFileName );

#endif
