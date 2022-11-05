#ifndef B2OS_GUI_EXP_HEADER_jj
#define B2OS_GUI_EXP_HEADER_jj

#define MAX_MESGMON_ENT	8

typedef struct {
	DWORD			dwWinID;						// 모니터링할 윈도우 ID
	DWORD			dwStartMesg, dwEndMesg;			// 모니터링할 시작 메시지와 마지막 메시지.
	DWORD			dwCounter;						// 메시지가 발생한 회수.
} MesgMonEntStt;

typedef struct {
	int				nTotal;
	MesgMonEntStt	ent[ MAX_MESGMON_ENT ];
} MesgMonStt;

extern GuiExportStt 	gui_exp;

extern int 				chk_mesgmon			( DWORD dwWinID, DWORD dwMesg );

#endif
