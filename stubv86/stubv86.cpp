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

	// 파일을 오픈한다.
	nHandle = open( pS, _O_BINARY | _O_RDONLY );
	if( nHandle < 0 )
	{
		printf( " %s - open error!\n", pS );
		return( NULL );
	}

	// 파일의 크기를 알아낸 후 메모리를 할당한다.
	pSize[0] = lseek( nHandle, 0, SEEK_END );
	lseek( nHandle, 0, SEEK_SET );
	pBuff = (BYTE*)malloc( pSize[0] );
	if( pBuff == NULL )
	{
		close( nHandle );
		printf( "Memory alocation is failed!\n" );
		return( NULL );
	}

	// 파일을 읽어들인다.
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

	// 파러메터를 입력하지 않았다.
	if( argc < 3 )
	{
		printf("stubv86 <Kernel Image File> <V86 Library File>!\n");
		return( 0 );
	}

	// kernel 파일을 오픈한다.
	nHandle = open( argv[1], _O_BINARY | _O_RDWR );
	if( nHandle < 0 )
	{
		printf( "%s - file open error!\n" );
		return( -1 );
	}

	// v86 라이브러리를 읽어들인다.
	pV86Lib = load_file( argv[2], (int*)&dwV86LibSize );
	if( pV86Lib == NULL )
	{
		close( nHandle );
		return( -1 );
	}				 

	// 앞부분 헤더를 읽어들인다.
	lseek( nHandle, 0, SEEK_SET );
	read( nHandle, dbuff, sizeof( dbuff ) );

	// 이미 심어져 있는가?
	if( dbuff[0x10] == V86PARAM_MAGIC )
	{
		dwLoc = dbuff[0x11];   // 기존의 위치를 구한다.
		printf( "Already embeded.\n" );
	}
	else
	{
		printf( "Newly embeded.\n" );
		dwLoc = (DWORD)lseek( nHandle, 0, SEEK_END );  // 파일 끝으로 잡는다.
	}

	// 헤더를 기록한다.
	dbuff[0x10] = V86PARAM_MAGIC;
	dbuff[0x11] = dwLoc;
	dbuff[0x12] = dwV86LibSize;
	lseek( nHandle, 0,SEEK_SET );
	write( nHandle, dbuff, sizeof( dbuff ) );

	// 파일 포인터를 이동한다.
	lseek( nHandle, dwLoc, SEEK_SET );
	write( nHandle, pV86Lib, dwV86LibSize );

	// 파일의 크기를 변경한다.
	chsize( nHandle, dwLoc + dwV86LibSize );

	// 파일을 닫고 메모리를 해제한다.
	close( nHandle );
	free( pV86Lib );

	// V86라이브러리 크기와 Embeded된 위치를 출력한다.
	printf( "V86 Library size = %d\n", dwV86LibSize );
	printf( "Embedding offset = %d\n", dwLoc );
	printf( "ok\n" );
	
	return( 0 );
}

