#include <types.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <conio.h>
#include <grx.h>

extern BYTE *decode_jpeg( char *pBuff, int nFileSize, unsigned int *width, unsigned int *height, unsigned long *pBuffSize );
extern unsigned short *rgb_to_16bit( unsigned short *pW, BYTE *pB, int nH, int nV, DWORD *pBuffSize );

int main( int argc, char *argv[] )
{
	RectStt			r;
	unsigned short	*pW;
	void			*pVImg;
	int				nFileSize;
	BYTE			*pB, *pRGB;
	char			*pFileName;
	DWORD			dwH, dwV, dwRGBBuffSize, dwWallHandle;

	printf( "B2OS JPEG Wallpaper Utility v1.0\n" );

	if( is_gui_mode() == 0 )
	{
		printf( "error: system is not in gui mode.\n");
		return( -2 );
	}

	if( argc <= 1 )
	{
		printf( "usage: wall <wall_paper.jpg>\n" );
		return( 0 );
	}
	
	// 파일을 읽어들인다.
	pFileName = argv[1];
	//pFileName = "/c/wall.jpg";

	pB = load_file( pFileName, &nFileSize );

	
	if( pB == NULL )
	{
		printf( "load_file( %s ) failed!\n", pFileName );
		return( -3 );
	}
	printf("load_file( %s ) success!\n", argv[1]);
	
	// 파일을 RGB로 번환한다.
	pRGB = decode_jpeg( pB, nFileSize, &dwH, &dwV, &dwRGBBuffSize );
	if( pRGB == NULL )
	{
		printf( "decode_jpeg: failed!\n" );
		free( pB );
		return( -4 );
	}

	// 이미지를 할당한다.
	pVImg = gr_alloc_image( 16, (int)dwH, (int)dwV );
	if( pVImg == NULL )
	{	// 이미지를 할당할 수 없다.
		printf( "gr_alloc_image: failed!\n" );
		return( -5 );
	}
	// 이미지의 버퍼를구한다.
	pW = gr_get_image_buff( pVImg );

	// RGB를 16비트 색상으로 변경한다.
	rgb_to_16bit( pW, pRGB, dwH, dwV, NULL );

	printf( "jpeg decoded: H=%d V=%d rgb_size=%d\n", 
		dwH, dwV, dwRGBBuffSize );

	dwWallHandle = gx_find_wall_window();
	printf( "wall window : 0x%X\n", dwWallHandle );

	r.nX = r.nY = 0;
	r.nH = dwH;
	r.nV = dwV;
	// 이미지를 바탕화면 윈도우에 복사한다.
	gx_copy_img_to_win( dwWallHandle, pVImg, 0, 0, &r );

	// 이미지를 해제한다.
	free( pVImg );

	// 바탕화면을 리프레시 한다.
	gx_refresh_win( dwWallHandle, NULL );

	return( 0 );
}