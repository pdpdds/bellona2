#ifndef BLOCK_DEVICE_DRIVER_HEADER_jj
#define BLOCK_DEVICE_DRIVER_HEADER_jj
typedef enum {
	BLKDEV_IOCTL_FLUSH	 = 1,					// ���۸��� �� ��� Dirty üũ�� ���� �ٽ� ����Ѵ�.
	BLKDEV_IOCTL_INVALIDATE,					// ���۸��ƴ� ��� �����͸� �׳� ������.
	BLKDEV_IOCTL_RESET,		        			// ����̽� ��ü�� ���½�Ų��.

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
	unsigned long			dwIndex;			// Block ��ȣ
	int						nBlocks;			// Block ����
	int						nError;				// �����ڵ�
	char					*pBuff;				// io ����
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
	char					szName[32];			// ����̽� ����̹� �̸�
	int						nMajor;				// Major Version
	int						nBlkSize;			// ��� ������
	BlkDevOPStt				op;					// ��� ����̽� ����̹� Operaion
};
typedef struct BlkDevTag BlkDevStt;

#define BLKDEV_ATTR_REMOVABLE	1				// block device object is based on removable block device
#define BLKDEV_ATTR_NESTED		2				// block device object is opened from other block device object

// Block Device Driver Object structure
typedef struct BlkDevObjTag {
	int			 			nMinor;				// ��� ����̽� ����̹��� minor number
	int			 			nVolume;	    	// ���� ��ȣ
	int			 			nAttr;		    	// BLKDEV_ATTR_REMOVABLE, BLKDEV_ATTR_NESTED
	BlkDevStt	 			*pDev;		    	// ��� ����̽� ����̹� ����ü
	void		 			*pPtr;		    	// ���µ� ��ü�� ���� ������  
	DWORD 					dwTotalBlk;			// ��� ����
	int			 			nBlkSize;	    	// ��� ũ��
	struct BlkDevObjTag		*pPre, *pNext;		// ��� ��� ����̽� ��ü�� ����Ʈ�� ����ȴ�.
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

// ��� ����̽� ����̹��� ��� �����Ѵ�.
extern int unregister_blkdev( int nMajor );

// ��� ����̽� ����̹��� ����Ѵ�.
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
