#include <bellona2.h>
#include "gui.h"

UINT16 *get_img_buff_addr16( ImageStt *pImg, int nX, int nY )
{
	UINT16 *pW;

	pW = &pImg->b.pW[ nX + (nY * pImg->nH) ];
	
	return( pW );
}

#define DARK_CONST	6
void darken_rgbquad( RGBQUAD *pR )
{
	if( pR->rgbBlue  > DARK_CONST )	pR->rgbBlue  -= DARK_CONST;
	if( pR->rgbGreen > DARK_CONST )	pR->rgbGreen -= DARK_CONST;
	if( pR->rgbRed   > DARK_CONST )	pR->rgbRed   -= DARK_CONST;
}

void brighten_rgbquad( RGBQUAD *pR )
{
	if( pR->rgbBlue  < 255-DARK_CONST ) pR->rgbBlue  += DARK_CONST;
	if( pR->rgbGreen < 255-DARK_CONST ) pR->rgbGreen += DARK_CONST;
	if( pR->rgbRed	 < 255-DARK_CONST ) pR->rgbRed	 += DARK_CONST;
}

void rgb16_to_rgbquad( UINT16 wColor, RGBQUAD *pRQ )
{
	BYTE	r, g, b;

	b = (BYTE)( (UINT16)wColor & (UINT16)31 );
	g = (BYTE)( (UINT16)(wColor >> 5 ) & (UINT16)61 );
	r = (BYTE)( (UINT16)(wColor >> 11) & (UINT16)31 );

	pRQ->rgbBlue  = (BYTE)( b << 3 );
	pRQ->rgbGreen = (BYTE)( g << 2 );
	pRQ->rgbRed   = (BYTE)( r << 3 );
}

UINT16 rgbquad_to_rgb16( RGBQUAD *pRQ )
{
	BYTE r, g, b;

	// RGB16에서 shift를 하기 때문에 여기서는 하지 않아도 된다.
	r = (BYTE)( (BYTE)pRQ->rgbRed   );	//>> 3 );
	g = (BYTE)( (BYTE)pRQ->rgbGreen );	//>> 2 );
	b = (BYTE)( (BYTE)pRQ->rgbBlue  );	//>> 3 );

	return( RGB16( r, g, b ) );
}

// kfree()로 해제하면 된다.
void *alloc_blank_image16( int nH, int nV )
{
	ImageStt			*pImg;
	int					nSize, nColorSize;

	// 사이즈를 계산한다. 
	nColorSize = (int)( nH * nV * 2 );
	nSize = nColorSize + (int)sizeof( ImageStt );

	// 헤더와 버퍼를 할당한다. 
	pImg = (ImageStt*)kmalloc( nSize );
	if( pImg == NULL )
		return( NULL );
	memset( pImg, 0, nSize );
	pImg->b.pW  	 = (UINT16*)( (DWORD)pImg + (DWORD)sizeof( ImageStt ) );
	pImg->nBPP  	 = 16;
	pImg->nH    	 = nH;
	pImg->nV    	 = nV;
	pImg->nSize 	 = nSize;
	pImg->nColorSize = nColorSize;

	return( pImg );
}

static void *bitmap_to_image16_ex( BITMAPINFO *pBitmap, int nH, int nV, int nUserFlag )
{
	unsigned short int	*pW;
	BYTE				*pB;
	ImageStt			*pImg;
	int					nDeltaH, nMaskSize, nColorSize, nSize, nColors, nX, nY;

	// 16비트 버퍼 사이즈를 계산한다. 
	nColorSize = nH * nV * 2;
	nMaskSize = 0;
	nSize = nColorSize + (int)sizeof( ImageStt );

	// 헤더와 버퍼를 할당한다. 
	if( nUserFlag == 0 )
		pImg = (ImageStt*)kmalloc( nSize );				// 커널 영역에서 메모리 할당.
	else
		pImg = (ImageStt*)umalloc( NULL, NULL, nSize );	// 현재 프로세스의 사용자 영역에서 메모리 할당.
	if( pImg == NULL )
		return( NULL );
		
	memset( pImg, 0, nSize );
	pW = pImg->b.pW  = (UINT16*)( (DWORD)pImg + (DWORD)sizeof( ImageStt ) );
	pImg->nBPP  = 16;
	pImg->nH    = nH;
	pImg->nV    = nV;
	pImg->nSize = nSize;

	if( pBitmap->bmiHeader.biBitCount == 24 )
	{	// 24 비트 비트맵은 컬러 테이블이 없이 바로 RGB 3바이트 색상 데이터가 온다.
		nDeltaH = ( ( pBitmap->bmiHeader.biWidth * 3 ) + 3 ) / 4;
		nDeltaH *= 4;

		pB = (BYTE*)pBitmap->bmiColors;
		// 가장 아래쪽 라인부터 처리한다.
		pB += nV * nDeltaH;
		for( nY = 0; nY < nV; nY++ )
		{
			pB -= nDeltaH;
			for( nX = 0; nX < nH; nX++ )
				pW[ nX ] = rgbquad_to_rgb16( (RGBQUAD*)&pB[ nX*3 ] );
			pW += nH;
		}
	}
	else
	{
		// 컬러 테이블의 개수를 구한다.
		nColors = 1;
		nColors = (int)( nColors << (int)pBitmap->bmiHeader.biBitCount );

		// 컬러 인덱스의 시작
		pB = (BYTE*)( (DWORD)&pBitmap->bmiColors[nColors] );
		
		if( nColors == 256 )
			nDeltaH = ( (nH+3) /4) * 4;
		else if( nColors == 16 )
			nDeltaH = ( (((nH+1)/2)+3) /4) * 4;
		else
		{	// 지원되지 않는 색상인 경우 에러 처리한다.
			kdbg_printf( "Unsupported color(%d) icon!", nColors );

			if( nUserFlag == 0 )
				kfree( pImg );	
			else
				ufree( NULL, pImg );

			return( NULL );
		}

		// 마스크는 할당하지 않고 리소스 로드한 것을 직접 사용한다?? (이렇게 하면 안된다.)
		pImg->pMask = (BYTE*)( (DWORD)pB + (DWORD)( nDeltaH * nV ) );
		nMaskSize = (((( nH + 31 ) / 32 ) * 32) * nV );

		pB += nV * nDeltaH;
		for( nY = 0; nY < nV; nY++ )
		{
			pB -= nDeltaH;
			
			if( nColors == 256 )
			{	// 256 컬러 아이콘
				for( nX = 0; nX < nH; nX++ )
					pW[ nX ] = rgbquad_to_rgb16( &pBitmap->bmiColors[ (DWORD)pB[ nX ] ] );
			}
			else if( nColors == 16 )
			{	// 16컬러 아이콘
				DWORD dwX;
				
				for( nX = 0; nX < nH; nX++ )
				{
					dwX = (DWORD)pB[ nX/2 ];
					if( nX & 1 )
						dwX = dwX & (DWORD)0x0F;
					else
					{
						dwX = (DWORD)( dwX >> 4 );
						dwX = dwX & (DWORD)0x0F;
					}
					pW[ nX ] = rgbquad_to_rgb16( &pBitmap->bmiColors[ dwX ] );
				}
			}
			
			pW += nH;
		}
	}

	pImg->nColorSize = nColorSize;
	pImg->nMaskSize  = nMaskSize;

	return( pImg );
}

// 비트맵을 16비트 컬러 이미지로 변환한다. 
void *bitmap_to_image16( BITMAPINFO *pBitmap, int nH, int nV )
{
	void *pV;
	pV = bitmap_to_image16_ex( pBitmap, nH, nV, 0 );
	return( pV );
}

// 아이콘의 마스크 비트를 구한다.
static int icon_mask_value( BYTE *pMask, int nIndex )
{
	BYTE b;
	int  nMod, nDiv;

	nMod = nIndex % 8;
	nDiv = nIndex / 8;

	b = 0x80;
	b = (BYTE)( b >> nMod );

	if( b & pMask[nDiv] )
		return( 1 );
	return( 0 );
}

// 윈도우의 GraBuff에 이미지를 복사한다.
int copy_image16( struct GraBuffTag *pGB, void *pVImg, int nX, int nY, int nXLimit, int nYLimit, int nSrcX, int nSrcY )
{
	ImageStt	*pImg;
	BYTE		*pMask;
	UINT16		*pW, *pIW;
	int			nH, nV, nMaskDelta;	

	if( pVImg == NULL || nX < 0 || nY < 0 )
	{
		kdbg_printf( "copy_image16: invalid image(0x%X) or pos(%d,%d)\n", (DWORD)pVImg, nX, nY );
		return( -1 );
	}
	
	// H, V가 음수이면 이미지 전체를 복사한다.	H, V가 Dest를 넘어서면 자른다.
	pImg  = (ImageStt*)pVImg;
	if( nXLimit < 0 )
		nXLimit = pImg->nH;
	else if( nXLimit > pGB->pR->nH - nX )
		nXLimit = pGB->pR->nH - nX;
	if( nYLimit < 0 )
		nYLimit = pImg->nV;
	else if( nYLimit > pGB->pR->nV - nY )
		nYLimit = pGB->pR->nV - nY;

	// H, V가 Src를 넘어서면 자른다.
	if( nXLimit > pImg->nH )
		nXLimit = pImg->nH;
	if( nYLimit > pImg->nV )
		nYLimit = pImg->nV;

	pIW   = get_img_buff_addr16( pImg, nSrcX, nSrcY );			// 그리고자 하는 이미지의 픽셀 데이터.
	pMask = pImg->pMask;		// 그리고자 하는 이미지의 마스크 데이터.

	// grabuff의 주소를 구한다.
	pW = get_gra_buff_addr16( pGB, nX, nY );
	nMaskDelta = ( ( (pImg->nH + 31 ) / 32 ) * 32 );  	
	for( nV = 0; nV < nYLimit; nV++ )
	{
		for( nH = 0; nH < nXLimit; nH++ )
		{
			// 픽셀을 찍는다. 
			if( pMask != NULL )
			{   // 마스크가 지정되어 있으면 그리지 않을 수도 있다.
				if( icon_mask_value( pMask, nH + (pImg->nV - nV -1)*nMaskDelta ) == 0 )
				{
					pW[nH] = pIW[nH];

					// 최적화 시킬 필요가 있다.  (일단은 작동하는지 부터 보자.)
					if( pGB->self_mask.pB != NULL )
						set_mask_bit( &pGB->self_mask, nX + nH, nY + nV, 1 );
				}
			}
			else			
			{	// 마스크가 없으므로 그냥 그린다.
				pW[nH] = pIW[nH];

				// 최적화 시킬 필요가 있다.  (일단은 작동하는지 부터 보자.)
				if( pGB->self_mask.pB != NULL )
					set_mask_bit( &pGB->self_mask, nX + nH, nY + nV, 1 );
			}
		}
		
		pW  = &pW[ pGB->pR->nH ];
		pIW = &pIW[ pImg->nH ];
	}

	return( 0 );
}

// pVImg : 그릴 비트 이미지.
// pVOrgImg : 그리게 될 영역을 보관할 이미지 버퍼.
int draw_image16( void *pVImg, void *pVOrgImg, int nX, int nY, int nSrcX, int nSrcY, int nSrcH, int nSrcV )
{
	GuiStt		*pGui;
	BYTE		*pMask;
	UINT16		*pW, *pIW, *pOW;
	ImageStt	*pImg, *pOrgImg;
	int			nT, nI, nH, nV, nMaskDelta;	

	pImg  = (ImageStt*)pVImg;
	pIW   = pImg->b.pW;			// 그리고자 하는 이미지의 픽셀 데이터.
	pMask = pImg->pMask;		// 그리고자 하는 이미지의 마스크 데이터.

	if( pVOrgImg != NULL )
	{	// 백업용.
		pOrgImg = (ImageStt*)pVOrgImg;
		pOW     = pOrgImg->b.pW;
	}
	else
		pOW = NULL;
	
	pGui = get_gui_stt();
	pW   = get_video_mem_addr16( nX, nY );
	nMaskDelta = ( ( (pImg->nH + 31 ) / 32 ) * 32 );
	
	for( nV = 0; nV < nSrcY + nSrcV; nV++ )
	{
		if( nV < nSrcY)
			goto SKIP_V;
		
		for( nT = nI = nH = 0; nH < nSrcX + nSrcH; nH++ )
		{
			if( nH < nSrcX )
				goto SKIP_H;
			
			// 원래 픽셀을 보관한다. 
			if( pOW != NULL )
				pOW[nI] = pW[nT];

			// 픽셀을 찍는다. 
			if( pMask != NULL )
			{   // 마스크가 지정되어 있으면 그리지 않을 수도 있다.
				if( icon_mask_value( pMask, nH + (pImg->nV - nV -1)*nMaskDelta ) == 0 )
					pW[nT] = pIW[nI];
			}
			else			
			{	// 마스크가 없으므로 그냥 그린다.
				pW[nT] = pIW[nI];
			}
			nT++;
SKIP_H:
			nI++;
		}
		pW	= (UINT16*)( (DWORD)pW + (DWORD)pGui->vmode.LinBytesPerScanLine );
SKIP_V:		
		pIW = (UINT16*)( (DWORD)pIW + (DWORD)( 2 * pImg->nH ) );
		if( pOW != NULL )
			pOW = (UINT16*)( (DWORD)pOW + (DWORD)( 2 * pOrgImg->nH ) );
	}

	return( 0 );
}

int draw_image16_ex( void *pVImg, void *pVOrgImg, int nX, int nY, struct RectTag *pR, struct RectTag *pNotRect )
{
	GuiStt			*pGui;
	BYTE			*pMask;
	UINT16			*pW;			// 스크린의 버퍼.
	UINT16			*pIW;			// 이미지의 버퍼.
	UINT16			*pOW;			// 원본 이미지를 복사할 곳의 주소.
	ImageStt		*pImg, *pOrgImg;
	int				nI, nH, nV, nMaskDelta, nXPos, nYPos;	

	pImg  = (ImageStt*)pVImg;
	pIW   = pImg->b.pW;
	pMask = pImg->pMask;

	if( pVOrgImg != NULL )
	{
		pOrgImg = (ImageStt*)pVOrgImg;
		pOW     = pOrgImg->b.pW;
	}
	else
		pOW = NULL;
	
	pGui = get_gui_stt();
	pW   = (UINT16*)( pGui->dwMemAddr + (DWORD)(nX * 2) + (DWORD)(nY * pGui->vmode.LinBytesPerScanLine) );
	nMaskDelta = (((pImg->nH + 31 ) / 32) * 32);
	nYPos = nY;
	for( nV = 0; nV < pImg->nV; nV++ )
	{
		nXPos = nX;
		for( nI = nH = 0; nH < pImg->nH; nH++, nI++ )
		{
			// 원래 픽셀을 보관한다. 
			if( pOW != NULL )
				pOW[nI] = pW[nI];

			// 픽셀을 찍는다. 
			if( pMask != NULL )
			{
				if( icon_mask_value( pMask, nH + (pImg->nV - nV -1)*nMaskDelta ) == 0 )
					pW[nI] = pIW[nI];
			}
			else
			{	// 영역 안에 있을 경우에만 픽셀을 그린다. 
				if( is_in_rect( pR, nXPos, nYPos ) && !(is_in_rect( pNotRect, nXPos, nYPos )) )
					pW[nI] = pIW[nI];
			}

			nXPos++;
		}
		pW  = (UINT16*)( (DWORD)pW + (DWORD)pGui->vmode.LinBytesPerScanLine );
		pIW = (UINT16*)( (DWORD)pIW + (DWORD)( 2 * pImg->nH ) );
		if( pOW != NULL )
			pOW = (UINT16*)( (DWORD)pOW + (DWORD)( 2 * pOrgImg->nH ) );

		nYPos++;
	}

	return( 0 );
}

// 리소스에서 비트맵을 로드한다. 
void *load_bitmap_image16( void *pV, int nID )
{
	ImageStt		*pImg;
	WinResEntStt	*pWinRes;
	BITMAPINFO		*pBitmap;

	pWinRes = load_winres( (WinResStt*)pV, (UINT16)nID );
	if( pWinRes == NULL )
	{	// 리소스를 찾을 수 없다. 
		kdbg_printf( "Res %d loading failed!\n", nID );
		return( NULL );
	}

	// 비트맵을 이미지로 변경한다. 
	pBitmap = (BITMAPINFO*)pWinRes->dwAddr;
	pImg = bitmap_to_image16( pBitmap, pBitmap->bmiHeader.biWidth, pBitmap->bmiHeader.biHeight );
	pImg->pMask = NULL;	// 이걸 꼭 NULL로 주어야 한다. 
	
	return( pImg );
}

// 비트맵 파일을 로드한다. 
void *load_bitmap_file_to_image16_ex( char *pFileName, int nUserFlag )
{
	int				nHandle, nR;
	BYTE			*pBuff;
	long			lSize;
	ImageStt		*pImg;
	BITMAPINFO		*pBitmap;

	// 파일을 오픈하여 크기를 구한다.
	nHandle = kopen( pFileName, FM_READ );
	if( nHandle == -1 )
	{
		kdbg_printf( "load_bitmap_file_to_image16: %s open error!\n", pFileName );
		return( 0 );
	}
	// 파일 크기를 알아낸다.
	lSize = klseek( nHandle, 0, FSEEK_END );
	if( lSize < 0 )
	{
		kdbg_printf( "load_bitmap_file_to_image16: klseek failed!\n" );
		kclose( nHandle );
		return( 0);
	}	
	klseek( nHandle, 0, FSEEK_SET );

	// 메모리를 할당한다.
	pBuff = (BYTE*)kmalloc( lSize );
	if( pBuff != NULL )
		nR = kread( nHandle, pBuff, lSize );
	// 파일을 닫는다.
	kclose( nHandle );

	kdbg_printf( "BITMAP %s (%d byte) loaded.\n", pFileName, lSize );

	if( pBuff == NULL )
	{	// 메모리를 할당할 수 없다.
		kdbg_printf( "load_bitmap_file_to_image16: malloc(%d) failed!\n", lSize );
		return( 0 );
	}

	// 비트맵을 이미지로 변경한다. 
	pBitmap = (BITMAPINFO*)&pBuff[14];	// 앞부분 14바이트(BITMAPFILEHEADER)를 SKIP한다.
	pImg = bitmap_to_image16_ex( pBitmap, pBitmap->bmiHeader.biWidth, pBitmap->bmiHeader.biHeight, nUserFlag );
	pImg->pMask = NULL; // 이걸 꼭 NULL로 주어야 한다. 

	// 메모리를 해제한다.
	kfree( pBuff );
	
	return( pImg );
}

void *load_bitmap_file_to_image16( char *pFileName )
{
	void *pV;

	pV = load_bitmap_file_to_image16_ex( pFileName, 0 );

	return( pV );
}


// 리소스에서 아이콘을 로드한다. 
void *load_icon_image16( void *pV, int nID )
{
	WinResStt		*pWR;
	ImageStt		*pImg;
	ICONDIR			*pIDir;
	BITMAPINFO		*pBitmap;
	WinResEntStt	*pWinRes, *pGrpRes;

	if( pV == NULL )
		pV = get_winres_stt();

	pWR = (WinResStt*)pV;
	pWinRes = load_winres( pWR, (UINT16)nID );	// 메모리 할당되는 것은 없다.
	if( pWinRes == NULL )
	{	// 리소스를 찾을 수 없다. 
		kdbg_printf( "Res %d loading failed!\n", nID );
		return( NULL );
	}

	// 아이콘으로 해석한다. 
	pGrpRes = &pWR->ent[ pWinRes->nPainIndex ];
	pIDir   = (ICONDIR*)pGrpRes->dwAddr;
	pBitmap = (BITMAPINFO*)pWinRes->dwAddr;

	// 비트멥을 이미지로 변경한다. (아이콘은 Height/2에 주의!)
	pImg = bitmap_to_image16( pBitmap, pBitmap->bmiHeader.biWidth, pBitmap->bmiHeader.biHeight/2 );
								   
	return( pImg );
}

// 아이콘 및 비트맵 이미지를 해제한다.
int free_image16( ImageStt *pImg )
{
	if( pImg != NULL )
		kfree( pImg );

	return( 0 );
}

ImageStt *kdup_image16( ImageStt *pSrcImg )
{
	ImageStt 	*pImg;

	if( pSrcImg == NULL || pSrcImg->b.pW == NULL || pSrcImg->nColorSize <= 0 )
	{
		kdbg_printf( "kdup_image16: invalid source image\n" );
		return( NULL );	// 잘못된 원본 이미지.
	}
	
	pImg = (ImageStt*)kmalloc( sizeof( ImageStt ) + pSrcImg->nColorSize + pSrcImg->nMaskSize );
	if( pImg == NULL )
		return( NULL );

	// 헤더를 복사한다.
	memcpy( pImg, pSrcImg, sizeof( ImageStt ) );

	// 컬러 데이터를 복사한다.
	pImg->b.pW	= (UINT16*)( (DWORD)pImg + sizeof( ImageStt ) );
	memcpy( pImg->b.pW, pSrcImg->b.pW, pSrcImg->nColorSize );

	if( pSrcImg->pMask != NULL )
	{	// 마스크는 존재하는 경우에만 복사한다.
		pImg->pMask = (BYTE*)( (DWORD)pImg->b.pW + (DWORD)pImg->nColorSize );
		memcpy( pImg->pMask, pSrcImg->pMask, pSrcImg->nMaskSize );
	}
	else
		pImg->pMask = NULL;

	return( pImg );
}
