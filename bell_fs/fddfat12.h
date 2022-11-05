#ifndef FDD_FAT12_HEADER_jj
#define FDD_FAT12_HEADER_jj

typedef struct FDDFat12Tag {
	int		nRootLoc;
	int		nRootBlocks;
	char	*pRootDir;

	int		nFatLoc;
	int		nFatBlocks;
	int		nFatEntries;
	char	*pFat;
	
	int		nRelocation;
	
	void	*pBaseDevObj;
	
};
typedef struct FDDFat12Tag FDDFat12Stt;

extern int init_fdd_fat12_driver();

extern int close_fdd_fat12_driver();

#endif
