#ifndef B2OS_GUI_EXP_HEADER_jj
#define B2OS_GUI_EXP_HEADER_jj

#define MAX_MESGMON_ENT	8

typedef struct {
	DWORD			dwWinID;						// ����͸��� ������ ID
	DWORD			dwStartMesg, dwEndMesg;			// ����͸��� ���� �޽����� ������ �޽���.
	DWORD			dwCounter;						// �޽����� �߻��� ȸ��.
} MesgMonEntStt;

typedef struct {
	int				nTotal;
	MesgMonEntStt	ent[ MAX_MESGMON_ENT ];
} MesgMonStt;

extern GuiExportStt 	gui_exp;

extern int 				chk_mesgmon			( DWORD dwWinID, DWORD dwMesg );

#endif
