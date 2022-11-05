#include "vfs.h"

static BCacheEntStt *insert_blk_cache( BCacheManStt *pCache, DWORD dwBlockNo, BYTE *pBuff, DWORD dwFlag );

// Block Device Object�� Cache Manager�� �߰��Ѵ�.
BCacheManStt *make_blk_cache( struct BlkDevObjTag *pDevObj, int nMaxHashIndex, int nMaxEntry )
{
	BCacheManStt	*pCache;
	
	if( pDevObj == NULL )
	{
		kdbg_printf( "make_blk_cache: pDevObj = NULL!\n" );
		return( NULL );
	}

	if( nMaxHashIndex <= 0 )
		nMaxHashIndex = 32;	   

	if( nMaxEntry <= 0 )
		nMaxEntry = 256;

	// ĳ�� �Ŵ��� ����ü�� �Ҵ��Ѵ�.
	pCache = (BCacheManStt*)MALLOC( sizeof( BCacheManStt ) );
	if( pCache == NULL )
		return( NULL );	
		
	memset( pCache, 0, sizeof( BCacheManStt ) );
	
	// �ؽ� �ε����� �Ҵ��Ѵ�.
	pCache->pDevObj			= pDevObj;
	pCache->nMaxEntry		= nMaxEntry;
	pCache->nMaxHashIndex	= nMaxHashIndex;
	pCache->pHashTbl		= (BHashIndexStt*)MALLOC( sizeof( BHashIndexStt ) * nMaxHashIndex );
	if(	pCache->pHashTbl == NULL )
	{
		FREE( pCache );
		return( NULL );
	}
	
	memset( pCache->pHashTbl, 0, sizeof( BHashIndexStt ) * nMaxHashIndex );
	
	return( pCache );
}

// Dirty Link���� ��Ʈ���� �����Ѵ�.
static int delete_from_dirty_link( BCacheManStt *pCache, BCacheEntStt *pEnt )
{
	if( pEnt->pDirtyPre == NULL )
		pCache->pDirtyStart = pEnt->pDirtyNext;
	else
		pEnt->pDirtyPre->pDirtyNext = pEnt->pDirtyNext;
	if( pEnt->pDirtyNext == NULL )
		pCache->pDirtyEnd = pEnt->pDirtyPre;
	else
		pEnt->pDirtyNext->pDirtyPre = pEnt->pDirtyPre;
		
	pEnt->pDirtyPre = pEnt->pDirtyNext = NULL;

	// DIRTY �Ӽ��� �����Ѵ�.
	pEnt->dwFlag &= (DWORD)~(DWORD)CACHE_DIRTY;
	
	return( 0 );
}

// dirty entry �ϳ��� flush �Ѵ�.  (Data�� ����� �� Dirty Link���� ����.)
static int flush_dirty_entry( BCacheManStt *pCache, BCacheEntStt *pEnt )
{
	int				nR;
	BlkDevIOStt		io;

	memset( &io, 0, sizeof( io ) );
	io.nBlocks = 1;
	io.pBuff   = pEnt->pBuff;
	io.dwIndex = pEnt->dwBlockNo;

	// ����� ����Ѵ�.
	nR = pCache->pDevObj->pDev->op.write( pCache->pDevObj, &io );
	if( nR < 0 )
	{
		kdbg_printf( "get_removable_cache_ent: op.write error!\n" );
		return( -1 );
	}

	// dirty link���� �����Ѵ�.
	delete_from_dirty_link( pCache, pEnt );

	return( 0 );
}

// Cache Manager ��ü�� Dirty Entry�� flushing �Ѵ�.
int flush_blk_cache( BCacheManStt *pCache )
{
	BCacheEntStt *pLast, *pPre;

	for( pLast = pCache->pDirtyEnd; pLast != NULL; pLast = pPre )
	{
		pPre = pLast->pDirtyPre;
		if( ( pLast->dwFlag & CACHE_DIRTY ) == 0 )
		{
			  kdbg_printf( "flush_blk_cache: dirty entry without CACHE_DIRTY flag!\n" );
		}

		flush_dirty_entry( pCache, pLast );
	}
	
	return( 0 );
}

// Cache Manager�� �����Ѵ�.
int close_blk_cache( BCacheManStt *pCache )
{
	int				nI;
	BHashIndexStt	*pIndex;
	BCacheEntStt	*pEnt, *pNext;

	if( pCache == NULL || pCache->pDevObj == NULL )
		return( -1 );

	// flushing �Ѵ�.
	flush_blk_cache( pCache );

	// ĳ�� �ε����� ����� ��Ʈ���� �����Ѵ�.
	for( nI = 0; nI < pCache->nMaxHashIndex; nI++ )
	{
		pIndex = &pCache->pHashTbl[nI];
		for( pEnt = pIndex->pStart; pEnt != NULL; pEnt = pNext )
		{
			pNext = pEnt->pNext;
			if( pEnt->pBuff != NULL )
				FREE( pEnt->pBuff );
			FREE( pEnt );
		}

	}
		
	if( pCache->pHashTbl != NULL )
		FREE( pCache->pHashTbl );
		
	FREE( pCache );
	
	return( 0 );
}

// �� ��ȣ�� ���� �ؽ� Ű ���� ��´�.
static int bhash( BCacheManStt *pCache, DWORD dwBlock )
{
	return( dwBlock % pCache->nMaxHashIndex );
}

// dwBlockNo�� ���� Cache Entry�� ã�´�.
BCacheEntStt *find_hash_ent( BCacheManStt *pCache, DWORD dwBlockNo )
{
	int				nI;
	BCacheEntStt	*pEnt;
 	BHashIndexStt	*pIndex;

	// �ؽ� Ű ���� ��´�.
	nI = bhash( pCache, dwBlockNo );
	pIndex = &pCache->pHashTbl[ nI ];
	for( pEnt = pIndex->pStart; ; pEnt = pEnt->pNext )
	{
		if( pEnt == NULL )
			return( NULL );
		if( pEnt->dwBlockNo == dwBlockNo )
			return( pEnt );
	}

	return( NULL );
}

// block cache buffer�κ��� �����͸� �����Ѵ�.
int read_blk_cache( BlkDevObjStt *pObj, BlkDevIOStt *pIO )
{
	BlkDevIOStt		io;
	int				nR;
	BCacheEntStt	*pEnt;
	BCacheManStt	*pCache;
	int				nI, nBlkSize;

	pCache = pObj->pCache;
	if( pCache == NULL || pCache->pDevObj == NULL )
		return( -1 );

	nBlkSize = pCache->pDevObj->nBlkSize;
	memcpy( &io, pIO, sizeof( io ) );
	io.nBlocks = 1;

	for( nI = 0; nI < pIO->nBlocks; nI++ )
	{
		// �ؽ� ��Ʈ���� ã�´�.
		pEnt = find_hash_ent( pCache, pIO->dwIndex + nI );
		if( pEnt != NULL )
		{
			// ��� �����ŭ �����͸� �����Ѵ�.
			memcpy( &pIO->pBuff[ nI* nBlkSize ], pEnt->pBuff, nBlkSize );
			continue;			
		}

		io.pBuff  = &pIO->pBuff[ nI* nBlkSize ];
		io.dwIndex = pIO->dwIndex + nI;
		nR = pObj->pDev->op.read( pObj, &io );
		if( nR != 0 )
			return( -1 );	//  ������ �߻��ߴ�.
	
		// ĳ�ÿ� �߰��Ѵ�.
		insert_blk_cache( pCache, io.dwIndex+nI, io.pBuff, CACHE_ACCESS );
	}

	// cache entry�� �����ϴ� ���۸� ������.
	return( 0 );
}

// ĳ�õǰ� �ִ� ��� ���۸� ���Ѵ�.  ���� ĳ�õǰ� ���� ������ �ε��Ϸ��� �õ��Ѵ�.
BYTE *get_blk_cache_buff( BlkDevObjStt *pObj, DWORD dwBlock )
{
	BlkDevIOStt		io;
	int				nR;
	BCacheEntStt	*pEnt;
	BCacheManStt	*pCache;
	BYTE			buff[512];

	pCache = pObj->pCache;
	if( pCache == NULL || pCache->pDevObj == NULL )
		return( NULL );

	// �ؽ� ��Ʈ���� ã�´�.
	pEnt = find_hash_ent( pCache, dwBlock );
	if( pEnt != NULL )
		return( pEnt->pBuff );	// ã�� ĳ�� ��Ʈ���� �����Ѵ�.			

	//kdbg_printf( "get_blk_cache_buff: block(%d) not found\n" );

	memset( &io, 0, sizeof( io ) );
	io.nBlocks = 1;
	io.pBuff   = buff;		;
	io.dwIndex = dwBlock;

	// ����� �о���δ�.
	nR = pObj->pDev->op.read( pObj, &io );
	if( nR < 0 )
	{
		kdbg_printf( "pObj->pDev->op.read = %d\n", nR );
		return( NULL );	//  ������ �߻��ߴ�.
	}
	
	// ĳ�ÿ� �߰��Ѵ�.
	pEnt = insert_blk_cache( pCache, dwBlock, buff, CACHE_ACCESS );
	if( pEnt == NULL )
	{
		return( NULL );		// ĳ�ÿ� �߰��� �� ������ ����.
	}

	return( pEnt->pBuff );
}	

// ĳ�� ��Ʈ���� Dirty Link�� �߰��Ѵ�.
static int insert_dirty_link( BCacheManStt *pCache, BCacheEntStt *pEnt )
{
	if( pCache == NULL )
		return( -1 );

	pEnt->pDirtyPre = pEnt->pDirtyNext = NULL;
	
	if( pCache->pDirtyStart == NULL )
	{
		pCache->pDirtyStart = pCache->pDirtyEnd = pEnt;
	}
	else
	{
		pCache->pDirtyEnd->pDirtyNext	= pEnt;
		pEnt->pDirtyPre					= pCache->pDirtyEnd;
		pCache->pDirtyEnd				= pEnt;
		pEnt->pDirtyNext                = NULL;
	}
	return( 0 );
}

// block cache�� ���������� �����͸� ����Ѵ�.
int write_blk_cache( BCacheManStt *pCache, DWORD dwBlockNo, BYTE *pBuff )
{
	BCacheEntStt	*pEnt;

	if( pCache == NULL || pCache->pDevObj == NULL )
		return( -1 );

	// �ؽ� ��Ʈ���� ã�´�.
	pEnt = find_hash_ent( pCache, dwBlockNo );
	if( pEnt == NULL )
	{	// ���� �ؽ� ��Ʈ������ ã�� �� ������ ���� �߰��Ѵ�.
		pEnt = insert_blk_cache( pCache, dwBlockNo, pBuff, CACHE_DIRTY );
		if( pEnt == NULL )
			return( -1 );
		return( 0 );
	}

	// ���� �ؽ� ��Ʈ������ ã������ ��� �����ŭ �����͸� �����Ѵ�.
	memcpy( pBuff, pEnt->pBuff, pCache->pDevObj->nBlkSize );

	// DIRTY Flag�� �����Ǿ� ���� ������ ���� Dirty Link�� �ִ´�.
	if( ( pEnt->dwFlag & CACHE_DIRTY ) == 0 )
	{
		pEnt->dwFlag |= CACHE_DIRTY;
		// Dirty Link�� �߰��Ѵ�.
		insert_dirty_link( pCache, pEnt );
	}	

	return( 0 );
}	

// ����� �� �ִ� ĳ�� ��Ʈ���� ã�´�.
static BCacheEntStt *get_removable_cache_ent( BCacheManStt *pCache )
{
	BCacheEntStt	*pEnt;
	BHashIndexStt	*pIndex;
	int				nR, nI, nMaxIndex, nMaxValue;

	nMaxValue = -1;
	// �ؽ� �ε��� ��� ��Ʈ���� ���� ���� ������ �ִ� ���� ���Ѵ�.
	for( nI = 0; nI < pCache->nMaxHashIndex; nI++ )
	{
		pIndex = &pCache->pHashTbl[nI];
		if( pIndex->nTotal == 0 )
			continue;

		if( pIndex->nTotal > nMaxValue )
		{
			nMaxIndex = nI;
			nMaxValue = pIndex->nTotal;
		}
	}

	pIndex = &pCache->pHashTbl[nMaxIndex];

	// ���� ���� ��Ʈ���� �����Ѵ�.
	if( pIndex->pStart == NULL )
	{
		kdbg_printf( "get_removable_cache_ent: invalid link!\n" );
		return( NULL );
	}

	pEnt = pIndex->pStart;

	pIndex->pStart = pIndex->pStart->pNext;
	if( pIndex->pStart != NULL )
		pIndex->pStart->pPre = NULL;
	else
		pIndex->pEnd = NULL;

	// Dirty ��Ʈ���� ������ �־�� �Ѵ�.
	if( pEnt->dwFlag & CACHE_DIRTY )
	{	// ���� ����̽��� �����͸� ����Ѵ�.
		nR = flush_dirty_entry( pCache, pEnt );
		if( nR < 0 )
			return( NULL );
	}	 

	pIndex->nTotal--;
	pCache->nTotalEntry--;

	return( pEnt );
}		

BCacheEntStt *alloc_bcache_ent( BCacheManStt *pCache )
{
	BCacheEntStt *pEnt;

	if( pCache->pDevObj->nBlkSize != 512 )
	{
		kdbg_printf( "alloc_bcache_ent: nBlkSize I= 512\n" );
		return( NULL );
	}
	
	pEnt = (BCacheEntStt*)MALLOC( sizeof( BCacheEntStt ) + pCache->pDevObj->nBlkSize );
	if( pEnt == NULL )
		return( NULL );
	
	memset( pEnt, 0,sizeof( BCacheEntStt ) );
	pEnt->pBuff = (BYTE*)( (DWORD)pEnt + sizeof( BCacheEntStt ) );

	return( pEnt );	
}

// dwBlockNo�� Key�� �Ͽ� �޸𸮸� �Ҵ��Ͽ� pBuff�� ������ ������ �д�.
static BCacheEntStt *insert_blk_cache( BCacheManStt *pCache, DWORD dwBlockNo, BYTE *pBuff, DWORD dwFlag )
{
	int				nI;
	BCacheEntStt	*pEnt;
	BYTE			*pCBuff;
	BHashIndexStt	*pIndex;

	if( pCache == NULL || pCache->pDevObj == NULL )
		return( NULL );

	// ���� ��Ʈ���� �����ϴ��� Ȯ���Ѵ�.
	pEnt = find_hash_ent( pCache, dwBlockNo );
	if( pEnt != NULL )
	{	// dirty�� �ƴϸ� �׳� ���ư��� �ȴ�.
		if( ( dwFlag & CACHE_DIRTY ) == 0 )
			return( pEnt );

		// ������ ������ �����ؾ� �Ѵ�.
		memcpy( pEnt->pBuff, pBuff, pCache->pDevObj->nBlkSize );

		if( ( pEnt->dwFlag & CACHE_DIRTY ) == 0 )
		{	// Dirty Flag�� ���� �߰��� ���.
			pEnt->dwFlag = dwFlag;
			goto SET_DIRTY;			// Dirty Link�� �߰��Ѵ�.
		}
		return( pEnt );
	}	

	if( pCache->nTotalEntry < pCache->nMaxEntry )
	{
		// ���ο� ĳ�� ��Ʈ���� �Ҵ��Ѵ�.
		pEnt = alloc_bcache_ent( pCache );
		if( pEnt == NULL )
			return( NULL );
		pCBuff = pEnt->pBuff;
	}
	else
	{	// ����� �� �ִ� ĳ�� ��Ʈ���� ã�´�.
		pEnt = get_removable_cache_ent( pCache );
		if( pEnt == NULL )
		{
			kdbg_printf( "insert_blk_cache: no removable cache entry!\n" );
			return( NULL );		// �߰��� ���� ���� ��Ʈ���� ��ü�� ���� ����.
		}
		pCBuff = pEnt->pBuff;
	}

	memset( pEnt, 0, sizeof( BCacheEntStt ) );
	pEnt->dwBlockNo = dwBlockNo;
	pEnt->dwFlag	= dwFlag;
	pEnt->pBuff     = pCBuff;
	memcpy( pEnt->pBuff, pBuff, pCache->pDevObj->nBlkSize );

	// �ؽ� Ű ���� ���Ѵ�.
	nI = bhash( pCache, dwBlockNo );
	pIndex = &pCache->pHashTbl[ nI ];

	pIndex->nTotal++;

	// Hash Index ��ũ�� �߰��Ѵ�.
	if( pIndex->pEnd == NULL )
	{	// ��ũ�� ó���� �߰��Ѵ�.
		pIndex->pStart = pIndex->pEnd = pEnt;
	}
	else
	{	// ��ũ�� ���� �������� �߰��Ѵ�.
		pIndex->pEnd->pNext = pEnt;
		pEnt->pPre			= pIndex->pEnd;
		pIndex->pEnd		= pEnt;
	}

	// DIrty�̸� Dirty Link�� �߰��Ѵ�.
	if( dwFlag & CACHE_DIRTY )
	{
SET_DIRTY:
		insert_dirty_link( pCache, pEnt );
	}

	pCache->nTotalEntry++;

	return( pEnt );
}

static int insert_blk_cache_ex( BCacheManStt *pCache, BCacheEntStt *pEnt )
{
	int 			nI;
	BCacheEntStt	*pE;
	BHashIndexStt	*pIndex;

	if( pCache == NULL || pEnt == NULL )
		return( -1 );

	// ���� ��Ʈ���� �����ϴ��� Ȯ���Ѵ�.
	pE = find_hash_ent( pCache, pEnt->dwBlockNo );
	if( pE != NULL )
		return( 0 );		// ĳ�� ��Ʈ���� �̹� ��ϵ� ���¸� ����.
		
	// �ؽ� Ű ���� ���Ѵ�.
	nI = bhash( pCache, pEnt->dwBlockNo );
	pIndex = &pCache->pHashTbl[ nI ];

	pIndex->nTotal++;

	// Hash Index ��ũ�� �߰��Ѵ�.
	if( pIndex->pEnd == NULL )
	{	// ��ũ�� ó���� �߰��Ѵ�.
		pIndex->pStart = pIndex->pEnd = pEnt;
	}
	else
	{	// ��ũ�� ���� �������� �߰��Ѵ�.
		pIndex->pEnd->pNext = pEnt;
		pEnt->pPre			= pIndex->pEnd;
		pIndex->pEnd		= pEnt;
		pEnt->pNext         = NULL;
	}

	// DIrty�̸� Dirty Link�� �߰��Ѵ�.
	if( pEnt->dwFlag & CACHE_DIRTY )
		insert_dirty_link( pCache, pEnt );

	pCache->nTotalEntry++;

	return( 0 );
}

int bcache_scatter_load( BlkDevObjStt *pDevObj, DWORD dwBlock, int nSectors )
{
	BCacheEntStt	**ppCEntArray;
	int 			nI, nSize, nR;

	// scatter read�� �������� �ʴ´�.
	if( pDevObj == NULL || pDevObj->pDev == NULL || pDevObj->pDev->op.scatter_read == NULL )
	{
		//kdbg_printf( "bcache_scatter_load: NULL pointer!\n" );
		return( -1 );
	}

	//kdbg_printf( "bcache_scatter_load( dwBlock=%d, nSectors=%d\n", dwBlock, nSectors );

	// ������ �迭�� �Ҵ��Ѵ�.
	nSize = sizeof(void*) * nSectors;
	ppCEntArray = (BCacheEntStt**)MALLOC( nSize );
	if( ppCEntArray == NULL )
	{
ERR:	kdbg_printf( "bcache_scatter_load: MALLOC failed!\n" );
		return( -1 );
	}

	// ĳ�� ��Ʈ������ �Ҵ��Ѵ�.
	for( nI = 0; nI < nSectors; nI++ )
	{
		ppCEntArray[nI] = alloc_bcache_ent( pDevObj->pCache );
		if( ppCEntArray[nI] == NULL )
		{	// ������ �迭�� �����ϰ� ����ó��.
			FREE( ppCEntArray );
			goto ERR;
		}

		ppCEntArray[nI]->dwBlockNo = dwBlock + nI;
		ppCEntArray[nI]->dwFlag    = CACHE_ACCESS;
	}

	// ���͵��� �о���δ�.
	nR = pDevObj->pDev->op.scatter_read( pDevObj, ppCEntArray, dwBlock, nSectors );

	// ĳ�� ��Ʈ���� �߰��Ѵ�.
	for( nI = 0; nI < nSectors; nI++ )
	{
		if( ppCEntArray[nI] == NULL || ppCEntArray[nI]->dwFlag == CACHE_INVALID )
			continue;
		
		insert_blk_cache_ex( pDevObj->pCache, ppCEntArray[nI] );
	}

	// ������ �迭�� �����Ѵ�.
	FREE( ppCEntArray );

	return( 0 );
}








