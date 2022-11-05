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

// pFileName�� ������ �̹����� pImg->pBuff�� �о���̰� ������ ũ��� 
// ������ ������ pImg�� �����Ѵ�.
static int nLoadImage( ImgStt *pImg, char *pFileName )
{
	int nHandle;

	// ������ ����.
	nHandle = open( pFileName, _O_BINARY | _O_RDONLY );
	if( nHandle == -1 )
	{
		printf( "file open error : %s\n", pFileName );
		return(-1);
	}

	// ������ ũ�⸦ ���Ѵ�.
	pImg->lSize = lseek( nHandle, 0, SEEK_END );
	lseek( nHandle, 0, SEEK_SET );  // ���� �����͸� ó������ 

	// Size�� 512�� ����� �Ѵ�.
	pImg->lSize = (long)( ( pImg->lSize + 511 ) / 512 );
	pImg->lSize = pImg->lSize * 512;
						 
	// �޸𸮸� �Ҵ��Ѵ�.
	pImg->pBuff = malloc( pImg->lSize );
	if( pImg->pBuff == NULL )
	{
		printf( "memory allocation failed!\n" );
		close( nHandle );
		return( -1 );			// �޸� �Ҵ� ����
	}

	// ������ �о���δ�.
	read( nHandle, pImg->pBuff, pImg->lSize );

	// ������ �ݴ´�.
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
	
	// ��Ʈ ���Ϳ��� �о���� ���ͼ� ���.
	wA = pI->bload.wSect + pI->bell.wSect;  // bload.com�� bellona2.bin�� �о�� �Ѵ�.

	// �о���� ���ͼ� ���
	pW = (UINT16*)&pI->boot.pBuff[3];  // ���� �ڵ� 3����Ʈ ��������
	pW[0] = wA;		

	//  ��Ʈ���Ϳ��� BLOAD�� �Ѿ�� �ּ� �����  (0x90�� 8�����޾� ������ �ּҸ� ã�´�.)
	for( nI = 0; nI < (int)pI->bload.lSize; nI++ )
	{
		if( memcmp( &pI->bload.pBuff[nI],  nops, sizeof( nops ) ) == 0 )
			break;
	}

	if( nI == (int)pI->bload.lSize )
		return(-1);  // NOP�� ã�� �� ����.

	// ��Ʈ ���Ϳ��� BLOAD.COM�� vBootEntry�ּҸ� ����Ѵ�.
	wA = (UINT16)0x7D0 + (UINT16)( pI->bell.wSect * 32 );
	wB = (UINT16)0x100 + (UINT16)nI;
	pW[1] = wB;	// Offset
	pW[2] = wA;	// Segment

	// nRBlock(Bellona.Bin�� ���(65536-1024) ����), pBuff[10], wRead�� ��ġ�� ã�´�.
	for( nI = 0; nI < (int)pI->bload.lSize; nI++ )
	{	// Heart 10���� ã�´�.
		if( memcmp( &pI->bload.pBuff[nI],  heart, sizeof( heart ) ) == 0 )
			break;
	}

	if( nI == (int)pI->bload.lSize )
		return(-1);  // HEART�� ã�� �� ����.

	pW = (UINT16*)&pI->bload.pBuff[ nI+ sizeof(heart) ];
	// BELLONA.BIN�� �ε�� �޸� �ּ�
	wA = (UINT16)( (pI->bell.lSize + (65536-1024-1)) / (65536-1024) );  // ��� ����
	wB = (UINT16)( 0x7E0 );			// ���� ��� ���׸�Ʈ �ּ�
	pW[0] = wA;	 // ��� ����
	for( wI = 0; wI < wA; wI++ )
	{
		pW[ wI*2 + 1 ] = 0;		// Offset
		pW[ wI*2 + 2 ] = wB;	// Segment
		wB += (65536-1024) / 16;
	}
	pW[21] = (UINT16)(pI->bell.lSize % (65536-1024));  // ������ ��� ũ��

	// �ϴ� ������ �����.
	remove( pFileName );  
	nHandle = open( pFileName, _O_BINARY | _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE );
	if( nHandle == -1 )
		return(-1);	  // ������ ������ �� ����.

	// ���������� ���Ͽ� ����Ѵ�.
	write( nHandle, pI->boot.pBuff,  pI->boot.lSize );
	write( nHandle, pI->bell.pBuff,  pI->bell.lSize );
	write( nHandle, pI->bload.pBuff, pI->bload.lSize );

	// ������ �ݴ´�.
	close( nHandle );

	return(0);
}	

int main( int argc, char *argv[] )
{
	// �ΰ� ����Ѵ�.
	printf( "Bellona2 Kernel Boot Image Maker (c) Copyright 1999 OHJJ.\n" );

	// Img ����ü�� 0���� �ʱ�ȭ 
	memset( &img, 0, sizeof( img ) );

	// Boot Sector�� �ε��Ѵ�.
	if( nLoadImage( &img.boot, "BOOT.BIN" ) != 0 ) 	goto ERROR;
	
	// BLOAD.COM�� �ε��Ѵ�.
	if( nLoadImage( &img.bload, "BLOAD.COM" ) != 0 ) 	goto ERROR;
	
	// BELLONA.BIN�� �ε��Ѵ�

	if( nLoadImage( &img.bell, "BELLONA2.BIN" ) != 0 ) 	goto ERROR;

	// BELLDISK.BIN�� ����Ѵ�.
	printf( "\nboot.bin + bload.com + bellona2.bin -> Bellona2.img\n" );
	if( nWriteImage( &img, "BELLONA2.IMG" ) != 0 ) goto ERROR;
		   
	// �޸𸮸� �����Ѵ�.
	free( img.boot.pBuff );
	free( img.bload.pBuff );
	free( img.bell.pBuff );

	printf( "ok.\n" );
	return( 0 );
						 
ERROR:
	printf( "error.\n" );
	return( -1 );
}