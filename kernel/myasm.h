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
	LT_NUMBER,			// ����
	LT_FARMEMORY,		// �տ� FAR�� �پ���. �޸� ���۷���
	LT_MEMORY,			// �޸� ���۷���
	LT_STRING,			// ��Ʈ�� 'ABC'
	LT_CHAR,			// ����	  []()+-*/,
	LT_SEGPRX,			// Segment Prefix
	LT_ABSADDR,			// JMP 110:111000 �� 110:�κ�. (���� �ּ�)
	LT_SIZEPRX,			// Size Prefix
	LT_FAR,				// ���� �״�� FAR
	LT_PTR,				// ���� �״�� PTR

	LT_REG8,			// 8��Ʈ ��������
	LT_REG16,			// 16��Ʈ ��������
	LT_REG32,			// 32��Ʈ ��������
	LT_SEGREG,			// ���׸�Ʈ ��������
	LT_CTRLREG,			// ��Ʈ�� ��������
	LT_DBGREG,			// ����� ��������
	LT_TESTREG,			// �׽�Ʈ ��������
	LT_STKREG,			// ���� ��������
			 	
	END_OF_LT
} LEX_TYPE_TAG;

typedef unsigned long DWORD;

extern RsvSymStt rsvTbl[MAX_RSVSYM];
extern int nMyAsm( char *pStr, unsigned char *pBinCode, DWORD dwEIP );
extern int nInitOpCodeTbl();
extern int nSearchTbl( RsvSymStt *pRTbl, int nMin, int nMax, char *pToken );

#endif





















