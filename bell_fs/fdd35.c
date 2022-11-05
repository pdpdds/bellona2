#include "vfs.h"

#ifdef BELLONA2
	extern int fdd_read_track( int nDrv, int nTrack, int nSide, char *pBuff );
	extern int fdd_write_track( int nDrv, int nTrack, int nSide, char *pBuff );
#endif

// 버퍼의 데이터를 비운다.
static int fdd35_discard_buffer( FDD35Stt *pFDD )
{
	int nI, nJ;

	if( pFDD == NULL )
		return( -1 );

	for( nJ = 0; nJ < FDD35_MAX_SIDE; nJ++ )
	{
		for( nI = 0; nI < FDD35_MAX_TRACK; nI++ )
		{
			if( pFDD->track[nJ][nI].pBuff != NULL )
			{
				FREE( pFDD->track[nJ][nI].pBuff );
				pFDD->track[nJ][nI].pBuff = NULL;
				pFDD->nTotalBuffered--;
			}						
		}
	}	

	return( 0 );
}

// Dirty Track을 기록한다.
static int fdd35_sync_buffer( FDD35Stt *pFDD )
{
	int nI, nJ, nR, nTotalSync;

	if( pFDD == NULL )
		return( -1 );

	nTotalSync = 0;
	for( nJ = 0; nJ < FDD35_MAX_SIDE; nJ++ )
	{
		for( nI = 0; nI < FDD35_MAX_TRACK; nI++ )
		{
			if( pFDD->track[nJ][nI].nFlag == FDD35_TRACK_DIRTY )
			{
				nR = fdd_write_track( 0, nI, nJ, pFDD->track[nJ][nI].pBuff );
				if( nR == 0 )
				{
					pFDD->track[nJ][nI].nFlag = 0;	// clear dirty buffer
					nTotalSync++;
				}
			}						
		}
	}

	JPRINTF( "Total %d tracks were synchronized.\n", nTotalSync );

	return( 0 );
}

// Device Dependent I/O Control
extern int fdd_reset();
static int fdd35_ioctl( void *pVoidObj, void *pVoidIoctl )
{
	int				nR;
	BlkDevObjStt	*pObj;
	FDD35Stt		*pFDD;
	BlkDevIoctlStt	*pIoctl;

	pIoctl = pVoidIoctl;
	pObj   = pVoidObj;
	pFDD   = pObj->pPtr;

	nR = -1;
	switch( pIoctl->nFunc )
	{
    case BLKDEV_IOCTL_RESET :
        fdd_reset();
        break;

	case BLKDEV_IOCTL_FLUSH :
		fdd35_sync_buffer( pFDD );
		break;

	case BLKDEV_IOCTL_INVALIDATE :		// 버퍼링된 모든 데이터를 버린다.
		fdd35_discard_buffer( pFDD );
		break;		  
	}	

	return( nR );
}

static int fdd35_close( void *pVoidObj )
{
	FDD35Stt		*pFDD;
	BlkDevObjStt	*pObj;

	pObj = (BlkDevObjStt*)pVoidObj;
	pFDD = (FDD35Stt*)pObj->pPtr;

	// only to release memory
	if( pFDD )
		FREE( pFDD );
	
	return( 0 );
}

// get track buffer address
static FDDTrackStt *fdd35_get_track_buffer( FDD35Stt *pFDD, int nDrv, int nTrack, int nSide )
{
	int				nR;
	FDDTrackStt		*pT;

	pT = &pFDD->track[nSide][nTrack];
		
	if( pT->pBuff == NULL )
	{	// allocate track buffer
		pT->pBuff = (char*)MALLOC( 512 * 18 );
		if( pT->pBuff == NULL )
			return( NULL );			// memory allocation failed!

		// read track
		#ifdef WIN32TEST
			nR = nW32FDD_ReadTrack( nDrv, nTrack, nSide, pT->pBuff );
		#endif
		#ifdef BELLONA2
			nR = fdd_read_track( nDrv, nTrack, nSide, pT->pBuff );
		#endif

		if( nR < 0 )
		{
			FREE( pT->pBuff );
			pT->pBuff = NULL;
			return( NULL );			// read track filed!
		} 

		pT->nFlag = FDD35_TRACK_ACCESS;
		pFDD->nTotalBuffered++;
	}

	return( pT );		// 트랙 구조체를 리턴한다.
}

// FDD에서 한 섹터를 읽어들인다.
static int _fdd35_read_one_sector( void *pVoidObj, void *pVoidIO )
{
	BlkDevIOStt		*pIO;
	BlkDevObjStt	*pObj;
	FDD35Stt		*pFDD;
	FDDTrackStt		*pTrack;
	char			*pTrackBuff;
	int				nTrackOffset;
	int				nSide, nTrack;

	pIO				= pVoidIO;
	pObj			= pVoidObj;
	pFDD			= pObj->pPtr;

	// get track and side number
	nTrack = ( pIO->dwIndex / 18 ) / 2;
	nSide  = ( pIO->dwIndex / 18 ) % 2;

	// get track buffer address
	pTrack = fdd35_get_track_buffer( pFDD, pObj->nVolume, nTrack, nSide );
	if( pTrack == NULL )
		return( -1 );

	pTrackBuff = pTrack->pBuff;

	// get offset in track buffer
	nTrackOffset = nTrack * 18 * 2 + ( nSide * 18 );
	nTrackOffset = (int)pIO->dwIndex - nTrackOffset;
	nTrackOffset = nTrackOffset * 512;

	memcpy( pIO->pBuff, &pTrackBuff[nTrackOffset], 512 );

	return( 0 );
}

// read one sector from fdd
static int fdd35_read( void *pVoidObj, void *pVoidIO )
{
	int				nI, nR;
	BlkDevIOStt		*pIO, io;

	pIO = pVoidIO;
	for( nI = 0; nI < pIO->nBlocks; nI++ )
	{
		io.nBlocks = 1;
		io.pBuff   = &pIO->pBuff[nI*512];
		io.dwIndex = pIO->dwIndex + (DWORD)nI;

		nR = _fdd35_read_one_sector( pVoidObj, &io );
		if( nR <0 )
			return( -1 );
	}					 

	return( 0 );
}	

// write one sector to fdd
static int fdd35_write( void *pVoidObj, void *pVoidIO )
{
	BlkDevIOStt		*pBio;
	FDD35Stt		*pFDD;
	BlkDevObjStt	*pDevObj;
	int				nTrack, nSide;

	pBio	= (BlkDevIOStt*)pVoidIO;
	pDevObj	= (BlkDevObjStt*)pVoidObj;

	// find track number
	nTrack = ( pBio->dwIndex / 18 ) / 2;
	nSide  = ( pBio->dwIndex / 18 ) % 2;

	// set track dirty
	pFDD = (FDD35Stt*)pDevObj->pPtr;
	pFDD->track[nSide][nTrack].nFlag = FDD35_TRACK_DIRTY;
	
	return(-1 );
}	

// 결과값이 pVoidObj로 리턴된다.
static int fdd35_open( int nMinor, void *pVoidObj, void *pDev )
{
	FDD35Stt		*pFDD;
	BlkDevObjStt	*pObj;

	pObj = (BlkDevObjStt*)pVoidObj;
					
	pFDD = (FDD35Stt*)MALLOC( sizeof( FDD35Stt ) );
	if( pFDD == NULL )
		return( -1 );	// 메모리를 할당할 수 없다.
	
	memset( pFDD, 0, sizeof( FDD35Stt ) );
	pObj->pPtr			= pFDD;
	pObj->pDev			= pDev;
	pObj->nMinor		= nMinor;					// minor == 0 일 때 A
	pObj->nVolume		= nMinor;
	pObj->nAttr			= BLKDEV_ATTR_REMOVABLE;	// fdd is removable
	
	return( 0 );
}

// FDD35 드라이버를 초기화한다.
int init_fdd35_driver()
{
	int			nR;
	BlkDevStt	fdd35;

	memset( &fdd35, 0, sizeof( fdd35 ) );

	strcpy( fdd35.szName, "FDD35" );
	fdd35.nBlkSize	    = 512;
	fdd35.nMajor	    = FDD35_MAJOR;
	fdd35.op.open	    = fdd35_open ;
	fdd35.op.close	    = fdd35_close;
	fdd35.op.read	    = fdd35_read ;
	fdd35.op.write	    = fdd35_write;
	fdd35.op.ioctl	    = fdd35_ioctl;

	// 등록 함수를 부른다.
	nR = register_blkdev( &fdd35 );

	return( nR );
}

int close_fdd35_driver()
{
	int nR;

	nR = unregister_blkdev( FDD35_MAJOR );

	return( nR );
}

