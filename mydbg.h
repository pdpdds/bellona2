/*
�� ������ COFFDBG���� ������� ����� ������ �ν��ϱ� ���� ����ε�
COFF ���� ������ �����ϰ� �ν����� ���ϰ� ����� ������ ũ�⵵ ���ʿ��ϰ�
ũ�ٴ� ������ �����Ƿ� 1999�� 9�� 26�� COFFDBG2�� NYDBG2.C, MYDBG2.H�� 
��ü�Ǿ���.
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

	int	nMyDbgSize;			// Debug ������ ��üũ��
	int	nTotalFileEnt;		// ���� ��Ʈ���� ����
	int nTotalFuncEnt;		// �Լ� ��Ʈ���� ����
	int	nTotalAsmFuncEnt;	// ASM �Լ� ��Ʈ���� ����
	int nTotalLineEnt;		// ���� ��Ʈ���� ����
	int	nStrTblSize;		// ��Ʈ�� ���̺��� ũ��

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

	// ��Ʈ���� ���� ���̺�
	MDFileEntStt		*pMDFile;
	MDFuncEntStt		*pMDFunc;
	MDAsmFuncEntStt		*pMDAFunc;
	MDLineEntStt		*pMDLine;

	// ��Ʈ�� �ε����� �����ϰ� �ִ� �迭
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
