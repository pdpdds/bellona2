#include "vfs.h"

static FileSystemStt fs;
static FDStt *alloc_sys_fd( int *pIndex );
static int vnode_close( VNodeStt *pVNode );
static VNodeStt *vnode_open( VNodeStt *pParentNode, char *pName, DWORD dwMode );

static SysFDManStt	sys_fd;
static SysFDManStt	*G_pSysFD = &sys_fd;
static VNodeStt		std_io_vnode[3];

// 파일 시스템 구조체를 얻는다.
FileSystemStt *get_fs_stt()
{
	return( &fs );
}

// display sys_fd information for debugging
void display_sys_fd()
{
	int			nI, nK;
	FDChunkStt	*pFDC;
	VNodeStt	*pVNode;

	JPRINTF( "total %d / %d system file descriptor chunks of max chunk\n", G_pSysFD->nTotalChunk, MAX_FD_CHUNK );

	for( nI = 0; nI < G_pSysFD->nTotalChunk; nI++ )
	{	// find non empty fd chunk
		pFDC = G_pSysFD->pFDChunk[ nI ];
		for( nK = 0; nK < MAX_FD_PER_CHUNK; nK++ )
		{	// find valid fd
			if( pFDC->fd[nK].pVNode != NULL )
			{
				pVNode = pFDC->fd[nK].pVNode;
				JPRINTF( "[%d/%d]  %4d  %08X %s\n", nK, nI, pFDC->fd[nK].nRefCount, (DWORD)pVNode, pVNode->szName );
			}	
			else if( pFDC->fd[nK].nRefCount > 0 )
				JPRINTF( "[%d/%d]  %4d  NULL\n", nK, nI, pFDC->fd[nK].nRefCount, pVNode->szName );
		}		
	}		
}

// initialize kernel file system
int init_filesystem()
{	
	FDStt	*p_fd[3];
	
	// 구조체를 0으로 클리어한다. 
	memset( &fs, 0, sizeof( fs ) );

	// 파일 디스크립터 테이블을 초기화한다.  (0으로 clear)
	memset( G_pSysFD, 0, sizeof( SysFDManStt ) );

	// allocate system fd
	p_fd[0] = alloc_sys_fd( NULL );		// 0
	p_fd[1] = alloc_sys_fd( NULL );		// 1
	p_fd[2] = alloc_sys_fd( NULL );		// 2

	// initialize kernel standard fd
	memset( std_io_vnode, 0, sizeof(VNodeStt) * 3 );
	std_io_vnode[0].op.read  = std_in_read;
	std_io_vnode[1].op.write = std_out_write;
	std_io_vnode[2].op.write = std_err_write;

	p_fd[0]->pVNode = &std_io_vnode[0];
	p_fd[1]->pVNode = &std_io_vnode[1];
	p_fd[2]->pVNode = &std_io_vnode[2];

	return(0);
}				

// 파일 시스템을 해제한다.  (디스크립터 해제)
int close_filesystem()
{
	int			nI;
	FDChunkStt	*pFDChunk;

	for( nI = 0; nI < G_pSysFD->nTotalChunk && nI < MAX_FD_CHUNK; nI++ )
	{
		pFDChunk = G_pSysFD->pFDChunk[nI];
		if( pFDChunk != NULL )
		{
			FREE( pFDChunk );
			G_pSysFD->pFDChunk[nI] = NULL;
		}
	}

	return(0);
}

// increase fd's ref counter
static int inc_sys_fd_ref_count( FDStt *pFD )
{
	pFD->nRefCount++;
	
	return( pFD->nRefCount );
}

// decrease fd's ref counter
static int dec_sys_fd_ref_count( FDStt *pFD )
{
	pFD->nRefCount--;
	
	return( pFD->nRefCount );
}

// allocate system file descriptor
static FDStt *alloc_sys_fd( int *pIndex )
{
	FDStt		*pFD;
	FDChunkStt	*pFDChunk;
	int			nI, nK, nTemp;;

	if( pIndex == NULL )
		pIndex = &nTemp;
	
	pIndex[0] = -1;
	for( nI = 0; ; nI++ )
	{
		if( nI >= MAX_FD_CHUNK )
			return( NULL );

		pFDChunk = G_pSysFD->pFDChunk[nI];
		if( pFDChunk == NULL )
			break;		// 새로운 FD 청크를 할당한다.

		if( pFDChunk->nTotalFD >= MAX_FD_PER_CHUNK )
			continue;	// 다음 FD 청크를 검사한다.	 (free FD가 없다.)

		// find free fd
		for( nK = 0; nK < MAX_FD_PER_CHUNK; nK++ )
		{	 
			pFD = &pFDChunk->fd[nK];
			if( pFD->pVNode != NULL || pFD->nRefCount > 0 )
				continue;
			else 
			{	// FREE FD를 찾았다.
				pFDChunk->nTotalFD++;
				inc_sys_fd_ref_count( pFD );
				pIndex[0] = (nI * MAX_FD_PER_CHUNK) + nK;
				return( pFD );
			}
		}												 
	}

	// 새로운 fd chunk를 할당한다.
	pFDChunk = (FDChunkStt*)MALLOC( sizeof( FDChunkStt ) );
	if( pFDChunk == NULL )
		return( NULL );

	// 0으로 클리어한다.
	memset( pFDChunk, 0, sizeof( FDChunkStt ) );
	G_pSysFD->pFDChunk[ G_pSysFD->nTotalChunk ] = pFDChunk;
	G_pSysFD->nTotalChunk++;

	pFDChunk->nTotalFD++;
	inc_sys_fd_ref_count( &pFDChunk->fd[0] );
	pIndex[0] = nI * MAX_FD_PER_CHUNK;

	// 0번째 FD를 리턴한다.
	return( &pFDChunk->fd[0] );
}

// release system file descriptor
int free_sys_fd( int nFD )
{
	FDStt		*pFD;
	int			nI, nK;
	
	nI = nFD / MAX_FD_PER_CHUNK;
	nK = nFD % MAX_FD_PER_CHUNK;

	if( nI >= G_pSysFD->nTotalChunk )
	{
		ERROR_PRINTF( "free_sys_fd_() : invalid fd handle!\n" );
		return( -1 );
	}

	pFD = &(G_pSysFD->pFDChunk[nI]->fd[nK]);
	if( pFD->nRefCount == 0 )
	{
		ERROR_PRINTF( "free_sys_fd: pFD.nRefCount == 0 !\n" );
		return( -1 );
	}

	// decrease ref counter
	if( dec_sys_fd_ref_count( pFD ) > 0 )
		return( 0 );

	pFD->pVNode		= NULL;
	pFD->nRefCount	= 0;
	G_pSysFD->pFDChunk[nI]->nTotalFD--;

	return( 0 );
}		

// get path entry
static char *get_path_entry( char *pName, char *pS )
{
	int		nI;

	if( pS[0] == '/' )
	{	// root directory
		strcpy( pName, "/" );
		return( &pS[1] );
	}
	
	pName[0] = 0;
	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( pS[nI] == '/' )
			return( &pS[nI+1] );

		pName[nI] = pS[nI];
		pName[nI+1] = 0;
	}	

	return( &pS[nI] );
}

static VNodeStt *vnode_open( VNodeStt *pParent, char *pName, DWORD dwMode )
{
	VNodeStt	*pVNode;

	if( pParent == NULL )
	{	// open system root vnode
		if( pName[0] == '/' && pName[1] == 0 )
		{	// root vnode ref count is increased in get_root_vnode function
			pVNode = get_root_vnode();	
			return( pVNode );
		}
		else
		{
			ERROR_PRINTF( "vnode_open() - parent vnode is NULL!\n" );
			return( NULL );
		}
	}
		
	// call open operation  (fat32_vnode_open)
	pVNode = pParent->op.open( pParent, pName, dwMode );
	if( pVNode == NULL )
		return( NULL );			
	
	// increase ref count
	inc_vnode_ref_count( pVNode );
	inc_vnode_ref_count( pParent );

	return( pVNode );
}

static int vnode_get_info( VNodeStt *pParent, char *pName, DIRENTStt *pDirEnt )
{
	int nR;

	if( pParent == NULL )
	{
		kdbg_printf( "vnode_get_info: error!\n" );
		return( -1 );
	}

	if( pParent->op.get_info == NULL )
	{
		kdbg_printf( "vnode_get_info: pParent->op.get_info = NULL!\n" );
		return( -1 );
	}

	nR = pParent->op.get_info( pParent, pName, pDirEnt );

	return( nR );	
}

// 절대 패스가 아닌 것은 에러 처리한다.
static VNodeStt *tunnel_path( char *pPath, char *pPathEntryName )
{
	int			nR;
	char		*pNext;
	VNodeStt	*pV, *pParent;
	char		szSubEntry[260];

	if( pPath == NULL || pPath[0] != '/' )
		return( NULL );

	pParent = NULL;
	pNext   = pPath;
	// get path subentry
	for( ; pNext != NULL && pNext[0] != 0; )
	{
		// get next path entry
		pNext = get_path_entry( szSubEntry, pNext );
		if( pNext[0] == 0 )
			break;

		// open next sub entry ('/'를 오픈할 땐 pParent가 'NULL'이다.)
		pV = vnode_open( pParent, szSubEntry, FM_WRITE | FM_READ );
		if( pV == NULL )
		{
			ERROR_PRINTF( "tunnel_path: resolving failed!\n" );
			nR = vnode_close( pParent );
			if( nR < 0 )
				goto CLOSE_ERROR;
			return( NULL );
		}
		else
		{	// 기존의 parent node를 해제한다.
			if( pParent != NULL )
			{
				nR = vnode_close( pParent );
				if( nR < 0 )
				{
CLOSE_ERROR:		ERROR_PRINTF( "tunnel_path: close parent vnode failed!\n" );
					return( NULL );
				}
			}

			pParent = pV;
		}					 
	}

	// 마지막 sub entry를 리턴한다.
	strcpy( pPathEntryName, szSubEntry );

	return( pParent );
}

// get FD struct from fd handle
static FDStt *pGetFD( int nFD )
{
	int nI, nK;

	if( nFD < 0 )
		return( NULL );
	
	nI = nFD / MAX_FD_PER_CHUNK;
	nK = nFD % MAX_FD_PER_CHUNK;

	if( nI >= G_pSysFD->nTotalChunk )
		return( NULL );

	if( nK >= MAX_FD_PER_CHUNK )
		return( NULL );

	return( &G_pSysFD->pFDChunk[nI]->fd[nK] );
}

// create new file
static VNodeStt *vnode_create_file( VNodeStt *pParent, char *pName, DWORD dwType )
{
	VNodeStt	*pVNode;

	// check if the file already exists.
	pVNode = vnode_open( pParent, pName, FM_READ );
	if( pVNode != NULL )
	{	// close vnode	
		vnode_close( pVNode );
		ERROR_PRINTF( "the file already exists.\n" );
		return( NULL );
	}
	
	// call create operation
	pVNode = pParent->op.create( pParent, pName, dwType );
	if( pVNode == NULL )		// 파일을 생성할 수 없다.
		return( NULL );
			  	  
	// increase ref count
	inc_vnode_ref_count( pVNode );
	inc_vnode_ref_count( pParent );
	
	return( pVNode );
}

// inner vnode close
static int inner_vnode_close_file( VNodeStt *pVNode )
{
	int			nR;

	// NULL pointer is not allowed
	if( pVNode == NULL )
	{
		ERROR_PRINTF( "vnode_close; the vnode pointer is NULL!\n" );
		return( -1 );
	}

	// no negative ref count is allowed
	if( pVNode->nRefCount <= 0 )
	{
		ERROR_PRINTF( "vnode_close: ref count is %d !\n", pVNode->nRefCount );
		return( -1 );
	}

	if( pVNode->nRefCount > 1 )
	{
		dec_vnode_ref_count( pVNode );
		return( 0 );
	}

	// vnode's ref count is 1
	// if ref count is 1 close vnode itself
	if( pVNode->op.close == NULL )
	{
		ERROR_PRINTF( "vnode_close; the vnode does not support close function!\n" );
		return( -1 );
	}

	// call vnode's close function
	nR = pVNode->op.close( pVNode );

	return( nR );
}

// close vnode 
static int vnode_close( VNodeStt *pVNode )
{
	int			nR;
	VNodeStt	*pParent;

	if( pVNode == NULL )
	{
		kdbg_printf( "vnode_close: pVNode = NULL!\n" );
		return( -1 );
	}

	// get parent vnode pointer
	if( pVNode->pMountPoint == NULL )
		pParent = pVNode->pParentVNode;
	else
	{
		pParent = pVNode->pMountPoint->pParentVNode;
	}

	// close vnode or decrease its ref count
	nR = inner_vnode_close_file( pVNode );
	if( nR < 0 )
		return( -1 );

	// close parent vnode or decrease ref count
	if( pParent != NULL )
		nR = inner_vnode_close_file( pParent );

	return( nR );
}					 

// read directory entry
static int vnode_read_dir( VNodeStt *pVNode, DWORD dwOffs, DIRENTStt *pDIRENT )
{
	int nRead;

	// vnode read function을 call한다.
	nRead = pVNode->op.readdir( pVNode, dwOffs, pDIRENT );

	return( nRead );
}

// mode로 파일을 생성한다.
int kcreate( char *pPath, DWORD dwType )
{
	FDStt		*pFD;
	char		szName[260];
	int			nHandle, nFD;
	VNodeStt	*pParentNode, *pVNode;

	nHandle = -1;

	if( pPath == NULL || pPath[0] == 0 )
		return( -1 );

 	// 시스템 파일 디스크립터를 할당한다.
	pFD = alloc_sys_fd( &nFD );
	if( pFD == NULL )
		return( -1 );		// 파일 디스크립터를 할당할 수 없다. 

	// path pS로부터 마지막 szName과 szName의 상위 디렉토리의 pVNode를 리턴한다.
	pParentNode = tunnel_path( pPath, szName );
	if( pParentNode == NULL )
	{	// FD를 반환한다.
		free_sys_fd( nFD );
		return( -1 );
	}

	// 찾은 Parent VNode에서 szName을 갖는 파일을 생성한다.
	pVNode = vnode_create_file( pParentNode, szName, dwType );
	if( pVNode == NULL )
	{	// FD를 반환한다.
		free_sys_fd( nFD );
		// parent directory를 close해 주어야 한다.
		vnode_close( pParentNode );
		return( -1 );
	}

	// FD의 값들을 설정한다.
	pFD->pVNode		= pVNode;				// 관련 VNode
	pFD->dwType		= dwType;				// 파일 타입
	pFD->dwMode		= FM_READ | FM_WRITE;	// 파일이 오픈된 모드
	pFD->dwOffs		= 0;					// 파일 포인터
	pFD->nRefCount	= 1;					// Reference Count

	// parent directory를 close해 주어야 한다.
	if( pParentNode != NULL )
		vnode_close( pParentNode );

	// nFD는 system file table의 index인데 이것을 per file fd, 즉 nHandle로 변경하여 리턴해야 한다.
	return( nFD );
}

// 파일에 데이터를 기록한다.
int kwrite( int nHandle, void *pBuff, int nSize )
{
	int		nWrite;
	FDStt	*pFD;
	
	// FD 구조체를 구한다.
	pFD = pGetFD( nHandle );
	if( pFD == NULL )
		return( -1 );			// invalid file descriptor

	if( pFD->pVNode == NULL )
		return( -1 );			// invalid vnode

	// 잘못된 파러메터
	if( pBuff == NULL || nSize <= 0 )
		return( -1 );

	// vnode write 루틴을 콜한다.
	if( pFD->pVNode->op.write == NULL )
		return( -1 );
	else	// 실제 write루틴을 call한다.
		nWrite = pFD->pVNode->op.write( pFD->pVNode, pFD->dwOffs, pBuff, nSize );
		
	// 파일 포인터를 변경해 준다.
	if( nWrite > 0 )		
		pFD->dwOffs += (DWORD)nWrite;

	return( nWrite );
}

// 파일에서 데이터를 읽는다.
int kread( int nHandle, void *pBuff, int nSize )
{
	int nRead;

	FDStt	*pFD;
	
	// FD 구조체를 구한다.
	pFD = pGetFD( nHandle );
	if( pFD == NULL )
		return( -1 );			// invalid file descriptor

	if( pFD->pVNode == NULL )
		return( -1 );			// invalid vnode

	// 잘못된 파러메터
	if( pBuff == NULL || nSize <= 0 )
		return( -1 );

	// vnode read 루틴을 콜한다.
	if( pFD->pVNode->op.read == NULL )
		return( -1 );
	else  // fat32_vnode_read() 가 호출된다.
		nRead = pFD->pVNode->op.read( pFD->pVNode, pFD->dwOffs, pBuff, nSize );
	
	// 파일 포인터를 변경해 준다.
	if( nRead > 0 )		
		pFD->dwOffs += (DWORD)nRead;

	return( nRead );
}

// 실제 파일 시스템의 lseek함수를 부른다.
static long vnode_lseek( VNodeStt *pVNode, long lOffset, int nOrigin )
{
	long lR;

	lR = pVNode->op.lseek( pVNode, lOffset, nOrigin );

	return( lR );
}

// 파일 포인터를 옮긴다.
long klseek( int nHandle, long lOffset, int nOrigin )
{
	long	lR, lNew, lCur;
	FDStt	*pFD;
	
	// FD 구조체를 구한다.
	pFD = pGetFD( nHandle );
	if( pFD == NULL )
		return( -1 );			// invalid file descriptor

	if( pFD->pVNode == NULL )
		return( -1 );			// invalid vnode

	lCur = (long)pFD->dwOffs;
	switch( nOrigin )
	{
	case FSEEK_SET :
		lNew = lOffset;
		break;
	case FSEEK_CUR :
		lNew = lOffset + lCur;
		break;
	case FSEEK_END :
		lNew = lOffset + (long)pFD->pVNode->dwFileSize;
		break;
	}

	// 파일 포인터가 파일크기보다 크면 포인터를 파일크기로 설정한다.
	if( lNew >= (long)pFD->pVNode->dwFileSize )
		lNew = (long)pFD->pVNode->dwFileSize;
	else if( lNew < 0 )
		lNew = 0;

	// 실제 파일 시스템의 lseek함수를 부른다.
	lR = vnode_lseek( pFD->pVNode, lOffset, nOrigin );
	if( lR != lNew )
		return( -1 );
				
	// 파일 포인터를 새로 설정한다.
	pFD->dwOffs = (DWORD)lNew;
	
	return( lNew );
}

// pFileName으로 지정된 파일을 오픈한다.
int kopen( char *pS, DWORD dwMode )
{
	FDStt		*pFD;
	char		szName[260];
	int			nHandle, nFD;
	VNodeStt	*pParentNode, *pVNode;

	nHandle = -1;
	pParentNode = pVNode = NULL;

	if( pS == NULL || pS[0] == 0 )
		return( -1 );

 	// 시스템 파일 디스크립터를 할당한다.
	pFD = alloc_sys_fd( &nFD );
	if( pFD == NULL )
		return( -1 );		// 파일 디스크립터를 할당할 수 없다. 

	// root node를 오픈하는 것인가?
	if( pS[0] == '/' && pS[1] == 0 )
	{	// root vnode는 별도의 open 과정을 거치지 않고 그냥 리턴한다.
		pVNode = get_root_vnode();
	}
	else
	{	// path pS로부터 마지막 szName과 szName의 상위 디렉토리의 pVNode를 리턴한다.
		pParentNode = tunnel_path( pS, szName );
		if( pParentNode == NULL )
		{	// FD를 반환한다.
			free_sys_fd( nFD );
			return( -1 );
		}

		// 찾은 Parent VNode에서 szName을 갖는 파일을 오픈한다.
		pVNode = vnode_open( pParentNode, szName, dwMode );
		if( pVNode == NULL )
		{	// FD를 반환한다.
			free_sys_fd( nFD );
			// parent directory를 close해 주어야 한다.
			vnode_close( pParentNode );
			return( -1 );
		}
	}

	// FD의 값들을 설정한다.
	pFD->pVNode		= pVNode;		// 관련 VNode
	pFD->dwType		= 0;			// 파일 타입
	pFD->dwMode		= dwMode;		// 파일이 오픈된 모드
	pFD->dwOffs		= 0;			// 파일 포인터
	pFD->nRefCount	= 1;			// Reference Count

	// parent directory를 close해 주어야 한다.
	if( pParentNode != NULL )
		vnode_close( pParentNode );

	// nFD는 system file table의 index인데 이것을 per file fd, 즉 nHandle로 변경하여 리턴해야 한다.
	return( nFD );
}

int kget_file_info( char *pS, DIRENTStt *pDirEnt )
{
	int 		nR; 
	DIRENTStt	dirent;
 	char		szName[260];
 	VNodeStt	*pParentNode;
	
	if( pS == NULL || pS[0] == 0 )
		return( -1 );

	if( pDirEnt == NULL )
		pDirEnt = &dirent;
	
 	// root node에 대한 정보를 얻는 것인가?
	if( pS[0] == '/' && pS[1] == 0 )
 		return( -1 );

	// path pS로부터 마지막 szName과 szName의 상위 디렉토리의 pVNode를 리턴한다.
	pParentNode = tunnel_path( pS, szName );
	if( pParentNode == NULL )
 		return( -1 );
 	
	// 찾은 Parent VNode에서 szName을 갖는 파일을 오픈한다.
	nR = vnode_get_info( pParentNode, szName, pDirEnt );
	
	// parent directory를 close해 주어야 한다.
	if( pParentNode != NULL )
		vnode_close( pParentNode );
	
	return( nR );
}

// 파일을 닫는다.
int kclose( int nHandle )
{
	FDStt	*pFD;
	int		nR;

	// FD 구조체를 구한다.
	pFD = pGetFD( nHandle );
	if( pFD == NULL )
	{
		ERROR_PRINTF( "close_file() - null fd .\n" );
		return( -1 );			// invalid file descriptor
	}

	// close vnode
	if( pFD->pVNode != NULL )
		nR = vnode_close( pFD->pVNode );
	
	// release system file descriptor
	free_sys_fd( nHandle );	

	// shit! what is hapening...
	if( nR < 0 )
		return( -1 );

	return( 0 );
}

// 디렉토리를 생성한다.
int kmkdir( char *pS )
{
	int nHandle;

	// 파일처럼 생성한다.
	nHandle = kcreate( pS, FT_DIRECTORY );
	if( nHandle == -1 )
	{
		kdbg_printf( "kmkdir(%s) failed!\n", pS );
		return( -1 );
	}

	// 핸들을 닫아준다. 
	kclose( nHandle );

	return( 0 );
}

// 디렉토리를 오픈한다.
int kopendir( char *pPath )
{
	int		nHandle;
	DWORD	dwMode;

	// 디렉토리를 파일처럼 오픈한다.
	dwMode = 0;		// (일단 0을 집어넣는다.)
	nHandle = kopen( pPath, dwMode );
	if( nHandle == -1 )
	{
		kdbg_printf( "kopendir: kopen(%s) - %d\n", pPath, nHandle );
		return( -1 );
	}

	// 파일 포인터를 처음으로 옮긴다.
	klseek( nHandle, 0, FSEEK_SET );

	return( nHandle );
}			

// 디렉토리를 닫는다.
int kclosedir( int nHandle )
{
	int nR;

	if( nHandle >= 0  )
		nR = kclose( nHandle );

	return( nR );
}

// 디렉토리 엔트리를 읽는다.
int kreaddir( DIRENTStt *pDIRENT, int nHandle )
{
	int		nRead;
	FDStt	*pFD;
	
	// FD 구조체를 구한다.
	pFD = pGetFD( nHandle );
	if( pFD == NULL )
		return( -1 );			// invalid file descriptor

	if( pFD->pVNode == NULL )
		return( -1 );			// invalid vnode

	// vnode read_dir 루틴을 콜한다.
	nRead = vnode_read_dir( pFD->pVNode, pFD->dwOffs, pDIRENT );
	if( nRead <= 0 )		
		return( -1 );

	// 파일 포인터를 변경해 준다.
	pFD->dwOffs += (DWORD)nRead;

	return( 1 );
}		

// pParentNode에서 pS 디렉토리 엔트리를 지운다.
static int vnode_remove_file( VNodeStt *pParentNode, char *pS )
{	
	int			nR;
	VNodeStt	*pV;

	// 해당 파일이 이미 오픈되어 있는 파일은 지울 수 없다. 
	pV = find_vnode( pParentNode->pVFS->pVNodeMan, pParentNode, pS );
	if( pV != NULL )
		return( -1 );

	// 실제 파일 시스템의 remove 함수를 call한다.
	nR = pParentNode->op.remove( pParentNode, pS );
		 
	return( nR );
}

// delete directory entry
int kremove( char *pS )
{
	int			nR;
	char		szName[260];
	VNodeStt	*pParentNode;

	// path pS로부터 마지막 szName과 szName의 상위 디렉토리의 pVNode를 리턴한다.
	pParentNode = tunnel_path( pS, szName );
	if( pParentNode == NULL )
		return( -1 );
						 	
	// Parent VNode에서 szName을 갖는 파일을 지운다.
	nR = vnode_remove_file( pParentNode, szName );

	// parent directory를 close해 주어야 한다.
	vnode_close( pParentNode );

	return( nR );
}

// 핸들로부터 vnode구조체를 리턴한다.
void *handle_to_vnode( int nHandle )
{
	FDStt	*pFD;
	
	// FD 구조체를 구한다.
	pFD = pGetFD( nHandle );
	if( pFD == NULL )
		return( NULL );			// invalid file descriptor

	return( pFD->pVNode );
}

// get fupppath from file handle
int get_handle_fullpath( int nHandle, char *pS )
{
	int		nR;
	FDStt	*pFD;
	
	// FD 구조체를 구한다.
	pFD = pGetFD( nHandle );
	if( pFD == NULL )
		return( -1 );			// invalid file descriptor

	if( pFD->pVNode == NULL )
		return( -1 );			// invalid vnode

	nR = get_vnode_fullpath( pFD->pVNode, pS );

	return( nR );
}

// rename
static int vnode_rename_file( VNodeStt *pOldVNode, char *pOldName, VNodeStt *pNewVNode, char *pNewName )
{
	int			nR;
	VNodeStt	*pV;

	// source file is in use
	pV = find_vnode( pOldVNode->pVFS->pVNodeMan, pOldVNode, pOldName );
	if( pV != NULL )
	{
		ERROR_PRINTF( "source file is in use.\n" );
		return( -1 );
	}

	// source file is in use
	pV = find_vnode( pNewVNode->pVFS->pVNodeMan, pNewVNode, pNewName );
	if( pV != NULL )
	{
		ERROR_PRINTF( "new name file already exists.\n" );
		return( -1 );
	}

	// call filesystem's real rename function
	if( pOldVNode->op.rename != NULL )
		nR = pOldVNode->op.rename( pOldVNode, pOldName, pNewVNode, pNewName );

	return( nR );
}

// rename file
int krename( char *pOld, char *pNew )
{	
	int			nR;
	VNodeStt	*pV, *pOldParent,	*pNewParent;
	char		szOldName[260], szNewName[260];

	// resolve old path
	pOldParent = tunnel_path( pOld, szOldName );
	if( pOldParent == NULL )
	{	
		ERROR_PRINTF( "invalid old name path\n" );
		return( -1 );
	}

	// resolve new path
	pNewParent = tunnel_path( pNew, szNewName );
	if( pNewParent == NULL )
	{	
		ERROR_PRINTF( "invalid new name path\n" );
		vnode_close( pOldParent );
		return( -1 );
	}

	// check if the file already exists.
	pV = vnode_open( pNewParent, szNewName, FM_READ );
	if( pV != NULL )
	{	
		vnode_close( pV );
		ERROR_PRINTF( "the new file name already exists.\n" );
		nR = -1;
		goto RETURN;
	}
	
	// the two vnodes must be in one vfs
	if( pNewParent->pVFS != pOldParent->pVFS )
	{
		ERROR_PRINTF( "the new path does not exist in the old vfs.\n" );
		nR = -1;
		goto RETURN;
	}

	// call vnode rename function
	nR = vnode_rename_file( pOldParent, szOldName, pNewParent, szNewName );

RETURN:
	// close parent vnodes
	vnode_close( pOldParent );
	vnode_close( pNewParent );
	return( nR );
}

// 지정된 회수만큼 파일을 열어서 읽는 테스트를 한다.
int file_test( char *pFileName, int nNumTest )
{
    char    *pBuff;
    long    lSize;
    int     nI, nHandle;

    for( nI = 0; nI < nNumTest; nI++ )
    {   
        nHandle = kopen( pFileName, FM_READ );
        if( nHandle < 0 )
        {   // 파일을 열 수 없다.
            kdbg_printf( "open error!\n" );
            break;
        }
        // 파일 크기를 알아낸다.
        lSize = klseek( nHandle, 0, FSEEK_END );
        klseek( nHandle, 0, FSEEK_SET );

        // 메모리를 할당한다.
        if( lSize > 0 )
        {   // 메모리를 할당하여 읽어들인다.
            pBuff = (char*)MALLOC( lSize );
            if( pBuff == NULL )
            {   // 메모리를 할당할 수 없다.
                kdbg_printf( "malloc failed!\n" );
                kclose( nHandle );
                break;
            }
            kread( nHandle, pBuff, lSize );
        }

        kclose( nHandle );
        FREE( pBuff );
        kdbg_printf( "\rfiletest size = %d, (%d)th : ok   ", lSize, nI );
    }
    kdbg_printf( "\n" );
    return( 0 );
}
