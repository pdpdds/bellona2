#ifndef BELLONA2_WINDOW_HEADER_20020521
#define BELLONA2_WINDOW_HEADER_20020521

// �������� ����
#define	WIN_STATE_NORMAL		0x0000
#define WIN_STATE_MINIMIZED		0x0001
#define WIN_STATE_HIDDEN		0x0002
#define WIN_STATE_INITIALIZED	0x8000

// ������ �Ӽ�
#define WIN_ATTR_TRANSPARENT	0x0100
#define WIN_ATTR_NONMOVABLE		0x0200			// �޴� ������� ���콺�� �Ű����� �ʵ���...

// ������ �ʱ�ȭ �Լ� 
// kcreate_window�� ���ڷ� ���޵Ǿ� ȣ��ȴ�. 
// �� �Լ� �ȿ��� �ش� �������� Message Handler Array���� ������ �� 
// WMESG_CREATE ���� ������.
typedef DWORD (*WINIT_FUNC)( struct WinTag *pWin, DWORD dwParam );

// �� �����찡 ������ �� �ִ� �޽����� ����
#define MAX_WMESG_IN_Q	32

// ������ �޽��� ť
typedef struct {
	int			nTotal;
	int			nStart, nEnd;
	WMesgStt	q[ MAX_WMESG_IN_Q ];
} WMesgQStt;

#define MASK_FLAG_BLACK		1	// ��� ����ũ ��Ʈ�� 0�̾ �׸� ���� ���� ���.
#define MASK_FLAG_WHITE		2	// ��� ����ũ ��Ʈ�� 1�̾ ����ũ ���� ��� �׸��� �Ǵ� ���.

struct MaskTag {
	RectStt	*pR;			// WinStt ����ü�� �ִ� rect�� �����Ѵ�.
	BYTE	*pB;			// ����ũ ����
	DWORD	dwLineBytes;	// �� ������ �����ϴ� ����Ʈ ��
	DWORD	dwSize;			// pB�� ��ü ũ��
	BYTE	byFlag;			// ����ũ�� �÷��� (��� 0 �Ǵ� ��� 1)
};
typedef struct MaskTag MaskStt;

// �׷��� ���� ����ü.
typedef struct GraBuffTag {
	DWORD		dwAttr;			// �Ӽ�
	DWORD		dwState;		// ����.
	RectStt		*pR;			// WinStt ����ü�� �ִ� rect�� �����Ѵ�.
	UINT16		*pW;			// ���۴� Rect�� ũ�⸸ŭ �������� �Ҵ�ȴ�.
	MaskStt		self_mask;		// ��ü������ ������ �ִ� ��Ʈ ����ũ.
} GraBuffStt;

// ������ ����ü.
struct WinTag {
	GuiObjStt				obj;						// GUI OBJECT
	RectStt					ct_r;						// Client ������ ��ǥ, ũ��  (��ũ�� �� ��ǥ)
	RectStt					client_r;					// CLIENT ������ ��ǥ ũ��.  (������ �� ��ǥ)
	DWORD					dwID;						// ������ ID
	struct WinTag			*pParentWin;				// Parent Window
	DWORD					dwMainIconID;				// Ÿ��Ʋ �ٿ� �� Main Icon ID
	struct WinStyleTag		*pWStyle;					// ������ Style
	WMFuncStt				*pWMArray;					// �� �޽����� ����Ʈ �ڵ鷯
	WMesgQStt				wmq;						// ������ �޽��� ť
	struct WinTag			*pPrev, *pNext;				// ������ �������� ��ũ
	struct WinThreadTag		*pWThread;					// �����찡 ���� ������
	char					szTitle[MAX_WINDOW_TITLE];	// ������ Ÿ��Ʋ
	void					*pPrivate;					// ������ �ڽ��� ���� ������
	void					*pStylePrivate;				// ������ ��Ÿ���� Private
	struct WinTag			*pPreLevel, *pNextLevel;	// ���� ��ũ
	struct MaskTag			mask;						// ��Ʈ ����ũ
	GraBuffStt				gb;							// �׷��� ����.
	struct WinTag			*pModalWin;					// ���� �����쿡 ���� ��� ������(�޽��� �ڽ�)
	int						nOrgCursorIndex;			// Ŀ���� ����� ����� ���� Ŀ�� INDEX.
};
typedef struct WinTag WinStt;									

// �ϳ��� �����忡 ���� �����Ǵ� ������� ��Ƴ��� ����ü.
struct WinThreadTag {
	ThreadStt				*pThread;					// �޽��� �ڵ鸵�� ���� ������ 
	int						nTotal;						// ����� �������� ����
	WinStt					*pStart, *pEnd;				// ���۰� �� ������
	EventStt				*pE;						// �޽����� ������ �� �̺�Ʈ�� �ް� �ȴ�. 
	BYTE					byThreadCreated;			// �����尡 ���� ������.
};
typedef struct WinThreadTag WinThreadStt;

// ������ ������ �԰�
typedef struct {
	int			nFrameWidth;
	int			nTitleV;
} WinFrameStt;

#define MAX_WIN_STYLE_NAME		16


// Predefined window style
struct WinStyleTag{
	char				szName[MAX_WIN_STYLE_NAME];
	DWORD				dwID;							// ������ ��Ÿ���� ID
	GuiObjFuncStt		*pWMArray;						// �� �޽����� ����Ʈ �ڵ鷯
	WinFrameStt			frame;
	UINT16				wDefaultBkColor;				// �⺻ ����.
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
