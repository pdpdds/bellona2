#ifndef BELLONA2_VCONSOLE_HEADER_jj
#define BELLONA2_VCONSOLE_HEADER_jj

#define VCONSOLE_H  80
#define VCONSOLE_V  50

//-----------------------------------------------------------------------------//
typedef void (*GUI_WRITE)( void *pWin, char *pS );
typedef void (*GUI_DIRECT_WRITE)( void *pWin, char *pS, int nX );
typedef void (*GUI_CURSOR_XY)( void *pWin, int nX, int nY );
typedef void (*GUI_CLS)( void *pWinStt, int nY );
typedef void (*GUI_FLUSHING)( void *pWin, struct VConsoleTag *pVC  );

// GUI 쪽에서 설정하는 함수 테이블.
typedef struct GuiConsoleFuncTag {
	void					*pWin;
	GUI_WRITE				pWrite;
	GUI_DIRECT_WRITE		pDirectWrite;
	GUI_CURSOR_XY			pCursorXY;
	GUI_CLS					pCls;
	GUI_FLUSHING			pFlushing;
} GuiConsoleFuncStt;
//-----------------------------------------------------------------------------//

typedef struct {
	BYTE	ch;
	BYTE	attr;
} VConCharStt;

typedef struct VConsoleTag {
	int					nID;
	VConCharStt			con_buff[ VCONSOLE_H * VCONSOLE_V ];	// 버퍼
	UINT16				wCursorX, wCursorY;						// 커서의 위치
	BYTE				byLInes;								// Vertical Lines
	struct ProcessTag	*pStartFg, *pEndFg;						// 가상 콘솔의 FG 프로세스
} VConsoleStt;

#define MAX_VCONSOLE	16

// 필요할 때마다 가상 콘솔을 생성하여 사용할 수 있는 형태.
typedef struct SysVConsoleTag {
	int			nMax;
	int			nTotal;
	int			nNextID;
	VConsoleStt	**pp_vcon;
	VConsoleStt *pActive;
} SysVConsoleStt;

extern BELL_EXPORT int set_fg_process			( VConsoleStt *pVC, struct ProcessTag *pP );
extern BELL_EXPORT int set_active_vconsole		( void *pVConsole );
extern BELL_EXPORT void *get_active_vconsole	();

extern int is_active_vconsole		( void *pVC );
extern int change_next_vconsole		();
extern int disp_vconsole			();
extern int init_sys_vconsole		();
extern int is_fg_process			( struct ProcessTag *pP );
extern int close_vconsole			( void *pVConsole );
extern int set_current_vconsole		( void *pVConsole );
extern int gui_flushing				( VConsoleStt *pVC );
extern int gui_direct_write			( char *pS, int nX );
extern int gui_set_cursor_xy		( int nX, int nY );
extern int gui_cls					( int nY );
extern int gui_write				( char *pS );
extern int del_fg_link				( VConsoleStt *pVC, ProcessStt *pP );

extern void *make_vconsole			();
extern void *get_current_vconsole	();
extern void *get_kernel_vconsole	();
extern void move_vconsole_cursor	( DWORD dwOffs, void *pVC );

#endif
