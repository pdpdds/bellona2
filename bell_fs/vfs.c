#include "vfs.h"

// 파일 시스템 타입을 얻는다.
int get_filesystem_type( char *pStr )
{
	if( strcmpi( pStr, "FAT32" ) == 0 )
		return( VFS_TYPE_FAT32 );
	else if( strcmpi( pStr, "FAT16" ) == 0 )
		return( VFS_TYPE_FAT16 );
	else if( strcmpi( pStr, "FAT12" ) == 0 )
		return( VFS_TYPE_FAT12 );
	else
		return( VFS_TYPE_UNKNOWN );
}

// display vnodes
void display_vnodes()
{
	VFSStt			*pVFS;
	VNodeManStt		*pMan;
	VNodeStt		*pVNode;
	VNodeChunkStt	*pVNodeChunk;
	int				nTotal, nI, nX;
	char			szVNodeName[260];

	// find the first vfs
	pVFS = find_first_vfs( &nTotal );

	nX = 0;
	for( ; pVFS != NULL; pVFS = pVFS->pNext )
	{	// get vnode manager
		pMan = pVFS->pVNodeMan;
		if( pMan == NULL )
			continue;
		
		// vnode chunk
		pVNodeChunk = pMan->pStart;
		if( pVNodeChunk == NULL || pMan->nTotalChunk == 0 )
			continue;

		// vnode array
		for( ; pVNodeChunk != NULL; pVNodeChunk = pVNodeChunk->pNext )
		{
			pVNode = pVNodeChunk->vnode;

			for( nI = 0; nI < MAX_CHUNK_ENT; nI++ )
			{
				if( pVNode[nI].dwType != VNTYPE_FREE && pVNode[nI].nRefCount > 0 )
				{
					if( pVNode[nI].szName[0] == 0 )
					{	 // root node  (display mount point directory)
						if( pVNode[nI].pMountPoint != NULL )
							sprintf( szVNodeName, "mounted on %s", pVNode[nI].pMountPoint->szName );
					}
					else // normal vnode
						strcpy( szVNodeName, pVNode[nI].szName );

					JPRINTF( "[%d] %d %-12s %s\n", nX, pVNode[nI].nRefCount, pVNode[nI].pVFS->szName, szVNodeName );
					nX++;
				}	
			}		
		}				 
	}
}

// return total number of the registered vfses and the pointer to the first vfs.
VFSStt *find_first_vfs( int *pTotal )
{
	FileSystemStt	*pFS;
	pFS = get_fs_stt();

	pTotal[0] = pFS->nTotal;

	return( pFS->pStart );
}

// return total number of the registered vfses and the pointer to the last vfs.
VFSStt *find_last_vfs( int *pTotal )
{
	FileSystemStt	*pFS;
	pFS = get_fs_stt();

	pTotal[0] = pFS->nTotal;

	return( pFS->pEnd );
}

// return system root vnode
VNodeStt *get_root_vnode()
{
	FileSystemStt	*pFS;
	pFS = get_fs_stt();

	// just increase ref count
	inc_vnode_ref_count( pFS->pRootNode );
	
	return( pFS->pRootNode );
}

// 전체 파일 시스템의 ROOT NODE를 설정한다.
int set_filesystem_root( VNodeStt *pVNode )
{
	FileSystemStt	*pFS;
	
	pFS = get_fs_stt();
	pFS->pRootNode = pVNode;
	strcpy( pVNode->szName, "/" );

	return( 0 );
}

// link a filesystem to a subdirectory of another filesystem
int link_filesystem( char *pPath, VNodeStt *pRootNode )
{
	int			nHandle;
	VNodeStt	*pBaseNode;

	// open directory
	nHandle = kopendir( pPath );
	if( nHandle == -1 )
		return( -1 );

	// 핸들을 통해 vnode를 구한다.
	pBaseNode = handle_to_vnode( nHandle );		
	if( pBaseNode == NULL )				// 이게 NULL일 수는 없지만...그래도
	{
		kclosedir( nHandle );			// handle을 close하고 리턴한다.
		return( -1 );
	}

	pBaseNode->pBranchVNode    = pRootNode;	// namei resolution에서 해당 pointer가 NULL이 아니면
	pRootNode->pMountPoint     = pBaseNode;	// 그에 맞는 적절한 처리를 하면 된다.
	pRootNode->nMountDirHandle = nHandle;		// handle must be preserved to unlink filesystem

	return( 0 );
}

// unlinlk filesystem from subdirectory
int unlink_filesystem( VNodeStt *pVNode )
{
	VNodeStt *pT;

	if( pVNode == NULL )
	{	// invalid vfs
		ERROR_PRINTF( "unlink_filesystem() : pVNode = NULL\n" );
		return( -1 );
	}

	// reset pointer
	pT = pVNode->pMountPoint;
	if( pT == NULL )
	{	// critical error! what the hell is this?
		ERROR_PRINTF( "unlink_filesystem() : pVNode->pMountPoint = NULL\n" );
		return( -1 );
	}
	else
	{	// clear branch vnode pointer
		pT->pBranchVNode = NULL;
	}


	// close mount point
	if( pVNode->nMountDirHandle >= 0 )
		kclosedir( pVNode->nMountDirHandle );

	pVNode->pMountPoint     = NULL;
	pVNode->nMountDirHandle = -1;

	return( 0 );
}

// unregister fileystem
int unregister_filesystem( VFSStt *pVFS )
{
	int				nI;
	VFSStt			*pT;
	FileSystemStt	*pFS;
	
	pFS = get_fs_stt();

	if( pFS->nTotal == 0 || pFS->pStart == NULL )
	{
		ERROR_PRINTF( "unregister_filesystem() : No FileSystem\n" );
		return( -1 );
	}

	// Find VFS
	for( pT = pFS->pStart, nI = 0; nI < pFS->nTotal && pT != NULL; pT = pT->pNext )
	{
		if( pT == pVFS )
		{
			if( pT->pPre == NULL )
				pFS->pStart = pT->pNext;
			else
				pT->pPre->pNext = pT->pNext;

			if( pT->pNext == NULL )
				pFS->pEnd = pT->pPre;
			else
				pT->pNext->pPre = pT->pPre;

			pFS->nTotal--;
			return( 1 );
		}
	}

	ERROR_PRINTF( "unregister_filesystem() : VFS not found.\n" );
	return( -1 );
}	

// register filesystem
int register_filesystem( VFSStt *pVFS )
{
	FileSystemStt	*pFS;
	pFS = get_fs_stt();

	if( pFS->pEnd == NULL )	// no registered filesystem
	{
		pFS->pStart = pFS->pEnd = pVFS;
		pVFS->pPre = pVFS->pNext = NULL;
	}
	else
	{
		pVFS->pPre		= pFS->pEnd; 
		pVFS->pNext		= NULL;
		pFS->pEnd->pNext	= pVFS;
		pFS->pEnd			= pVFS;
	}

	pFS->nTotal++;
	
	return( 0 );
}

// link vfs to device object
int link_filesystem_device( VFSStt *pVFS, BlkDevObjStt *pDevObj )
{
	pVFS->pDevObj = pDevObj;

	return( 0 );
}

// unlink device object from vfs
int unlink_filesystem_device( VFSStt *pVFS )
{
	pVFS->pDevObj = NULL;

	return( 0 );
}

// 파일 시스템에 관련된 IOCTL 필요에 따라 연결된 블록 디바이스의 ioctl을 이용한다.
int vfs_ioctl( VFSStt *pVFS, int nCmd, DWORD dwParam )
{
	int nR;

	if( pVFS->op.ioctl == NULL )
		return( -1 );

	// 실제 파일 시스템의 ioctl을 부른다.
	nR = pVFS->op.ioctl( nCmd, dwParam );

	return( nR );
}

// 마운트 한다.
int vfs_mount( char *pPath, VFSStt *pVFS )
{
	int nR;

	if( pVFS->op.mount == NULL )		// Mount Operation이 등록되어 있지 않다.
	{
		ERROR_PRINTF( "vfs_mount() : pVFS->op.mount = NULL\n" );
		return( -1 );
	}

	if( pVFS->pMountNode != NULL )		// 이미 다른 곳에 마운트 되어 있다.
	{
		ERROR_PRINTF( "vfs_mount() : pVFS->pMountNode\n" );
		return( -1 );
	}

	// 실제 마운트 함수를 호출한다. fat32의 경우 fat32_fs_mount()가 호출된다.
	nR = pVFS->op.mount( pVFS );    
	if( nR < 0 )
	{
		kdbg_printf( "vfs_mount: pVFS->op.mount() - %d\n", nR );
		return(-1 );
	}

	// ROOT Vnode의 RefCount를 증가시킨다.
	inc_vnode_ref_count( pVFS->pRootNode );

	// link root vnode of this vfs to 
	// the system root directory or the directory indicated by the pPath.
	if( pPath[0] == '/' && pPath[1] == 0 )
		nR = set_filesystem_root( pVFS->pRootNode );
	else
		nR = link_filesystem( pPath, pVFS->pRootNode );

	return( nR );
}

// unmount vfs
int vfs_unmount( VFSStt *pVFS )
{
	int nR;
		
	if( pVFS == NULL )
	{
        ERROR_PRINTF( "vfs_unmount() : pVFS = NULL\n" );
		return( -1 );
	}

	if( pVFS->pRootNode == NULL )
	{
		ERROR_PRINTF( "vfs_unmount() : pVFS->pRootNode = NULL\n" );
		return( -1 );
	}

	// validate unmount function
	if( pVFS->op.unmount == NULL )
		return( -1 );

	// is it a mounted vfs?
	if( pVFS->pRootNode->pMountPoint != NULL )
	{
		nR = unlink_filesystem( pVFS->pRootNode );
		if( nR < 0 )
			return( -1 );
	}

	// call unmount function
	nR = pVFS->op.unmount( pVFS );
	if( nR < 0 )
		return( -1 );		
	
	return( nR );
}

// allocate vnode manager structure and initialize that.
VNodeManStt	*make_vnode_manager( VNodeOPStt *pVNodeOP, int nTotalHashIndex )
{
	VNodeManStt		*pVNodeMan;
	VNodeHashEntStt	*pHashIndex;
					
	// allocate vnode manager structure
	pVNodeMan = (VNodeManStt*)MALLOC( sizeof( VNodeManStt ) );
	if( pVNodeMan == NULL )
		return( NULL );

	// allocate hash table
	pHashIndex = (VNodeHashEntStt*)MALLOC( sizeof( VNodeHashEntStt ) * nTotalHashIndex );
	if( pHashIndex == NULL )
	{
		FREE( pVNodeMan );
		return( NULL );
	}					  

	// clear by 0
	memset( pVNodeMan,   0, sizeof( VNodeManStt ) );
	memset( pHashIndex,  0, sizeof( VNodeHashEntStt ) * nTotalHashIndex );
	pVNodeMan->pVHash		= pHashIndex;
	pVNodeMan->nTotalHash	= nTotalHashIndex;
	
	// copy vnode operations
	memcpy( &pVNodeMan->op, pVNodeOP, sizeof( VNodeOPStt ) );

	return( pVNodeMan );
}

// release vnode manager structure and its sub structure
int delete_vnode_manager( VNodeManStt *pVNodeMan )
{
	int				nI;
	VNodeChunkStt	*pChunk, *pNext;

	// release all the vnode cunks
	pChunk = pVNodeMan->pStart;
	for( nI = 0; nI < pVNodeMan->nTotalChunk && pChunk != NULL; nI++ )
	{
		pNext = pChunk->pNext;
		FREE( pChunk );
		pChunk = pNext;
	}

	// release vnode manager structure
	FREE( pVNodeMan->pVHash );
	FREE( pVNodeMan );

	return( 0 );
}

// allocate new vnode structure
VNodeStt *new_vnode( VNodeManStt *pVNodeMan )
{
	VNodeStt		*pVNode;
	VNodeChunkStt	*pChunk;
	int				nI;

	pChunk = NULL;
	if( pVNodeMan->nTotalChunk > 0 )
	{  // find a chunk with empty slot
		for( pChunk = pVNodeMan->pStart; pChunk != NULL; pChunk = pChunk->pNext )
		{
			if( pChunk->nTotalUsed < MAX_CHUNK_ENT )
				break;
		}
	}

	if( pChunk == NULL )
	{	// allocate new chunk array
		pChunk = (VNodeChunkStt*)MALLOC( sizeof( VNodeChunkStt ) );
		if( pChunk == NULL )
		{	// allocation failed!
			return( NULL );
		}
		pVNodeMan->nTotalChunk++;
		// clear the chunk with 0
		memset( pChunk, 0, sizeof( VNodeChunkStt ) );
		if( pVNodeMan->pEnd == NULL )
		{	// no allocated chunk
			pVNodeMan->pStart = pVNodeMan->pEnd = pChunk;
			pChunk->pPre = pChunk->pNext = NULL;
		}
		else
		{
			pVNodeMan->pEnd->pNext	= pChunk;
			pChunk->pPre			= pVNodeMan->pEnd;
			pChunk->pNext			= NULL;
			pVNodeMan->pEnd			= pChunk;
		}	 
	}

	// find empty entry in the chunk
	for( nI = 0; nI < MAX_CHUNK_ENT; nI++ )
	{
		if( pChunk->vnode[nI].dwType == 0 )
			break;
	}
	if( nI >= MAX_CHUNK_ENT )
		return( NULL );

	pVNode = &pChunk->vnode[nI];
	
	// clear vnode with 0
	memset( pVNode, 0, sizeof( VNodeStt ) );

	// set other fields
	pVNode->dwType = VNTYPE_RESERVED;
	pVNode->pChunk = pChunk;		
	memcpy( &pVNode->op, &pVNodeMan->op, sizeof( VNodeOPStt ) );
	pVNode->nRefCount = 0;
	
	return( pVNode );
}

// increase vnode reference count
int inc_vnode_ref_count( VNodeStt *pVNode )
{
	if( pVNode == NULL )
		return( -1 );
	pVNode->nRefCount++;

	return(0);
}

// decrease vnode reference count
int dec_vnode_ref_count( VNodeStt *pVNode )
{
	if( pVNode == NULL )
		return( -1 );
	pVNode->nRefCount--;

	return(0);
}


// release vnode
int delete_vnode( VNodeStt *pVNode )
{
	if( pVNode == NULL )
		return( -1 );

	// decrease reference count
	dec_vnode_ref_count( pVNode );
	
	if( pVNode->nRefCount <= 0 )
	{
		pVNode->dwType = VNTYPE_FREE;
		pVNode->pChunk->nTotalUsed--;		// decrease the number of total entry int the chunk
		return( 0 );
	}		
	else
		return( 1 );
}

// hash char string
static DWORD dwGetHashKey( unsigned char *pS )
{
	int				nI;
	DWORD			*pDW;
	unsigned char	c[4];

	if( pS == NULL || pS[0] == 0 )
	{
		ERROR_PRINTF( "dwGetHashKey() - Empty hash string.\n" );
		return( 0 );
	}

	pDW		= (DWORD*)c;
	pDW[0]	= 0;
	
	for( nI = strlen( pS )-1; nI >= 0; nI-- )
	{
		c[nI%4] ^= pS[nI];
	}

	return( pDW[0] );
}

//  hash by vnode filename
int register_vnode_hash( VNodeManStt *pVNodeMan, VNodeStt *pVNode )
{
	DWORD			dwKey;
	VNodeHashEntStt	*pHEnt;

	// get hash key
	dwKey = dwGetHashKey( (unsigned char*)pVNode->szName );
	dwKey = dwKey % (DWORD)pVNodeMan->nTotalHash;

	pHEnt = &pVNodeMan->pVHash[ dwKey  ];	
	
	if( pHEnt->nTotal == 0 )
	{
		pHEnt->pStart = pHEnt->pEnd = pVNode;
		pVNode->pHashPre = pVNode->pHashNext = NULL;
	}
	else
	{
		pVNode->pHashPre		= pHEnt->pEnd;
		pVNode->pHashNext		= NULL;
		pHEnt->pEnd->pHashNext	= pVNode;
		pHEnt->pEnd				= pVNode;
	}

	pHEnt->nTotal++;

	return( 0 );
}

//  VNode의 파일명으로 Hash된 것을 제거한다.
int unregister_vnode_hash( VNodeManStt *pVNodeMan, VNodeStt *pVNode )
{
	int				nI;
	VNodeStt		*pVT;
	DWORD			dwKey;
	VNodeHashEntStt	*pHEnt;

	// Key를 구한다.
	dwKey = dwGetHashKey( (unsigned char*)pVNode->szName );
	dwKey = dwKey % (DWORD)pVNodeMan->nTotalHash;

	pHEnt = &pVNodeMan->pVHash[ dwKey ];	
	
	if( pHEnt->nTotal == 0 )
	{
		ERROR_PRINTF( "unregister_vnode_hash() : no entry.\n" );
		return( -1 );
	}

	// find vnode to release
	for( pVT = pHEnt->pStart, nI = 0; pVT != NULL && nI < pHEnt->nTotal; nI++, pVT = pVT->pHashNext )
	{
		if( pVT == pVNode )
		{
			if( pVT->pHashPre != NULL )
				pVT->pHashPre->pHashNext = pVT->pHashNext;
			else
				pHEnt->pStart = pVT->pHashNext;

			if( pVT->pHashNext != NULL )
				pVT->pHashNext->pHashPre = pVT->pHashPre;
			else
				pHEnt->pEnd = pVT->pHashPre;

			pHEnt->nTotal--;
			return( 0 );
		}
	}

	ERROR_PRINTF( "nunregister_vnode_hash() : vnode not found.\n" );

	return( -1 );
}

//  pParentVNode 를 상위 디렉토리로 하는 pS엔트리의 VNode가 이미 만들어져 있는지 찾아본다. 
VNodeStt *find_vnode( VNodeManStt *pVNodeMan, VNodeStt *pParentVNode, char *pS )
{
	DWORD			dwKey;
	VNodeStt		*pV;
	VNodeHashEntStt	*pHEnt;

	// Key를 구한다.
	dwKey = dwGetHashKey( (unsigned char*)pS );

	pHEnt = &pVNodeMan->pVHash[ dwKey % (DWORD)pVNodeMan->nTotalHash  ];	
	
	if( pHEnt->nTotal == 0 )
		return( NULL );

	for( pV = pHEnt->pStart; pV != NULL; pV = pV->pHashNext )
	{
		if( pV->dwType != VNTYPE_FREE && pV->pParentVNode == pParentVNode )
			return( pV );
	}

	// 찾을 수 없다.
	return( NULL );
}

static 	VFSStt			*pVFS;					// FAT32 file system on ram disk (root file system)
static  VFSStt			*pFAT32VFS;				// first IDE Partition
static  VFSStt			*pFDDFAT12;				// FDD Fat12 file system

static  BlkDevObjStt	*p_ram_blk_dev;			// Ram Disk
static  BlkDevObjStt	*p_idehdd_blk_dev;		// IDE HDD
static  BlkDevObjStt	*p_fdd35_blk_dev;		// FDD 3.5 Inch

static  BlkDevObjStt	*pPartObj;				// Partition

//CWDStt	cwd;

// vfs 파일 시스템을 초기화 한다.
int init_vfs()
{
	int	nR;

	// allocate block device objects
	p_ram_blk_dev		= (BlkDevObjStt*)MALLOC( sizeof( BlkDevObjStt ) );
	p_idehdd_blk_dev	= (BlkDevObjStt*)MALLOC( sizeof( BlkDevObjStt ) );
	p_fdd35_blk_dev		= (BlkDevObjStt*)MALLOC( sizeof( BlkDevObjStt ) );
	if( !p_ram_blk_dev || !p_idehdd_blk_dev || !p_fdd35_blk_dev )
	{
		if( p_ram_blk_dev		) FREE( p_ram_blk_dev		);
		if( p_idehdd_blk_dev	) FREE( p_idehdd_blk_dev	);
		if( p_fdd35_blk_dev		) FREE( p_fdd35_blk_dev		);
		return( -1 );
	}
	memset( p_ram_blk_dev	   , 0,	sizeof( BlkDevObjStt ) );
	memset( p_idehdd_blk_dev   , 0,	sizeof( BlkDevObjStt ) );
	memset( p_fdd35_blk_dev	   , 0,	sizeof( BlkDevObjStt ) );
			
	// initialize data structure of the root file system only
	init_filesystem();
					 
	// Register block device driver //////////////////////////////////////////////////////
	// these function are not called in the kernel code, but in each device driver initialization code.
	init_ram_disk_driver ();
	init_ide_hdd_driver	 ();
	init_fdd35_driver	 ();
	init_fdd_fat12_driver();
	init_fat16_driver	 ();
	///////////////////////////////////////////////////////////////////////////////////////

	// open each device driver ////////////////////////////////////////////////////////////
	// ram disk driver
	nR = open_block_device( p_ram_blk_dev, RAM_DISK_MAJOR, 0 );		// minor = 0
	if( nR == -1 )
	{
		ERROR_PRINTF( "Ram Disk Driver open failed.\n" );
		return( -1 );
	}

	// allocate fat32 filesystem for system's root filesystem
	pVFS = alloc_fat_fs_struct( p_ram_blk_dev, VFS_TYPE_FAT32 );
	// create fat32 filesystem via ioctl function call
	vfs_ioctl( pVFS, VFS_IOCTL_INIT, (DWORD)p_ram_blk_dev );
	// mount root
	vfs_mount( "/", pVFS );
	///////////////////////////////////////////////////////////////////////////////////////
	
	nR = kmkdir( "/dev" );
	
	// 3.5inch fdd driver
	nR = open_block_device( p_fdd35_blk_dev, FDD35_MAJOR, 0 );	// minor = 0 (A)
	if( nR == -1 )
	{
		ERROR_PRINTF( "3.5\" FDD driver open failed.\n" );
	}
	else
	{
		/* 부팅 속도가 너무 느려서 Comment out 2003-05-27
		// FDD를 FAT12으로 마운트 시도해 본다. (실패할 수도 있다.)/////////////////////////////
		nR= make_directory( "/a" );
		pFDDFAT12 = alloc_fat_fs_struct( p_fdd35_blk_dev, VFS_TYPE_FAT12 );
		
		// 자동으로 A를 마운트 한다. 
		JPRINTF( "mount fdd35a to /a" );
		// 마운트 한다.  (수동 마운트 한다.)
		nR = vfs_mount( "/a", pFDDFAT12 );
		if( nR == 0 )
			JPRINTF( " - ok\n" );
		else
			JPRINTF( " - error!\n" );
		///////////////////////////////////////////////////////////////////////////////////////
		*/
	}

	// IDE hard disk driver
	nR = open_block_device( p_idehdd_blk_dev, IDE_HDD_MAJOR, 0 );	// minor = 0
	if( nR == -1 )
	{
		ERROR_PRINTF( "IDE HDD Driver open failed.\n" );
		return( -1 );
	}				 	
	///////////////////////////////////////////////////////////////////////////////////////

	// 실제 HDD Partition을 mount할 디렉토리를 만든다./////////////////////////////////////
	nR = kmkdir( "/c" );
	// find the first open partition driver object
	pPartObj = find_dev_obj( HDD_PART_MAJOR, -1 );	// major, minor	( -1 = don't care )
	if( pPartObj != NULL )
	{	
		int nMediaType;

		nMediaType = chk_media_type( pPartObj );

		if( nMediaType == VFS_TYPE_FAT32 || nMediaType == VFS_TYPE_FAT16 )
		{
			// allocate fat32 filesystem on the partition device object
			pFAT32VFS = alloc_fat_fs_struct( pPartObj, nMediaType );
			// and mount
			JPRINTF( "mount hda1 to /c" );
			nR = vfs_mount( "/c", pFAT32VFS );
			if( nR == 0 )
				JPRINTF( " - ok\n" );
			else
				JPRINTF( " - error!\n" );
		}
		else
			JPRINTF( "unmountable media type : mounting /c failed!\n" );
	}
	///////////////////////////////////////////////////////////////////////////////////////

	return( 0 );
}

// unmount all the mounted filesystems
static int unmount_all()
{
	VFSStt			*pVFS, *pT;
	int				nTotal, nI, nR;

	// find the last vfs
	pVFS = find_last_vfs( &nTotal );
	if( pVFS == NULL || nTotal <= 0 )
	{
		JPRINTF( "no mounted filesystem.\n" );
		return( -1 );
	}

	// unmount and release vfs
	for( nI = 0; nI < nTotal && pVFS != NULL; pVFS = pT, nI++ )
	{
		if( pVFS->pRootNode != NULL )
		{	// call unmount function
			nR = vfs_unmount( pVFS );
			if( nR < 0 )
				JPRINTF( "[%2d]  %-12s - unmount failed!\n", nI, pVFS->szName );
			else
				JPRINTF( "[%2d]  %-12s - unmount ok.\n", nI, pVFS->szName );
		}
		else
			JPRINTF( "[%2d]  %-12s - not mounted.\n", nI, pVFS->szName );

		pT = pVFS->pPre;
		if( pVFS->op.free != NULL )
		{	// release vfs struct
			nR = pVFS->op.free( pVFS );
			if( nR < 0 )
				JPRINTF( "unmount_all() - vfs.op.free() returned error.\n" ); 
		}
		else
			JPRINTF( "unmount_all() - the vfs does not support free function.\n" );

	}	

	return( 0 );
}	

// close vfs
int close_vfs()
{
	int				nR, nTotal;
	BlkDevObjStt	*pObj, *pNextObj;

	// close CWD
#ifdef WIN32TEST
	close_cwd();
#endif

	// unmount and release filesystems
	unmount_all();

	// close block device objects
	pObj = find_first_blkdev_obj( &nTotal );
	for( ; pObj != NULL; pObj = pNextObj )
	{
		pNextObj = pObj->pNext;
		if( !( pObj->nAttr & BLKDEV_ATTR_NESTED ) )
		{
			nR = close_block_device( pObj );
			FREE( pObj );		// must release object memory
		}
	}

	// relinquis device driver	( keep this oeder )
	close_ram_disk_driver	();
	close_ide_hdd_driver	();
	close_fdd35_driver		();
	close_fdd_fat12_driver	();
	close_fat16_driver		();
	
	// relinquish root filesystem
	close_filesystem();

	return( 0 );
}

// get subpath of the vnode
static int _get_vnode_subpath( VNodeStt *pVNode, char *pPath )
{
	int			nR;
	char		szT[260];
	VNodeStt	*pT;

	pPath[0] = szT[0] = 0;

	pT = pVNode;
	if( pT->pMountPoint != NULL )
		pT = pT->pMountPoint;	

	if( pT->pParentVNode != NULL )
	{
		nR = _get_vnode_subpath( pT->pParentVNode, szT );
		if( nR < 0 )
			return( -1 );

		sprintf( pPath, "%s/%s", szT, pT->szName );
	}

	return( 0 );
}

// get full path of the vnode
int get_vnode_fullpath( VNodeStt *pVNode, char *pPath )
{
	int		nR;

	pPath[0] = 0;

	// system root directory
	if( pVNode->szName[0] == '/' && pVNode->szName[1] == 0 )
		strcpy( pPath, "/" );
	else // sub directory 
		nR = _get_vnode_subpath( pVNode, pPath );

	return( nR );
}

// check media type  (return : VFS_TYPE_xx )
int chk_media_type( BlkDevObjStt *pDevObj )
{
	HddPartStt	*pHP;

	if( pDevObj->pDev->nMajor == FDD35_MAJOR )
		return( VFS_TYPE_FAT12 );

	else if( pDevObj->pDev->nMajor == HDD_PART_MAJOR )
	{
		pHP = pDevObj->pPtr;

		if( pHP->p.byType == 0x0C || pHP->p.byType == 0x0B )
			return( VFS_TYPE_FAT32 );
		else if( pHP->p.byType == 0x06 )
			return( VFS_TYPE_FAT16 );
	}												

	JPRINTF( "unknown hdd partition type = 0x%X\n", pHP->p.byType );
	return( VFS_TYPE_UNKNOWN );
}











