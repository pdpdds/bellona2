/*
	BlkDevObj oriented Cache Manager
	date: 2003-08-06

*/
#ifndef NEW_BLOCK_DEVICE_CACHE_HEADER_jj
#define NEW_BLOCK_DEVICE_CACHE_HEADER_jj

// cache block type
#define CACHE_INVALID	0
#define CACHE_ACCESS	1	// read
#define CACHE_DIRTY		2	// write

struct BCacheEntTag;

// cache entry structure
typedef struct BCacheEntTag {
	DWORD					dwBlockNo;					// 읽은 블록 번호
	DWORD					dwFlag;						// VALID, ACCESS, DIRTY...
	BYTE					*pBuff;						// 데이터가 저장된 버퍼
	struct BCacheEntTag		*pPre, *pNext;				// hash한 값이 같을 때의 pre, next 연결.
	struct BCacheEntTag  	*pDirtyPre, *pDirtyNext;	// Dirty Link
};
typedef struct BCacheEntTag BCacheEntStt;

// hash index stt
typedef struct BHashIndexTag {
	int						nTotal;						// 동일한 값으로 해시된 것들의 개수.
	BCacheEntStt			*pStart, *pEnd;				// 동일한 값으로 해시된 것들을 위한 연결 리스트.
};
typedef struct BHashIndexTag BHashIndexStt;		  

// cache manager struct
typedef struct BCacheManTag {
	int						nMaxHashIndex;				// 해시 엔트리 개수
	int						nMaxEntry;					// 최대한 캐시될 수 있는 개수.
	int						nTotalEntry;				// 현재 Cache되고 있는 엔트리의 개수
	BHashIndexStt			*pHashTbl;					// 해시 테이블
	BCacheEntStt			*pDirtyStart;				// Dirty Link만 따로 관리해 주면 된다.
	BCacheEntStt		 	*pDirtyEnd;	
	struct BlkDevObjTag		*pDevObj;					// Cache 대상이 되는 블록 디바이스 OBJ
};
typedef struct BCacheManTag BCacheManStt;

// Cache Manager를 생성한다. 
// Block Device Object, Hash index entry, Max hash entry를 파러메터로 받는다.
extern BCacheManStt *make_blk_cache( struct BlkDevObjTag *pDevObj, int nMaxHashIndex, int nMaxEntry );

// Cache Manager를 제거한다.
extern int close_blk_cache( BCacheManStt *pCache );							

// block cache의 버퍼로부터 데이터를 읽는다.
extern int read_blk_cache( BlkDevObjStt *pObj, BlkDevIOStt *pIO );

// 캐시되고 있는 블록 버퍼를 구한다.  만일 캐시되고 있지 않으면 로드하려고 시도한다.
extern BYTE *get_blk_cache_buff( BlkDevObjStt *pObj, DWORD dwBlock );

// block cache의 버퍼쪽으로 데이터를 기록한다.
extern int write_blk_cache( BCacheManStt *pCache, DWORD dwBlock, BYTE *pBuff );

// Cache Manager 전체의 Dirty Entry를 flushing 한다.
extern int flush_blk_cache( BCacheManStt *pCache );

extern int bcache_scatter_load( BlkDevObjStt *pDevObj, DWORD dwBlock, int nSectors );
extern BCacheEntStt *alloc_bcache_ent( BCacheManStt *pCache );
extern BCacheEntStt *find_hash_ent( BCacheManStt *pCache, DWORD dwBlockNo );
#endif

