#ifndef BELLONA2_GUI_KCMD_HEADER
#define BELLONA2_GUI_KCMD_HEADER

#define KCMDWIN_FONT		IDR_BF_SIMPLE9
#define KCMDWIN_LINE_GAP	0

#define KCMDWIN_BACK_COLOR	RGB16( 90, 90, 90 )
#define KCMDWIN_TEXT_COLOR	RGB16( 64, 255, 64 )

typedef struct KCmdWinPrivateTag {
	struct	FontTag		*pFont;
	BYTE				byShowCursor;	// 커서의 현재 상태.	
	int					nXPos, nYPos;
	int					nLineV;
	int					nTotalLine;
	int					nTotalCol;
} KCmdWinPrivateStt;

extern int calc_KCmdWin_size	( RectStt *pR, DWORD dwPredefStyleID );

#endif
