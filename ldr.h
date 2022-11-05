#ifndef BELLONA2_LOADER_HEADER_jj
#define BELLONA2_LOADER_HEADER_jj

#define LDR_BASE_ADDR	0x80000000					// 2GB
#define LDR_TEMP_ADDR	(0x80000000+(128*0x100000))	// 2GB + 128M

typedef struct LDRTag {
	MY_IMAGE_DOS_HEADER			*pDH;
	MY_IMAGE_FILE_HEADER		*pIF;
	MY_IMAGE_OPTIONAL_HEADER	*pIO;
	DWORD						dwSectOffs;		// offset of the first section header
	MY_IMAGE_SECTION_HEADER		*p_sect[16];	// sections
	MY_IMAGE_EXPORT_DIRECTORY	*pExp;			// Export Directory
	MY_IMAGE_IMPORT_DESCRIPTOR  *pImp;			// Import Descriptor

	//MyCoffDbg2Stt				*pDbg;			// 가공된 디버그 정보

	long						lFileSize;		// file size
	int							nTotalPage;		// total used page
	DWORD						dwLoadAddr;		// load address

	int							nTotalDbgPage;	// total debug page
	DWORD						dwAfterRelocDbgAddr;	// debug info address after relocation
	DWORD						dwBeforeRelocDbgAddr;	// debug info address before relocation
};
typedef struct LDRTag LDRStt;

extern DWORD load_pe_file( char *pFileName, DWORD dwLoadAddr, DWORD dwTempAddr, LDRStt *pLDR );

#endif
