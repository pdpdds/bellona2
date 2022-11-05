/** @file vfs.h
 * @brief vfs ���� Definition
 */

#ifndef VFS_HEADER_jj
#define VFS_HEADER_jj

#ifdef WIN32TEST
	#include <windows.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <io.h>
#endif

#ifdef BELLONA2
	#include <types.h>
    #include <string.h>
    #include <stdlib.h>
#endif

#ifdef BELL_FS
	extern void *kmalloc( DWORD dwSize );
	extern int  kfree( void *pV );
	extern int  kdbg_printf( char *lpFmt, ... );
#endif       

#ifndef BELL_EXPORT
#define BELL_EXPORT __declspec( dllexport )
#endif

#include "blkdev.h"
#include "file.h"
#include "fdd35.h"
#include "fat32.h"
#include "../major.h"
//#include "cache.h"
#include "bcache.h"
#include "idehdd.h"
#include "ramdisk.h"
#include "fddfat12.h"
#include "f16part.h"
#include "stdinout.h"
#include "digit.h"

#ifdef WIN32TEST/////////////////////////////////////////////////////////

extern int jprintf( char *lpFmt, ... );

#define ERROR_PRINTF	printf

#include "w32ide.h"
#define  MALLOC		malloc
#define  FREE		free
#define  JPRINTF	jprintf

#endif//-----------------------------------------------------------------
#ifdef  BELLONA2

#include "..\util.h"

#define  ERROR_PRINTF	kdbg_printf
#define  MALLOC			kmalloc
#define  FREE			kfree
#define  JPRINTF		kdbg_printf

#endif
/////////////////////////////////////////////////////////////////////////


struct VFSTag;
struct VNodeTag;

typedef enum VFS_IOCTL_TAG {
	VFS_IOCTL_INIT = 1,		// ���� �ý��� ����.  (����� ����̽��� IOCTL�� CALL �Ѵ�.)

	END_OF_IOCTL
};// VFS_IOCTL_TAG;

// file struct  //////////////////////////////////////////
typedef struct file_t_TAG{
	DWORD	dwMode;			// Open�̳� Create�� Mode
	int		nRefCount;		// Referenct Count
	DWORD	dwOffs;			// File Pointer
};
typedef struct file_t_TAG file_t;

// vnode member operation
typedef struct VNodeOPTag{
	struct VNodeTag *( *open   )( struct VNodeTag *pParentNode, char *pName, DWORD dwMode );  
	struct VNodeTag *( *create )( struct VNodeTag *pParentNode, char *pName, DWORD dwType );  
	int ( *close   )( struct VNodeTag *pVNode );
	int ( *read    )( struct VNodeTag *pVNode, DWORD dwOffs, void *pBuff, int nSize );
	int ( *write   )( struct VNodeTag *pVNode, DWORD dwOffs, void *pBuff, int nSize );
	int ( *lseek   )( struct VNodeTag *pVNode, long lOffset, int nOrigin );
	int ( *readdir )( struct VNodeTag *pVNode, DWORD dwOffs, DIRENTStt *pDIRENT );
	int	( *remove  )( struct VNodeTag *pVNode, char *pS );
	int	( *rename  )( struct VNodeTag *pOldVNode, char *pOldName, struct VNodeTag *pNewVNode, char *pNewName  );
	int ( *get_info)( struct VNodeTag *pVNode, char *pName, DIRENTStt *pDirEnt );
};
typedef struct VNodeOPTag VNodeOPStt;

typedef enum {
	VNTYPE_FREE		= 0,		// empty entry
	VNTYPE_RESERVED,			// reserved to use

	VNTYPE_ROOT = 0x1000,		// root vnode

	ENDOF_VNTYPE
} VNODE_TYPE_TAG;

struct VNodeChunkTag;
// vnode  ////////////////////////////////////////////////
typedef struct VNodeTag{
	DWORD					dwType;				// File�� Type
	DWORD					dwFileSize;			// File Size
	char					szName[260];		// ������ �̸�... (�ƹ����� vnode�� ������ �̸��� �ξ�� �� �Ͱ���.)
	int						nRefCount;			// Reference Count
	VNodeOPStt				op;					// vnode operations
	struct VFSTag			*pVFS;				// VNode�� ���� VFS
	struct VNodeChunkTag	*pChunk;			// VNode�� ���� VNode Chunk 
	struct VNodeTag			*pParentVNode;		// �θ� ���丮�� VNODE
	void					*pFSSD;				// file system specific data
	int						nMountDirHandle;	// directory handle of the mount point directory.
	struct VNodeTag			*pBranchVNode;		// Ư�� ���丮 Node���� ����Ʈ�� Root Node�� ���ϴ� ��ũ
	struct VNodeTag			*pMountPoint;		// Root Node���� �ٸ� ���Ͻý����� ���丮 Node�� ���ϴ� ��ũ
							
	// VNode Name�� ���� Hash Pre, Next
	struct VNodeTag			*pHashPre;		// �ٸ� �̸��� ���� ������ ������ Hash�� �� pPre, pNext�� �̿��Ͽ� �����Ѵ�.
	struct VNodeTag			*pHashNext;		
}; 
typedef struct VNodeTag VNodeStt;

#define MAX_CHUNK_ENT	32

// vnode chunk structure//////////////////////////////////
typedef struct VNodeChunkTag {
	int						nTotalUsed;
	VNodeStt				vnode[MAX_CHUNK_ENT];
	struct VNodeChunkTag	*pPre, *pNext;
};
typedef struct VNodeChunkTag VNodeChunkStt;

// vnode hash entry structure
typedef struct VNodeHashEntTag {
	int			nTotal;
	VNodeStt	*pStart, *pEnd;
};
typedef struct VNodeHashEntTag VNodeHashEntStt;

// vnode manager structure
typedef struct VNodeManTag {
	int				nTotalChunk;
	VNodeChunkStt	*pStart, *pEnd;
	VNodeOPStt		op;				// vnode�� ������ ������ �� Operation���� ����ȴ�.

	// VNode Hash
	int				nTotalHash;		// Hash Index�� ����
	VNodeHashEntStt	*pVHash;		// Hash Index �迭�� ���� ������

};
typedef struct VNodeManTag VNodeManStt;

// vops  /////////////////////////////////////////////////
typedef struct VFSOPTag {
	int ( *mount   )( void *pV );
	int ( *unmount )( void *pV );
	int ( *sync    )( void *pV );
	int ( *free	   )( void *pV );
	int ( *ioctl   )( int nCmd, DWORD dwParam );
};
typedef struct VFSOPTag VFSOPStt;

/*! @enum VFS_TYPE_TAG
 *  @brief File system type.
 *  VFSStt.dwType�� ����ȴ�.
 */
typedef enum {						
	VFS_TYPE_UNKNOWN = 0,			/*!< UNKNOWN <BR>*/
	VFS_TYPE_FAT32,					/*!< FAT32   <BR>*/
	VFS_TYPE_FAT16,					/*!< FAT16   <BR>*/
	VFS_TYPE_FAT12,					/*!< FAT12   <BR>*/
	END_OF_VFS_TYPE
} VFS_TYPE_TAG;						

// vfs access mode
typedef enum {
	VFS_MODE_RDONLY = 0,			// open read only
	VFS_MODE_RDWR,					// open read write

	END_OF_VFS_MODE	
} VFS_MODE_TAG;						// VFSStt.dwMode

// vfs  //////////////////////////////////////////////////
typedef struct VFSTag{
	char			szName[32];		// filesystem name
	DWORD			dwType;			// vfs type
	VFSOPStt		op;				// vfs operations			
	BlkDevObjStt	*pDevObj;		// device object
	DWORD			dwMode;			// vfs access mode
	
	//------------------------------// Mount �������� ������ ��.
	VNodeStt		*pMountNode;	// Mount�� ��ġ�� �ִ� VNODE
	VNodeStt		*pRootNode;		// ���� ���� �ý����� Root VNODE  ( �Ҵ��ؼ� ����Ѵ�. )
	VNodeManStt		*pVNodeMan;		// VNode Mamager
	//CacheManStt		*pCache;		// Cache Manager�� ���� ������
	void			*pPrivate;		// op�� �� operation���� pPrivate�� ���Ǵ�� ����Ѵ�.  (���� �ý��۸��� �ٸ���.)
	//------------------------------// ��� mount �������� �Ҵ�ǰ� unmount�������� �����ȴ�.

	struct  VFSTag  *pPre;
	struct	VFSTag	*pNext;
};
typedef struct VFSTag VFSStt;

// file system  //////////////////////////////////////////

// ��ü ���� �ý����� ROOT NODE�� �����Ѵ�.
extern int set_filesystem_root( VNodeStt *pVNode );

// register filesystem
extern int register_filesystem( VFSStt *pVFS );

// unregister fileystem
extern int unregister_filesystem( VFSStt *pVFS );

// ���� �ý��۰� ��� ����̽��� �����Ѵ�.
extern int link_filesystem_device( VFSStt *pVFS, BlkDevObjStt *pDevObj );
extern int unlink_filesystem_device( VFSStt *pVFS );

// VFS�� IOCTL
extern vfs_ioctl( VFSStt *pVFS, int nCmd, DWORD dwParam );

// MOUNT�Ѵ�.
extern int vfs_mount( char *pPath, VFSStt *pVFS );

// UNMOUNT�Ѵ�.
extern int vfs_unmount( VFSStt *pVFS );

// VNODE�� �����Ѵ�.
extern int delete_vnode( VNodeStt *pVNode );

// ���ο� VNODE ����ü�� �Ҵ��Ѵ�.
extern VNodeStt *new_vnode( VNodeManStt *pVNodeMan );

// VNode Manager�� ����ϰ� �ִ� �޸𸮸� ��� �����Ѵ�.
extern int delete_vnode_manager( VNodeManStt *pVNodeMan );

// VNODE Manager struct�� �Ҵ��ϰ� �ʱ�ȭ �Ѵ�.
extern VNodeManStt	*make_vnode_manager( VNodeOPStt *pVNodeOP, int nTotalHashIndex );

// ���� �ý����� ��Ʈ VNode�� ���Ѵ�.
extern VNodeStt *get_root_vnode();

// VNode�� ���ϸ����� Hash�Ѵ�.
extern int register_vnode_hash( VNodeManStt *pVNodeMan, VNodeStt *pVNode );

//  VNode�� ���ϸ����� Hash�� ���� �����Ѵ�.
extern int unregister_vnode_hash( VNodeManStt *pVNodeMan, VNodeStt *pVNode );

//  pParentVNode �� ���� ���丮�� �ϴ� pS��Ʈ���� VNode�� �̹� ������� �ִ��� ã�ƺ���. 
extern VNodeStt *find_vnode( VNodeManStt *pVNodeMan, VNodeStt *pParentVNode, char *pS );

// VNODE�� RefCount�� �ϳ� ���ҽ�Ų��.
extern int dec_vnode_ref_count( VNodeStt *pVNode );

// VNODE�� RefCount�� �ϳ� ������Ų��.
extern int inc_vnode_ref_count( VNodeStt *pVNode );

// vfs�� �ʱ�ȭ�Ѵ�.
extern int init_vfs();

// vfs�� �ݴ´�. 
extern int close_vfs();

// return total number of the registered vfses and the pointer to the first vfs.
extern VFSStt *find_first_vfs( int *pTotal );

// get full path of the vnode
extern get_vnode_fullpath( VNodeStt *pVNode, char *pPath );

// display vnodes
extern void display_vnodes();

// get filesystem type from string
extern int get_filesystem_type( char *pStr );

// check media type  (return : VFS_TYPE_xx )
extern int chk_media_type( BlkDevObjStt *pdevObj );


#endif
