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
    // End of BPB (��������� FAT32 BOOTSECTOR�� ���� �κ��̴�.)

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

// fat32 filesystem�� private data structure
typedef struct FAT32PrivateTag{
	int		nClusterSize;
    BYTE    bySectorsPerCluster; 
    WORD    wReservedSectors;	 
    WORD    wSectorsPerTrack;
    WORD    wHeads;
    DWORD   dwBigTotalSectors;
    DWORD   dwBigSectorsPerFat;
    DWORD   dwRootDirStrtCluster;
	
	DWORD	dwFATLoc;				// FAT1�� ���� ���͹�ȣ
	DWORD	dwRootLoc;				// ROOT�� ���� ���͹�ȣ

};
typedef struct FAT32PrivateTag FAT32PrivateStt;
   
/////////////////////////////////////////////////////////////////////
struct FAT32FILE;
typedef struct DirEntInfoTag {
	int					nLongEntOffset;		// long entry�� ������
	int					nShortEntOffset;	// short entry�� ������
	SHORTDIRENT			short_ent;			// short entry ���纻
	char				szLongName[260];	// long file name
	char				szShortName[13];	// short file name
};
typedef struct DirEntInfoTag DirEntInfoStt;

typedef struct FAT32FILE {
	DirEntInfoStt		ei;					// ���丮 ��Ʈ�� ����

	int					nEntryInfoDirty;	// Short Entry�� ���丮�� �ٽ� ����ؾ��Ѵ�. (����ũ�Ⱑ ���ߴ�.)
	int					nTotalCluster;		// ������ �����ϰ� �ִ� ��ü Ŭ������ ��
	DWORD				dwStartCluster;		// ���� Ŭ������ ��ȣ
	DWORD				dwLastCluster;		// ������ Ŭ������ ��ȣ
	DWORD				dwLastAvailable;	// ������ Ŭ�������� ���� ����Ʈ ��

	DWORD				dwCurCluster;		// ���� Ŭ�����͹�ȣ
	int					nClusterOffset;		// ���� Ŭ������ ���� ������
	long				lFileSize;			// ������ ũ��
	long				lFileOffset;		// ���� ���� ������
};
typedef struct FAT32FILE FAT32FileStt;
/////////////////////////////////////////////////////////////////////

extern int  free_fat_fs_struct( void *pVoidVFS );
extern int  fat32_find_dirent( void *VoidpDir, char *pFileName );
extern void *alloc_fat_fs_struct( BlkDevObjStt *pDevObj, DWORD dwType );
extern DWORD fat32_cluster_to_block( FAT32PrivateStt *pPrivate, DWORD dwCluster );							   

#endif
