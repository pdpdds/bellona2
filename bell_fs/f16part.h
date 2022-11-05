#ifndef FDD_FAT16_HEADER_jj
#define FDD_FAT16_HEADER_jj

typedef struct F16PartTag {
	int		nRootLoc;
	int		nRootBlocks;
	int		nRootClusters;
	int		nRelocation;
	int		nDataLoc;
	void	*pBaseDevObj;
	
};
typedef struct F16PartTag F16PartStt;

extern int init_fat16_driver();

extern int close_fat16_driver();

#endif
