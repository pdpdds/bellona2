#ifndef B2OS_SYS_COLOR_HEADER_jj
#define B2OS_SYS_COLOR_HEADER_jj

#define COLOR_MENU_BACK RGB16( 203, 175, 214 )

typedef struct SysColorEntTag {
	int 		nIndex;
	UINT16		wColor;
} SysColorEntStt;

extern GUI_EXPORT UINT16 get_sys_color( int nIndex );

#endif

