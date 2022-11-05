#ifndef BELLONA2_WINDOW_HEADER_20020521
#define BELLONA2_WINDOW_HEADER_20020521

// 윈도우의 상태
#define	WIN_STATE_NORMAL		0x0000
#define WIN_STATE_MINIMIZED		0x0001
#define WIN_STATE_HIDDEN		0x0002
#define WIN_STATE_INITIALIZED	0x8000

// 윈도우 속성
#define WIN_ATTR_TRANSPARENT	0x0100
#define WIN_ATTR_NONMOVABLE		0x0200			// 메뉴 윈도우는 마우스로 옮겨지지 않도록...

// 윈도우 초기화 함수 
// kcreate_window의 인자로 전달되어 호출된다. 
// 이 함수 안에서 해당 윈도우의 Message Handler Array등을 설정한 후 
// WMESG_CREATE 등을 날린다.
typedef DWORD (*WINIT_FUNC)( struct WinTag *pWin, DWORD dwParam );

// 한 윈도우가 보관할 수 있는 메시지의 개수
#define MAX_WMESG_IN_Q	32

// 윈도우 메시지 큐
typedef struct {
	int			nTotal;
	int			nStart, nEnd;
	WMesgStt	q[ MAX_WMESG_IN_Q ];
} WMesgQStt;

#define MASK_FLAG_BLACK		1	// 모든 마스크 비트가 0이어서 그릴 것이 없는 경우.
#define MASK_FLAG_WHITE		2	// 모든 마스크 비트가 1이어서 마스크 없이 모두 그리면 되는 경우.

struct MaskTag {
	RectStt	*pR;			// WinStt 구조체에 있는 rect를 공유한다.
	BYTE	*pB;			// 마스크 버퍼
	DWORD	dwLineBytes;	// 한 라인을 구성하는 바이트 수
	DWORD	dwSize;			// pB의 전체 크기
	BYTE	byFlag;			// 마스크의 플래그 (모두 0 또는 모두 1)
};
typedef struct MaskTag MaskStt;

// 그래픽 버퍼 구조체.
typedef struct GraBuffTag {
	DWORD		dwAttr;			// 속성
	DWORD		dwState;		// 상태.
	RectStt		*pR;			// WinStt 구조체에 있는 rect를 공유한다.
	UINT16		*pW;			// 버퍼는 Rect의 크기만큼 동적으로 할당된다.
	MaskStt		self_mask;		// 자체적으로 가지고 있는 비트 마스크.
} GraBuffStt;

// 윈도우 구조체.
struct WinTag {
	GuiObjStt				obj;						// GUI OBJECT
	RectStt					ct_r;						// Client 영역의 좌표, 크기  (스크린 내 좌표)
	RectStt					client_r;					// CLIENT 영역의 좌표 크기.  (윈도우 내 좌표)
	DWORD					dwID;						// 윈도우 ID
	struct WinTag			*pParentWin;				// Parent Window
	DWORD					dwMainIconID;				// 타이틀 바에 들어갈 Main Icon ID
	struct WinStyleTag		*pWStyle;					// 윈도우 Style
	WMFuncStt				*pWMArray;					// 각 메시지별 디폴트 핸들러
	WMesgQStt				wmq;						// 윈도우 메시지 큐
	struct WinTag			*pPrev, *pNext;				// 윈도우 쓰레드의 링크
	struct WinThreadTag		*pWThread;					// 윈도우가 속한 쓰레드
	char					szTitle[MAX_WINDOW_TITLE];	// 윈도우 타이틀
	void					*pPrivate;					// 윈도우 자신의 임의 데이터
	void					*pStylePrivate;				// 윈도우 스타일의 Private
	struct WinTag			*pPreLevel, *pNextLevel;	// 레벨 링크
	struct MaskTag			mask;						// 비트 마스크
	GraBuffStt				gb;							// 그래픽 버퍼.
	struct WinTag			*pModalWin;					// 현재 윈도우에 대한 모달 윈도우(메시지 박스)
	int						nOrgCursorIndex;			// 커서가 변경된 경우의 원래 커서 INDEX.
};
typedef struct WinTag WinStt;									

// 하나의 쓰레드에 의해 관리되는 윈도우들 모아놓은 구조체.
struct WinThreadTag {
	ThreadStt				*pThread;					// 메시지 핸들링을 위한 쓰레드 
	int						nTotal;						// 연결된 윈도우의 개수
	WinStt					*pStart, *pEnd;				// 시작과 끝 윈도우
	EventStt				*pE;						// 메시지가 들어왔을 때 이벤트를 받게 된다. 
	BYTE					byThreadCreated;			// 쓰레드가 새로 생성됨.
};
typedef struct WinThreadTag WinThreadStt;

// 윈도우 프레임 규격
typedef struct {
	int			nFrameWidth;
	int			nTitleV;
} WinFrameStt;

#define MAX_WIN_STYLE_NAME		16


// Predefined window style
struct WinStyleTag{
	char				szName[MAX_WIN_STYLE_NAME];
	DWORD				dwID;							// 윈도우 스타일의 ID
	GuiObjFuncStt		*pWMArray;						// 각 메시지별 디폴트 핸들러
	WinFrameStt			frame;
	UINT16				wDefaultBkColor;				// 기본 배경색.
};
typedef struct WinStyleTag WinStyleStt;

// System Predefined window style
typedef struct {
	int				nTotal;
	WinStyleStt		*ptr[MAX_PREDEF_WIN_STYLE];
} SysWinStyleStt;

extern int 		disp_wmesg_list				();
extern int		alloc_gra_buff				( WinStt *pWin );
extern int		alloc_gra_buff_ex			( GraBuffStt *pGB, RectStt *pR );
extern int		free_bit_mask				( WinStt *pWin );
extern int		free_gra_buff				( WinStt *pWin );
extern int		free_gra_buff_ex			( GraBuffStt *pGB );
extern int		recalc_bit_mask				();
extern int		init_predef_winstyle		();
extern int		is_win_show					( WinStt *pWin );
extern int		kclose_window				( WinStt *pW );
extern int		is_top_window				( WinStt *pWin );
extern int		set_top_window				( WinStt *pWin );
extern int		alloc_bit_mask				( WinStt *pWin );
extern int		minimize_window				( WinStt *pWin );
extern int		append_win_to_scr			( WinStt *pWin );
extern int		recalc_client_area			( WinStt *pWin );
extern int		show_window					( WinStt *pWIn, int nShow );
extern int		kclose_win_thread			( WinThreadStt *pWThread );
extern int		flush_gra_buff				( WinStt *pWin, RectStt *pR );
extern int		set_window_title			( WinStt *pWin, char *pTitle );
extern int		in_window_area				( WinStt *pWin, int nX, int nY );
extern int		insert_win_to_scr			( WinStt *pParent, WinStt *pWin );
extern int		client_to_win_pos			( WinStt *pWin, int *pX, int *pY );
extern int		internal_alloc_bit_mask		( MaskStt *pMask, RectStt *pRect );
extern int		append_win_to_thread		( WinThreadStt *pWThread, WinStt *pWin );
extern int		screen_to_win				( RectStt *pResult, RectStt *pBaseWin, RectStt *pR );
extern int		client_to_screen			( RectStt *pResult, RectStt *pBaseWin, RectStt *pR );
extern int		kpost_message				( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB );
extern int		ksend_message				( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB );
extern int		kforward_message			( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB );
extern int		call_message_func			( WinStt *pWin, DWORD dwWMesgID, DWORD dwParamA, DWORD dwParamB );
extern int		scroll_gb					( WinStt *pWin, RectStt *pR, int nScrollVPixel, RectStt *pClearR, UINT16 wClearColor );
extern int 		call_win_message_handler	( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB );
extern int 		pop_win_mesg				( WMesgQStt *pWMQ, WMesgStt *pWM );	
extern int 		copy_gra_buff				( GraBuffStt *pDestGB, GraBuffStt *pSrcGB, int nDestX, int nDestY );
extern int 		copy_gra_buff_ex			( GraBuffStt *pDestGB, int nDestX, int nDestY, GraBuffStt *pSrcGB, RectStt *pSrcR );

extern DWORD 	call_pre_window_func		( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB );
extern DWORD 	call_post_window_func		( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB );
extern DWORD 	call_pre_style_func			( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB );
extern DWORD	call_post_style_func 		( WinStt *pWin, DWORD dwMesgID, DWORD dwParamA, DWORD dwParamB );
extern DWORD 	get_wmesg_value				( char *pMesgStr );

extern void		inner_pos					( RectStt *pR, int nX, int nY, int *pNewX, int *pNewY );
extern WinStt	*find_window_by_pos			( int nX, int nY );
extern WinStt	*kcreate_window				( WinThreadStt *pWThread, WinStt *pParentWin, DWORD dwPredefStyleID, RectStt *pRect,
												WMFuncStt *pWMArray, DWORD dwState, DWORD dwAttr,DWORD dwMainIconID, DWORD dwParamA, DWORD dwParamB );
extern WinStt	*find_window_by_id			( DWORD dwID );

extern char 		*get_wmesg_str			( DWORD dwID );
extern RectStt 		*get_client_rect		( WinStt *pWIn, RectStt *pR );
extern WinThreadStt *kcreate_win_thread		( ThreadStt *pT );
extern WinStyleStt  *find_predef_wstyle		( DWORD dwID );
extern WMESG_FUNC 	find_r0_wmesg_func		( WMFuncStt *pArray, DWORD dwID );
extern WMESG_FUNC	find_r3_wmesg_func		( WMFuncStt *pArray, DWORD dwID );


#endif
