#ifndef MY_ASM_GLOBAL_HEADER
#define MY_ASM_GLOBAL_HEADER

//#include <stdio.h>
//#include <stdlib.h>
//#include <io.h>
//#include <string.h>

#define MAX_RSVSYM            62

#define MAX_LEX               64
#define TOKEN_SIZE            128
#define ONEBYTE_OPTBL_SIZE    256
#define TWOBYTE_OPTBL_SIZE    89
	   
struct LexStt{
	int				nType;
	int				nSubType;
	unsigned long	dwValue;
};
typedef struct LexStt LexStt;

struct RsvSymTag{
	char *pStr;
	int  nType;
	int  nSubType;
};
typedef struct RsvSymTag RsvSymStt;

typedef enum {
	LT_UNKNOWN	= 0,
	
	LT_OPCODE,			// OPCODE
	LT_NUMBER,			// 숫자
	LT_FARMEMORY,		// 앞에 FAR가 붙었다. 메모리 오퍼랜드
	LT_MEMORY,			// 메모리 오퍼랜드
	LT_STRING,			// 스트링 'ABC'
	LT_CHAR,			// 문자	  []()+-*/,
	LT_SEGPRX,			// Segment Prefix
	LT_ABSADDR,			// JMP 110:111000 의 110:부분. (절대 주소)
	LT_SIZEPRX,			// Size Prefix
	LT_FAR,				// 글자 그대로 FAR
	LT_PTR,				// 글자 그대로 PTR

	LT_REG8,			// 8비트 레지스터
	LT_REG16,			// 16비트 레지스터
	LT_REG32,			// 32비트 레지스터
	LT_SEGREG,			// 세그먼트 레지스터
	LT_CTRLREG,			// 컨트롤 레지스터
	LT_DBGREG,			// 디버그 레지스터
	LT_TESTREG,			// 테스트 레지스터
	LT_STKREG,			// 스택 레지스터
			 	
	END_OF_LT
} LEX_TYPE_TAG;

typedef unsigned long DWORD;

extern RsvSymStt rsvTbl[MAX_RSVSYM];
extern int nMyAsm( char *pStr, unsigned char *pBinCode, DWORD dwEIP );
extern int nInitOpCodeTbl();
extern int nSearchTbl( RsvSymStt *pRTbl, int nMin, int nMax, char *pToken );

#endif





















