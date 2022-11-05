#ifndef BELLONA2_GUI_SIMPLEW_20020522
#define BELLONA2_GUI_SIMPLEW_20020522


typedef struct {
	RectStt				title_r;						// Ÿ��Ʋ�� ����
	struct ButtonTag	*pMain, *pMin, *pMax, *pExit;	// Ÿ��Ʋ�� ������
} SimpleWPrivateStt;

#define SIMPLEW_MAIN_ICON_H			16
#define SIMPLEW_MAIN_ICON_V 		16

#define SIMPLEW_FRAME_WIDTH			3
#define SIMPLEW_TITLE_V				16
#define SIMPLEW_FRAME_COLOR			RGB16( 203, 175, 214 )
#define SIMPLEW_TITLE_COLOR			RGB16( 158, 156, 209 )
#define SIMPLEW_TITLE_TEXT_COLOR	RGB16( 255, 255, 255 )

extern WinStyleStt *init_simple_win();

#endif

