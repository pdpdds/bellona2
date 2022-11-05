#include "vfs.h"

// ������� pVoidObj�� ���ϵȴ�.
static int ram_disk_open( int nMinor, void *pVoidObj, void *pDev )
{
	RamDiskStt		*pRD;
	BlkDevObjStt	*pObj;
	
	pObj = (BlkDevObjStt*)pVoidObj;

	pRD = (RamDiskStt*)MALLOC( sizeof( RamDiskStt ) );
	if( pRD == NULL )
		return( -1 );	// �޸𸮸� �Ҵ��� �� ����.
	else
		memset( pRD, 0, sizeof( RamDiskStt ) );

	pObj->pPtr			= pRD;
	pObj->pDev			= pDev;
	pObj->nMinor		= nMinor;
	pObj->dwTotalBlk	= 128 * 2;
	pObj->nBlkSize		= pObj->pDev->nBlkSize;

	pRD->pBuff			= (char*)MALLOC( pObj->dwTotalBlk * pObj->nBlkSize );
	if( pRD->pBuff == NULL )
		return( -1 );

	// �׳� �޸𸮸� �Ҵ��� �ΰ� ������ ������ Boot Sector, FAT, Root Dir��
	// �ʱ�ȭ�� RAM DISK�� �̿��ϴ� ���� �ý��� �������� ó���Ѵ�.
	
	return( 0 );
}

static int ram_disk_close( void *pVoidObj )
{
	RamDiskStt		*pRD;
	BlkDevObjStt	*pObj;

	pObj = (BlkDevObjStt*)pVoidObj;
	pRD  = (RamDiskStt*)pObj->pPtr;

	// �޸𸮸� �����ϸ� �ȴ�.
	FREE( pRD->pBuff );
	FREE( pRD );

	return( 0 );
}

// RAM DISK�� �� ���� ����
static int ram_disk_write( void *pVoidObj, void *_pIO )
{
	int				nI;
	RamDiskStt		*pRD;
	BlkDevIOStt		*pIO;
	BlkDevObjStt	*pObj;
	DWORD			dwOffs, dwT;

	pObj = (BlkDevObjStt*)pVoidObj;
	pRD  = (RamDiskStt*)pObj->pPtr;
	pIO  = (BlkDevIOStt*)_pIO;

	// �ùٸ� �ε������� Ȯ���Ѵ�.
	if( (DWORD)pObj->dwTotalBlk <= pIO->dwIndex )
		return( -1 );
	
	// ���� ����� ������ ó���� �� �ֵ��� �Ѵ�.
	for( nI = 0; nI < pIO->nBlocks; nI++ )
	{	// �� ��Ͼ� ����� ������ŭ ����Ѵ�.
		dwOffs = (DWORD)( pObj->nBlkSize * (int)( pIO->dwIndex + nI ) );
		dwT    = (DWORD)( pObj->nBlkSize * nI );
		memcpy( &pRD->pBuff[dwOffs], &pIO->pBuff[ dwT ], pObj->nBlkSize );
	}

	return( 0 );
}

// RAM DISK�� �� ���� �б�
static int ram_disk_read( void *pVoidObj, void *_pIO )
{
	RamDiskStt		*pRD;
	BlkDevIOStt		*pIO;
	DWORD			dwOffs;
	BlkDevObjStt	*pObj;

	pObj = (BlkDevObjStt*)pVoidObj;
	pRD = (RamDiskStt*)pObj->pPtr;
	pIO = (BlkDevIOStt*)_pIO;

	// �ùٸ� �ε������� Ȯ���Ѵ�.
	if( pObj->dwTotalBlk <= pIO->dwIndex )
		return( -1 );

	dwOffs = (DWORD)( pObj->nBlkSize * pIO->dwIndex );

	memcpy( pIO->pBuff, &pRD->pBuff[dwOffs], pObj->nBlkSize );

	return( 0 );
}

// Device Dependent I/O Control
static int ram_disk_ioctl( void *pVoidObj, void *pV )
{


	return( 0 );
}

// ����ũ ����̽� ����̹��� ����Ѵ�.
// ����ũ�� ����ڰ� ����� ũ��� ������ �����ϱ⶧���� setup���� ������ �ְ� ������
// HDD, FDD�� ����̽� ���½ÿ� �����ȴ�.
int init_ram_disk_driver()
{
	int			nR;
	BlkDevStt	ram_dev;

	memset( &ram_dev, 0, sizeof( ram_dev ) );

	strcpy( ram_dev.szName, "RAM DISK" );
	ram_dev.nBlkSize	= 512;
	ram_dev.nMajor		= RAM_DISK_MAJOR;
	ram_dev.op.open		= ram_disk_open ;
	ram_dev.op.close	= ram_disk_close;
	ram_dev.op.read		= ram_disk_read ;
	ram_dev.op.write	= ram_disk_write;
	ram_dev.op.ioctl	= ram_disk_ioctl;

	// ��� �Լ��� �θ���.
	nR = register_blkdev( &ram_dev );
	
	return( nR );
}

//����ũ ����̽� ����̹��� ��������Ѵ�.
int close_ram_disk_driver()
{
	int nR;

	nR = unregister_blkdev( RAM_DISK_MAJOR );

	return( nR );
}

