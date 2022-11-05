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
	DWORD					dwBlockNo;					// ���� ��� ��ȣ
	DWORD					dwFlag;						// VALID, ACCESS, DIRTY...
	BYTE					*pBuff;						// �����Ͱ� ����� ����
	struct BCacheEntTag		*pPre, *pNext;				// hash�� ���� ���� ���� pre, next ����.
	struct BCacheEntTag  	*pDirtyPre, *pDirtyNext;	// Dirty Link
};
typedef struct BCacheEntTag BCacheEntStt;

// hash index stt
typedef struct BHashIndexTag {
	int						nTotal;						// ������ ������ �ؽõ� �͵��� ����.
	BCacheEntStt			*pStart, *pEnd;				// ������ ������ �ؽõ� �͵��� ���� ���� ����Ʈ.
};
typedef struct BHashIndexTag BHashIndexStt;		  

// cache manager struct
typedef struct BCacheManTag {
	int						nMaxHashIndex;				// �ؽ� ��Ʈ�� ����
	int						nMaxEntry;					// �ִ��� ĳ�õ� �� �ִ� ����.
	int						nTotalEntry;				// ���� Cache�ǰ� �ִ� ��Ʈ���� ����
	BHashIndexStt			*pHashTbl;					// �ؽ� ���̺�
	BCacheEntStt			*pDirtyStart;				// Dirty Link�� ���� ������ �ָ� �ȴ�.
	BCacheEntStt		 	*pDirtyEnd;	
	struct BlkDevObjTag		*pDevObj;					// Cache ����� �Ǵ� ��� ����̽� OBJ
};
typedef struct BCacheManTag BCacheManStt;

// Cache Manager�� �����Ѵ�. 
// Block Device Object, Hash index entry, Max hash entry�� �ķ����ͷ� �޴´�.
extern BCacheManStt *make_blk_cache( struct BlkDevObjTag *pDevObj, int nMaxHashIndex, int nMaxEntry );

// Cache Manager�� �����Ѵ�.
extern int close_blk_cache( BCacheManStt *pCache );							

// block cache�� ���۷κ��� �����͸� �д´�.
extern int read_blk_cache( BlkDevObjStt *pObj, BlkDevIOStt *pIO );

// ĳ�õǰ� �ִ� ��� ���۸� ���Ѵ�.  ���� ĳ�õǰ� ���� ������ �ε��Ϸ��� �õ��Ѵ�.
extern BYTE *get_blk_cache_buff( BlkDevObjStt *pObj, DWORD dwBlock );

// block cache�� ���������� �����͸� ����Ѵ�.
extern int write_blk_cache( BCacheManStt *pCache, DWORD dwBlock, BYTE *pBuff );

// Cache Manager ��ü�� Dirty Entry�� flushing �Ѵ�.
extern int flush_blk_cache( BCacheManStt *pCache );

extern int bcache_scatter_load( BlkDevObjStt *pDevObj, DWORD dwBlock, int nSectors );
extern BCacheEntStt *alloc_bcache_ent( BCacheManStt *pCache );
extern BCacheEntStt *find_hash_ent( BCacheManStt *pCache, DWORD dwBlockNo );
#endif

