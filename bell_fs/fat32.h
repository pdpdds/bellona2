#ifndef FAT32_HEADER_jj
#define FAT32_HEADER_jj          

typedef struct _BOOTSECT {
    BYTE    byJump[3];      // jmp instruction
    char    byOemName[8];   // OEM name and version

    // This portion is the BPB (BIOS Parameter Block)
    WORD    wBytesPerSector;     
    BYTE    bySectorsPerCluster; 
    WORD    wReservedSectors;	 
    BYTE    byNumberOfFATs;      
    WORD    wRootEntries;        
    WORD    wTotalSectors;       
    BYTE    byMediaDescriptor;   
    WORD    wSectorsPerFAT;      
    WORD    wSectorsPerTrack;    
    WORD    wHeads;              
    DWORD   dwHiddenSectors;     
    DWORD   dwBigTotalSectors;   
    // End of BPB (여기까지가 FAT32 BOOTSECTOR와 같은 부분이다.)

    BYTE    byDriveNumber;      // 80h if first hard drive
    BYTE    byReserved;
    BYTE    byBootSignature;    // 29h if extended boot-signature record
    DWORD   dwVolumeID;         // volume ID number
    char    bsVolumeLabel[11];  // volume label
    char    bsFileSysType[8];   // file-system type (FAT12 or FAT16)
};
typedef struct _BOOTSECT BOOTSECTOR;


typedef struct _BOOTSECT32
{
    BYTE    byJmpCode[3];        // jmp instruction
    char    oemName[8];          // OEM name and version

    WORD    wBytesPerSector;
    BYTE    bySectorsPerCluster; 
    WORD    wReservedSectors;	 
    BYTE    byNumberOfFATs;
    WORD    wRootEntries;
    WORD    wTotalSectors;
    BYTE    byMediaDescriptor;
    WORD    wSectorsPerFAT;
    WORD    wSectorsPerTrack;
    WORD    wHeads;
    DWORD   dwHiddenSectors;
    DWORD   dwBigTotalSectors;
    DWORD   dwBigSectorsPerFat;
    WORD    wExtFlags;
    WORD    wFS_Version;
    DWORD   dwRootDirStrtClus;
    WORD    wFSInfoSec;
    WORD    wBkUpBootSec;
    WORD    wReserved[6];

    BYTE    byDriveNumber;       // 80h if first hard drive
    BYTE    byReserved;
    BYTE    byBootSignature;     // 29h if extended boot-signature record
    DWORD   dwVolumeID;          // volume ID number
    char    volumeLabel[11];     // volume label
    char    fileSysType[8];      // file-system type (FAT12 or FAT16)

	char	rsv[512-90];
};
typedef struct _BOOTSECT32 BOOTSECTOR32;                

typedef struct _SHORTDIRENT {
	char			name[8];
	char			ext[3];
	BYTE			byAttr;
	char			rsv[6];
	unsigned short	wLastAccessDate;
	unsigned short	wEAHandle;
	unsigned short  wCreateTime;
	unsigned short  wCreateDate;
	unsigned short  wStartCluster;
	DWORD			dwFileSize;
};
typedef struct _SHORTDIRENT SHORTDIRENT;

typedef struct _LONGDIRENT {
	char			sequence;
	char			name[5*2];
	BYTE			byAttr;
	BYTE			byType;
	BYTE			byChkSum;
	char			name2[6*2];
	unsigned short	wZero;
	char			name3[2*2];
};
typedef struct _LONGDIRENT LONGDIRENT;

typedef struct FAT32DirEntTag {
	union {
		SHORTDIRENT	s;
		LONGDIRENT	l;
	} u;
};
typedef struct FAT32DirEntTag FAT32DirEntStt;	   

// fat32 filesystem의 private data structure
typedef struct FAT32PrivateTag{
	int		nClusterSize;
    BYTE    bySectorsPerCluster; 
    WORD    wReservedSectors;	 
    WORD    wSectorsPerTrack;
    WORD    wHeads;
    DWORD   dwBigTotalSectors;
    DWORD   dwBigSectorsPerFat;
    DWORD   dwRootDirStrtCluster;
	
	DWORD	dwFATLoc;				// FAT1의 시작 섹터번호
	DWORD	dwRootLoc;				// ROOT의 시작 섹터번호

};
typedef struct FAT32PrivateTag FAT32PrivateStt;
   
/////////////////////////////////////////////////////////////////////
struct FAT32FILE;
typedef struct DirEntInfoTag {
	int					nLongEntOffset;		// long entry의 오프셋
	int					nShortEntOffset;	// short entry의 오프셋
	SHORTDIRENT			short_ent;			// short entry 복사본
	char				szLongName[260];	// long file name
	char				szShortName[13];	// short file name
};
typedef struct DirEntInfoTag DirEntInfoStt;

typedef struct FAT32FILE {
	DirEntInfoStt		ei;					// 디렉토리 엔트리 정보

	int					nEntryInfoDirty;	// Short Entry를 디렉토리에 다시 기록해야한다. (파일크기가 변했다.)
	int					nTotalCluster;		// 파일을 구성하고 있는 전체 클러스터 수
	DWORD				dwStartCluster;		// 시작 클러스터 번호
	DWORD				dwLastCluster;		// 마지막 클러스터 번호
	DWORD				dwLastAvailable;	// 마지막 클러스터의 가용 바이트 수

	DWORD				dwCurCluster;		// 현재 클러스터번호
	int					nClusterOffset;		// 현재 클러스터 내의 오프셋
	long				lFileSize;			// 파일의 크기
	long				lFileOffset;		// 현재 파일 포인터
};
typedef struct FAT32FILE FAT32FileStt;
/////////////////////////////////////////////////////////////////////

extern int  free_fat_fs_struct( void *pVoidVFS );
extern int  fat32_find_dirent( void *VoidpDir, char *pFileName );
extern void *alloc_fat_fs_struct( BlkDevObjStt *pDevObj, DWORD dwType );
extern DWORD fat32_cluster_to_block( FAT32PrivateStt *pPrivate, DWORD dwCluster );							   

#endif
