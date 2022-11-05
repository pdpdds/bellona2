#ifndef BELLONA2_LIB_UAREA_HEADER
#define BELLONA2_LIB_UAREA_HEADER

#include "env.h"
#include "fileapi.h"
#include "signal.h"

extern struct ProcFDTblTag;

#define MAX_CWD		260

// ���μ����� User Area
typedef struct {
	struct ProcFDTblTag		fdtbl;			// ���� ��ũ���� ���̺�.
    UEnvStt					env;			// ȯ�� ����.
	unsigned char			byInitFlag;
	char					szCWD[MAX_CWD];
	SignalStt 				sig;
} UAreaStt;

extern int 			init_uarea		( UAreaStt *pU );
extern int 			close_uarea		();
extern UAreaStt 	*get_uarea		();

#endif
