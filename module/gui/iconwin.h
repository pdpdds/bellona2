#ifndef BELLONA2_GUI_ICON_WIN_HEADER_jj
#define BELLONA2_GUI_ICON_WIN_HEADER_jj

#define ICON_WIN_H	38
#define ICON_WIN_V	(36+12+2)

typedef struct {
	struct ImageTag		*pImg;
} IconWinPrivateStt;

extern int init_iconwin( WinStt *pWin, DWORD dwInitParam );
extern int initialize_iconwin( WinStt *pWin, char *pTitle, DWORD dwIconID );

#endif
