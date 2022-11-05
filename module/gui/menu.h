#ifndef BELLONA2_GUI_MENU_H_jj
#define BELLONA2_GUI_MENU_H_jj

#define MAX_MENU_STR		32
#define MENU_MARGIN_V		4
#define MENU_MARGIN_H		10
#define MENU_ENT_V			18
#define MENU_BREAK_V		6
#define MENU_ICON_H			16

#define MENU_BASE_FONT		IDR_BF_BASE11

#define MENU_DK_TEXT_COLOR	RGB16(  60,  60,  60 )
#define MENU_LT_TEXT_COLOR	RGB16( 220, 220, 220 )
#define MENU_BACK_COLOR		RGB16( 203, 175, 214 )
#define MENU_DK_BACK_COLOR	RGB16( 140,  90, 100 )
#define MENU_DK_COLOR		RGB16( 115,  76,  84 )
#define MENU_LT_COLOR		RGB16( 242, 218, 235 )

#define MENU_ENT_ID_BREAK		0xFFFF
#define MENU_ENT_ID_BRANCH		0xFFFE

typedef struct MenuEntTag {
	GuiObjStt				obj;
	UINT16					wID;
	UINT16					wAttr;
	UINT16					wState;					// 메뉴 엔트리의 현재 상태.
	char					szStr[MAX_MENU_STR];	// 메뉴에 표시될 스트링.
	DWORD					dwIconID;				// 메뉴 좌측에 표시할 아이콘.
	struct	ImageTag		*pImg;					// 아이콘 dwIconID를 로드한 이미지 포인터.
	struct	MenuTag			*pSubMenu;				// Sub MEnu가 있는 경우.
} MenuEntStt;

typedef struct MenuTag {
	int						nTotalEnt;				
	MenuEntStt				*pEnt;
	struct WinTag			*pWin;
} MenuStt;

typedef struct {
	MenuStt					*pMenu;
} MenuWinPrivateStt;

extern int close_menu( MenuStt *pMenu );
extern int calc_menu_v( MenuStt *pMenu );
extern int create_menu( MenuStt *pMenu, int nX, int nY, struct WinThreadTag *pWThread, struct WinTag *pParentWin );

#endif



