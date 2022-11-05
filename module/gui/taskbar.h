#ifndef BELLONA2_GUI_TASK_BAR_HEADER_JJ
#define BELLONA2_GUI_TASK_BAR_HEADER_JJ

#define TBICON_H				20
#define TASKBAR_V				24
#define TASKBAR_MENU_H			50
#define TASKBAR_MENU_LT_COLOR	RGB16( 255, 255, 255)
#define TASKBAR_MENU_DK_COLOR	RGB16( 25, 25, 25)

#define TASKBAR_TIME_H			70

#define TASKBAR_COLOR			RGB16( 128, 128, 214 )
#define TASKBAR_UPLINE_COLOR	RGB16( 200, 200, 214 )
#define TASKBAR_FONT_COLOR		RGB16( 200, 200, 200 )

#define TBICON_BUTTON_H	  20
#define TBICON_BUTTON_V	  20

#define MAX_POPUP_STRING  64
typedef struct {
	struct ButtonTag		*pBtn;				// �½�ũ �ٿ� �߰��� ������ ��ư.
	struct WinTag			*pWin;				// �������� ����� ���� ������.
	struct PopUpMenuTag		*pPopUpMenu;		// ���콺 ������ ��ư�� ������ ���� �˾� �޴�.
	char					szPopUpString[ MAX_POPUP_STRING ];  // ǳ�������� �ִ� ����.
} TBIconStt;

#define MAX_TB_ICON	32
typedef struct {
	RectStt					time_r;				// Taskbar���� ���� �ð��� ��Ÿ���� �κ�.
	struct ButtonTag		*pMenuBtn;			// �޴� ��ư
	void					*pTimeOutHandle;	// ���� �ð��� �����ϱ� ���� Ÿ�Ӿƿ� �ڵ�.
	
	// ������ ��ư ���� ����ü.
	TBIconStt				*pFocusTB;			// ���콺�� ������ �ִ� TB
	TBIconStt				tb_icon[MAX_TB_ICON];

	// Menu ��ư�� ������ �� ������ ����ü.
	struct MenuTag			*pMenu;

} TaskBarPrivateStt;

extern int tb_del_icon		( int nID );
extern int tb_add_icon		( int nIconID, char *pPopUpString, struct PopUpMenuTag *pMenu, struct WinTag *pWin );
extern int tb_add_icon_ex	( struct ImageTag *pImg, char *pPopUpString, struct PopUpMenuTag *pMenu, WinStt *pWin );

#endif
