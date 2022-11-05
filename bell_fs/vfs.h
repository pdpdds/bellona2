/** @file vfs.h
 * @brief vfs 관련 Definition
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
	VFS_IOCTL_INIT = 1,		// 파일 시스템 포맷.  (연결된 디바이스의 IOCTL을 CALL 한다.)

	END_OF_IOCTL
};// VFS_IOCTL_TAG;

// file struct  //////////////////////////////////////////
typedef struct file_t_TAG{
	DWORD	dwMode;			// Open이나 Create된 Mode
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
	DWORD					dwType;				// File의 Type
	DWORD					dwFileSize;			// File Size
	char					szName[260];		// 파일의 이름... (아무래도 vnode에 파일의 이름을 두어야 할 것같다.)
	int						nRefCount;			// Reference Count
	VNodeOPStt				op;					// vnode operations
	struct VFSTag			*pVFS;				// VNode가 속한 VFS
	struct VNodeChunkTag	*pChunk;			// VNode가 속한 VNode Chunk 
	struct VNodeTag			*pParentVNode;		// 부모 디렉토리의 VNODE
	void					*pFSSD;				// file system specific data
	int						nMountDirHandle;	// directory handle of the mount point directory.
	struct VNodeTag			*pBranchVNode;		// 특정 디렉토리 Node에서 마운트된 Root Node로 통하는 링크
	struct VNodeTag			*pMountPoint;		// Root Node에서 다른 파일시스템의 디렉토리 Node로 통하는 링크
							
	// VNode Name에 의한 Hash Pre, Next
	struct VNodeTag			*pHashPre;		// 다른 이름이 서로 동일한 값으로 Hash될 때 pPre, pNext를 이용하여 연결한다.
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
	VNodeOPStt		op;				// vnode가 생성될 때마다 이 Operation들이 복사된다.

	// VNode Hash
	int				nTotalHash;		// Hash Index의 개수
	VNodeHashEntStt	*pVHash;		// Hash Index 배열에 대한 포인터

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
 *  VFSStt.dwType에 저장된다.
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
	
	//------------------------------// Mount 시점에서 설정할 것.
	VNodeStt		*pMountNode;	// Mount한 위치에 있는 VNODE
	VNodeStt		*pRootNode;		// 현재 파일 시스템의 Root VNODE  ( 할당해서 사용한다. )
	VNodeManStt		*pVNodeMan;		// VNode Mamager
	//CacheManStt		*pCache;		// Cache Manager에 대한 포인터
	void			*pPrivate;		// op의 각 operation들이 pPrivate를 임의대로 사용한다.  (파일 시스템마다 다르다.)
	//------------------------------// 통상 mount 시점에서 할당되고 unmount시점에서 해제된다.

	struct  VFSTag  *pPre;
	struct	VFSTag	*pNext;
};
typedef struct VFSTag VFSStt;

// file system  //////////////////////////////////////////

// 전체 파일 시스템의 ROOT NODE를 설정한다.
extern int set_filesystem_root( VNodeStt *pVNode );

// register filesystem
extern int register_filesystem( VFSStt *pVFS );

// unregister fileystem
extern int unregister_filesystem( VFSStt *pVFS );

// 파일 시스템과 블록 디바이스를 연결한다.
extern int link_filesystem_device( VFSStt *pVFS, BlkDevObjStt *pDevObj );
extern int unlink_filesystem_device( VFSStt *pVFS );

// VFS의 IOCTL
extern vfs_ioctl( VFSStt *pVFS, int nCmd, DWORD dwParam );

// MOUNT한다.
extern int vfs_mount( char *pPath, VFSStt *pVFS );

// UNMOUNT한다.
extern int vfs_unmount( VFSStt *pVFS );

// VNODE를 해제한다.
extern int delete_vnode( VNodeStt *pVNode );

// 새로운 VNODE 구조체를 할당한다.
extern VNodeStt *new_vnode( VNodeManStt *pVNodeMan );

// VNode Manager가 사용하고 있던 메모리를 모두 해제한다.
extern int delete_vnode_manager( VNodeManStt *pVNodeMan );

// VNODE Manager struct를 할당하고 초기화 한다.
extern VNodeManStt	*make_vnode_manager( VNodeOPStt *pVNodeOP, int nTotalHashIndex );

// 파일 시스템의 루트 VNode를 구한다.
extern VNodeStt *get_root_vnode();

// VNode의 파일명으로 Hash한다.
extern int register_vnode_hash( VNodeManStt *pVNodeMan, VNodeStt *pVNode );

//  VNode의 파일명으로 Hash된 것을 제거한다.
extern int unregister_vnode_hash( VNodeManStt *pVNodeMan, VNodeStt *pVNode );

//  pParentVNode 를 상위 디렉토리로 하는 pS엔트리의 VNode가 이미 만들어져 있는지 찾아본다. 
extern VNodeStt *find_vnode( VNodeManStt *pVNodeMan, VNodeStt *pParentVNode, char *pS );

// VNODE의 RefCount를 하나 감소시킨다.
extern int dec_vnode_ref_count( VNodeStt *pVNode );

// VNODE의 RefCount를 하나 증가시킨다.
extern int inc_vnode_ref_count( VNodeStt *pVNode );

// vfs를 초기화한다.
extern int init_vfs();

// vfs를 닫는다. 
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
