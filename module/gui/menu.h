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
	UINT16					wState;					// �޴� ��Ʈ���� ���� ����.
	char					szStr[MAX_MENU_STR];	// �޴��� ǥ�õ� ��Ʈ��.
	DWORD					dwIconID;				// �޴� ������ ǥ���� ������.
	struct	ImageTag		*pImg;					// ������ dwIconID�� �ε��� �̹��� ������.
	struct	MenuTag			*pSubMenu;				// Sub MEnu�� �ִ� ���.
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



