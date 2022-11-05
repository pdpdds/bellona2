#include "vfs.h"

static int fdd_fat16_ioctl( void *pVoidObj, void *pVoidIoctl )
{

	return( -1 );
}

// Win32 HDD의 한 섹터 쓰기
static int fdd_fat16_write( void *pVoidObj, void *_pIO )
{

	return( -1 );
}

static int fdd_fat16_close( void *pVoidObj )
{
	// nothing to process
	return( 0 );
}

// read one sector on base fdd block device driver
static int _fdd_fat16_read_one_sector( void *pVoidObj, void *pVoidIO )
{
	int				nR;
	BlkDevIOStt		*pIO, io;
	F16PartStt		*pF16Part;
	BlkDevObjStt	*pObj, *pBaseObj;
	
	pIO			= pVoidIO;
	pObj		= pVoidObj;
	pF16Part	= pObj->pPtr;
	pBaseObj	= pF16Part->pBaseDevObj;

	memcpy( &io, pIO, sizeof( io ) );

	if( pF16Part->nRootLoc <= (int)pIO->dwIndex && (int)pIO->dwIndex < pF16Part->nRootLoc + pF16Part->nRootBlocks )
		;
	else if( pF16Part->nRootLoc + pF16Part->nRootBlocks <= (int)pIO->dwIndex && (int)pIO->dwIndex < pF16Part->nDataLoc )
	{
		memset( io.pBuff, 0, io.nBlocks * 512 );
		return( 0 );
	}
	else if( pF16Part->nDataLoc <= (int)pIO->dwIndex )
		io.dwIndex -= (DWORD)pF16Part->nRelocation;		// relocation = logical root block number - physical root block number

	nR = read_block( pBaseObj, io.dwIndex, io.pBuff, io.nBlocks );
	
	return( nR );
}	

// read multiple sectors on base fdd block device driver
static int fdd_fat16_read( void *pVoidObj, void *pVoidIO )
{
	int			nI, nR;
	BlkDevIOStt	*pIO, io;

	pIO = pVoidIO;

	for( nI = 0; nI < pIO->nBlocks; nI++ )
	{
		io.dwIndex = pIO->dwIndex + (DWORD)nI;
		io.pBuff   = &pIO->pBuff[nI*512];
		io.nBlocks = 1;
		nR = _fdd_fat16_read_one_sector( pVoidObj, &io );
		if( nR < 0 )
			return( -1 );
	}				  

	return( 0 );
}				 

// pVoidObj will be allocated and filled.
static int fdd_fat16_open( int nMinor, void *pVoidObj, void *pDev )
{
	F16PartStt		*pF16Part;
	BlkDevObjStt	*pObj;

	pObj = (BlkDevObjStt*)pVoidObj;
					
	pF16Part = (F16PartStt*)MALLOC( sizeof( F16PartStt ) );
	if( pF16Part == NULL )
		return( -1 );	// allocation failed!
	
	memset( pF16Part, 0, sizeof( F16PartStt ) );
	pObj->pPtr    = pF16Part;
	pObj->pDev    = pDev;
	pObj->nMinor  = nMinor;			// minor == 0 means A drive
	pObj->nVolume = nMinor;

	return( 0 );
}

// init fat16 block device driver
int init_fat16_driver()
{
	int			nR;
	BlkDevStt	fdd_fat16;
	
	memset( &fdd_fat16, 0, sizeof( fdd_fat16 ) );

	strcpy( fdd_fat16.szName, "FAT16 PART" );
	fdd_fat16.nBlkSize	= 512;
	fdd_fat16.nMajor	= FAT16_PART_MAJOR;
	fdd_fat16.op.open	= fdd_fat16_open ;
	fdd_fat16.op.close	= fdd_fat16_close;
	fdd_fat16.op.read	= fdd_fat16_read ;
	fdd_fat16.op.write	= fdd_fat16_write;
	fdd_fat16.op.ioctl	= fdd_fat16_ioctl;

	// 등록 함수를 부른다.
	nR = register_blkdev( &fdd_fat16 );

	return( nR );
}

int close_fat16_driver()
{
	int nR;

	nR = unregister_blkdev( FAT16_PART_MAJOR );

	return( nR );
}

