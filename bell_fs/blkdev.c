#include "vfs.h"

static BlockDeviceStt	blk;

// return the first registered block device objet
BlkDevObjStt *find_first_blkdev_obj( int *pTotal )
{
	BlkDevObjStt	*pObj;

	pObj	= blk.pObjStart;
	*pTotal = blk.nTotalObj;
	
	return( pObj );
}

// open block device and return its device object.
int open_block_device( BlkDevObjStt *pObj, int nMajor, int nMinor )
{
	BlkDevStt	*pDev;
	int			nR;

	pDev = blk.ent[nMajor];
	if( pDev == NULL )			// device is not registered.
		return( -1 );

	// dispatch open call (ide hdd�� ��� ide_hdd_open()�� ȣ��ȴ�.)
	nR = pDev->op.open( nMinor, pObj, pDev );
	if( nR == -1 )
		return( -1 );

	// ��� ����� �����Ѵ�.
	pObj->nBlkSize = pDev->nBlkSize;

	// ���µ� ��� ����̽� ��ü�� ����Ʈ�� �����Ѵ�.
	if( blk.pObjEnd == NULL )		// �ϳ��� ����Ǿ����� �ʴ�.
	{
		blk.pObjStart  = blk.pObjEnd = pObj;
		pObj->pPre = pObj->pNext = NULL;
	}
	else
	{
		pObj->pPre			= blk.pObjEnd;
		pObj->pNext			= NULL;
		blk.pObjEnd->pNext	= pObj;
		blk.pObjEnd			= pObj;
	}

	blk.nTotalObj++;
	
	// Cache Manager�� �����Ѵ�. (�����ص� �������.)
	pObj->pCache = make_blk_cache( pObj, 32, 256 );

	return( 0 );
}

// ����̽� ������Ʈ�� ã�´�.
BlkDevObjStt *find_dev_obj( int nMajor, int nMinor )
{
	BlkDevObjStt	*pObj;

	for( pObj = blk.pObjStart; pObj != NULL; pObj = pObj->pNext )
	{
		if( pObj->pDev->nMajor == nMajor )		// Major Number�� ����.
		{	// -1 = don't care
			if( nMinor == -1 || nMinor == pObj->nMinor )
				return( pObj );
		}
	}	

	return( NULL );
}

// ��� ����̽� ����̹��� �ݴ´�.
int close_block_device( BlkDevObjStt *pObj )
{
	BlkDevObjStt	*pT;
	int				nR;

	// Cache Manager�� �����Ѵ�.
	if( pObj->pCache != NULL )
	{
		close_blk_cache( pObj->pCache );
		pObj->pCache = NULL;
	}

	nR = pObj->pDev->op.close( pObj );

	if( pObj->pPre != NULL )
	{
		pT = pObj->pPre;
		pT->pNext = pObj->pNext;
		if( pT->pNext == NULL )
			blk.pObjEnd = pT;
	}

	if( pObj->pNext != NULL )
	{
		pT = pObj->pNext;
		pT->pPre = pObj->pPre;
		if( pT->pPre == NULL )
			blk.pObjStart = pT;
	}

	blk.nTotalObj--;
	
	return( nR );
}

// register block device driver
int register_blkdev( BlkDevStt *pB )
{
	BlkDevStt	*pT;

	if( pB->nMajor <= 0 || pB->nMajor >= MAX_BLK_DEV )
	{	// invalid MAJOR NUMBER
		return( -1 );
	}

	if( blk.ent[ pB->nMajor ] != NULL )
	{	// the MAJOR_NUMBER is already being used.
		return( -1 );
	}

	// allocate memory
	pT = (BlkDevStt*)MALLOC( sizeof( BlkDevStt ) );
	if( pT == NULL )
	{	// memory allocation failed.
		return( -1 );
	}

	memcpy( pT, pB, sizeof( BlkDevStt ) );

	blk.ent[pB->nMajor] = pT;
	blk.nTotal++;

	return( 0 );
}

// unregister block device driver
int unregister_blkdev( int nMajor )
{
	if( nMajor <= 0 || nMajor >= MAX_BLK_DEV )
	{	// invalid MAJOR NUMBER
		return( -1 );
	}

	if( blk.ent[ nMajor ] == NULL )
	{	// the block device is not being used.
		return( -1 );
	}

	FREE( blk.ent[nMajor] );
	blk.nTotal--;
	blk.ent[nMajor] = NULL;

	return( 0 );
}

// write to block device
int write_block( BlkDevObjStt *pObj, DWORD dwIndex, char *pBuff, int nBlocks )
{
	BlkDevStt	*pDev;
	BlkDevIOStt	io;	
	int			nR;

	pDev = (BlkDevStt*)pObj->pDev;

	io.dwIndex	= dwIndex;
	io.pBuff	= pBuff;
	io.nBlocks	= nBlocks;

	if( pDev->op.write == NULL )
		return( -1 );

	// ���� IO�� �����Ѵ�.
	nR = pDev->op.write( pObj, &io );
	if( nR != 0 )
		return( -1 );	//  ������ �߻��ߴ�.

	return( 0 );
}

// ��� �б�
int read_block( BlkDevObjStt *pObj, DWORD dwIndex, char *pBuff, int nBlocks )
{
	BlkDevIOStt		io;	
	int				nR;
	BlkDevStt		*pDev;

    if( pObj == NULL || pObj->pDev == NULL || pObj->pDev->op.read == NULL )
        return( -1 );

	pDev = (BlkDevStt*)pObj->pDev;

	io.dwIndex	= dwIndex;
	io.pBuff	= pBuff;
	io.nBlocks	= nBlocks;

	// block cache�� �����ϴ� ���۸� �����Ѵ�.
	nR = read_blk_cache( pObj, &io );
		
	return( nR );
}

// ��� ����̽��κ��� �о� ĳ�ÿ� �ִ� ������ ������.
// ioctl INVALIDATE�� �̿��Ѵ�.
int discard_block_device( BlkDevObjStt *pObj )
{
    int             nR;
    BlkDevIoctlStt  ioc;                       

    if( pObj == NULL || pObj->pDev == NULL || pObj->pDev->op.ioctl == NULL )
        return( -1 );

    memset( &ioc, 0, sizeof( ioc ) );
    ioc.nFunc = BLKDEV_IOCTL_INVALIDATE;
    nR = pObj->pDev->op.ioctl( pObj, &ioc );

    return( nR );
}

// ioctl�� �̿��Ͽ� ��� ����̽��� ���½�Ų��.
int reset_block_device( BlkDevObjStt *pObj )
{
    int             nR;
    BlkDevIoctlStt  ioc;                       

    if( pObj == NULL || pObj->pDev == NULL || pObj->pDev->op.ioctl == NULL )
        return( -1 );

    memset( &ioc, 0, sizeof( ioc ) );
    ioc.nFunc = BLKDEV_IOCTL_RESET;
    nR = pObj->pDev->op.ioctl( pObj, &ioc );

    return( nR );
}








