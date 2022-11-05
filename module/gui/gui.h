#ifndef BELLONA2_GUI_HEADER_20020516
#define BELLONA2_GUI_HEADER_20020516

#include "wconst.h"
#include "resource.h"
#include "winres.h"
#include "mpointer.h"
#include "bitmap.h"
#include "win.h"
#include "button.h"
#include "simplew.h" 
#include "flatw.h"
#include "framew.h"
#include "about.h"
#include "taskbar.h"
#include "kcmdwin.h"
#include "draw.h"
#include "font.h"
#include "iconwin.h"
#include "popup.h"
#include "mbox.h"
#include "wall.h"
#include "menu.h"
#include "syscolor.h"
#include "guiexp.h"

#define WALL_TYPE_TILE				1		// ���� ����
#define WALL_TYPE_CENTER			2
#define WALL_TYPE_UP_LEFT			3
#define WALL_TYPE_FILL_COLOR		4
#define WALL_TYPE_WALL_PAPER		5		// ȭ�� ��ü ũ���� Wall paper

#define GUI_THREAD_NAME			"gui_thread"

typedef enum {
	ECLIP_UP_RIGHT = 0,
	ECLIP_UP_LEFT,
	ECLIP_DOWN_RIGHT,
	ECLIP_DOWN_LEFT,
	ECLIP_UP_CENTER,
	ECLIP_DOWN_CENTER,
	ECLIP_RIGHT_CENTER,
	ECLIP_LEFT_CENTER
} ECLIP_TAG;

#define GUI_MOVE_MODE    1
#define GUI_RESIZE_MODE	 2

// ��üȭ�� (����ȭ�� ������)
typedef struct {
	VESAModeStt			vmode;				// ���� ������ VESA MODE
	DWORD				dwMemSize;			// �������� ���� �޸� ũ��
	DWORD				dwMemAddr;			// �������� ���� �޸� �ּ�
	DWORD				dwWallType;			// ��� ���̸� �׸��� ���
	UINT16				wcolor[2];			// ���� 2 ����
	struct ImageTag		*pWallPaper;		// ��� ����
	int					nLockFlag;			// Lock Flag
	char				szWallPaper[260];	// Wall paper BMP path  

	int					nMoveResizeFlag;	// Move Flag
	int					nResizeEdgeClip;	// Resize Edge Clip
	int					nShowMoveFlag;		// Show Move Flag
	struct WinTag		*pMoveWin;			// Move Win
	int					nMPX, nMPY;			// Move Mode�� ������ ���� ���콺 �������� ��ġ
	RectStt				mr,resize_org_r;	// Move Rect, Resize Original Rect
	
	struct WinTag		wall;				// ����ȭ�� ������
	MousePointerStt		mpointer;			// �ý��� ���콺 ������

	// ���� ���� �ִ� ������� ���� �Ʒ��� �ִ� ������.
	struct WinTag		*pStartLevelWin, *pEndLevelWin;		
} GuiStt;

//extern BELL_EXPORT int change_screen_res( VESAModeStt *pVM );
extern GUI_EXPORT int enter_gui( VESAModeStt *pVM );
extern GUI_EXPORT int leave_gui();

extern GuiStt			*get_gui_stt				();
extern WinResStt		*get_winres_stt				();
extern WinThreadStt		*get_kwin_thread			();
extern void				screen_lock					( int nFlag );

extern int				leave_move_mode				();
extern int 				leave_resize_mode			();
extern int				enter_move_mode				( WinStt *pWin );
extern int				recover_resize_mouse_pointer( WinStt *pWin );
extern int				set_move_rect				( WinStt *pWin );
extern int				show_move_rect				( int nShowFlag );
extern int				move_rect_change_pos		( int nX, int nY );
extern int 				resize_rect_change_shape	( int nX, int nY );
extern int				close_gui_obj				( GuiObjStt *pObj );
extern int				repaint_down_layer			( WinStt *pWIn, RectStt *pR );
extern int				repaint_upper_transparent	( WinStt *pWIn, RectStt *pR );
extern int				is_in_rect					( RectStt *pR, int nX, int nY );
extern int				change_resize_mouse_pointer ( WinStt *pWin, int nEdgeClip );
extern int				enter_resize_mode			( WinStt *pWin, int nEdgeClip );
extern int				init_gui_obj				( GuiObjStt *pOwner, GuiObjStt *pObj, UINT16 wType, GuiObjFuncStt *pWMArray );

extern DWORD 			rect_xy_to_dword			( RectStt *pR );
extern DWORD 			rect_hv_to_dword			( RectStt *pR );	
extern void 			dword_to_rect_hv			( DWORD dwR, RectStt *pR );
extern void 			dword_to_rect_xy			( DWORD dwR, RectStt *pR );



#endif
