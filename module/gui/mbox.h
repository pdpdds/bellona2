#ifndef B2_GUI_MBOX_HEADER_jj
#define B2_GUI_MBOX_HEADER_jj

#define MBOX_BACK_COLOR			RGB16( 203, 175, 214 )//RGB16( 100, 255, 255 )
#define MBOX_DK_COLOR			RGB16( 115,  76,  84 )//RGB16( 25,   64,  64 )
#define MBOX_LT_COLOR			RGB16( 242, 218, 235 )//RGB16( 128, 128, 250 )
#define MBOX_BTN_TEXT_COLOR		RGB16(  88,  88,  88 )
#define MBOX_BTN_BORDER_COLOR	RGB16( 220, 220, 220 )

#define MBOX_TITLE_V			20
#define MBOX_FONT				IDR_BF_BASE11
#define MBOX_BUTTON_V			16
#define MBOX_BUTTON_H			40
#define MBOX_MARGIN				10

#define MBOX_BUTTON_OK			0x0001
#define MBOX_BUTTON_YES			0x0002
#define MBOX_BUTTON_NO			0x0004
#define MBOX_BUTTON_CANCEL		0x0008

typedef struct MBoxPrivateTag {
	char				szStr[64];	
	DWORD				dwStyle;
	struct ImageTag		*pMBoxIcon;
	struct ButtonTag	*pExit;
	struct ButtonTag	*pOk, *pYes, *pNo, *pCancel;
	DWORD				dwMBoxID;
} MBoxPrivateStt;


extern WinStt *message_box( WinStt *pParent, DWORD dwMBoxID, char *pStr, char *pTitle, int nH, int nV, DWORD dwStyle );

#endif

