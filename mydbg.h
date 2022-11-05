/*
이 파일은 COFFDBG에서 만들어진 디버그 정보를 인식하기 위한 헤더인데
COFF 파일 포맷을 완전하게 인식하지 못하고 디버그 정보의 크기도 불필요하게
크다는 단점이 있으므로 1999년 9월 26일 COFFDBG2와 NYDBG2.C, MYDBG2.H로 
대체되었다.
*/

#ifndef MY_DEBUG_INFO_HEADER
#define MY_DEBUG_INFO_HEADER

// My Debug Symbol Entry Struct
typedef struct MDFuncEntTag {
    DWORD	dwNameOffs;			// String Table Offser
	DWORD	dwRVA;				// Relative Virtual Address
	DWORD	dwFuncSize;			// Function Size
	int		nTotalLinenumber;	// Total Linenumbers
	int		nLinenumberIndex;	// Index to Linenumber Table
	int		nFileIndex;
};
typedef struct MDFuncEntTag MDFuncEntStt;

// My debug File Entry Struct
typedef struct MDFileEntTag {
	int		nFuncIndex;			// First Symbol Entry Index
	int		nTotalFunc;			// Total Symbol(Function) in this file
	DWORD	dwNameOffs;			// String Table Offser
};
typedef struct MDFileEntTag MDFileEntStt;		

// My debug Linenumber entry Struct
typedef struct MDLineEntTag {
	int		nLinenumber;
	DWORD	dwRVA;
	int		nFuncIndex;
};
typedef struct MDLineEntTag MDLineEntStt;

// My Asm Func struct
typedef struct MDAsmFuncEntTag {
	DWORD	dwRVA;
	DWORD	dwNameOffs;
};
typedef struct MDAsmFuncEntTag MDAsmFuncEntStt;

#define MDB_MAGIC 0x5DB1F0  //5 DBG INFO (??)
// My Debug struct
typedef struct MDbgTag{
	DWORD	dwMagic;		// Magic NUMBER

	int	nMyDbgSize;			// Debug 정보의 전체크기
	int	nTotalFileEnt;		// 파일 엔트리의 개수
	int nTotalFuncEnt;		// 함수 엔트리의 개수
	int	nTotalAsmFuncEnt;	// ASM 함수 엔트리의 개수
	int nTotalLineEnt;		// 라인 엔트리의 개수
	int	nStrTblSize;		// 스트링 테이블의 크기

	int nMDFileOffs;
	int nMDFuncOffs;
	int nMDAsmFuncOffs;
	int nMDLineOffs;
	int nFileNameITblOffs;
	int nFuncNameITblOffs;
	int nFuncAddrITblOffs;
	int nAsmFuncNameITblOffs;
	int nAsmFuncAddrITblOffs;
	int nStrTblOffs;

	// 소트되지 않은 테이블
	MDFileEntStt		*pMDFile;
	MDFuncEntStt		*pMDFunc;
	MDAsmFuncEntStt		*pMDAFunc;
	MDLineEntStt		*pMDLine;

	// 소트된 인덱스를 보관하고 있는 배열
	DWORD				*pFileNameITbl;
	DWORD				*pFuncNameITbl;
	DWORD				*pFuncAddrITbl;
	DWORD				*pAsmFuncNameITbl;
	DWORD				*pAsmFuncAddrITbl;
	char				*pStrTbl;

};
typedef struct MDbgTag MDbgStt;

extern int		nGetMyDbgInfo( UCHAR *pB );
extern MDbgStt	*pGetMD();
extern char		*pGetFuncNameByAddr( DWORD dwAddr, int *pIndex, int *pAsmFunc );

#endif
