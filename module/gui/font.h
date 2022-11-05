#ifndef BELLONA2_GUI_FONT_20020517
#define BELLONA2_GUI_FONT_20020517

#define BELL_FONT_MAGIC 0x544E4F46			// "FONT"

typedef struct FontTag {
	DWORD					dwMagic;		// BELL_FONT_MAGIC
	DWORD					dwVersion;		// 버전
	unsigned short int		wStartChar;		// 시작 문자 번호
	unsigned short int		wEndChar;		// 끝 문자 번호
	char					cLinByte;		// 라인당 할당된 바이트 수
	char					cBitPerPixel;	// BPP
	char					cH;				// HSize
	char					cV;				// VSize
	union {
		BYTE				*pB;
		unsigned short int	*pW;
		unsigned long		*pD;
	} b;
	char					rsv[12];
} FontStt;

typedef struct {
	int			nID;
	FontStt		*pFont;
} FontTableEntStt;

#define MAX_SYSTEM_FONT	8
typedef struct {
	int					nTotal;
	FontTableEntStt		ent[ MAX_SYSTEM_FONT ];
} FontTableStt;

extern FontStt	*get_system_font	( int nID );

extern int load_system_fonts		();
extern int unload_system_fonts		();
extern int drawtext_xy				( struct GraBuffTag *pGB, int nX, int nY, FontStt *pFont, char *pStr, unsigned short int  wColor, DWORD dwEffect );
extern int drawchar_xy				( GraBuffStt *pGB, int nX, int nY, FontStt *pFont, char ch, UINT16 wColor, DWORD dwEffect );

#endif