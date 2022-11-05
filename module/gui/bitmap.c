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

	// RGB16���� shift�� �ϱ� ������ ���⼭�� ���� �ʾƵ� �ȴ�.
	r = (BYTE)( (BYTE)pRQ->rgbRed   );	//>> 3 );
	g = (BYTE)( (BYTE)pRQ->rgbGreen );	//>> 2 );
	b = (BYTE)( (BYTE)pRQ->rgbBlue  );	//>> 3 );

	return( RGB16( r, g, b ) );
}

// kfree()�� �����ϸ� �ȴ�.
void *alloc_blank_image16( int nH, int nV )
{
	ImageStt			*pImg;
	int					nSize, nColorSize;

	// ����� ����Ѵ�. 
	nColorSize = (int)( nH * nV * 2 );
	nSize = nColorSize + (int)sizeof( ImageStt );

	// ����� ���۸� �Ҵ��Ѵ�. 
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

	// 16��Ʈ ���� ����� ����Ѵ�. 
	nColorSize = nH * nV * 2;
	nMaskSize = 0;
	nSize = nColorSize + (int)sizeof( ImageStt );

	// ����� ���۸� �Ҵ��Ѵ�. 
	if( nUserFlag == 0 )
		pImg = (ImageStt*)kmalloc( nSize );				// Ŀ�� �������� �޸� �Ҵ�.
	else
		pImg = (ImageStt*)umalloc( NULL, NULL, nSize );	// ���� ���μ����� ����� �������� �޸� �Ҵ�.
	if( pImg == NULL )
		return( NULL );
		
	memset( pImg, 0, nSize );
	pW = pImg->b.pW  = (UINT16*)( (DWORD)pImg + (DWORD)sizeof( ImageStt ) );
	pImg->nBPP  = 16;
	pImg->nH    = nH;
	pImg->nV    = nV;
	pImg->nSize = nSize;

	if( pBitmap->bmiHeader.biBitCount == 24 )
	{	// 24 ��Ʈ ��Ʈ���� �÷� ���̺��� ���� �ٷ� RGB 3����Ʈ ���� �����Ͱ� �´�.
		nDeltaH = ( ( pBitmap->bmiHeader.biWidth * 3 ) + 3 ) / 4;
		nDeltaH *= 4;

		pB = (BYTE*)pBitmap->bmiColors;
		// ���� �Ʒ��� ���κ��� ó���Ѵ�.
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
		// �÷� ���̺��� ������ ���Ѵ�.
		nColors = 1;
		nColors = (int)( nColors << (int)pBitmap->bmiHeader.biBitCount );

		// �÷� �ε����� ����
		pB = (BYTE*)( (DWORD)&pBitmap->bmiColors[nColors] );
		
		if( nColors == 256 )
			nDeltaH = ( (nH+3) /4) * 4;
		else if( nColors == 16 )
			nDeltaH = ( (((nH+1)/2)+3) /4) * 4;
		else
		{	// �������� �ʴ� ������ ��� ���� ó���Ѵ�.
			kdbg_printf( "Unsupported color(%d) icon!", nColors );

			if( nUserFlag == 0 )
				kfree( pImg );	
			else
				ufree( NULL, pImg );

			return( NULL );
		}

		// ����ũ�� �Ҵ����� �ʰ� ���ҽ� �ε��� ���� ���� ����Ѵ�?? (�̷��� �ϸ� �ȵȴ�.)
		pImg->pMask = (BYTE*)( (DWORD)pB + (DWORD)( nDeltaH * nV ) );
		nMaskSize = (((( nH + 31 ) / 32 ) * 32) * nV );

		pB += nV * nDeltaH;
		for( nY = 0; nY < nV; nY++ )
		{
			pB -= nDeltaH;
			
			if( nColors == 256 )
			{	// 256 �÷� ������
				for( nX = 0; nX < nH; nX++ )
					pW[ nX ] = rgbquad_to_rgb16( &pBitmap->bmiColors[ (DWORD)pB[ nX ] ] );
			}
			else if( nColors == 16 )
			{	// 16�÷� ������
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

// ��Ʈ���� 16��Ʈ �÷� �̹����� ��ȯ�Ѵ�. 
void *bitmap_to_image16( BITMAPINFO *pBitmap, int nH, int nV )
{
	void *pV;
	pV = bitmap_to_image16_ex( pBitmap, nH, nV, 0 );
	return( pV );
}

// �������� ����ũ ��Ʈ�� ���Ѵ�.
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

// �������� GraBuff�� �̹����� �����Ѵ�.
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
	
	// H, V�� �����̸� �̹��� ��ü�� �����Ѵ�.	H, V�� Dest�� �Ѿ�� �ڸ���.
	pImg  = (ImageStt*)pVImg;
	if( nXLimit < 0 )
		nXLimit = pImg->nH;
	else if( nXLimit > pGB->pR->nH - nX )
		nXLimit = pGB->pR->nH - nX;
	if( nYLimit < 0 )
		nYLimit = pImg->nV;
	else if( nYLimit > pGB->pR->nV - nY )
		nYLimit = pGB->pR->nV - nY;

	// H, V�� Src�� �Ѿ�� �ڸ���.
	if( nXLimit > pImg->nH )
		nXLimit = pImg->nH;
	if( nYLimit > pImg->nV )
		nYLimit = pImg->nV;

	pIW   = get_img_buff_addr16( pImg, nSrcX, nSrcY );			// �׸����� �ϴ� �̹����� �ȼ� ������.
	pMask = pImg->pMask;		// �׸����� �ϴ� �̹����� ����ũ ������.

	// grabuff�� �ּҸ� ���Ѵ�.
	pW = get_gra_buff_addr16( pGB, nX, nY );
	nMaskDelta = ( ( (pImg->nH + 31 ) / 32 ) * 32 );  	
	for( nV = 0; nV < nYLimit; nV++ )
	{
		for( nH = 0; nH < nXLimit; nH++ )
		{
			// �ȼ��� ��´�. 
			if( pMask != NULL )
			{   // ����ũ�� �����Ǿ� ������ �׸��� ���� ���� �ִ�.
				if( icon_mask_value( pMask, nH + (pImg->nV - nV -1)*nMaskDelta ) == 0 )
				{
					pW[nH] = pIW[nH];

					// ����ȭ ��ų �ʿ䰡 �ִ�.  (�ϴ��� �۵��ϴ��� ���� ����.)
					if( pGB->self_mask.pB != NULL )
						set_mask_bit( &pGB->self_mask, nX + nH, nY + nV, 1 );
				}
			}
			else			
			{	// ����ũ�� �����Ƿ� �׳� �׸���.
				pW[nH] = pIW[nH];

				// ����ȭ ��ų �ʿ䰡 �ִ�.  (�ϴ��� �۵��ϴ��� ���� ����.)
				if( pGB->self_mask.pB != NULL )
					set_mask_bit( &pGB->self_mask, nX + nH, nY + nV, 1 );
			}
		}
		
		pW  = &pW[ pGB->pR->nH ];
		pIW = &pIW[ pImg->nH ];
	}

	return( 0 );
}

// pVImg : �׸� ��Ʈ �̹���.
// pVOrgImg : �׸��� �� ������ ������ �̹��� ����.
int draw_image16( void *pVImg, void *pVOrgImg, int nX, int nY, int nSrcX, int nSrcY, int nSrcH, int nSrcV )
{
	GuiStt		*pGui;
	BYTE		*pMask;
	UINT16		*pW, *pIW, *pOW;
	ImageStt	*pImg, *pOrgImg;
	int			nT, nI, nH, nV, nMaskDelta;	

	pImg  = (ImageStt*)pVImg;
	pIW   = pImg->b.pW;			// �׸����� �ϴ� �̹����� �ȼ� ������.
	pMask = pImg->pMask;		// �׸����� �ϴ� �̹����� ����ũ ������.

	if( pVOrgImg != NULL )
	{	// �����.
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
			
			// ���� �ȼ��� �����Ѵ�. 
			if( pOW != NULL )
				pOW[nI] = pW[nT];

			// �ȼ��� ��´�. 
			if( pMask != NULL )
			{   // ����ũ�� �����Ǿ� ������ �׸��� ���� ���� �ִ�.
				if( icon_mask_value( pMask, nH + (pImg->nV - nV -1)*nMaskDelta ) == 0 )
					pW[nT] = pIW[nI];
			}
			else			
			{	// ����ũ�� �����Ƿ� �׳� �׸���.
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
	UINT16			*pW;			// ��ũ���� ����.
	UINT16			*pIW;			// �̹����� ����.
	UINT16			*pOW;			// ���� �̹����� ������ ���� �ּ�.
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
			// ���� �ȼ��� �����Ѵ�. 
			if( pOW != NULL )
				pOW[nI] = pW[nI];

			// �ȼ��� ��´�. 
			if( pMask != NULL )
			{
				if( icon_mask_value( pMask, nH + (pImg->nV - nV -1)*nMaskDelta ) == 0 )
					pW[nI] = pIW[nI];
			}
			else
			{	// ���� �ȿ� ���� ��쿡�� �ȼ��� �׸���. 
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

// ���ҽ����� ��Ʈ���� �ε��Ѵ�. 
void *load_bitmap_image16( void *pV, int nID )
{
	ImageStt		*pImg;
	WinResEntStt	*pWinRes;
	BITMAPINFO		*pBitmap;

	pWinRes = load_winres( (WinResStt*)pV, (UINT16)nID );
	if( pWinRes == NULL )
	{	// ���ҽ��� ã�� �� ����. 
		kdbg_printf( "Res %d loading failed!\n", nID );
		return( NULL );
	}

	// ��Ʈ���� �̹����� �����Ѵ�. 
	pBitmap = (BITMAPINFO*)pWinRes->dwAddr;
	pImg = bitmap_to_image16( pBitmap, pBitmap->bmiHeader.biWidth, pBitmap->bmiHeader.biHeight );
	pImg->pMask = NULL;	// �̰� �� NULL�� �־�� �Ѵ�. 
	
	return( pImg );
}

// ��Ʈ�� ������ �ε��Ѵ�. 
void *load_bitmap_file_to_image16_ex( char *pFileName, int nUserFlag )
{
	int				nHandle, nR;
	BYTE			*pBuff;
	long			lSize;
	ImageStt		*pImg;
	BITMAPINFO		*pBitmap;

	// ������ �����Ͽ� ũ�⸦ ���Ѵ�.
	nHandle = kopen( pFileName, FM_READ );
	if( nHandle == -1 )
	{
		kdbg_printf( "load_bitmap_file_to_image16: %s open error!\n", pFileName );
		return( 0 );
	}
	// ���� ũ�⸦ �˾Ƴ���.
	lSize = klseek( nHandle, 0, FSEEK_END );
	if( lSize < 0 )
	{
		kdbg_printf( "load_bitmap_file_to_image16: klseek failed!\n" );
		kclose( nHandle );
		return( 0);
	}	
	klseek( nHandle, 0, FSEEK_SET );

	// �޸𸮸� �Ҵ��Ѵ�.
	pBuff = (BYTE*)kmalloc( lSize );
	if( pBuff != NULL )
		nR = kread( nHandle, pBuff, lSize );
	// ������ �ݴ´�.
	kclose( nHandle );

	kdbg_printf( "BITMAP %s (%d byte) loaded.\n", pFileName, lSize );

	if( pBuff == NULL )
	{	// �޸𸮸� �Ҵ��� �� ����.
		kdbg_printf( "load_bitmap_file_to_image16: malloc(%d) failed!\n", lSize );
		return( 0 );
	}

	// ��Ʈ���� �̹����� �����Ѵ�. 
	pBitmap = (BITMAPINFO*)&pBuff[14];	// �պκ� 14����Ʈ(BITMAPFILEHEADER)�� SKIP�Ѵ�.
	pImg = bitmap_to_image16_ex( pBitmap, pBitmap->bmiHeader.biWidth, pBitmap->bmiHeader.biHeight, nUserFlag );
	pImg->pMask = NULL; // �̰� �� NULL�� �־�� �Ѵ�. 

	// �޸𸮸� �����Ѵ�.
	kfree( pBuff );
	
	return( pImg );
}

void *load_bitmap_file_to_image16( char *pFileName )
{
	void *pV;

	pV = load_bitmap_file_to_image16_ex( pFileName, 0 );

	return( pV );
}


// ���ҽ����� �������� �ε��Ѵ�. 
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
	pWinRes = load_winres( pWR, (UINT16)nID );	// �޸� �Ҵ�Ǵ� ���� ����.
	if( pWinRes == NULL )
	{	// ���ҽ��� ã�� �� ����. 
		kdbg_printf( "Res %d loading failed!\n", nID );
		return( NULL );
	}

	// ���������� �ؼ��Ѵ�. 
	pGrpRes = &pWR->ent[ pWinRes->nPainIndex ];
	pIDir   = (ICONDIR*)pGrpRes->dwAddr;
	pBitmap = (BITMAPINFO*)pWinRes->dwAddr;

	// ��Ʈ���� �̹����� �����Ѵ�. (�������� Height/2�� ����!)
	pImg = bitmap_to_image16( pBitmap, pBitmap->bmiHeader.biWidth, pBitmap->bmiHeader.biHeight/2 );
								   
	return( pImg );
}

// ������ �� ��Ʈ�� �̹����� �����Ѵ�.
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
		return( NULL );	// �߸��� ���� �̹���.
	}
	
	pImg = (ImageStt*)kmalloc( sizeof( ImageStt ) + pSrcImg->nColorSize + pSrcImg->nMaskSize );
	if( pImg == NULL )
		return( NULL );

	// ����� �����Ѵ�.
	memcpy( pImg, pSrcImg, sizeof( ImageStt ) );

	// �÷� �����͸� �����Ѵ�.
	pImg->b.pW	= (UINT16*)( (DWORD)pImg + sizeof( ImageStt ) );
	memcpy( pImg->b.pW, pSrcImg->b.pW, pSrcImg->nColorSize );

	if( pSrcImg->pMask != NULL )
	{	// ����ũ�� �����ϴ� ��쿡�� �����Ѵ�.
		pImg->pMask = (BYTE*)( (DWORD)pImg->b.pW + (DWORD)pImg->nColorSize );
		memcpy( pImg->pMask, pSrcImg->pMask, pSrcImg->nMaskSize );
	}
	else
		pImg->pMask = NULL;

	return( pImg );
}
