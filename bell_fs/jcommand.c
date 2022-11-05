#include "jshell.h"
#include "jcommand.h"
#include "vfs.h"

#include "digit.h"

static int nJFUNC_ls		 ( int argc, char *argv[] );
static int nJFUNC_ren		 ( int argc, char *argv[] );
static int nJFUNC_bdev		 ( int argc, char *argv[] );
static int nJFUNC_rdev		 ( int argc, char *argv[] );
static int nJFUNC_type		 ( int argc, char *argv[] );
static int nJFUNC_exit		 ( int argc, char *argv[] );
static int nJFUNC_mkdir		 ( int argc, char *argv[] );
static int nJFUNC_rmdir		 ( int argc, char *argv[] );
static int nJFUNC_mount		 ( int argc, char *argv[] );
static int nJFUNC_files		 ( int argc, char *argv[] );
static int nJFUNC_vnodes	 ( int argc, char *argv[] );
static int nJFUNC_unmount	 ( int argc, char *argv[] );
static int nJFUNC_attachfs	 ( int argc, char *argv[] );
static int nJFUNC_openblkdev ( int argc, char *argv[] );
static int nJFUNC_filesystem ( int argc, char *argv[] );
static int nJFUNC_closeblkdev( int argc, char *argv[] );

////////////////////////////////////////////
typedef enum {
	JC_UNKNOWN = 0,		JC_EXIT,		JC_LS,
	JC_MKDIR,			JC_TYPE,
	JC_RMDIR,			JC_BDEV,		
	JC_RDEV,			JC_REN,
	JC_MOUNT,			JC_UNMOUNT,
	JC_FILESYSTEM,		JC_FILES,
	JC_VNODES,			JC_OPENBLKDEV,
	JC_CLOSEBLKDEV,		JC_ATTACHFS,

	END_OF_JC
} JC_TAG;


typedef struct {
	int		nType;
	int		(*pFunc)( int argc, char *argv[] );
	char	*pS;
	char	*pHelp;
} JCStrStt;

static JCStrStt jc[] = {
	{ JC_ATTACHFS,		nJFUNC_attachfs,	"attachfs",		"attach file system."		},
	{ JC_BDEV,			nJFUNC_bdev,		"bdev",			"list block device drivers."},
	{ JC_CLOSEBLKDEV,	nJFUNC_closeblkdev,	"closeblkdev",	"<major_number> <minor_number>; close block device."		},
	{ JC_EXIT,			nJFUNC_exit,		"exit",			"not used.."				},
	{ JC_FILES,			nJFUNC_files,		"files",		"list open files."			},
	{ JC_FILESYSTEM,	nJFUNC_filesystem,	"filesystem",	"list filesystems."			},
	{ JC_LS,			nJFUNC_ls,			"ls",			"list directory entries."	},
	{ JC_MKDIR,			nJFUNC_mkdir,		"mkdir",		"make new directory."		},
	{ JC_MOUNT,			nJFUNC_mount,		"mount",		"mount filesystem."			},
	{ JC_OPENBLKDEV,	nJFUNC_openblkdev,	"openblkdev",	"open blockd evice."		},
	{ JC_EXIT,			nJFUNC_exit,		"q",			"not used."					},
	{ JC_RMDIR,			nJFUNC_rmdir,		"rmdir",		"remove directory."			},
	{ JC_REN,			nJFUNC_ren,			"ren",			"rename file."				},
	{ JC_RDEV,			nJFUNC_rdev,		"rdev",			"<major> <minor> <index>; read block device."		},
	{ JC_TYPE,			nJFUNC_type,		"type",			"display file's contents."	},
	{ JC_UNMOUNT,		nJFUNC_unmount,		"unmount",		"unmount filesystem."		},
	{ JC_VNODES,		nJFUNC_vnodes,		"vnodes",		"display vnodes."			},

	{ 0,			NULL,				NULL	},
};	
///////////////////////////////////////////////
static void *get_jshell_commamd_type( char *pS, int *pType )
{
    int nI;

	*pType = JC_UNKNOWN;

    for( nI = 0; jc[nI].pS != NULL; nI++ )
    {
        if( strcmpi( pS, jc[nI].pS ) == 0 )
		{
         	*pType = jc[nI].nType;
			return( jc[nI].pFunc );
		}
    }

    return( NULL );
}

int disp_jhelp()
{
	int nI;

	for( nI = 0; jc[nI].nType != 0; nI++ )
		JPRINTF( "%-10s : %s\n", jc[nI].pS, jc[nI].pHelp );

	return( 0 );
}

// display file contents
static int nJFUNC_type( int argc, char *argv[] )
{
	char	szStr[81];
	int		nHandle, nR;

	if( argc < 1 )
	{
		JPRINTF( "type <file_path>\n" );
		return( 0 );
	}

	// open file
	nHandle = kopen( argv[0], FM_READ );

	for( ;; )
	{
		nR = kread( nHandle, szStr, sizeof( szStr ) -1 );
		if( nR <= 0 )
			break;

		szStr[ sizeof( szStr ) -1 ] = 0;

		JPRINTF( "%s", szStr );
	}	

	// close file
	kclose( nHandle );

	// line feed
	JPRINTF( "\n" );

	return(0);
}

// shell을 종료한다.
static int nJFUNC_exit( int argc, char *argv[] )
{
	// -100을 리턴해야 shell을 빠져 나간다.
	return(-100);
}

// list directory
static  int nJFUNC_ls( int argc, char *argv[] )
{
	DIRENTStt	dir_ent;
	char		szDir[260];
	int			nHandle, nR, nI;

	if( argc == 0 )
		strcpy( szDir, "/" );		// 인자가 없으면 루트 디렉토리를 대상으로 한다.
	else
		strcpy( szDir, argv[argc-1] );		// 가장 마지막 인자를 디렉토리로 가정한다.

	nHandle = kopendir( szDir );
	if( nHandle >= 0 )
	{
		
		for( nI = 0; ; nI++ )
		{
			nR = kreaddir( &dir_ent, nHandle );
			if( nR < 0 )
				break;

			JPRINTF( "[%3d] (%08X)%8d / %-8d %5d %-13s %s\n", nI, dir_ent.dwStartCluster, dir_ent.dwStartCluster, 
				dir_ent.dwStartBlock, dir_ent.lFileSize, dir_ent.szAlias, dir_ent.szFileName );
		}	
		kclosedir( nHandle );
	}	
	else
		JPRINTF( "ls - open_directory( %s ) failed!\n", szDir );

	return( 0 );
}

static  int nJFUNC_mkdir( int argc, char *argv[] )
{
	int nR;
	
	if( argc < 1 )
	{
		JPRINTF( "mkdir <dir_path>\n" );
		return( 0 );
	}

	nR = kmkdir( argv[0] );
	JPRINTF( "mkdir(%s) - %d\n", argv[0], nR );
	
	return( nR );
}

static  int nJFUNC_rmdir( int argc, char *argv[] )
{
	int nR;
	
	if( argc < 1 )
	{
		JPRINTF( "rmdir <dir_path>\n" );
		return( 0 );
	}

	nR = kremove( argv[0] );
	
	return( nR );
}

static  int nJFUNC_bdev( int argc, char *argv[] )
{
	BlkDevObjStt	*pObj;
	int				nTotal, nI;
	
	// find the first registered block device object
	pObj = find_first_blkdev_obj( &nTotal );
	if( pObj == NULL )
		return( -1 );

	JPRINTF( "name           major     minor     total_block\n" );
	for( nI = 0; pObj != NULL && nI < nTotal; nI++ )
	{
		JPRINTF( "%-16s %-4d      %-4d      %-4d\n", pObj->pDev->szName, pObj->pDev->nMajor, pObj->nMinor, pObj->dwTotalBlk );
		pObj = pObj->pNext;
	}
	return( 0 );
}

static void vDumpBuffer( UCHAR *pBuff, int nSize )
{
	int		nI, nJ;
	char	szT[100], szX[10];

	for( nJ = nI = 0; nI < nSize; nI+=16 )
	{
		szT[0] = 0;
		for( nJ = 0; nI+nJ < nSize && nJ < 16; nJ++ )
		{	// 16진수 값.
			sprintf( szX, "%02X ", pBuff[nI+nJ] );
			strcat( szT, szX );
		}
		strcat( szT, "  " );

		for( nJ = 0; nI+nJ < nSize && nJ < 16; nJ++ )
		{	// 문자
			if( (' ' <= pBuff[nI+nJ] && pBuff[nI+nJ] <= '}') || (UCHAR)pBuff[nI+nJ] == (UCHAR)0xE5 )
			{
				sprintf( szX, "%c", pBuff[nI+nJ] );
				strcat( szT, szX );
			}
			else
				strcat( szT, "." );
		}

		JPRINTF( "%s\n", szT );
	}	
}

// read block device "RDEV major minor index"
static  int nJFUNC_rdev( int argc, char *argv[] )
{
	BlkDevObjStt	*pObj;
	DWORD			dwIndex;
	unsigned char	buff[512];
	int				nMajor, nMinor, nR;

	if( argc < 3 )
	{
		JPRINTF( "usage : rdev <major> <minor> <index>" );
		return( -1 );
	}

	nMajor  = (int)atoul( argv[0] );
	nMinor  = (int)atoul( argv[1] );
	dwIndex = atoul( argv[2] );
		
	// 지정된 블록 디바이스 드라이버를 찾는다.
	pObj = find_dev_obj( nMajor, nMinor );
	if( pObj == NULL )
		return( -1 );

	// 블록 디바이스를 읽는다.
	nR = read_block( pObj, dwIndex, buff, 1 );
	if( nR < 0 )
		return( -1 );

	// 읽은 것을 출력한다.
	vDumpBuffer( buff, sizeof( buff )/2 );

	return( 0 );
}

// 새 파일 시스템을 마운트 한다.( argv[0] = CFS ID )
// 마운트 하기 전에 alloc_fat_fs_struct()를 호출할 것.
static int nJFUNC_mount( int argc, char *argv[] )
{
	VFSStt	*pVFS;
	int		nR, nI, nID, nTotal;

	if( argc < 2 )
	{   // 사용법 출력.
		ERROR_PRINTF("mount <vfs_id> <mount_point>\n" );
		return( -1 );
	}	

	// VFS ID를 구한다.
	nID = (int)atoul( argv[0] );
	if( nID <= 0 )
	{
		ERROR_PRINTF( "Invalid VFS ID(%d).\n", nID );
		return( -1 );
	}

	// VFS를 찾는다.
	pVFS = find_first_vfs( &nTotal );
	for( nI = 0; nI < nTotal && pVFS != NULL; nI++, pVFS = pVFS->pNext )
	{
		if( nI == nID )
		{	// 실제 mount 함수를 호출한다.
			nR = vfs_mount( argv[1], pVFS );
			return( nR );
		}
	}

	ERROR_PRINTF( "VFS not found.\n" );

	return( -1 );
}

// unmount attached file system
static int nJFUNC_unmount( int argc, char *argv[] )
{
	int			nR;
	VFSStt		*pVFS;
	int			nHandle;
	VNodeStt	*pVNode;

	if( argc < 1 )
	{
		ERROR_PRINTF( "unmount <mount_point>\n" );
		return( -1 );
	}

	// open directory
	nHandle = kopendir( argv[0] );
	if( nHandle == -1 )
	{
		ERROR_PRINTF("nJFUNC_unmount() - directory open error\n" );
		return( -1 );
	}

	// get vnode pointer
	pVNode = handle_to_vnode( nHandle );
	if( pVNode == NULL )
	{
		ERROR_PRINTF("nJFUNC_unmount() - vnode pointer not found.\n" );
		return( -1 );
	}

	// get vfs
	pVFS = pVNode->pVFS;
	kclosedir( nHandle );

	// call real unmount function
	nR = vfs_unmount( pVFS );

	return( nR );
}

// display file descriptors
static int nJFUNC_files( int argc, char *argv[] )
{
	// display system file descriptor
	display_sys_fd();

	return( 0 );
}

// display vnodes
static int nJFUNC_vnodes( int argc, char *argv[] )
{
	// display vnodes
	display_vnodes();

	return( 0 );
}


// display filesystem information
static int nJFUNC_filesystem( int argc, char *argv[] )
{
	VFSStt			*pVFS;
	BlkDevObjStt	*pDevObj;
	int				nTotal, nI;
	VNodeStt		*pMountPoint;
	char			szDeviceName[64];
	char			szMountPoint[260];

	// find the first vfs
	pVFS = find_first_vfs( &nTotal );
	if( pVFS == NULL || nTotal <= 0 )
	{
		JPRINTF( "no vfs.\n" );
		return( -1 );
	}

	for( nI = 0; nI < nTotal && pVFS != NULL; pVFS = pVFS->pNext, nI++ )
	{
		pDevObj		= pVFS->pDevObj;

		if( pVFS->pRootNode != NULL )
		{	// get mount point string
			pMountPoint = pVFS->pRootNode->pMountPoint;
			if( pMountPoint != NULL )
				get_vnode_fullpath( pMountPoint, szMountPoint ); // mount point
			else if( strcmp( pVFS->pRootNode->szName, "/" ) == 0 )
				strcpy( szMountPoint, "/" );					 // root node
			else
				strcpy( szMountPoint, "not mounted" );			 // not mounted
		}
		else
			strcpy( szMountPoint, "not mounted" );

		if( pDevObj != NULL )
			strcpy( szDeviceName, pDevObj->pDev->szName );
		else
			strcpy( szDeviceName, "UNKNOWN" );

		JPRINTF( "[%2d]  %-12s  %-16s %s\n", nI, pVFS->szName, szDeviceName, szMountPoint );
	}	

	return( 0 );
}

// attach block device object to a specific filesystem 
// ( parameter : block device major & minor number, filesystem type)
static int nJFUNC_attachfs( int argc, char *argv[] )
{
	VFSStt			*pVFS;
	BlkDevObjStt	*pObj;
	int				nMajor, nMinor;
	int				nFileSystemType;

	if( argc < 3 )
	{
		JPRINTF( "attachfs <block_device_major> <block_device_minor> <filesystem_type>\n" );
		return( -1 );
	}

	// get major and minor number
	nMajor = atoul( argv[0] );
	nMinor = atoul( argv[1] );

	// get file system type
	nFileSystemType = get_filesystem_type( argv[2] );
	if( nFileSystemType <= 0 )
	{
		JPRINTF( "unknown filesystem type %s!\n", argv[2] );
		return( -1 );
	}

	// find block device
	pObj = find_dev_obj( nMajor, nMinor );
	if( pObj == NULL )
	{
		ERROR_PRINTF( "nJFUNC_attachfs() - block device object( %d, %d ) not found!\n", nMajor, nMinor );
		return( -1 );
	}				 

	// allocate fileystem
	pVFS = alloc_fat_fs_struct( pObj, nFileSystemType );  // in the function the filesystem will be registered
	if( pVFS == NULL )
	{
		ERROR_PRINTF( "nJFUNC_attachfs() - alloc_fat_fs_struct returned an error!\n" );
		return( -1 );
	}

	return( 0 );
}

// open block device
static int nJFUNC_openblkdev( int argc, char *argv[] )
{
	BlkDevObjStt	*pObj;
	int				nR, nMajor, nMinor;
	
	if( argc < 2 )
	{	// insufficient parameter
		ERROR_PRINTF( "openblkdev <major_number> <minor_number>\n" );
		return( -1 );
	}

	// get major and minor number
	nMajor = atoul( argv[0] );
	nMinor = atoul( argv[1] );

	// allocate block device object
	pObj = (BlkDevObjStt*)MALLOC( sizeof( BlkDevObjStt ) );
	if( pObj == NULL )
	{
		ERROR_PRINTF( "nJFUNC_openblkdev() - memory allocation failed!\n" );
		return( -1 );
	}  
	memset( pObj, 0, sizeof( BlkDevObjStt ) );

	// open block device
	// the block device object and its nested object will be registered int this function
	nR = open_block_device( pObj, nMajor, nMinor );	 
	if( nR < 0 )
	{
		FREE( pObj );	// release memory
		ERROR_PRINTF( "nJFUNC_openblkdev() - opening block device failed!\n" );
		return( -1 );
	}				 

	return( 0 );
}

// close block device
static int nJFUNC_closeblkdev( int argc, char *argv[] )
{
	BlkDevObjStt	*pObj;
	int				nR, nMajor, nMinor;

	if( argc < 2 )
	{	// insufficient parameter
		ERROR_PRINTF( "closeblkdev <major_number> <minor_number>\n" );
		return( -1 );
	}

	// get major and minor number
	nMajor = atoul( argv[0] );
	nMinor = atoul( argv[1] );

	// find block device object
	pObj = find_dev_obj( nMajor, nMinor );
	if( pObj == NULL )
	{	// block device not found
		ERROR_PRINTF( "nJFUNC_closeblkdev() - block device object( %d, %d ) not found!\n", nMajor, nMinor );
		return( -1 );
	}

	// close block device object
	nR = close_block_device( pObj );
	if( nR < 0 )
	{
		ERROR_PRINTF( "nJFUNC_closeblkdev() - closing block device failed!\n", nMajor, nMinor );
		return( -1 );
	}
	FREE( pObj );

	return( 0 );
}

static  int nJFUNC_ren( int argc, char *argv[] )
{
	int nR;

	if( argc < 2 )
	{
		ERROR_PRINTF("ren <old_path> <new_path>\n" );
		return( -1 );
	}
	
	nR = krename( argv[0], argv[1] );
	
	return( nR );
}

// 입력된 명령 스트링을 argv로 파싱한다.
static int jshell_arg_parsing( char *argv[], char *pS )
{
	int  nTotal, nI, nX;
	
	nTotal = 0;
	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( nI == 0 && pS[0] != ' ')
			argv[nTotal++] = pS;
		else
		{
			if( pS[nI-1] == ' ' && pS[nI] != ' ' )
				argv[nTotal++] = &pS[nI];
		}	
	}

	for( nX = 1; nX < nI; nX++ )
	{
		if( pS[nX-1] != ' ' && pS[nX] == ' ' )
			pS[nX] = 0;
	} 

	return( nTotal );
}

// display jshell prompt
int jshell_disp_prompt()
{
	// 프롬프트를 조합한다.
	JPRINTF( "J>" );

	return(0);
}

#define MAX_JCOMMAND_ARGV	32
		
// 입력된 명령을 실행한다.
int jshell_command( char *pCmdStr )
{
	int		argc, nResult;
	int		nType;
	char	*argv[ MAX_JCOMMAND_ARGV ];
	int		(*pFunc)( int argc, char *argv[] );

	nResult = 0;

	// Parsing한다.
	argc = jshell_arg_parsing( argv, pCmdStr );
	
/*	//Parsing한 것이 맞는지 출력해 본다.
	JPRINTF( "argc = %d\n", argc );
	{
		int nI;
		for( nI = 0; nI < argc; nI++ )
			JPRINTF( "argv[%d] : %s\n", nI, argv[nI] );
	}
*/	
	if( argc == 0 )		// 입력된 것이 없다.
		return( 0 );
	
	// 명령의 타입을 구한다.
	pFunc = get_jshell_commamd_type( argv[0], &nType );

	// 함수가 존재하면 CALL한다.
	if( pFunc != NULL )
		nResult = pFunc( argc-1, &argv[1] );

	return( nResult );			// shell에서 계속 루프를 돈다.
}


