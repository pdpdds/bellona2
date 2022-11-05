#include "bellona2.h"

static ShMemStt	shmem;

// 실메모리의 크기를 구한다.
int nGetPhysMemSize()
{
	int		nI, nSize;
	UCHAR	*pX, byTe;;

	// 실메모리의 크기를 구한다.
    
    // 원래 512M까지 뒤졌는데 1기가로 변경. (2002-11-27)
	// 메모리는 2메가부터 1기가 바이트까지 뒤진다.
	for( nSize = nI = 2; nI < 1024; nI++ )
	{
		pX = (UCHAR*)( nI * (ULONG)0x100000 );  
		
		byTe = pX[0];
		byTe++;
		pX[0] = byTe;
		if( byTe != pX[0] )
			break;
		else
			nSize++;
        
        // 2002-11-27 추가.
        pX[0] -= 1;
	}

	return( nSize * 0x100000 );  // 바이트 단위로 리턴한다. 
}

// 현재 가용 물리 메모리의 크기를 구한다.
int get_available_phys_size( int *pSingleMapped, int *pMultiMapped )
{
	UCHAR	*pTbl;
	int		nI, nSingle, nMulti, nFree;

	_asm PUSHFD
	_asm CLI

	pTbl = bell.pPhysRefTbl;
	nSingle = nMulti = nFree = 0;
	for( nI = 0; nI < bell.nPhysRefSize; nI++ )
	{
		if( pTbl[nI] == 0 )
			nFree++;
		else if( pTbl[nI] == 1 )
			nSingle++;
		else
			nMulti++;
	}
	_asm POPFD

	if( pSingleMapped != NULL )
		pSingleMapped[0] = nSingle;

	if( pMultiMapped != NULL )
		pMultiMapped[0] = nMulti;

	return( nFree );
}

// initialize shared memory structure
int init_shared_mem()
{
	ShMemStt *pShMem = &shmem;

	memset( pShMem, 0, sizeof( ShMemStt ) );

	return( 0 );
}

// find shared memory entry
int find_shmem_ent( char *pName, int nSize )
{
	int				nI, nK;
	ShMemChunkStt	*pChunk;
	ShMemStt		*pShMem = &shmem;

	// find empty slot
	for( nI = 0; nI < pShMem->nTotalChunk; nI++ )
	{
		pChunk = pShMem->p_chunk[nI];
		if( pChunk == NULL )
		{
			kdbg_printf( "find_shmem_ent() - critical error!\n" );
			return( -1 );
		}

		if( pChunk->nTotalEnt < MAX_SHMEM_ENT )
		{
			for( nK = 0; nK < MAX_SHMEM_ENT; nK++ )
			{	
				// match size
				if( nSize == -1 || nSize == pChunk->ent[nK].nSize )
				{
					if( pName == NULL || strcmpi( pName, pChunk->ent[nK].szName ) == 0 )
							return( nK + nI * MAX_SHMEM_ENT );
				}
			}
		}
	}

	return( -1 );
}

// allocate new chunk
static int inc_shmem_chunk()
{
	ShMemChunkStt *pChunk;

	if( shmem.nTotalChunk >= MAX_SHMEM_CHUNK )
		return( -1 );

	// allocate new chunk
	pChunk = kmalloc( sizeof( ShMemChunkStt ) );
	if( pChunk == NULL )
	{
		kdbg_printf( "inc_shmem_chunk() - kmalloc failed!\n" );
		return( -1 );
	}

	// clear with 0
	memset( pChunk, 0, sizeof( ShMemChunkStt ) );
	shmem.p_chunk[ shmem.nTotalChunk++ ] = pChunk;	

	return( 0 );
}

// create new shmem structure
int create_shmem( char *pName, int nSize, int nFlag )
{
	ShMemEntStt	*pEnt;
	DWORD		*pPhysList;
	DWORD		dwPhysAddr;
	int			nID, nI, nR, nTotalPage;

	// allocate page list
	nTotalPage = ( nSize + 4095 ) / 4096;
	pPhysList  = (DWORD*)kmalloc( nTotalPage * 4 );
	memset( pPhysList, 0, nTotalPage * 4 );
	
	// check whether the same entry exists.
	nR = find_shmem_ent( pName, -1 );	// don't care size
	if( nR >= 0 )
		goto ERROR;

	// find empty slot
	nR = find_shmem_ent( "", 0 );
	if( nR < 0 )
	{	// increase shared memory chunk
		nR = inc_shmem_chunk();
		if( nR == -1 )
			goto ERROR;
		nR = find_shmem_ent( "", 0 );  // try again
		if( nR < 0 )
			goto ERROR;
	}

	nID = nR;
	pEnt = &shmem.p_chunk[ nID / MAX_SHMEM_ENT ]->ent[ nID % MAX_SHMEM_ENT ];
	shmem.p_chunk[ nID / MAX_SHMEM_ENT ]->nTotalEnt++;
	
	// initialize entry
	memset( pEnt, 0, sizeof( ShMemEntStt ) );

	// allocate physical pages
	for( nI = 0; nI < nTotalPage; nI++ )
	{
		dwPhysAddr = dwAllocPhysPage();  // reference counter is increased by 1.
		if( dwPhysAddr == 0 )
		{
			kdbg_printf( "create_shmem() - dwAllocPhysPage failed!\n" );
			return( -1 );
		}
		pPhysList[nI] = dwPhysAddr;
	}

	// copy shared memory name
	strcpy( pEnt->szName, pName );
	pEnt->nSize			= nSize;
	pEnt->nFlag			= nFlag;
	pEnt->nTotalPage	= nTotalPage;
	pEnt->pPhysList		= pPhysList;
			 
	return( nR );
	
ERROR:
	kfree( pPhysList );
	return( -1 );
}

static ShMemEntStt *get_shmem_ent( int nID )
{
	ShMemEntStt		*pEnt;
	ShMemChunkStt	*pChunk;

	if( nID < 0 || nID / MAX_SHMEM_ENT >= shmem.nTotalChunk  )
		return( NULL );

	pChunk = shmem.p_chunk[ nID / MAX_SHMEM_ENT ];
	if(	pChunk == NULL || pChunk->nTotalEnt == 0 )
		return( NULL );

	pEnt = &pChunk->ent[ nID % MAX_SHMEM_ENT ];
	if( pEnt->nSize == 0 || pEnt->pPhysList == NULL )
		return( NULL );

	return( pEnt );
}

// release shmem structure
int close_shmem( int nID )	// nI is the ID of the shared memory
{
	int				nI;
	ShMemEntStt		*pEnt;
	ShMemChunkStt	*pChunk;

	pEnt = get_shmem_ent( nID );
	if( pEnt == NULL )
		return( -1 );
	
	// if the reference counter of the entry is not zero, it can not be released
	if( pEnt->nRefCount > 0 )
		return( -1 );

	// release page list
	if( pEnt->pPhysList != NULL )
	{
		for( nI = 0; nI < pEnt->nTotalPage; nI++ )
			nFreePage( pEnt->pPhysList[nI] );		
		
		kfree( pEnt->pPhysList );
	}
		
	pChunk = shmem.p_chunk[ nID / MAX_SHMEM_ENT ];
	pChunk->nTotalEnt--;

	memset( pEnt, 0, sizeof( ShMemEntStt ) );	

	return( 0 );
}

// attach shared memory
int attach_shmem( int nID, DWORD dwVAddr )
{
	ShMemEntStt		*pEnt;
	int				nI, nR;
	ThreadStt		*pThread;
	DWORD			dwVAddress, dwPhysAddr, *pPD;

	// get shared memory entry
	pEnt = get_shmem_ent( nID );
	if( pEnt == NULL )
		return( -1 );

	pThread = get_current_thread();
	if( pThread == NULL )
		return( -1 );
									
	dwVAddress	= (dwVAddr + 4095) / 4096;
	dwVAddress  *= 4096;
	pPD			= (DWORD*)get_thread_page_dir( pThread );

	// mapping pages
	for( nI = 0; nI < pEnt->nTotalPage; nI++ )
	{
		dwPhysAddr = pEnt->pPhysList[nI];
		
		if( dwVAddress < (DWORD)0x80000000 )
			nR = _nMappingVAddr( pPD, dwVAddress, dwPhysAddr );		// kernel space
		else
			nR = _nMappingUserVAddr( pPD, dwVAddress, dwPhysAddr );	// user space
		if( nR < 0 )
		{
			kdbg_printf( "critical error: attach_shmem() - _nMappingVAddr failed!\n" );
			return( -1 );
		}

		dwVAddress += 4096;
	}					   

	// increase reference counter
	pEnt->nRefCount++;

	kdbg_printf( "attaching shared memory 0x%08X (%d)- ok\n", dwVAddr, pEnt->nTotalPage );

	return( 0 );
}

int detach_shmem( int nID, DWORD dwVAddr )
{
	int				nI;
	ShMemEntStt		*pEnt;
	ThreadStt		*pThread;
	DWORD			dwVAddress, *pPD;

	// get shared memory entry
	pEnt = get_shmem_ent( nID );
	if( pEnt == NULL )
		return( 1 );

	pThread = get_current_thread();
	if( pThread == NULL )
		return( -1 );
									
	dwVAddress	= (dwVAddr + 4095) / 4096;
	dwVAddress  *= 4096;
	pPD			= (DWORD*)get_thread_page_dir( pThread );

	// mapping pages
	for( nI = 0; nI < pEnt->nTotalPage; nI++ )
	{
		dwReleaseMappingVAddr( pPD, dwVAddress );

		dwVAddress += 4096;
	}					   

	// increase reference counter
	pEnt->nRefCount--;

	return( 0 );
}

// display shared memory structure
int disp_shmem()
{
	ShMemEntStt		*pEnt;
	ShMemChunkStt	*pChunk;
	int				nI, nK, nTotalEnt;

	nTotalEnt = 0;
	for( nI = 0; nI < shmem.nTotalChunk; nI++ )
	{
		pChunk = shmem.p_chunk[nI];
		for( nK = 0; nK < MAX_SHMEM_ENT; nK++ )
		{
			pEnt = &pChunk->ent[nK];
			if( pEnt->nSize > 0 )
			{
				kdbg_printf( "[%3d] 0x%X (%-7d) %08X %s\n", 
					nI * MAX_SHMEM_ENT + nK, pEnt->nSize, pEnt->nSize, pEnt->nFlag, pEnt->szName );
				nTotalEnt++;
			}
		}
	}

	kdbg_printf( "total %d shared memory struct.\n", nTotalEnt );
	return( 0 );
}

int lock_shmem( int nID )
{
	int			nR;
	ShMemEntStt	*pEnt;

	pEnt = get_shmem_ent( nID );
	if( pEnt == NULL )
		return( -1 );				// invalid shmem id

	_asm {
		PUSHFD
		CLI
	}

	if( pEnt->nLock == 0 )
		nR = pEnt->nLock = 1;		// ok, now locked
	else
		nR = 0;						// failed. already locked

	_asm POPFD

	return( nR );
}

int unlock_shmem( int nID )
{
	int			nR;
	ShMemEntStt	*pEnt;

	pEnt = get_shmem_ent( nID );
	if( pEnt == NULL )
		return( -1 );				// invalid shmem id

	_asm {
		PUSHFD
		CLI
	}

	if( pEnt->nLock != 0 )
	{
		pEnt->nLock = 0;		// ok, now unlocked
		nR = 1;
	}
	else
		nR = 0;						// failed. not locked

	_asm POPFD

	return( nR );
}















