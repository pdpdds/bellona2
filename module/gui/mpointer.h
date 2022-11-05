#ifndef BELLONA2_GUI_MPOINTER_20020516
#define BELLONA2_GUI_MPOINTER_20020516

#define MOUSE_LBTN_DOWN	1
#define MOUSE_RBTN_DOWN	2

#define MPOINTER_H	32
#define MPOINTER_V	32

typedef struct {
	int					nIndex;
	ICONDIR				*pIDir;
	BITMAPINFO			*pBit;
	unsigned short int 	wHotX, wHotY;
	void				*pVoidImg;
} BCursorStt;

typedef enum {
	CSINDEX_ARROW = 0,		// Cursor Index Arrow
	CSINDEX_RS_H,			// Cursor Index Resize Horizontal
	CSINDEX_RS_V,			// Cursor Index Resize Vertical
	CSINDEX_RS_UR,			// Cursor Index Resize Up Right
	CSINDEX_RS_UL,			// Cursor Index Resize Up Left
	MAX_CURSOR_SET_ENT 
} CURSOR_SET_INDEX_TAG;

typedef struct BCursorSetTag {
	BCursorStt	ent[MAX_CURSOR_SET_ENT];
} BCursorSetStt;

// 마우스 포인터
typedef struct {
	BYTE			byState;		// 버튼의 눌려짐 상태. 
	int				nX;				// 화면 상에서의 X 좌표
	int				nY;				// 화면 상에서의 Y 좌표
	int				nH;
	int				nV;
	BCursorStt		*pCurrentCursor;
	BCursorSetStt	cursor_set;
	void			*pVoidBackImg;
	int				nDrawFalg;		// 그려져 있는지의 여부
	struct WinTag	*pIncludeWin;	// 이전까지 커서를 포함하고 있던 윈도우
} MousePointerStt;

// 마우스 데이터 구조체
typedef struct {
	BYTE	data[3];
} MouseDataStt;

extern int				get_mouse_pointer_index 	();
extern int 				get_system_mouse_draw		();
extern int 				set_mouse_pointer			( int nCSIndex );
extern int 				gui_mouse_call_back			( BYTE *pMData );
extern int 				invalidate_mouse_owner		( struct WinTag * pWin );
extern void 			init_mpointer				();
extern void 			draw_mouse_pointer			( int nFlag );
extern MousePointerStt 	*get_system_mpointer		();
extern struct WinTag 	*get_mouse_owner_win		();


#endif
