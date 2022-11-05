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
	struct ButtonTag		*pBtn;				// 태스크 바에 추가될 아이콘 버튼.
	struct WinTag			*pWin;				// 아이콘이 연결된 실제 윈도우.
	struct PopUpMenuTag		*pPopUpMenu;		// 마우스 오른쪽 버튼을 눌렀을 때의 팝업 메뉴.
	char					szPopUpString[ MAX_POPUP_STRING ];  // 풍선도움말의 최대 길이.
} TBIconStt;

#define MAX_TB_ICON	32
typedef struct {
	RectStt					time_r;				// Taskbar에서 현재 시간을 나타내는 부분.
	struct ButtonTag		*pMenuBtn;			// 메뉴 버튼
	void					*pTimeOutHandle;	// 현재 시간을 갱신하기 위한 타임아웃 핸들.
	
	// 아이콘 버튼 들의 구조체.
	TBIconStt				*pFocusTB;			// 마우스를 가지고 있는 TB
	TBIconStt				tb_icon[MAX_TB_ICON];

	// Menu 버튼을 눌렀을 때 생성할 구조체.
	struct MenuTag			*pMenu;

} TaskBarPrivateStt;

extern int tb_del_icon		( int nID );
extern int tb_add_icon		( int nIconID, char *pPopUpString, struct PopUpMenuTag *pMenu, struct WinTag *pWin );
extern int tb_add_icon_ex	( struct ImageTag *pImg, char *pPopUpString, struct PopUpMenuTag *pMenu, WinStt *pWin );

#endif
