#ifndef BELLONA2_MALLOC_HEADER_oh
#define BELLONA2_MALLOC_HEADER_oh

#define MBT_MAGIC	0xF00D

#define KERNEL_MEM_POOL_ADDR	0x800000		// kernel memory pool is started at 8M area
#define KERNEL_MEM_POOL_SIZE	(1024 * 128)	// initial 128k
#define PROCESS_MEM_POOL_ADDR	0x80800000		// process memory pool is started at 8M area
#define PROCESS_MEM_POOL_SIZE	0				//(1024 * 128)  // initial 128k

#define MBT_USED	0x1		// block in using
#define MBT_FREE	0xF		// free block

typedef struct MemBlockTag {
	UINT16	wMagic;
	UCHAR	byUsage;		// state
	UCHAR	rsv;
	DWORD	dwSize; 		// block size
	struct  MemBlockTag *pPosPre,  *pPosNext;	// positional pre next link
	struct  MemBlockTag *pSizePre, *pSizeNext;	// pre next link by size
};
typedef struct MemBlockTag MemBlockStt;

typedef struct MemBlockIndexTag	{
	int			nTotalBlk;
	MemBlockStt	*pStartBlk, *pEndBlk;
};
typedef struct MemBlockIndexTag MemBlockIndexStt;

typedef enum {
	MFL_64	= 0,
	MFL_128,
	MFL_256,
	MFL_512,
	MFL_1024,
	MFL_2048,
	MFL_4096,

	TOTAL_MFL
} MemFreeLinkTag;

typedef struct MemPoolTag{
	DWORD				dwStartAddr;
	DWORD				dwSize;

	MemBlockIndexStt	mfl[TOTAL_MFL];

	MemBlockStt			*pEndPos;

};
typedef struct MemPoolTag MemPoolStt;

extern MemPoolStt kmp;

extern BELL_EXPORT int  	kfree			( void *pV );
extern BELL_EXPORT int  	ufree			( MemPoolStt *pMP, void *pV );
extern BELL_EXPORT void 	*kmalloc		( DWORD dwSize );
extern BELL_EXPORT void 	*umalloc		( DWORD *pPD, MemPoolStt *pMP, DWORD dwSize );

extern int  validate_mem_pool				( MemPoolStt *pMemPool );
extern int  init_memory_pool				( DWORD *pPD, MemPoolStt *pMp, DWORD dwStartAddr, DWORD dwSize );

#endif

