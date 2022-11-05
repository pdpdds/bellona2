#ifndef BELLONA2_CHARDEV_HEADER_jj
#define BELLONA2_CHARDEV_HEADER_jj

// character device driver operations
typedef struct CharDevOpTag {
	int ( *open   )( int nMinor, void *pCharDevObj, void *pDev );
	int ( *close  )( void *pCharDevObj );
	int ( *read   )( void *pCharDevObj );
	int ( *write  )( void *pCharDevObj, UCHAR *pB, int nSize );
	int ( *ioctl  )( void *pCharDevObj, void *pV );
};
typedef struct CharDevOpTag CharDevOPStt;

// character device driver structure
typedef struct CharDevTag {
	char			szName[32];			// device drivaer name
	int				nMajor;				// Major Version
	CharDevOPStt	op;					// character device driver operations
};
typedef struct CharDevTag CharDevStt;

// character device driver object structure
typedef struct CharDevObjTag {
	int			nMinor;		// minor number
	CharDevStt	*pDev;		// character device pointer
	void		*pPtr;		// privater data
	struct CharDevObjTag	*pPre, *pNext;
};
typedef struct CharDevObjTag  CharDevObjStt;

#define MAX_CHAR_DEV		128
typedef struct CharDeviceTag {
	int				nTotal;
	CharDevStt		*ent[ MAX_CHAR_DEV ];

	int				nTotalObj;
	CharDevObjStt	*pObjStart, *pObjEnd;
};
typedef struct CharDeviceTag CharDeviceStt;
								 
extern int init_char_dev();
extern int register_chardev( CharDevStt *pC );
extern int unregister_chardev( CharDevStt *pC );
extern int read_char_device( CharDevObjStt *pObj );
extern int close_char_device( CharDevObjStt *pObj );
extern int write_char_device( CharDevObjStt *pObj, UCHAR *pB, int nSize );
extern CharDevObjStt *find_first_chardev_obj( int *pTotal );
extern int open_char_device( CharDevObjStt *pObj, int nMajor, int nMinor );

#endif
