#ifndef BLOCK_DEVICE_DRIVER_HEADER_jj
#define BLOCK_DEVICE_DRIVER_HEADER_jj
typedef enum {
	BLKDEV_IOCTL_FLUSH	 = 1,					// 버퍼링된 것 가운데 Dirty 체크된 것은 다시 기록한다.
	BLKDEV_IOCTL_INVALIDATE,					// 버퍼링됐던 모든 데이터를 그냥 버린다.
	BLKDEV_IOCTL_RESET,		        			// 디바이스 자체를 리셋시킨다.

	END_OF_BLKDEV_IOCTL
} BLKDEV_IOCTL_TAG;

// block device driver ioctl structure
typedef struct BlkDevIoctlTag {
	int		nFunc;
	//...
} ;
typedef struct BlkDevIoctlTag BlkDevIoctlStt;

// block device driver io structure
typedef struct BlkDevIoTag {
	unsigned long			dwIndex;			// Block 번호
	int						nBlocks;			// Block 개수
	int						nError;				// 에러코드
	char					*pBuff;				// io 버퍼
};
typedef struct BlkDevIoTag BlkDevIOStt;

// block device driver operations
typedef struct BlkDevOpTag {
	int ( *open         )( int nMinor, void *pBlkDevObj, void *pDev );
	int ( *close        )( struct BlkDevObjTag *pBlkDevObj );
	int ( *read         )( struct BlkDevObjTag *pBlkDevObj, void *pIO );
	int ( *write        )( struct BlkDevObjTag *pBlkDevObj, BlkDevIOStt *pIO );
	int ( *ioctl        )( struct BlkDevObjTag *pBlkDevObj, void *pV );
	int (* scatter_read )( struct BlkDevObjTag *pBlkDevObj, struct BCacheEntTag **ppCEntArray, DWORD dwBlock, int nSectors );
};
typedef struct BlkDevOpTag BlkDevOPStt;

// block device driver structure
typedef struct BlkDevTag {
	char					szName[32];			// 디바이스 드라이버 이름
	int						nMajor;				// Major Version
	int						nBlkSize;			// 블록 사이즈
	BlkDevOPStt				op;					// 블록 디바이스 드라이버 Operaion
};
typedef struct BlkDevTag BlkDevStt;

#define BLKDEV_ATTR_REMOVABLE	1				// block device object is based on removable block device
#define BLKDEV_ATTR_NESTED		2				// block device object is opened from other block device object

// Block Device Driver Object structure
typedef struct BlkDevObjTag {
	int			 			nMinor;				// 블록 디바이스 드라이버의 minor number
	int			 			nVolume;	    	// 볼륨 번호
	int			 			nAttr;		    	// BLKDEV_ATTR_REMOVABLE, BLKDEV_ATTR_NESTED
	BlkDevStt	 			*pDev;		    	// 블록 디바이스 드라이버 구조체
	void		 			*pPtr;		    	// 오픈된 객체에 대한 포인터  
	DWORD 					dwTotalBlk;			// 블록 개수
	int			 			nBlkSize;	    	// 블록 크기
	struct BlkDevObjTag		*pPre, *pNext;		// 모든 블록 디바이스 객체는 리스트로 연결된다.
	struct BCacheManTag		*pCache;			// Cache Manager
};
typedef struct BlkDevObjTag  BlkDevObjStt;

#define MAX_BLK_DEV		128
typedef struct BlockDeviceTag {
	int				nTotal;
	BlkDevStt		*ent[ MAX_BLK_DEV ];

	int				nTotalObj;
	BlkDevObjStt	*pObjStart, *pObjEnd;
};
typedef struct BlockDeviceTag BlockDeviceStt;

// 블록 디바이스 드라이버를 등록 해제한다.
extern int unregister_blkdev( int nMajor );

// 블록 디바이스 드라이버를 등록한다.
extern int register_blkdev( BlkDevStt *pB );

extern int reset_block_device( BlkDevObjStt *pObj );
extern int close_block_device( BlkDevObjStt *pObj );
extern int discard_block_device( BlkDevObjStt *pObj );
extern BlkDevObjStt *find_first_blkdev_obj( int *pTotal );
extern BlkDevObjStt *find_dev_obj( int nMajor, int nMinor );

extern int open_block_device( BlkDevObjStt *pObj, int nMajor, int nMinor );
extern int read_block( BlkDevObjStt *pDev, unsigned long dwIndex, char *pBuff, int nBlocks );
extern int write_block( BlkDevObjStt *pDev, unsigned long dwIndex, char *pBuff, int nBlocks );

#endif
