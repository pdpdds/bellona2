#ifndef BELLONA2_COFF_DBG2_HEADER
#define BELLONA2_COFF_DBG2_HEADER

struct _MY_IMAGE_LINENUMBER;

typedef struct MY_COFF_DBG2_FILE_TAG {
	int				nNameIndex;
	int				nLineIndex;
	int				nTotalLine;
	unsigned long	dwAddr;
	unsigned long	dwSize;
};
typedef struct MY_COFF_DBG2_FILE_TAG MyCoffDbg2FileStt;

typedef struct MY_COFF_DBG2_FUNC_TAG {
	int				nNameIndex;
	int				nFileIndex;
	int				nLineIndex;
	int				nLocalIndex;
	int				nTotalLocal;
	unsigned long	dwAddr;
	unsigned long   dwSize;
};
typedef struct MY_COFF_DBG2_FUNC_TAG MyCoffDbg2FuncStt;

// 함수의 nLocalIndex부터 여러 개의 parameter, local variable이 배열로 등장.
// nEBPAdder가 '-'이면 Local Variable, '+'이면 파러메터로 인식.
typedef struct MY_COFF_DBG2_LOCAL_TAG {
	int				nNameIndex;		// 함수 파러메터나 지역변수 이름 인덱스.  
	int				nEBPAdder;		// EBP에 더하거나 뺄 값.
} MyCoffDbg2LocalStt;

#define MY_COFF_DBG2_MAGIC_STR "COFF"

typedef struct MY_COFF_DBG2_TAG {
	char				szMagicStr[5];			// magic string "COFF"
	DWORD				dwSize;					// debug info size

	MyCoffDbg2FileStt	*pFileTbl;
	int					nTotalFileEnt;
	
	MyCoffDbg2FuncStt	*pFuncTbl;
	int					nTotalFuncEnt;

	int					*pFuncNameIndex;
	int					*pFuncAddrIndex;
								  
	struct _MY_IMAGE_LINENUMBER *pLineTbl;
	int					nTotalLineEnt;

	MyCoffDbg2LocalStt	*pLocalTbl;
	int					nTotalLocalEnt;

	char				*pStrTbl;
	int					nStrTblSize;
};
typedef struct MY_COFF_DBG2_TAG MyCoffDbg2Stt;

extern int is_coff_dbg2( char *pB );
extern MyCoffDbg2Stt *get_coff_dbg2( char *pB );
extern MyCoffDbg2Stt *load_mydbg2_info( char *pFileName );
extern MyCoffDbg2FuncStt *get_func_ent_by_name( MyCoffDbg2Stt *pMy, char *pS );
extern MyCoffDbg2FuncStt *get_func_ent_by_addr( MyCoffDbg2Stt *pMy, DWORD dwAddr );
extern MY_IMAGE_DEBUG_DIRECTORY* find_coff_dbg_info( MY_IMAGE_DEBUG_DIRECTORY *pDbgDir );
extern MyCoffDbg2FuncStt *get_nearest_func_ent_by_addr( MyCoffDbg2Stt *pMy, DWORD dwAddr );
extern int get_file_func_lineno( MyCoffDbg2Stt *pMy, char *pFileName, char *pFuncName, DWORD dwAddr );

#endif
