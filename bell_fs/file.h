#ifndef VFS_FILE_HEADER_jj
#define VFS_FILE_HEADER_jj

#define FM_READ		1
#define FM_WRITE	2

#define FT_RDONLY		0x01
#define FT_HIDDEN		0x02
#define FT_SYSTEM		0x04
#define FT_DIRECTORY	0x10
#define FT_ARCHIVE		0x20
			  
// 128 * 128 = 16384���� ������ ������ �� �ִ�. 
#define MAX_FD_CHUNK			128
#define MAX_FD_PER_CHUNK		128

struct VNodeTag;

// file descriptor structure
typedef struct FDTag {
	struct VNodeTag	*pVNode;			// ���� VNode
	DWORD			dwType;				// ���� Ÿ��
	DWORD			dwMode;				// ������ ���µ� ���
	DWORD			dwOffs;				// ���� ������
	int				nRefCount;			// Reference Count
};
typedef struct FDTag FDStt;

// file descriptor chunk
typedef struct FDChunkTag {
	int		nTotalFD;
	FDStt	fd[ MAX_FD_PER_CHUNK ];
};
typedef struct FDChunkTag FDChunkStt;

// system file descriptor table
typedef struct SysFDManTag {
	int			nTotalChunk;
	FDChunkStt	*pFDChunk[MAX_FD_CHUNK];
};
typedef struct SysFDManTag SysFDManStt;

#define FSEEK_SET	0
#define FSEEK_CUR	1
#define FSEEK_END	2

// ���丮 ��Ʈ�� ����ü
typedef struct DIRENTTag {
	char	szFileName[260];	// long file name
	char	szAlias[13];		// short file name
	long	lFileSize;			// file size
	DWORD	dwFileType;			// file type

	//////////////////////////////////////////////
	DWORD	dwStartCluster;		// start cluster
	DWORD	dwStartBlock;		// start block

	char	rsv[16];
};
typedef struct DIRENTTag DIRENTStt;

// �ý��� ��ü�� ���� �ý���
typedef struct FileSystemTag {
	int					nTotal;				// ��ϵ� ���� �ý����� ����
	struct VFSTag		*pStart, *pEnd;		// ���� �ý��� ��ũ
	struct VNodeTag		*pRootNode;			// ���� �ý����� �ֻ��� ROOT Directory VNODE ������
};
typedef struct FileSystemTag FileSystemStt;

// ������ ������ �����Ѵ�.
extern BELL_EXPORT int kopen( char *pFileName, DWORD dwMode );

// Ư�� ������ ������ ���Ѵ�.(������ ���翩�θ� Ȯ���ϴ� �뵵�ε� ��� ����.)
extern BELL_EXPORT int kget_file_info( char *pFileName, DIRENTStt *pDirEnt );

// ������ �ݴ´�.
extern BELL_EXPORT int kclose( int nHandle );
// ���Ͽ��� �����͸� �д´�.
extern BELL_EXPORT int kread( int nHandle, void *pBuff, int nSize );

// ���� �����͸� �̵��Ѵ�.
extern BELL_EXPORT long klseek( int nHandle, long lOffset, int nOrigin );

// ���� �ý��� ����ü�� ��´�.
extern FileSystemStt *get_fs_stt();

// ��ü ���� �ý��� �ʱ�ȭ
extern int init_filesystem();

// ��ü ���� �ý����� û���Ѵ�.
extern int close_filesystem();

// �ý��� ���� ��ũ���͸� �Ҵ��Ѵ�.
extern int sys_fd_alloc();

// �ý��� ���� ��ũ���͸� ��ȯ�Ѵ�.
extern int sys_fd_free( int nFD );

// ������ �����Ѵ�.
extern int kcreate( char *pS, DWORD dwType );

// ���Ͽ� �����͸� ����Ѵ�.
extern int kwrite( int nHandle, void *pBuff, int nSize );

// ���丮�� �����.
extern int kmkdir( char *pS );

// ���丮�� �����Ѵ�.
extern int kopendir( char *pPath );
 
// ���丮�� �ݴ´�.
extern int kclosedir( int nHandle );       

// ���丮 ��Ʈ���� �д´�.
extern int kreaddir( DIRENTStt *pDIRENT, int nHandle );

// ���丮 ��Ʈ���� �����.
extern int kremove( char *pFilePath );
 
// �ڵ�κ��� vnode����ü�� �����Ѵ�.
extern void *handle_to_vnode( int nHandle ); 

// display sys_fd information for debugging
extern void display_sys_fd();

// get fupppath from file handle
extern int get_handle_fullpath( int nHandle, char *pS );

// rename file
extern int krename( char *pOld, char *pNew );

// ������ ȸ����ŭ ������ ��� �д� �׽�Ʈ�� �Ѵ�.
extern int file_test( char *pFileName, int nNumTest );


#endif
