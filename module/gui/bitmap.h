#ifndef BELLONA2_GUI_BITMAP_20020518
#define BELLONA2_GUI_BITMAP_20020518

extern int		free_image16					( struct ImageTag  *pImg );
extern int		draw_image16_ex					( void *pVImg, void *pVOrgImg, int nX, int nY, struct RectTag *pR, struct RectTag *pNotRect );
extern int		draw_image16					( void *pVImg, void *pVOrgImg, int nX, int nY, int nSrcX, int nSrcY, int nSrcH, int nSrcV );
extern int		copy_image16					( struct GraBuffTag *pGB, void *pVImg, int nX, int nY, int nXLimit, int nYLimit, int nSrcX, int nSrcY );

extern void 	brighten_rgbquad				( RGBQUAD *pR );
extern void		darken_rgbquad					( RGBQUAD *pR );
extern void		rgb16_to_rgbquad				( UINT16 wColor, RGBQUAD *pRQ );
extern void		*alloc_blank_image16			( int nH, int nV );
extern void		*bitmap_to_image16				( BITMAPINFO *pBitmap, int nH, int nV );
extern void		*load_icon_image16				( void *pWR, int nID );
extern void		*load_bitmap_image16			( void *pWR, int nID );
extern void 	*load_bitmap_file_to_image16	( char *pFileName );
extern void 	*load_bitmap_file_to_image16_ex	( char *pFileName, int nUserFlag );


extern UINT16	rgbquad_to_rgb16				( RGBQUAD *pRQ );
extern UINT16	*get_img_buff_addr16			( struct ImageTag *pImg, int nX, int nY );

extern struct ImageTag *kdup_image16			( struct ImageTag *pSrcImg );

#endif
