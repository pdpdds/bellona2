#ifndef BELLONA2_GUI_WALL_HEADER_jj
#define BELLONA2_GUI_WALL_HEADER_jj

typedef struct WallPrivateTag {
	struct WinTag	*pExitMBox;			// ALT-F4�� ������ �� ������ �������� ���� �޽��� �ڽ�.
} WallPrivateStt;

extern int		close_gui_wall		();
extern int		init_wall_win		( WinThreadStt *pWThread, int nWinH, int nWInV );
extern int		flush_wall			( RectStt *pR );

extern struct WinTag *get_wall_win	();

#endif


