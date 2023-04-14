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
	
	// ������ �о���δ�.
	pFileName = argv[1];
	//pFileName = "/c/wall.jpg";

	pB = load_file( pFileName, &nFileSize );

	
	if( pB == NULL )
	{
		printf( "load_file( %s ) failed!\n", pFileName );
		return( -3 );
	}
	printf("load_file( %s ) success!\n", argv[1]);
	
	// ������ RGB�� ��ȯ�Ѵ�.
	pRGB = decode_jpeg( pB, nFileSize, &dwH, &dwV, &dwRGBBuffSize );
	if( pRGB == NULL )
	{
		printf( "decode_jpeg: failed!\n" );
		free( pB );
		return( -4 );
	}

	// �̹����� �Ҵ��Ѵ�.
	pVImg = gr_alloc_image( 16, (int)dwH, (int)dwV );
	if( pVImg == NULL )
	{	// �̹����� �Ҵ��� �� ����.
		printf( "gr_alloc_image: failed!\n" );
		return( -5 );
	}
	// �̹����� ���۸����Ѵ�.
	pW = gr_get_image_buff( pVImg );

	// RGB�� 16��Ʈ �������� �����Ѵ�.
	rgb_to_16bit( pW, pRGB, dwH, dwV, NULL );

	printf( "jpeg decoded: H=%d V=%d rgb_size=%d\n", 
		dwH, dwV, dwRGBBuffSize );

	dwWallHandle = gx_find_wall_window();
	printf( "wall window : 0x%X\n", dwWallHandle );

	r.nX = r.nY = 0;
	r.nH = dwH;
	r.nV = dwV;
	// �̹����� ����ȭ�� �����쿡 �����Ѵ�.
	gx_copy_img_to_win( dwWallHandle, pVImg, 0, 0, &r );

	// �̹����� �����Ѵ�.
	free( pVImg );

	// ����ȭ���� �������� �Ѵ�.
	gx_refresh_win( dwWallHandle, NULL );

	return( 0 );
}