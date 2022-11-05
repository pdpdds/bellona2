#include "vfs.h"

static BCacheEntStt *insert_blk_cache( BCacheManStt *pCache, DWORD dwBlockNo, BYTE *pBuff, DWORD dwFlag );

// Block Device Object에 Cache Manager를 추가한다.
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

	// 캐시 매니저 구조체를 할당한다.
	pCache = (BCacheManStt*)MALLOC( sizeof( BCacheManStt ) );
	if( pCache == NULL )
		return( NULL );	
		
	memset( pCache, 0, sizeof( BCacheManStt ) );
	
	// 해시 인덱스를 할당한다.
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

// Dirty Link에서 엔트리를 제거한다.
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

	// DIRTY 속성을 제거한다.
	pEnt->dwFlag &= (DWORD)~(DWORD)CACHE_DIRTY;
	
	return( 0 );
}

// dirty entry 하나를 flush 한다.  (Data를 기록한 후 Dirty Link에서 제거.)
static int flush_dirty_entry( BCacheManStt *pCache, BCacheEntStt *pEnt )
{
	int				nR;
	BlkDevIOStt		io;

	memset( &io, 0, sizeof( io ) );
	io.nBlocks = 1;
	io.pBuff   = pEnt->pBuff;
	io.dwIndex = pEnt->dwBlockNo;

	// 블록을 기록한다.
	nR = pCache->pDevObj->pDev->op.write( pCache->pDevObj, &io );
	if( nR < 0 )
	{
		kdbg_printf( "get_removable_cache_ent: op.write error!\n" );
		return( -1 );
	}

	// dirty link에서 제거한다.
	delete_from_dirty_link( pCache, pEnt );

	return( 0 );
}

// Cache Manager 전체의 Dirty Entry를 flushing 한다.
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

// Cache Manager를 제거한다.
int close_blk_cache( BCacheManStt *pCache )
{
	int				nI;
	BHashIndexStt	*pIndex;
	BCacheEntStt	*pEnt, *pNext;

	if( pCache == NULL || pCache->pDevObj == NULL )
		return( -1 );

	// flushing 한다.
	flush_blk_cache( pCache );

	// 캐시 인덱스에 연결된 엔트리를 해제한다.
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

// 블럭 번호를 통해 해시 키 값을 얻는다.
static int bhash( BCacheManStt *pCache, DWORD dwBlock )
{
	return( dwBlock % pCache->nMaxHashIndex );
}

// dwBlockNo를 통해 Cache Entry를 찾는다.
BCacheEntStt *find_hash_ent( BCacheManStt *pCache, DWORD dwBlockNo )
{
	int				nI;
	BCacheEntStt	*pEnt;
 	BHashIndexStt	*pIndex;

	// 해시 키 값을 얻는다.
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

// block cache buffer로부터 데이터를 복사한다.
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
		// 해시 엔트리를 찾는다.
		pEnt = find_hash_ent( pCache, pIO->dwIndex + nI );
		if( pEnt != NULL )
		{
			// 블록 사이즈만큼 데이터를 복사한다.
			memcpy( &pIO->pBuff[ nI* nBlkSize ], pEnt->pBuff, nBlkSize );
			continue;			
		}

		io.pBuff  = &pIO->pBuff[ nI* nBlkSize ];
		io.dwIndex = pIO->dwIndex + nI;
		nR = pObj->pDev->op.read( pObj, &io );
		if( nR != 0 )
			return( -1 );	//  에러가 발생했다.
	
		// 캐시에 추가한다.
		insert_blk_cache( pCache, io.dwIndex+nI, io.pBuff, CACHE_ACCESS );
	}

	// cache entry에 존재하는 버퍼를 복사함.
	return( 0 );
}

// 캐시되고 있는 블록 버퍼를 구한다.  만일 캐시되고 있지 않으면 로드하려고 시도한다.
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

	// 해시 엔트리를 찾는다.
	pEnt = find_hash_ent( pCache, dwBlock );
	if( pEnt != NULL )
		return( pEnt->pBuff );	// 찾은 캐시 엔트리를 리턴한다.			

	//kdbg_printf( "get_blk_cache_buff: block(%d) not found\n" );

	memset( &io, 0, sizeof( io ) );
	io.nBlocks = 1;
	io.pBuff   = buff;		;
	io.dwIndex = dwBlock;

	// 블록을 읽어들인다.
	nR = pObj->pDev->op.read( pObj, &io );
	if( nR < 0 )
	{
		kdbg_printf( "pObj->pDev->op.read = %d\n", nR );
		return( NULL );	//  에러가 발생했다.
	}
	
	// 캐시에 추가한다.
	pEnt = insert_blk_cache( pCache, dwBlock, buff, CACHE_ACCESS );
	if( pEnt == NULL )
	{
		return( NULL );		// 캐시에 추가할 수 없으면 에러.
	}

	return( pEnt->pBuff );
}	

// 캐시 엔트리를 Dirty Link에 추가한다.
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

// block cache의 버퍼쪽으로 데이터를 기록한다.
int write_blk_cache( BCacheManStt *pCache, DWORD dwBlockNo, BYTE *pBuff )
{
	BCacheEntStt	*pEnt;

	if( pCache == NULL || pCache->pDevObj == NULL )
		return( -1 );

	// 해시 엔트리를 찾는다.
	pEnt = find_hash_ent( pCache, dwBlockNo );
	if( pEnt == NULL )
	{	// 기존 해시 엔트리에서 찾을 수 없으면 새로 추가한다.
		pEnt = insert_blk_cache( pCache, dwBlockNo, pBuff, CACHE_DIRTY );
		if( pEnt == NULL )
			return( -1 );
		return( 0 );
	}

	// 기존 해시 엔트리에서 찾았으면 블록 사이즈만큼 데이터를 복사한다.
	memcpy( pBuff, pEnt->pBuff, pCache->pDevObj->nBlkSize );

	// DIRTY Flag가 설정되어 있지 않으면 새로 Dirty Link로 넣는다.
	if( ( pEnt->dwFlag & CACHE_DIRTY ) == 0 )
	{
		pEnt->dwFlag |= CACHE_DIRTY;
		// Dirty Link에 추가한다.
		insert_dirty_link( pCache, pEnt );
	}	

	return( 0 );
}	

// 폐기할 수 있는 캐시 엔트리를 찾는다.
static BCacheEntStt *get_removable_cache_ent( BCacheManStt *pCache )
{
	BCacheEntStt	*pEnt;
	BHashIndexStt	*pIndex;
	int				nR, nI, nMaxIndex, nMaxValue;

	nMaxValue = -1;
	// 해시 인덱스 가운데 엔트리를 가장 많이 가지고 있는 것을 구한다.
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

	// 가장 앞쪽 엔트리를 제거한다.
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

	// Dirty 엔트리도 제거해 주어야 한다.
	if( pEnt->dwFlag & CACHE_DIRTY )
	{	// 실제 디바이스에 데이터를 기록한다.
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

// dwBlockNo를 Key로 하여 메모리를 할당하여 pBuff의 내용을 보관해 둔다.
static BCacheEntStt *insert_blk_cache( BCacheManStt *pCache, DWORD dwBlockNo, BYTE *pBuff, DWORD dwFlag )
{
	int				nI;
	BCacheEntStt	*pEnt;
	BYTE			*pCBuff;
	BHashIndexStt	*pIndex;

	if( pCache == NULL || pCache->pDevObj == NULL )
		return( NULL );

	// 기존 엔트리가 존재하는지 확인한다.
	pEnt = find_hash_ent( pCache, dwBlockNo );
	if( pEnt != NULL )
	{	// dirty가 아니면 그냥 돌아가면 된다.
		if( ( dwFlag & CACHE_DIRTY ) == 0 )
			return( pEnt );

		// 버퍼의 내용을 갱신해야 한다.
		memcpy( pEnt->pBuff, pBuff, pCache->pDevObj->nBlkSize );

		if( ( pEnt->dwFlag & CACHE_DIRTY ) == 0 )
		{	// Dirty Flag가 새로 추가된 경우.
			pEnt->dwFlag = dwFlag;
			goto SET_DIRTY;			// Dirty Link에 추가한다.
		}
		return( pEnt );
	}	

	if( pCache->nTotalEntry < pCache->nMaxEntry )
	{
		// 새로운 캐시 엔트리를 할당한다.
		pEnt = alloc_bcache_ent( pCache );
		if( pEnt == NULL )
			return( NULL );
		pCBuff = pEnt->pBuff;
	}
	else
	{	// 폐기할 수 있는 캐시 엔트리를 찾는다.
		pEnt = get_removable_cache_ent( pCache );
		if( pEnt == NULL )
		{
			kdbg_printf( "insert_blk_cache: no removable cache entry!\n" );
			return( NULL );		// 추가할 수도 기존 엔트리를 대체할 수도 없다.
		}
		pCBuff = pEnt->pBuff;
	}

	memset( pEnt, 0, sizeof( BCacheEntStt ) );
	pEnt->dwBlockNo = dwBlockNo;
	pEnt->dwFlag	= dwFlag;
	pEnt->pBuff     = pCBuff;
	memcpy( pEnt->pBuff, pBuff, pCache->pDevObj->nBlkSize );

	// 해시 키 값을 구한다.
	nI = bhash( pCache, dwBlockNo );
	pIndex = &pCache->pHashTbl[ nI ];

	pIndex->nTotal++;

	// Hash Index 링크에 추가한다.
	if( pIndex->pEnd == NULL )
	{	// 링크의 처음에 추가한다.
		pIndex->pStart = pIndex->pEnd = pEnt;
	}
	else
	{	// 링크의 가장 마지막에 추가한다.
		pIndex->pEnd->pNext = pEnt;
		pEnt->pPre			= pIndex->pEnd;
		pIndex->pEnd		= pEnt;
	}

	// DIrty이면 Dirty Link에 추가한다.
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

	// 기존 엔트리가 존재하는지 확인한다.
	pE = find_hash_ent( pCache, pEnt->dwBlockNo );
	if( pE != NULL )
		return( 0 );		// 캐시 엔트리에 이미 등록된 상태면 성공.
		
	// 해시 키 값을 구한다.
	nI = bhash( pCache, pEnt->dwBlockNo );
	pIndex = &pCache->pHashTbl[ nI ];

	pIndex->nTotal++;

	// Hash Index 링크에 추가한다.
	if( pIndex->pEnd == NULL )
	{	// 링크의 처음에 추가한다.
		pIndex->pStart = pIndex->pEnd = pEnt;
	}
	else
	{	// 링크의 가장 마지막에 추가한다.
		pIndex->pEnd->pNext = pEnt;
		pEnt->pPre			= pIndex->pEnd;
		pIndex->pEnd		= pEnt;
		pEnt->pNext         = NULL;
	}

	// DIrty이면 Dirty Link에 추가한다.
	if( pEnt->dwFlag & CACHE_DIRTY )
		insert_dirty_link( pCache, pEnt );

	pCache->nTotalEntry++;

	return( 0 );
}

int bcache_scatter_load( BlkDevObjStt *pDevObj, DWORD dwBlock, int nSectors )
{
	BCacheEntStt	**ppCEntArray;
	int 			nI, nSize, nR;

	// scatter read를 지원하지 않는다.
	if( pDevObj == NULL || pDevObj->pDev == NULL || pDevObj->pDev->op.scatter_read == NULL )
	{
		//kdbg_printf( "bcache_scatter_load: NULL pointer!\n" );
		return( -1 );
	}

	//kdbg_printf( "bcache_scatter_load( dwBlock=%d, nSectors=%d\n", dwBlock, nSectors );

	// 포인터 배열을 할당한다.
	nSize = sizeof(void*) * nSectors;
	ppCEntArray = (BCacheEntStt**)MALLOC( nSize );
	if( ppCEntArray == NULL )
	{
ERR:	kdbg_printf( "bcache_scatter_load: MALLOC failed!\n" );
		return( -1 );
	}

	// 캐시 엔트리들을 할당한다.
	for( nI = 0; nI < nSectors; nI++ )
	{
		ppCEntArray[nI] = alloc_bcache_ent( pDevObj->pCache );
		if( ppCEntArray[nI] == NULL )
		{	// 포인터 배열을 리턴하고 에러처리.
			FREE( ppCEntArray );
			goto ERR;
		}

		ppCEntArray[nI]->dwBlockNo = dwBlock + nI;
		ppCEntArray[nI]->dwFlag    = CACHE_ACCESS;
	}

	// 섹터들을 읽어들인다.
	nR = pDevObj->pDev->op.scatter_read( pDevObj, ppCEntArray, dwBlock, nSectors );

	// 캐시 엔트리에 추가한다.
	for( nI = 0; nI < nSectors; nI++ )
	{
		if( ppCEntArray[nI] == NULL || ppCEntArray[nI]->dwFlag == CACHE_INVALID )
			continue;
		
		insert_blk_cache_ex( pDevObj->pCache, ppCEntArray[nI] );
	}

	// 포인터 배열을 해제한다.
	FREE( ppCEntArray );

	return( 0 );
}








