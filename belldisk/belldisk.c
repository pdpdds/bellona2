#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

typedef unsigned short UINT16;
typedef unsigned char  UCHAR;
typedef unsigned long  DWORD;
typedef struct {
	UCHAR	*pBuff;
	long    lSize;
	UINT16	wSect;
}  ImgStt;

typedef struct {
	ImgStt boot;
	ImgStt bload;
	ImgStt bell;
} ImageStt;

ImageStt img;

// pFileName에 지정된 이미지를 pImg->pBuff에 읽어들이고 파일의 크기와 
// 섹터의 개수를 pImg에 리턴한다.
static int nLoadImage( ImgStt *pImg, char *pFileName )
{
	int nHandle;

	// 파일을 연다.
	nHandle = open( pFileName, _O_BINARY | _O_RDONLY );
	if( nHandle == -1 )
	{
		printf( "file open error : %s\n", pFileName );
		return(-1);
	}

	// 파일의 크기를 구한다.
	pImg->lSize = lseek( nHandle, 0, SEEK_END );
	lseek( nHandle, 0, SEEK_SET );  // 파일 포인터를 처음으로 

	// Size를 512의 배수로 한다.
	pImg->lSize = (long)( ( pImg->lSize + 511 ) / 512 );
	pImg->lSize = pImg->lSize * 512;
						 
	// 메모리를 할당한다.
	pImg->pBuff = malloc( pImg->lSize );
	if( pImg->pBuff == NULL )
	{
		printf( "memory allocation failed!\n" );
		close( nHandle );
		return( -1 );			// 메모리 할당 실패
	}

	// 파일을 읽어들인다.
	read( nHandle, pImg->pBuff, pImg->lSize );

	// 파일을 닫는다.
	close( nHandle );

	pImg->wSect = (UINT16)( (pImg->lSize + 511 ) / 512 );

	printf( "%-12s : %7d bytes / %7d sectors.\n", pFileName, pImg->lSize, pImg->wSect );

	return(0);
}

static UCHAR   nops[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
static UCHAR   heart[] = { 3,3,3,3,3, 3,3,3,3,3 };
static int nWriteImage( ImageStt *pI, char *pFileName )
{
	int		nHandle, nI;
	UINT16	wA, wB, *pW, wI;
	
	// 부트 섹터에서 읽어들일 섹터수 계산.
	wA = pI->bload.wSect + pI->bell.wSect;  // bload.com과 bellona2.bin을 읽어야 한다.

	// 읽어들일 섹터수 기록
	pW = (UINT16*)&pI->boot.pBuff[3];  // 점프 코드 3바이트 다음부터
	pW[0] = wA;		

	//  부트섹터에서 BLOAD로 넘어가는 주소 만들기  (0x90이 8개연달아 나오는 주소를 찾는다.)
	for( nI = 0; nI < (int)pI->bload.lSize; nI++ )
	{
		if( memcmp( &pI->bload.pBuff[nI],  nops, sizeof( nops ) ) == 0 )
			break;
	}

	if( nI == (int)pI->bload.lSize )
		return(-1);  // NOP를 찾을 수 없다.

	// 부트 섹터에서 BLOAD.COM의 vBootEntry주소를 계산한다.
	wA = (UINT16)0x7D0 + (UINT16)( pI->bell.wSect * 32 );
	wB = (UINT16)0x100 + (UINT16)nI;
	pW[1] = wB;	// Offset
	pW[2] = wA;	// Segment

	// nRBlock(Bellona.Bin의 블록(65536-1024) 개수), pBuff[10], wRead의 위치를 찾는다.
	for( nI = 0; nI < (int)pI->bload.lSize; nI++ )
	{	// Heart 10개를 찾는다.
		if( memcmp( &pI->bload.pBuff[nI],  heart, sizeof( heart ) ) == 0 )
			break;
	}

	if( nI == (int)pI->bload.lSize )
		return(-1);  // HEART를 찾을 수 없다.

	pW = (UINT16*)&pI->bload.pBuff[ nI+ sizeof(heart) ];
	// BELLONA.BIN이 로드될 메모리 주소
	wA = (UINT16)( (pI->bell.lSize + (65536-1024-1)) / (65536-1024) );  // 블록 개수
	wB = (UINT16)( 0x7E0 );			// 최초 블록 세그먼트 주소
	pW[0] = wA;	 // 블록 개수
	for( wI = 0; wI < wA; wI++ )
	{
		pW[ wI*2 + 1 ] = 0;		// Offset
		pW[ wI*2 + 2 ] = wB;	// Segment
		wB += (65536-1024) / 16;
	}
	pW[21] = (UINT16)(pI->bell.lSize % (65536-1024));  // 마지막 블록 크기

	// 일단 파일을 지운다.
	remove( pFileName );  
	nHandle = open( pFileName, _O_BINARY | _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE );
	if( nHandle == -1 )
		return(-1);	  // 파일을 생성할 수 없다.

	// 순차적으로 파일에 기록한다.
	write( nHandle, pI->boot.pBuff,  pI->boot.lSize );
	write( nHandle, pI->bell.pBuff,  pI->bell.lSize );
	write( nHandle, pI->bload.pBuff, pI->bload.lSize );

	// 파일을 닫는다.
	close( nHandle );

	return(0);
}	

int main( int argc, char *argv[] )
{
	// 로고를 출력한다.
	printf( "Bellona2 Kernel Boot Image Maker (c) Copyright 1999 OHJJ.\n" );

	// Img 구조체를 0으로 초기화 
	memset( &img, 0, sizeof( img ) );

	// Boot Sector를 로드한다.
	if( nLoadImage( &img.boot, "BOOT.BIN" ) != 0 ) 	goto ERROR;
	
	// BLOAD.COM을 로드한다.
	if( nLoadImage( &img.bload, "BLOAD.COM" ) != 0 ) 	goto ERROR;
	
	// BELLONA.BIN을 로드한다

	if( nLoadImage( &img.bell, "BELLONA2.BIN" ) != 0 ) 	goto ERROR;

	// BELLDISK.BIN에 기록한다.
	printf( "\nboot.bin + bload.com + bellona2.bin -> Bellona2.img\n" );
	if( nWriteImage( &img, "BELLONA2.IMG" ) != 0 ) goto ERROR;
		   
	// 메모리를 해제한다.
	free( img.boot.pBuff );
	free( img.bload.pBuff );
	free( img.bell.pBuff );

	printf( "ok.\n" );
	return( 0 );
						 
ERROR:
	printf( "error.\n" );
	return( -1 );
}