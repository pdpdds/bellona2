// stubv86.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

#define V86PARAM_MAGIC	0x1128

typedef unsigned char	BYTE;
typedef unsigned long   DWORD;

static BYTE  *load_file( char *pS, int *pSize )
{
	BYTE *pBuff;
	int nHandle;

	pSize[0] = 0;

	// ������ �����Ѵ�.
	nHandle = open( pS, _O_BINARY | _O_RDONLY );
	if( nHandle < 0 )
	{
		printf( " %s - open error!\n", pS );
		return( NULL );
	}

	// ������ ũ�⸦ �˾Ƴ� �� �޸𸮸� �Ҵ��Ѵ�.
	pSize[0] = lseek( nHandle, 0, SEEK_END );
	lseek( nHandle, 0, SEEK_SET );
	pBuff = (BYTE*)malloc( pSize[0] );
	if( pBuff == NULL )
	{
		close( nHandle );
		printf( "Memory alocation is failed!\n" );
		return( NULL );
	}

	// ������ �о���δ�.
	read( nHandle, pBuff, pSize[0] );
	close( nHandle );
	
	return( pBuff );
}

int main( int argc, char* argv[] )
{
	int		nHandle;
	BYTE	*pV86Lib;
	DWORD	dwV86LibSize;
	DWORD	dbuff[0x13];
	DWORD	dwLoc;
		 
	printf( "Stub Built-in V86 Library for Bellona2 Kernel.\n" );

	// �ķ����͸� �Է����� �ʾҴ�.
	if( argc < 3 )
	{
		printf("stubv86 <Kernel Image File> <V86 Library File>!\n");
		return( 0 );
	}

	// kernel ������ �����Ѵ�.
	nHandle = open( argv[1], _O_BINARY | _O_RDWR );
	if( nHandle < 0 )
	{
		printf( "%s - file open error!\n" );
		return( -1 );
	}

	// v86 ���̺귯���� �о���δ�.
	pV86Lib = load_file( argv[2], (int*)&dwV86LibSize );
	if( pV86Lib == NULL )
	{
		close( nHandle );
		return( -1 );
	}				 

	// �պκ� ����� �о���δ�.
	lseek( nHandle, 0, SEEK_SET );
	read( nHandle, dbuff, sizeof( dbuff ) );

	// �̹� �ɾ��� �ִ°�?
	if( dbuff[0x10] == V86PARAM_MAGIC )
	{
		dwLoc = dbuff[0x11];   // ������ ��ġ�� ���Ѵ�.
		printf( "Already embeded.\n" );
	}
	else
	{
		printf( "Newly embeded.\n" );
		dwLoc = (DWORD)lseek( nHandle, 0, SEEK_END );  // ���� ������ ��´�.
	}

	// ����� ����Ѵ�.
	dbuff[0x10] = V86PARAM_MAGIC;
	dbuff[0x11] = dwLoc;
	dbuff[0x12] = dwV86LibSize;
	lseek( nHandle, 0,SEEK_SET );
	write( nHandle, dbuff, sizeof( dbuff ) );

	// ���� �����͸� �̵��Ѵ�.
	lseek( nHandle, dwLoc, SEEK_SET );
	write( nHandle, pV86Lib, dwV86LibSize );

	// ������ ũ�⸦ �����Ѵ�.
	chsize( nHandle, dwLoc + dwV86LibSize );

	// ������ �ݰ� �޸𸮸� �����Ѵ�.
	close( nHandle );
	free( pV86Lib );

	// V86���̺귯�� ũ��� Embeded�� ��ġ�� ����Ѵ�.
	printf( "V86 Library size = %d\n", dwV86LibSize );
	printf( "Embedding offset = %d\n", dwLoc );
	printf( "ok\n" );
	
	return( 0 );
}

