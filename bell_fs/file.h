#ifndef VFS_FILE_HEADER_jj
#define VFS_FILE_HEADER_jj

#define FM_READ		1
#define FM_WRITE	2

#define FT_RDONLY		0x01
#define FT_HIDDEN		0x02
#define FT_SYSTEM		0x04
#define FT_DIRECTORY	0x10
#define FT_ARCHIVE		0x20
			  
// 128 * 128 = 16384개의 파일을 오픈할 수 있다. 
#define MAX_FD_CHUNK			128
#define MAX_FD_PER_CHUNK		128

struct VNodeTag;

// file descriptor structure
typedef struct FDTag {
	struct VNodeTag	*pVNode;			// 관련 VNode
	DWORD			dwType;				// 파일 타입
	DWORD			dwMode;				// 파일이 오픈된 모드
	DWORD			dwOffs;				// 파일 포인터
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

// 디렉토리 엔트리 구조체
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

// 시스템 전체의 파일 시스템
typedef struct FileSystemTag {
	int					nTotal;				// 등록된 파일 시스템의 개수
	struct VFSTag		*pStart, *pEnd;		// 파일 시스템 링크
	struct VNodeTag		*pRootNode;			// 파일 시스템의 최상위 ROOT Directory VNODE 포인터
};
typedef struct FileSystemTag FileSystemStt;

// 기존의 파일을 오픈한다.
extern BELL_EXPORT int kopen( char *pFileName, DWORD dwMode );

// 특정 파일의 정보를 구한다.(파일의 존재여부를 확인하는 용도로도 사용 가능.)
extern BELL_EXPORT int kget_file_info( char *pFileName, DIRENTStt *pDirEnt );

// 파일을 닫는다.
extern BELL_EXPORT int kclose( int nHandle );
// 파일에서 데이터를 읽는다.
extern BELL_EXPORT int kread( int nHandle, void *pBuff, int nSize );

// 파일 포인터를 이동한다.
extern BELL_EXPORT long klseek( int nHandle, long lOffset, int nOrigin );

// 파일 시스템 구조체를 얻는다.
extern FileSystemStt *get_fs_stt();

// 전체 파일 시스템 초기화
extern int init_filesystem();

// 전체 파일 시스템을 청산한다.
extern int close_filesystem();

// 시스템 파일 디스크립터를 할당한다.
extern int sys_fd_alloc();

// 시스템 파일 디스크립터를 반환한다.
extern int sys_fd_free( int nFD );

// 파일을 생성한다.
extern int kcreate( char *pS, DWORD dwType );

// 파일에 데이터를 기록한다.
extern int kwrite( int nHandle, void *pBuff, int nSize );

// 디렉토리를 만든다.
extern int kmkdir( char *pS );

// 디렉토리를 오픈한다.
extern int kopendir( char *pPath );
 
// 디렉토리를 닫는다.
extern int kclosedir( int nHandle );       

// 디렉토리 엔트리를 읽는다.
extern int kreaddir( DIRENTStt *pDIRENT, int nHandle );

// 디렉토리 엔트리를 지운다.
extern int kremove( char *pFilePath );
 
// 핸들로부터 vnode구조체를 리턴한다.
extern void *handle_to_vnode( int nHandle ); 

// display sys_fd information for debugging
extern void display_sys_fd();

// get fupppath from file handle
extern int get_handle_fullpath( int nHandle, char *pS );

// rename file
extern int krename( char *pOld, char *pNew );

// 지정된 회수만큼 파일을 열어서 읽는 테스트를 한다.
extern int file_test( char *pFileName, int nNumTest );


#endif
