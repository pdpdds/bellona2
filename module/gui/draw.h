#ifndef BELLONA2_GUI_DRAW_20020523
#define BELLONA2_GUI_DRAW_20020523

extern UINT16	*get_next_screen_line	( UINT16 *pW );
extern UINT16	*get_video_mem_addr16	( int nX, int nY );
extern UINT16	*get_gra_buff_addr16	( GraBuffStt *pGB, int nX, int nY );
extern UINT16 	k_rgb32_to_rgb16		( DWORD dwColor );

extern int		get_mask_bit			( MaskStt *pMask, int nX, int nY );
extern int		set_mask_bit			( MaskStt *pMask, int nX, int nY, int nBit );
extern int		modify_bit_vect			( MaskStt *pMask, RectStt *pR, DWORD dwBit );
extern int		get_overlapped_rect		( RectStt *pResult, RectStt *pA, RectStt *pB );
extern int		k_fill_rect				( GraBuffStt *pGB, RectStt *pR, UINT16 wRGB );
extern int 		k_fill_rect_or			( GraBuffStt *pGB, RectStt *pR, UINT16 wRGB );
extern int		k_fill_rect2			( GraBuffStt *pGB, RectStt *pR, UINT16 wRGB );
extern int		k_rect					( GraBuffStt *pGB, RectStt *pR, int nOuter, UINT16 wColor );
extern int		k_line_h				( GraBuffStt *pGB, int nX, int nY, int nLength, UINT16 wRGB );
extern int		k_line_v				( GraBuffStt *pGB, int nX, int nY, int nLength, UINT16 wRGB );
extern int		k_3d_look				( GraBuffStt *pGB, RectStt *pR, int nOuter, int nType, UINT16 wLightColor, UINT16 wDkColor );
extern int		k_incremental_fill_rect	( GraBuffStt *pGB, RectStt *pR, RectStt *pDrawR, DWORD dwLinesByte, UINT16 wRGB1, UINT16 wRGB2, MaskStt *pMask );
extern int		k_invert_rect			( GraBuffStt *pGB, RectStt *pRect );
extern int 		k_bresenhem_line		( GraBuffStt *pGB, int nX1, int nY1, int nX2, int nY2, DWORD dwColor );
extern int 		k_bresenhem_line16		( GraBuffStt *pGB, int nX1, int nY1, int nX2, int nY2, UINT16 color, int nOrFlag );

#endif
