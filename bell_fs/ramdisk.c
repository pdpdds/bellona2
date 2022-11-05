#include "vfs.h"

// 결과값이 pVoidObj로 리턴된다.
static int ram_disk_open( int nMinor, void *pVoidObj, void *pDev )
{
	RamDiskStt		*pRD;
	BlkDevObjStt	*pObj;
	
	pObj = (BlkDevObjStt*)pVoidObj;

	pRD = (RamDiskStt*)MALLOC( sizeof( RamDiskStt ) );
	if( pRD == NULL )
		return( -1 );	// 메모리를 할당할 수 없다.
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

	// 그냥 메모리만 할당해 두고 나머지 논리적인 Boot Sector, FAT, Root Dir의
	// 초기화는 RAM DISK를 이용하는 파일 시스템 레벨에서 처리한다.
	
	return( 0 );
}

static int ram_disk_close( void *pVoidObj )
{
	RamDiskStt		*pRD;
	BlkDevObjStt	*pObj;

	pObj = (BlkDevObjStt*)pVoidObj;
	pRD  = (RamDiskStt*)pObj->pPtr;

	// 메모리만 해제하면 된다.
	FREE( pRD->pBuff );
	FREE( pRD );

	return( 0 );
}

// RAM DISK의 한 섹터 쓰기
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

	// 올바른 인덱스인지 확인한다.
	if( (DWORD)pObj->dwTotalBlk <= pIO->dwIndex )
		return( -1 );
	
	// 여러 블록일 때에도 처리할 수 있도록 한다.
	for( nI = 0; nI < pIO->nBlocks; nI++ )
	{	// 한 블록씩 블록의 개수만큼 기록한다.
		dwOffs = (DWORD)( pObj->nBlkSize * (int)( pIO->dwIndex + nI ) );
		dwT    = (DWORD)( pObj->nBlkSize * nI );
		memcpy( &pRD->pBuff[dwOffs], &pIO->pBuff[ dwT ], pObj->nBlkSize );
	}

	return( 0 );
}

// RAM DISK의 한 섹터 읽기
static int ram_disk_read( void *pVoidObj, void *_pIO )
{
	RamDiskStt		*pRD;
	BlkDevIOStt		*pIO;
	DWORD			dwOffs;
	BlkDevObjStt	*pObj;

	pObj = (BlkDevObjStt*)pVoidObj;
	pRD = (RamDiskStt*)pObj->pPtr;
	pIO = (BlkDevIOStt*)_pIO;

	// 올바른 인덱스인지 확인한다.
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

// 램디스크 디바이스 드라이버를 등록한다.
// 램디스크는 사용자가 블록의 크기와 개수를 지정하기때문에 setup에서 설정해 주고 나머지
// HDD, FDD는 디바이스 오픈시에 결정된다.
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

	// 등록 함수를 부른다.
	nR = register_blkdev( &ram_dev );
	
	return( nR );
}

//램디스크 디바이스 드라이버를 등록해제한다.
int close_ram_disk_driver()
{
	int nR;

	nR = unregister_blkdev( RAM_DISK_MAJOR );

	return( nR );
}

