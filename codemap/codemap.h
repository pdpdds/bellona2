#ifndef BELLONA2_CODEMAP_HEADER_jj
#define BELLONA2_CODEMAP_HEADER_jj

#include <windows.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "..\kernel\pefile.h"
#include "..\kernel\mydbg2.h"

#define CM_PRINTF			printf
#define CM_MALLOC			malloc
#define CM_FREE				free

#define CM_DEFAULT_ENT		128

struct HashEntTag {
	int					nSymIndex;			// 심볼 인덱스를 가리킨다.
	struct HashEntTag	*pNext;				// 다음 해시 엔트리의 포인터 (충돌난 경우의 링크)
};
typedef struct HashEntTag HashEntStt;

#define MAX_HASH_ENT	8192		

typedef struct {
	MyCoffDbg2Stt	d;
	int				nMaxFileEnt;			// 현재 할당되어 있는 파일 엔트리의 개수
	int				nMaxFuncEnt;			// 현재 할당되어 있는 함수 엔트리의 개수
	int				nMaxLineEnt;			// 현재 할당되어 있는 라인 엔트리의 개수
	int				nMaxLocalEnt;			// 현재 할당되어 있는 로컬 엔트리의 개수
	int				nMaxStrTblSize;			// 현재 할당되어 있는 심볼 버퍼의 크기
	int				nTotalProcessedFiles;	// 처리된 총 파일 수 (ASM의 OBJ는 COD 파일이 없다.)
	int				nTotalUselessFunction;	// COD 파일에는 있지만 MAP 파일에는 등장하지 않은(LINK되지 않은) 함수
	
	int				*pSymIndex;				// 심볼의 인덱스
	int				nTotalSymIndex;			// 입력된 심볼 인덱스 개수
	int				nMaxSymIndex;			// 빔볼 인덱스 엔트리의 개수

	HashEntStt		*hash_index[8192];		// 해시 인덱스
	int				nTotalCollision;		// 충돌난 개수	
	int				nMaxCollision;			// 최대로 긴 충돌 개수
	DWORD			dwLinkerBaseAddr;		// 링커가 가정한 베이스 주소

} CodeMapStt;
							 
extern int codemap( char *pExeFile, char *pDbgFile );

#endif