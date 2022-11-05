#include "vfs.h"							
#include "hdd.h"

#ifdef WIN32TEST
	#define READ_HDD_SECTOR		nW32IDE_Read
#endif
#ifdef BELLONA2
	extern int read_sectors( int nDrv, DWORD dwIndex, int nTotalSector, char *pBuff );
	#define READ_HDD_SECTOR		read_sectors
#endif										

extern int scatter_read_sectors	( int nDrv, struct BCacheEntTag **ppCEntArray, DWORD dwBlock, int nSectors );

static int ide_hdd_write( void *pVoidObj, void *_pIO )
{
	return(-1);
}
static int ide_hdd_read( void *pVoidObj, void *pVoidIO )
{
	int				nR;
	BlkDevIOStt		*pIO;
	BlkDevObjStt	*pObj;

	pObj = pVoidObj;
	pIO  = pVoidIO;

	nR = READ_HDD_SECTOR( pObj->nVolume, pIO->dwIndex, pIO->nBlocks, pIO->pBuff );

	return( nR );
}

static int ide_hdd_scatter_read( BlkDevObjStt *pDevObj, BCacheEntStt **ppCEntArray, DWORD dwBlock, int nSectors )
{
	int nR;

	nR = scatter_read_sectors( pDevObj->nVolume, ppCEntArray, dwBlock, nSectors );
	
	return( nR );
}

static int ide_hdd_ioctl( void *pVoidObj, void *pV )
{
	return(-1);
}

static int ide_hdd_close( void *pVoidObj )
{
	IDEHddStt		*pIH;
	BlkDevObjStt	*pObj;
	int				nR, nI;

	pObj = (BlkDevObjStt*)pVoidObj;
	pIH  = pObj->pPtr;

	// close the opened partition block device object
	for( nI = 0; nI < 4; nI++ )
	{
		if( pIH->part_dev_obj[nI].pDev != NULL )
		{	// close partition block device object
			nR = close_block_device( &pIH->part_dev_obj[nI] );
			if( nR < 0 )
				ERROR_PRINTF( "ide_hdd_close() - partition device closing failed!\n" );
		}
	}

	if( pIH )
		FREE( pIH );

	return(-1);
}

static int ide_hdd_geometry( BlkDevObjStt *pObj )
{
	//^^^^^

	return(0);
}

// ide hdd�� �����Ѵ�.
static int ide_hdd_open( int nMinor, void *pVoidObj, void *pDev )
{
	IDEHddStt		*pIH;
	BlkDevObjStt	*pObj;
	int 			nR, nI;

	pObj = (BlkDevObjStt*)pVoidObj;
					
	pIH = (IDEHddStt*)MALLOC( sizeof( IDEHddStt ) );
	if( pIH == NULL )
		return( -1 );	// �޸𸮸� �Ҵ��� �� ����.
	
	memset( pIH, 0, sizeof( IDEHddStt ) );
	pObj->pPtr    = pIH;
	pObj->pDev    = pDev;
	pObj->nMinor  = nMinor;			// minor == 0 �� �� C�� �ȴ�.
	pObj->nVolume =	nMinor;			// C = 0

#ifdef WIN32TEST
	pObj->nVolume += 2;				// A = 0, B = 1, C = 2...
#endif														 
	
	// Geometry������ MBS�� �о���δ�.	
	{
		BlkDevIOStt		io;

		// geometry������ ��´�.
		nR = ide_hdd_geometry( pObj );
		if( nR < 0 )
		{	// geometry������ ���� �� ����.
			FREE( pIH );
			return( -1 );
		}

		io.pBuff	= (char*)&pIH->mbs;
		io.dwIndex	= 0;
		io.nBlocks	= 1;

		// MBS�� �д´�.
		nR = ide_hdd_read( pObj, &io );		
		if( nR < 0 )
		{	// MBS�� ���� �� ����.
			FREE( pIH );
			return( -1 );
		}
	}

	// mbs�� partition�� �ν��ؼ� partition driver�� ������ �ش�.
	for( nI = 0; nI < 4; nI++ )
	{
		if( pIH->mbs.part[nI].byType != 0 && pIH->mbs.part[nI].dwStartingSector > 0 )
		{
			HddPartStt *pHP;

			nR = open_block_device( &pIH->part_dev_obj[nI], HDD_PART_MAJOR, nI + nMinor*4 );
			if( nR == 0 )
			{	// partition�� �����Ѵ�.
				pHP = pIH->part_dev_obj[nI].pPtr;
				memcpy( &pHP->p, &pIH->mbs.part[nI], sizeof( PartitionStt ) );		// copy partition
				pHP->pBaseDevObj = pObj;											// set Base Device Object

				// total block�� �����Ѵ�.
				pIH->part_dev_obj[nI].dwTotalBlk = pIH->mbs.part[nI].dwTotalSector;
				pIH->part_dev_obj[nI].nAttr = BLKDEV_ATTR_NESTED;
			}
		}
	}

	return( 0 );
}

// IDE HDD ����̹��� �ʱ�ȭ�Ѵ�.
int init_ide_hdd_driver()
{
	int			nR, nX;
	BlkDevStt	ide_hdd;

	memset( &ide_hdd, 0, sizeof( ide_hdd ) );

	strcpy( ide_hdd.szName, "IDE HDD" );
	ide_hdd.nBlkSize		= 512;
	ide_hdd.nMajor			= IDE_HDD_MAJOR;
	ide_hdd.op.open			= ide_hdd_open ;
	ide_hdd.op.close		= ide_hdd_close;
	ide_hdd.op.read			= ide_hdd_read ;
	ide_hdd.op.write		= ide_hdd_write;
	ide_hdd.op.ioctl		= ide_hdd_ioctl;
	ide_hdd.op.scatter_read = ide_hdd_scatter_read;

	// ��� �Լ��� �θ���.
	nR = register_blkdev( &ide_hdd );

#ifdef WIN32TEST/////////////
	nR = nW32IDE_Init();   
#endif///////////////////////

	// partition driver�� �ʱ�ȭ�Ѵ�.
	nX = init_hdd_part_driver( HDD_PART_MAJOR );

	return( nR );
}

int close_ide_hdd_driver()
{
	int nR, nX;

	nR = unregister_blkdev( IDE_HDD_MAJOR );

	// partition driver�� close�Ѵ�.
	nX = close_hdd_part_driver();

#ifdef WIN32TEST///////////////
	nR = nW32IDE_Close();	 //
#endif/////////////////////////

	return( nR );
}

