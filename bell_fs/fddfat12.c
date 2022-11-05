#include "vfs.h"

static int fdd_fat12_ioctl( void *pVoidObj, void *pVoidIoctl )
{

	return( -1 );
}

// Win32 FDD의 한 섹터 쓰기
static int fdd_fat12_write( void *pVoidObj, void *_pIO )
{

	return( -1 );
}

static int fdd_fat12_close( void *pVoidObj )
{
	FDDFat12Stt		*pFDDFat12;
	BlkDevObjStt	*pObj;

	pObj	  = (BlkDevObjStt*)pVoidObj;
	pFDDFat12 = (FDDFat12Stt*)pObj->pPtr;

	// release buffers
	if( pFDDFat12->pFat != NULL )
		FREE( pFDDFat12->pFat );

	if( pFDDFat12->pRootDir != NULL )
		FREE( pFDDFat12->pRootDir );
		
	if( pFDDFat12 )
		FREE( pFDDFat12 );
	
	return( 0 );
}

// read one sector on base fdd block device driver
static int _fdd_fat12_read_one_sector( void *pVoidObj, void *pVoidIO )
{
	int				nR, nBuffOffset;
	BlkDevObjStt	*pObj, *pBaseObj;
	BlkDevIOStt		*pIO, io;
	FDDFat12Stt		*pFDDFat12;

	pIO			= pVoidIO;
	pObj		= pVoidObj;
	pFDDFat12	= pObj->pPtr;
	pBaseObj	= pFDDFat12->pBaseDevObj;

	// dbs
	if( pIO->dwIndex == 0 )
		nR = read_block( pBaseObj, pIO->dwIndex, pIO->pBuff, pIO->nBlocks );
	else if( pFDDFat12->nRootLoc <= (int)pIO->dwIndex && (int)pIO->dwIndex < pFDDFat12->nRootLoc + pFDDFat12->nRootBlocks )
	{	// root directory
		nBuffOffset = (int)pIO->dwIndex - pFDDFat12->nRootLoc;
		nBuffOffset *= 512;
		memcpy( pIO->pBuff, &pFDDFat12->pRootDir[nBuffOffset], 512 );	// copy one sector
		return( 0 );
	}
	else if( pFDDFat12->nFatLoc <= (int)pIO->dwIndex && (int)pIO->dwIndex < pFDDFat12->nFatLoc + pFDDFat12->nFatBlocks )
	{
		nBuffOffset = (int)pIO->dwIndex - pFDDFat12->nFatLoc;
		nBuffOffset *= 512;
		memcpy( pIO->pBuff, &pFDDFat12->pFat[nBuffOffset], 512 );		// copy one sector
		return( 0 );
	}
	else
	{
		memcpy( &io, pIO, sizeof( io ) );
		io.dwIndex -= (DWORD)pFDDFat12->nRelocation;		// relocation = logical root block number - physical root block number

		nR = read_block( pBaseObj, io.dwIndex, io.pBuff, io.nBlocks );
	}
	
	return( nR );
}	

// read multiple sectors on base fdd block device driver
static int fdd_fat12_read( void *pVoidObj, void *pVoidIO )
{
	int			nI, nR;
	BlkDevIOStt	*pIO, io;

	pIO = pVoidIO;

	for( nI = 0; nI < pIO->nBlocks; nI++ )
	{
		io.dwIndex = pIO->dwIndex + (DWORD)nI;
		io.pBuff   = &pIO->pBuff[nI*512];
		io.nBlocks = 1;
		nR = _fdd_fat12_read_one_sector( pVoidObj, &io );
		if( nR < 0 )
			return( -1 );
	}				  

	return( 0 );
}				 

// pVoidObj will be allocated and filled.
static int fdd_fat12_open( int nMinor, void *pVoidObj, void *pDev )
{
	FDDFat12Stt		*pFDDFat12;
	BlkDevObjStt	*pObj;

	pObj = (BlkDevObjStt*)pVoidObj;
					
	pFDDFat12 = (FDDFat12Stt*)MALLOC( sizeof( FDDFat12Stt ) );
	if( pFDDFat12 == NULL )
		return( -1 );	// allocation failed!
	
	memset( pFDDFat12, 0, sizeof( FDDFat12Stt ) );
	pObj->pPtr    = pFDDFat12;
	pObj->pDev    = pDev;
	pObj->nMinor  = nMinor;			// minor == 0 means A drive
	pObj->nVolume = nMinor;

	return( 0 );
}

// FDD35 드라이버를 초기화한다.
int init_fdd_fat12_driver()
{
	int			nR;
	BlkDevStt	fdd_fat12;

	memset( &fdd_fat12, 0, sizeof( fdd_fat12 ) );

	strcpy( fdd_fat12.szName, "FDD FAT12" );
	fdd_fat12.nBlkSize	= 512;
	fdd_fat12.nMajor	= FDD_FAT12_MAJOR;
	fdd_fat12.op.open	= fdd_fat12_open ;
	fdd_fat12.op.close	= fdd_fat12_close;
	fdd_fat12.op.read	= fdd_fat12_read ;
	fdd_fat12.op.write	= fdd_fat12_write;
	fdd_fat12.op.ioctl	= fdd_fat12_ioctl;

	// 등록 함수를 부른다.
	nR = register_blkdev( &fdd_fat12 );

	return( nR );
}

int close_fdd_fat12_driver()
{
	int nR;

	nR = unregister_blkdev( FDD_FAT12_MAJOR );

	return( nR );
}

