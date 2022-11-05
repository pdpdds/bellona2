// HDD Partition Driver
   
#include "vfs.h"

// Device Dependent I/O Control
static int hdd_part_ioctl( void *pVoidObj, void *pV )
{


	return( -1 );
}

// Win32 HDD�� ���� ����
static int hdd_part_write( void *pVoidObj, void *_pIO )
{

	return( -1 );
}

static int hdd_part_close( void *pVoidObj )
{
	HddPartStt		*pHP;
	BlkDevObjStt	*pObj;

	pObj = (BlkDevObjStt*)pVoidObj;
	pHP  = (HddPartStt*)pObj->pPtr;

	// �޸𸮸� �����ϸ� �ȴ�.
	if( pHP )
		FREE( pHP );
	
	return( 0 );
}

// partition������ ���� ���� �б�
static int hdd_part_read( void *pVoidObj, void *pVoidIO )
{
	int				nR;
	BlkDevIOStt		*pIO;
	HddPartStt		*pHP;
	DWORD			dwIndex;
	BlkDevObjStt	*pDevObj, *pBaseObj;

	pIO			= pVoidIO;
	pDevObj		= pVoidObj;
	pHP			= pDevObj->pPtr;
	pBaseObj	= pHP->pBaseDevObj;

	dwIndex = pIO->dwIndex + pHP->p.dwStartingSector;
	nR = read_block( pBaseObj, dwIndex, pIO->pBuff, pIO->nBlocks );
	
	return( nR );
}

static int hdd_part_scatter_read( BlkDevObjStt *pDevObj, BCacheEntStt **ppCEntArray, DWORD dwBlock, int nSectors )
{
	int 			nR;
	HddPartStt		*pHP;
	BlkDevObjStt	*pBaseObj;

	if( pDevObj == NULL || pDevObj->pPtr == NULL )
	{
		kdbg_printf( "hdd_part_scatter_read: pDevObj or pDevObj->pPtr = NULL\n" );
		return( -1 );
	}
	
	pHP	= pDevObj->pPtr;
	pBaseObj = pHP->pBaseDevObj;
	if( pBaseObj == NULL )
	{
		kdbg_printf( "hdd_part_scatter_read: pBaseObj = NULL\n" );
		return( -1 );
	}	
	
	dwBlock += pHP->p.dwStartingSector;

	//kdbg_printf( "hdd_part_scatter_read: direct call to ide_hdd_scatter_read\n" );

	nR = pBaseObj->pDev->op.scatter_read( pBaseObj, ppCEntArray, dwBlock, nSectors );
	
	return( nR );
}


// ������� pVoidObj�� ���ϵȴ�.
static int hdd_part_open( int nMinor, void *pVoidObj, void *pDev )
{
	HddPartStt		*pHP;
	BlkDevObjStt	*pObj;

	pObj = (BlkDevObjStt*)pVoidObj;
					
	pHP = (HddPartStt*)MALLOC( sizeof( HddPartStt ) );
	if( pHP == NULL )
		return( -1 );	// �޸𸮸� �Ҵ��� �� ����.
	
	memset( pHP, 0, sizeof( HddPartStt ) );
	pObj->pPtr    = pHP;
	pObj->pDev    = pDev;
	pObj->nMinor  = nMinor;			// minor == 0 �� �� primary master�� ù��° partition
	pObj->nVolume = nMinor / 4;		// 
	
	return( 0 );
}

// HDD Partition ����̹��� ����Ѵ�.
int init_hdd_part_driver()
{
	int			nR;
	BlkDevStt	hdd_part;

	memset( &hdd_part, 0, sizeof( hdd_part ) );

	strcpy( hdd_part.szName, "HDD PARTITION" );
	hdd_part.nBlkSize		 = 512;
	hdd_part.nMajor			 = HDD_PART_MAJOR;
	hdd_part.op.open		 = hdd_part_open ;
	hdd_part.op.close		 = hdd_part_close;
	hdd_part.op.read		 = hdd_part_read ;
	hdd_part.op.write		 = hdd_part_write;
	hdd_part.op.ioctl		 = hdd_part_ioctl;
	hdd_part.op.scatter_read = hdd_part_scatter_read ;

	// ��� �Լ��� �θ���.
	nR = register_blkdev( &hdd_part );

	return( 0 );
}

int close_hdd_part_driver()
{
	int nR;

	nR = unregister_blkdev( HDD_PART_MAJOR );

	return( nR );
}
