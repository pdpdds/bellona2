#ifndef BELLONA2_MEMORY_HEADER_oh
#define BELLONA2_MEMORY_HEADER_oh

#define SHMEM_NAME_SIZE		32
#define MAX_SHMEM_ENT		32
#define MAX_SHMEM_CHUNK		32

typedef struct {
	int				nSize;						// sizeof shared memory
	int				nRefCount;					// reference counter
	int				nFlag;						// flag
	int				nLock;						// locking flag
	char			szName[ SHMEM_NAME_SIZE ];	// name of the shared memory
	int				nTotalPage;					// total pages
	DWORD			*pPhysList;					// physical memory list	
} ShMemEntStt;

typedef struct {
	int				nTotalEnt;
	ShMemEntStt		ent[ MAX_SHMEM_ENT ];
} ShMemChunkStt;

typedef struct {
	int				nTotalChunk;		// total chunks of shared memory
	ShMemChunkStt	*p_chunk[ MAX_SHMEM_CHUNK ];
} ShMemStt;

//////////////////////////////////////////////////////////////////////////
//						공유 메모리에 관련된 API들						//
//////////////////////////////////////////////////////////////////////////
extern int disp_shmem();
extern int init_shared_mem();
extern int close_shmem( int nI );
extern int attach_shmem( int nID, DWORD dwVAddr );
extern int detach_shmem( int nID, DWORD dwVAddr );
extern int find_shmem_ent( char *pName, int nSize );
extern int create_shmem( char *pName, int nSize, int nFlag );
extern int lock_shmem( int nID );
extern int unlock_shmem( int nID );
//////////////////////////////////////////////////////////////////////////////

extern int nGetPhysMemSize();
extern int get_available_phys_size( int *pSingleMapped, int *pMultiMapped );

#endif