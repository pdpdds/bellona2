#include "vfs.h"

static FileSystemStt fs;
static FDStt *alloc_sys_fd( int *pIndex );
static int vnode_close( VNodeStt *pVNode );
static VNodeStt *vnode_open( VNodeStt *pParentNode, char *pName, DWORD dwMode );

static SysFDManStt	sys_fd;
static SysFDManStt	*G_pSysFD = &sys_fd;
static VNodeStt		std_io_vnode[3];

// ���� �ý��� ����ü�� ��´�.
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
	
	// ����ü�� 0���� Ŭ�����Ѵ�. 
	memset( &fs, 0, sizeof( fs ) );

	// ���� ��ũ���� ���̺��� �ʱ�ȭ�Ѵ�.  (0���� clear)
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

// ���� �ý����� �����Ѵ�.  (��ũ���� ����)
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
			break;		// ���ο� FD ûũ�� �Ҵ��Ѵ�.

		if( pFDChunk->nTotalFD >= MAX_FD_PER_CHUNK )
			continue;	// ���� FD ûũ�� �˻��Ѵ�.	 (free FD�� ����.)

		// find free fd
		for( nK = 0; nK < MAX_FD_PER_CHUNK; nK++ )
		{	 
			pFD = &pFDChunk->fd[nK];
			if( pFD->pVNode != NULL || pFD->nRefCount > 0 )
				continue;
			else 
			{	// FREE FD�� ã�Ҵ�.
				pFDChunk->nTotalFD++;
				inc_sys_fd_ref_count( pFD );
				pIndex[0] = (nI * MAX_FD_PER_CHUNK) + nK;
				return( pFD );
			}
		}												 
	}

	// ���ο� fd chunk�� �Ҵ��Ѵ�.
	pFDChunk = (FDChunkStt*)MALLOC( sizeof( FDChunkStt ) );
	if( pFDChunk == NULL )
		return( NULL );

	// 0���� Ŭ�����Ѵ�.
	memset( pFDChunk, 0, sizeof( FDChunkStt ) );
	G_pSysFD->pFDChunk[ G_pSysFD->nTotalChunk ] = pFDChunk;
	G_pSysFD->nTotalChunk++;

	pFDChunk->nTotalFD++;
	inc_sys_fd_ref_count( &pFDChunk->fd[0] );
	pIndex[0] = nI * MAX_FD_PER_CHUNK;

	// 0��° FD�� �����Ѵ�.
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

// ���� �н��� �ƴ� ���� ���� ó���Ѵ�.
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

		// open next sub entry ('/'�� ������ �� pParent�� 'NULL'�̴�.)
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
		{	// ������ parent node�� �����Ѵ�.
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

	// ������ sub entry�� �����Ѵ�.
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
	if( pVNode == NULL )		// ������ ������ �� ����.
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

	// vnode read function�� call�Ѵ�.
	nRead = pVNode->op.readdir( pVNode, dwOffs, pDIRENT );

	return( nRead );
}

// mode�� ������ �����Ѵ�.
int kcreate( char *pPath, DWORD dwType )
{
	FDStt		*pFD;
	char		szName[260];
	int			nHandle, nFD;
	VNodeStt	*pParentNode, *pVNode;

	nHandle = -1;

	if( pPath == NULL || pPath[0] == 0 )
		return( -1 );

 	// �ý��� ���� ��ũ���͸� �Ҵ��Ѵ�.
	pFD = alloc_sys_fd( &nFD );
	if( pFD == NULL )
		return( -1 );		// ���� ��ũ���͸� �Ҵ��� �� ����. 

	// path pS�κ��� ������ szName�� szName�� ���� ���丮�� pVNode�� �����Ѵ�.
	pParentNode = tunnel_path( pPath, szName );
	if( pParentNode == NULL )
	{	// FD�� ��ȯ�Ѵ�.
		free_sys_fd( nFD );
		return( -1 );
	}

	// ã�� Parent VNode���� szName�� ���� ������ �����Ѵ�.
	pVNode = vnode_create_file( pParentNode, szName, dwType );
	if( pVNode == NULL )
	{	// FD�� ��ȯ�Ѵ�.
		free_sys_fd( nFD );
		// parent directory�� close�� �־�� �Ѵ�.
		vnode_close( pParentNode );
		return( -1 );
	}

	// FD�� ������ �����Ѵ�.
	pFD->pVNode		= pVNode;				// ���� VNode
	pFD->dwType		= dwType;				// ���� Ÿ��
	pFD->dwMode		= FM_READ | FM_WRITE;	// ������ ���µ� ���
	pFD->dwOffs		= 0;					// ���� ������
	pFD->nRefCount	= 1;					// Reference Count

	// parent directory�� close�� �־�� �Ѵ�.
	if( pParentNode != NULL )
		vnode_close( pParentNode );

	// nFD�� system file table�� index�ε� �̰��� per file fd, �� nHandle�� �����Ͽ� �����ؾ� �Ѵ�.
	return( nFD );
}

// ���Ͽ� �����͸� ����Ѵ�.
int kwrite( int nHandle, void *pBuff, int nSize )
{
	int		nWrite;
	FDStt	*pFD;
	
	// FD ����ü�� ���Ѵ�.
	pFD = pGetFD( nHandle );
	if( pFD == NULL )
		return( -1 );			// invalid file descriptor

	if( pFD->pVNode == NULL )
		return( -1 );			// invalid vnode

	// �߸��� �ķ�����
	if( pBuff == NULL || nSize <= 0 )
		return( -1 );

	// vnode write ��ƾ�� ���Ѵ�.
	if( pFD->pVNode->op.write == NULL )
		return( -1 );
	else	// ���� write��ƾ�� call�Ѵ�.
		nWrite = pFD->pVNode->op.write( pFD->pVNode, pFD->dwOffs, pBuff, nSize );
		
	// ���� �����͸� ������ �ش�.
	if( nWrite > 0 )		
		pFD->dwOffs += (DWORD)nWrite;

	return( nWrite );
}

// ���Ͽ��� �����͸� �д´�.
int kread( int nHandle, void *pBuff, int nSize )
{
	int nRead;

	FDStt	*pFD;
	
	// FD ����ü�� ���Ѵ�.
	pFD = pGetFD( nHandle );
	if( pFD == NULL )
		return( -1 );			// invalid file descriptor

	if( pFD->pVNode == NULL )
		return( -1 );			// invalid vnode

	// �߸��� �ķ�����
	if( pBuff == NULL || nSize <= 0 )
		return( -1 );

	// vnode read ��ƾ�� ���Ѵ�.
	if( pFD->pVNode->op.read == NULL )
		return( -1 );
	else  // fat32_vnode_read() �� ȣ��ȴ�.
		nRead = pFD->pVNode->op.read( pFD->pVNode, pFD->dwOffs, pBuff, nSize );
	
	// ���� �����͸� ������ �ش�.
	if( nRead > 0 )		
		pFD->dwOffs += (DWORD)nRead;

	return( nRead );
}

// ���� ���� �ý����� lseek�Լ��� �θ���.
static long vnode_lseek( VNodeStt *pVNode, long lOffset, int nOrigin )
{
	long lR;

	lR = pVNode->op.lseek( pVNode, lOffset, nOrigin );

	return( lR );
}

// ���� �����͸� �ű��.
long klseek( int nHandle, long lOffset, int nOrigin )
{
	long	lR, lNew, lCur;
	FDStt	*pFD;
	
	// FD ����ü�� ���Ѵ�.
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

	// ���� �����Ͱ� ����ũ�⺸�� ũ�� �����͸� ����ũ��� �����Ѵ�.
	if( lNew >= (long)pFD->pVNode->dwFileSize )
		lNew = (long)pFD->pVNode->dwFileSize;
	else if( lNew < 0 )
		lNew = 0;

	// ���� ���� �ý����� lseek�Լ��� �θ���.
	lR = vnode_lseek( pFD->pVNode, lOffset, nOrigin );
	if( lR != lNew )
		return( -1 );
				
	// ���� �����͸� ���� �����Ѵ�.
	pFD->dwOffs = (DWORD)lNew;
	
	return( lNew );
}

// pFileName���� ������ ������ �����Ѵ�.
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

 	// �ý��� ���� ��ũ���͸� �Ҵ��Ѵ�.
	pFD = alloc_sys_fd( &nFD );
	if( pFD == NULL )
		return( -1 );		// ���� ��ũ���͸� �Ҵ��� �� ����. 

	// root node�� �����ϴ� ���ΰ�?
	if( pS[0] == '/' && pS[1] == 0 )
	{	// root vnode�� ������ open ������ ��ġ�� �ʰ� �׳� �����Ѵ�.
		pVNode = get_root_vnode();
	}
	else
	{	// path pS�κ��� ������ szName�� szName�� ���� ���丮�� pVNode�� �����Ѵ�.
		pParentNode = tunnel_path( pS, szName );
		if( pParentNode == NULL )
		{	// FD�� ��ȯ�Ѵ�.
			free_sys_fd( nFD );
			return( -1 );
		}

		// ã�� Parent VNode���� szName�� ���� ������ �����Ѵ�.
		pVNode = vnode_open( pParentNode, szName, dwMode );
		if( pVNode == NULL )
		{	// FD�� ��ȯ�Ѵ�.
			free_sys_fd( nFD );
			// parent directory�� close�� �־�� �Ѵ�.
			vnode_close( pParentNode );
			return( -1 );
		}
	}

	// FD�� ������ �����Ѵ�.
	pFD->pVNode		= pVNode;		// ���� VNode
	pFD->dwType		= 0;			// ���� Ÿ��
	pFD->dwMode		= dwMode;		// ������ ���µ� ���
	pFD->dwOffs		= 0;			// ���� ������
	pFD->nRefCount	= 1;			// Reference Count

	// parent directory�� close�� �־�� �Ѵ�.
	if( pParentNode != NULL )
		vnode_close( pParentNode );

	// nFD�� system file table�� index�ε� �̰��� per file fd, �� nHandle�� �����Ͽ� �����ؾ� �Ѵ�.
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
	
 	// root node�� ���� ������ ��� ���ΰ�?
	if( pS[0] == '/' && pS[1] == 0 )
 		return( -1 );

	// path pS�κ��� ������ szName�� szName�� ���� ���丮�� pVNode�� �����Ѵ�.
	pParentNode = tunnel_path( pS, szName );
	if( pParentNode == NULL )
 		return( -1 );
 	
	// ã�� Parent VNode���� szName�� ���� ������ �����Ѵ�.
	nR = vnode_get_info( pParentNode, szName, pDirEnt );
	
	// parent directory�� close�� �־�� �Ѵ�.
	if( pParentNode != NULL )
		vnode_close( pParentNode );
	
	return( nR );
}

// ������ �ݴ´�.
int kclose( int nHandle )
{
	FDStt	*pFD;
	int		nR;

	// FD ����ü�� ���Ѵ�.
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

// ���丮�� �����Ѵ�.
int kmkdir( char *pS )
{
	int nHandle;

	// ����ó�� �����Ѵ�.
	nHandle = kcreate( pS, FT_DIRECTORY );
	if( nHandle == -1 )
	{
		kdbg_printf( "kmkdir(%s) failed!\n", pS );
		return( -1 );
	}

	// �ڵ��� �ݾ��ش�. 
	kclose( nHandle );

	return( 0 );
}

// ���丮�� �����Ѵ�.
int kopendir( char *pPath )
{
	int		nHandle;
	DWORD	dwMode;

	// ���丮�� ����ó�� �����Ѵ�.
	dwMode = 0;		// (�ϴ� 0�� ����ִ´�.)
	nHandle = kopen( pPath, dwMode );
	if( nHandle == -1 )
	{
		kdbg_printf( "kopendir: kopen(%s) - %d\n", pPath, nHandle );
		return( -1 );
	}

	// ���� �����͸� ó������ �ű��.
	klseek( nHandle, 0, FSEEK_SET );

	return( nHandle );
}			

// ���丮�� �ݴ´�.
int kclosedir( int nHandle )
{
	int nR;

	if( nHandle >= 0  )
		nR = kclose( nHandle );

	return( nR );
}

// ���丮 ��Ʈ���� �д´�.
int kreaddir( DIRENTStt *pDIRENT, int nHandle )
{
	int		nRead;
	FDStt	*pFD;
	
	// FD ����ü�� ���Ѵ�.
	pFD = pGetFD( nHandle );
	if( pFD == NULL )
		return( -1 );			// invalid file descriptor

	if( pFD->pVNode == NULL )
		return( -1 );			// invalid vnode

	// vnode read_dir ��ƾ�� ���Ѵ�.
	nRead = vnode_read_dir( pFD->pVNode, pFD->dwOffs, pDIRENT );
	if( nRead <= 0 )		
		return( -1 );

	// ���� �����͸� ������ �ش�.
	pFD->dwOffs += (DWORD)nRead;

	return( 1 );
}		

// pParentNode���� pS ���丮 ��Ʈ���� �����.
static int vnode_remove_file( VNodeStt *pParentNode, char *pS )
{	
	int			nR;
	VNodeStt	*pV;

	// �ش� ������ �̹� ���µǾ� �ִ� ������ ���� �� ����. 
	pV = find_vnode( pParentNode->pVFS->pVNodeMan, pParentNode, pS );
	if( pV != NULL )
		return( -1 );

	// ���� ���� �ý����� remove �Լ��� call�Ѵ�.
	nR = pParentNode->op.remove( pParentNode, pS );
		 
	return( nR );
}

// delete directory entry
int kremove( char *pS )
{
	int			nR;
	char		szName[260];
	VNodeStt	*pParentNode;

	// path pS�κ��� ������ szName�� szName�� ���� ���丮�� pVNode�� �����Ѵ�.
	pParentNode = tunnel_path( pS, szName );
	if( pParentNode == NULL )
		return( -1 );
						 	
	// Parent VNode���� szName�� ���� ������ �����.
	nR = vnode_remove_file( pParentNode, szName );

	// parent directory�� close�� �־�� �Ѵ�.
	vnode_close( pParentNode );

	return( nR );
}

// �ڵ�κ��� vnode����ü�� �����Ѵ�.
void *handle_to_vnode( int nHandle )
{
	FDStt	*pFD;
	
	// FD ����ü�� ���Ѵ�.
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
	
	// FD ����ü�� ���Ѵ�.
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

// ������ ȸ����ŭ ������ ��� �д� �׽�Ʈ�� �Ѵ�.
int file_test( char *pFileName, int nNumTest )
{
    char    *pBuff;
    long    lSize;
    int     nI, nHandle;

    for( nI = 0; nI < nNumTest; nI++ )
    {   
        nHandle = kopen( pFileName, FM_READ );
        if( nHandle < 0 )
        {   // ������ �� �� ����.
            kdbg_printf( "open error!\n" );
            break;
        }
        // ���� ũ�⸦ �˾Ƴ���.
        lSize = klseek( nHandle, 0, FSEEK_END );
        klseek( nHandle, 0, FSEEK_SET );

        // �޸𸮸� �Ҵ��Ѵ�.
        if( lSize > 0 )
        {   // �޸𸮸� �Ҵ��Ͽ� �о���δ�.
            pBuff = (char*)MALLOC( lSize );
            if( pBuff == NULL )
            {   // �޸𸮸� �Ҵ��� �� ����.
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
