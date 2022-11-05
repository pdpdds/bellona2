#include "bellona2.h"

static int enlarge_memory_pool( DWORD *pPD, MemPoolStt *pMp, DWORD dwSize );
static MemBlockStt *merge_memblk( MemPoolStt *pMp, MemBlockStt *pMb );
static int disp_memory_block( MemBlockStt *pMb );

MemPoolStt kmp;

// initialize memory pool 
int init_memory_pool( DWORD *pPD, MemPoolStt *pMp, DWORD dwStartAddr, DWORD dwSize )
{
	int			nR;

	memset( pMp, 0, sizeof( MemPoolStt ) );
	pMp->dwStartAddr = dwStartAddr;

	// mapping physical memory
	if( dwSize > 0 )
		nR = enlarge_memory_pool( pPD, pMp, dwSize );

	return( 0 );
}			

// Size에 따라 Memory Free Link의 Index를 구한다.
static int get_mfl_index( DWORD dwSize )
{
	if( dwSize < 128 )
		return( MFL_64 );
	else if( dwSize < 256 )
		return( MFL_128 );
	else if( dwSize < 512 )
		return( MFL_256 );
	else if( dwSize < 1024 )
		return( MFL_512 );
	else if( dwSize < 2048 )
		return( MFL_1024 );
	else if( dwSize < 4096 )
		return( MFL_2048 );
	else 
		return( MFL_4096 );
}

// make new memory block
static MemBlockStt *make_new_memory_block( MemBlockStt *pMb, DWORD dwSize )
{
	memset( pMb, 0, sizeof( MemBlockStt ) );

	pMb->wMagic   = MBT_MAGIC;
	pMb->byUsage  = MBT_FREE;
	pMb->dwSize   = dwSize - sizeof( MemBlockStt );
	pMb->pPosPre  = pMb->pPosNext = NULL;
	pMb->pSizePre = pMb->pSizeNext = NULL;

	return( pMb );
}

// link new block to size chain
static int append_memblk_to_size_link( MemPoolStt *pMp, MemBlockStt *pMb )
{
	int					nI;
	MemBlockIndexStt	*pMIndex;

	if( pMb->wMagic != MBT_MAGIC )
	{
		kdbg_printf( "append_memblk_to_size_link: pMb(0x%X)->dwMagic != MBT_MAGIC\n", (DWORD)pMb );
		_asm int 1
		return( -1 );
	}

	// get memory block index pointer
	nI = get_mfl_index( pMb->dwSize );
	pMIndex = &pMp->mfl[nI];

	if( pMIndex->nTotalBlk == 0 )
	{
		pMIndex->pStartBlk	= pMIndex->pEndBlk	= pMb;
		pMb->pSizePre		= pMb->pSizeNext	= NULL;
	}
	else
	{
		pMIndex->pEndBlk->pSizeNext	= pMb;
		pMb->pSizePre				= pMIndex->pEndBlk;
		pMb->pSizeNext				= NULL;
		pMIndex->pEndBlk			= pMb;
	}

	pMIndex->nTotalBlk++;

	return( 0 );
}

// unlink new block from size chain
static int delete_memblk_from_size_link( MemPoolStt *pMp, MemBlockStt *pMb )
{
	int					nI;
	MemBlockIndexStt	*pMIndex;

	if( pMb == NULL || pMb->wMagic != MBT_MAGIC )
	{
		kdbg_printf( "delete_memblk_from_size_link: invalid memory block( 0x%08X )!\n", (DWORD)pMb );
		return( -1 );
	}

	// get memory block index pointer
	nI = get_mfl_index( pMb->dwSize );
	pMIndex = &pMp->mfl[nI];

	if( pMIndex->nTotalBlk <= 0 )
	{
		kdbg_printf( "delete_memblk_from_size_link: critical error!\n" );
		return( -1 );
	}	

	if( pMb->pSizePre == NULL )
		pMIndex->pStartBlk = pMb->pSizeNext;
	else
		pMb->pSizePre->pSizeNext = pMb->pSizeNext;

	if( pMb->pSizeNext == NULL )
		pMIndex->pEndBlk = pMb->pSizePre;
	else
		pMb->pSizeNext->pSizePre = pMb->pSizePre;

	pMb->pSizePre = pMb->pSizeNext = NULL;

	pMIndex->nTotalBlk--;

	return( 0 );
}  

// 메모리 풀을 확장한다.
static int enlarge_memory_pool( DWORD *pPD, MemPoolStt *pMp, DWORD dwSize )
{
	int			nR;
	MemBlockStt *pMb;
	DWORD		dwPhys, dwHeadAddr, dwAddr, dwI, dwStartPhys;
	
	dwStartPhys = 0;	  

	// 최소 크기는 128k
	if( dwSize < 128 * 1024 )	
		dwSize = 128*1024;

	// 4096의 배수
	dwSize += 4095;
	dwSize = (DWORD)( dwSize / 4096 );
	dwSize = (DWORD)( dwSize * 4096 );

	dwHeadAddr = dwAddr = pMp->dwStartAddr + pMp->dwSize;

	for( dwI = 0; dwI < dwSize; )
	{	// 물리 페이지를 할당한다.
		dwPhys = dwAllocPhysPage();
		if( dwPhys == 0 )		// allocation failed!
		{
			kdbg_printf( "enlarge_memory_pool: physical memory allocation failed!\n" );
			return( -1 );
		}

		// mapping physical address
		if( dwAddr < 0x80000000 )
			nR = _nMappingVAddr( pPD, dwAddr, dwPhys );
		else
			nR = _nMappingUserVAddr( pPD, dwAddr, dwPhys );
		if( nR < 0 )
		{
			kdbg_printf( "enlarge_memory_pool: mapping failed [ 0x%08X -> 0x%08X ]\n", dwAddr, dwPhys );
			return( -1 );
		}

		if( dwStartPhys == 0 )
			dwStartPhys = dwPhys;

		pMp->dwSize += 4096;
		dwI			+= 4096;
		dwAddr		+= 4096;
	} 

	// 메모리 확장된 영역에 대해 메모리 블럭을 생성한다.
	pMb = make_new_memory_block( (MemBlockStt*)dwHeadAddr, dwSize );

	// link position chain
	if( pMp->pEndPos == NULL )
		pMp->pEndPos = pMb;
	else
	{
		pMp->pEndPos->pPosNext = pMb;
		pMb->pPosPre = pMp->pEndPos;
		pMp->pEndPos = pMb;
	}						 

	// merge block if possible
	pMb = merge_memblk( pMp, pMb );
	if( pMb == NULL )
		return( -1 );

	// link new block to size chain
	append_memblk_to_size_link( pMp, pMb );

	return( 0 );
}

// 메모리 블록을 분할한다.
static int split_memblk( MemPoolStt *pMp, MemBlockStt *pMb, DWORD dwShrinkedSize )
{
	int			nR;
	MemBlockStt *pNew;

	pNew = (MemBlockStt*)( (DWORD)pMb + sizeof( MemBlockStt ) + dwShrinkedSize );

	// make new memory block
	make_new_memory_block( pNew, pMb->dwSize - dwShrinkedSize );

	// shrink memory block's size
	pMb->dwSize = dwShrinkedSize;

	// reorder position link
	if( pMb->pPosNext != NULL )
		pMb->pPosNext->pPosPre = pNew;
	else
		pMp->pEndPos = pNew;

	pNew->pPosNext = pMb->pPosNext;						
	pNew->pPosPre  = pMb;
	pMb->pPosNext  = pNew;

	// append new block to size link
	nR = append_memblk_to_size_link( pMp, pNew );
	
	return( 0 );
}	

// merge memory block b2 to b1
static MemBlockStt *merge_this_memblk( MemPoolStt *pMp, MemBlockStt *pB1, MemBlockStt *pB2 )
{
	DWORD		dwT;
	MemBlockStt *pT;

	if( pB1 == NULL || pB1->wMagic != MBT_MAGIC )
	{
		kdbg_printf( "merge_this_block( 0x%08X ) - invalid block\n", pB1 );
		return( NULL );
	}
	if( pB2 == NULL || pB2->wMagic != MBT_MAGIC )
	{
		kdbg_printf( "merge_this_block( 0x%08X ) - invalid block\n", pB2 );
		return( NULL );
	}																	

	// order by address
	if( (DWORD)pB1 > (DWORD)pB2 )
	{
		pT  = pB1;
		pB1 = pB2;
		pB2 = pT;
	}

	dwT = pB1->dwSize;

	if( pB1->pPosNext != pB2 || pB2->pPosPre != pB1 )
	{
		kdbg_printf( "merge_this_memblk: not adjacent blocks!\n" );
		return( NULL );
	}

	pB1->pPosNext = pB2->pPosNext;
	if( pB2->pPosNext != NULL )
		pB2->pPosNext->pPosPre = pB1;
	else
		pMp->pEndPos = pB1;

	pB1->dwSize += pB2->dwSize + sizeof( MemBlockStt );

	pB1->byUsage = MBT_FREE;

//	kdbg_printf( "merge : 0x%08X(0x%08X) + 0x%08X(0x%08X) -> 0x%08X(0x%08X)\n", 
//		pB1, dwT, pB2, pB2->dwSize, pB1, pB1->dwSize );

	// clear block 2 header
	memset( pB2, 0, sizeof( MemBlockStt ) );

	return( pB1 );
}

// merge memory bllock
static MemBlockStt *merge_memblk( MemPoolStt *pMp, MemBlockStt *pMb )
{
	int			nR;
	MemBlockStt	*pT;

	// merge forward
	for( pT = pMb->pPosPre; pT != NULL; pT = pT->pPosPre )
	{
		if( pT->byUsage != MBT_FREE )
			break;

		nR = delete_memblk_from_size_link( pMp, pT );
		if( nR == 0 )
		{
			pT = pMb = merge_this_memblk( pMp, pT, pMb );
			if( pMb == NULL )
				return( NULL );
		}
		else
		{
			kdbg_printf( "merge_memblk: block is free but failed to delete from size link!\n" );
			return( NULL );
		}
	}

	// merge backward
	for( pT = pMb->pPosNext; pT != NULL; pT = pT->pPosNext )
	{
		if( pT->byUsage != MBT_FREE )	
			break;
		
		nR = delete_memblk_from_size_link( pMp, pT );
		if( nR == 0 )
		{
		pT = pMb = merge_this_memblk( pMp, pMb, pT );
			if( pMb == NULL )
				return( NULL );
		}
		else
		{
			kdbg_printf( "merge_memblk: block is free but failed to delete from size link!\n" );
			return( NULL );
		}
	}

	return( pMb );
}

// allocate memory
static void *internal_malloc( DWORD *pPD, MemPoolStt *pMP, DWORD dwSize )
{
	MemBlockStt			*pMb;
	char				*pAddr;
	MemBlockIndexStt	*pMIndex;
	int					nR, nI, nEnlarged;
	
	if( dwSize == 0 || dwSize > 0x100000*128 )
	{
		kdbg_printf("internal_malloc: invalid size ( size=%d )\n", dwSize );
		_asm int 1
		return( NULL );
	}		
	
	if( dwSize < 64 )
		dwSize = 64;

	nEnlarged = 0;

RETRY:
	// Free memory block을 찾는다.
	nI = get_mfl_index( dwSize );
	for( ; nI < TOTAL_MFL; nI++ )
	{
		pMIndex = &pMP->mfl[nI];
		// 해당 사이즈 보다 더 큰 블록을 찾는다.
		for( pMb = pMIndex->pStartBlk; pMb != NULL; pMb = pMb->pSizeNext )
		{
			if( pMb->byUsage != MBT_FREE )
			{
				kdbg_printf( "internal_malloc( pMb = 0x%08X ): byUsage(0x%X) != MBT_FREE\n", pMb, pMb->byUsage );
				kdbg_printf( "pMIndex: nTotalBlk = %d, pStartBlk=0x%X, pEndBlk=0x%X\n", pMIndex->nTotalBlk, pMIndex->pStartBlk, pMIndex->pEndBlk );
				_asm int 1
				return( NULL );
			}
			else if( pMb->dwSize >= dwSize )
				goto ALLOC_THIS;
		}
	}

	if( pMb == NULL || nI == TOTAL_MFL )
	{
		if( nEnlarged == 0 )
		{
			nR = enlarge_memory_pool( pPD, pMP, dwSize + 4096 );	// real block size is dwSize - sizeof( MemBlockStt )
			if( nR >= 0 )
				goto RETRY;
			else
			{
				kdbg_printf( "kmalloc: enlarging failed!\n" );
				return( NULL );
			}
		}
		else
		{
			kdbg_printf( "kmalloc: enlarged but allocation failed!\n" );
			return( NULL );			// memory allocation failed
		}

	}

ALLOC_THIS:
	// unlink free memory block and get it
	nR = delete_memblk_from_size_link( pMP, pMb );
	if( nR < 0 )
		return( NULL );

	//  64 바이트 + 블록 헤더 보다 큰 블록이면 분할한다.
	if( pMb->dwSize >= dwSize + sizeof( MemBlockStt ) + 64 )
	{
		nR = split_memblk( pMP, pMb, dwSize );
		if( nR < 0 )
		{
			kdbg_printf( "kmalloc: split_memblk failed!\n" );
			return( NULL );
		}
	}

	pAddr = (char*)( (DWORD)pMb + sizeof( MemBlockStt ) );
	pMb->byUsage = MBT_USED;

	return( pAddr );
}

// 무조건 bell.pPD를 사용하면 안된다.  (버그 수정 2003-12-13)
void *kmalloc( DWORD dwSize )
{
	void 		*pV;
	ProcessStt	*pP;
	DWORD		dwPageDir;

	pP = k_get_current_process();
	if( pP == NULL )
		pV = internal_malloc( bell.pPD, &kmp, dwSize );
	else
	{
		dwPageDir = get_process_page_dir( pP );
		pV = internal_malloc( (DWORD*)dwPageDir, &kmp, dwSize );
	}

	return( pV );
}

// allocate user memory
void *umalloc( DWORD *pPD, MemPoolStt *pMP, DWORD dwSize )
{
	void 		*pV;
	ProcessStt	*pP;

	if( pPD == NULL )
	{
		pP = k_get_current_process();
		if( pP == NULL )
			return( NULL );	// 현재 프로세스를 구할 수 없다.
		pPD = pP->pAddrSpace->pPD;
	}

	if( pMP == NULL )
	{
		pP = k_get_current_process();
		if( pP == NULL )
			return( NULL );
		pMP = &pP->mp;
	}

	pV = internal_malloc( pPD, pMP, dwSize );

	return( pV );
}

// release memory block
static int internal_free( MemPoolStt *pMP, void *pV )
{
	MemBlockStt *pMb;

	pMb = (MemBlockStt*)( (DWORD)pV - sizeof(MemBlockStt) );

	if( pMb->wMagic != MBT_MAGIC )
	{
		kdbg_printf( "kfree: invalid memory block!\n" );
		return( -1 );				// invalid memory address
	}

	if( pMb->byUsage != MBT_USED )
	{
		kdbg_printf( "kfree: the block(0x%X) is not being used!\n", (DWORD)pV  );
		return( -1 );				// the block is already free one
	}

	// try to merge bllock
	pMb = merge_memblk( pMP, pMb );
	if( pMb == NULL )
	{
		kdbg_printf( "kfree: block merge failed!\n" );
		return( -1 );
	}
		
	pMb->byUsage = MBT_FREE;

	// link new block to size chain
	append_memblk_to_size_link( pMP, pMb );

	return( 0 );
}

// release kernel memory block
int kfree( void *pV )
{
	int nR;

	nR = internal_free( &kmp, pV );

	return( nR );
}

// release user memory block
int ufree( MemPoolStt *pMP, void *pV )
{
	int			nR;
	ProcessStt 	*pP;

	if( pMP == NULL )
	{
		pP = k_get_current_process();
		if( pP == NULL )
			return( -1 );
		pMP = &pP->mp;
	}

	nR = internal_free( pMP, pV );

	return( nR );
}

// display one memory block
static int disp_memory_block( MemBlockStt *pMb )
{
	if( pMb == NULL || pMb->wMagic != MBT_MAGIC )
		return( -1 );

	kdbg_printf( "%08X (0x%X/%d) %02X S[%08X-%08X] P[%08X-%08X]\n", 
		pMb,
		pMb->dwSize,   pMb->dwSize,
		pMb->byUsage,
		pMb->pSizePre, pMb->pSizeNext,
		pMb->pPosPre,  pMb->pPosNext  );

	return( 0 );
}

// check memory pool link
int validate_mem_pool( MemPoolStt *pMemPool )
{
	MemBlockStt			*pMb;
	MemBlockIndexStt	*pMIndex;
	int					nR, nI, nJ, nSize, nTotalFree, nFreeInPos, nTotalBlock;

	nSize = 64;
	nTotalFree = 0;

	if( pMemPool == NULL )
		pMemPool = &kmp;

	// display size chain
	for( nI = 0; nI < TOTAL_MFL; nI++ )
	{
		pMIndex = (MemBlockIndexStt*)&pMemPool->mfl[nI];

		kdbg_printf( "MFL_%d  Total %d blocks\n", nSize, pMIndex->nTotalBlk );
		
		pMb = pMIndex->pStartBlk;
		for( nJ = 0; nJ < pMIndex->nTotalBlk; nJ++ )
		{
			kdbg_printf( "[%d] ", nJ );
			nR = disp_memory_block( pMb );						   		
			if( nR < 0 )
				return( -1 );

			pMb = pMb->pSizeNext;
			nTotalFree++;
		}	

		nSize *= 2;
	}			

	kdbg_printf( "Total %d free blocks\n", nTotalFree );

	// get user input
	if( getchar() == BK_ESC )
		return(0);
	
	nFreeInPos = nTotalBlock = nI = 0;
	pMb = (MemBlockStt*)pMemPool->dwStartAddr;
	// display position chain
	for( ; pMb != NULL; pMb = pMb->pPosNext )
	{
		kdbg_printf( "[%d] ", nI++ );
		nR = disp_memory_block( pMb );
		if( nR < 0 )
			return( -1 );
	
		nTotalBlock++;
	
		if( pMb->byUsage == MBT_FREE )
			nFreeInPos++;

		if( ( nTotalBlock % 22 ) == 0 )
		{
			kdbg_printf( "press any key to continue..." );

			if( getchar() == BK_ESC )
				return(0);
			
			kdbg_printf( "\r                             \r" );
		}
	}

	kdbg_printf( "Total %d blocks, %d free blocks in p-link, %d free blocks in s-link\n", nTotalBlock, nFreeInPos, nTotalFree );

	return( 0 );
}	

