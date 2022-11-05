#ifndef BELLONA2_LIB_UAREA_HEADER
#define BELLONA2_LIB_UAREA_HEADER

#include "env.h"
#include "fileapi.h"
#include "signal.h"

extern struct ProcFDTblTag;

#define MAX_CWD		260

// 프로세스의 User Area
typedef struct {
	struct ProcFDTblTag		fdtbl;			// 파일 디스크립터 테이블.
    UEnvStt					env;			// 환경 변수.
	unsigned char			byInitFlag;
	char					szCWD[MAX_CWD];
	SignalStt 				sig;
} UAreaStt;

extern int 			init_uarea		( UAreaStt *pU );
extern int 			close_uarea		();
extern UAreaStt 	*get_uarea		();

#endif
