#include <bellona2.h>
#include "gui.h"

// 좌표 X, Y에 해당되는 비디오 메모리의 주소를 구한다.
UINT16 *get_video_mem_addr16( int nX, int nY )
{
	UINT16	*pW;
	GuiStt	*pGui;

	pGui = get_gui_stt();
	pW = (UINT16*)( pGui->dwMemAddr + (DWORD)(nX * 2) + (DWORD)(nY * pGui->vmode.LinBytesPerScanLine) );

	return( pW );
}

UINT16 *get_next_screen_line( UINT16 *pW )
{
	GuiStt	*pGui;

	pGui = get_gui_stt();
	pW = (UINT16*)( (DWORD)pW + (DWORD)pGui->vmode.LinBytesPerScanLine );

	return( pW );
}	

UINT16 *get_gra_buff_addr16( GraBuffStt *pGB, int nX, int nY )
{
	UINT16 *pW;
	//pW = (UINT16*)( (DWORD)pGB->pW + (DWORD)( (nX*2) + ( nY * (pGB->pR->nH*2) ) ) );
	pW = &pGB->pW[ nX + ( nY * pGB->pR->nH ) ];	
	return( pW );
}

int k_3d_look( GraBuffStt *pGB, RectStt *pR, int nOuter, int nType, UINT16 wLightColor, UINT16 wDkColor )
{
	UINT16	wColor1, wColor2;
	int		nX1, nY1, nX2, nY2;

	nX1 = pR->nX;
	nY1 = pR->nY;
	nX2 = pR->nX + pR->nH;
	nY2 = pR->nY + pR->nV;

	if( nOuter == LOOK_3D_IN )
	{
		//nX1 += 1;	 // 이건 해 줄 필요가 없다.
		//nY1 += 1;
		nX2 -= 1;
		nY2 -= 1;
	}
	else
	{
		nX1 -= 1;
		nY1 -= 1;
		//nX2 += 1;	 // 이것도 해 줄 필요가 없다.
		//nY2 += 1;
	}

	if( nType == LOOK_3D_DEPRESSED )
	{	// 양각 형태일 때 (ex:버튼이 튀어나온 것과 같은...)
		wColor1 = wLightColor;
		wColor2 = wDkColor;
	}
	else if( nType == LOOK_3D_PRESSED )
	{	// 음각 형태일 때 (ex:버튼이 눌린 것과 같은...)
		wColor1 = wDkColor;
		wColor2 = wLightColor;;
	}
	else
		return( 0 );

	k_line_h( pGB, nX1, nY1, nX2-nX1, wColor1 );
	k_line_v( pGB, nX1, nY1, nY2-nY1, wColor1 );
	k_line_h( pGB, nX1, nY2, nX2-nX1, wColor2 );
	k_line_v( pGB, nX2, nY1, nY2-nY1, wColor2 );

	return( 0 );
}

int k_rect( GraBuffStt *pGB, RectStt *pR, int nOuter, UINT16 wColor )
{
	int		nX1, nY1, nX2, nY2;

	nX1 = pR->nX;
	nY1 = pR->nY;
	nX2 = pR->nX + pR->nH;
	nY2 = pR->nY + pR->nV;

	if( nOuter == LOOK_3D_IN )
	{
		nX2 -= 1;
		nY2 -= 1;
	}
	else
	{
		nX1 -= 1;
		nY1 -= 1;
	}

	k_line_h( pGB, nX1, nY1, nX2-nX1, wColor );
	k_line_v( pGB, nX1, nY1, nY2-nY1, wColor );
	k_line_h( pGB, nX1, nY2, nX2-nX1, wColor );
	k_line_v( pGB, nX2, nY1, nY2-nY1, wColor );

	return( 0 );
}

int k_line_h( GraBuffStt *pGB, int nX, int nY, int nLength, UINT16 wRGB )
{
	int				nI;
	UINT16			*pW;

	pW  = get_gra_buff_addr16( pGB, nX, nY );
	for( nI = 0; nI < nLength; nI++ )
		pW[nI] = wRGB;

	return( 0 );
}

int k_line_v( GraBuffStt *pGB, int nX, int nY, int nLength, UINT16 wRGB )
{
	int				nI;
	UINT16			*pW;

	pW  = get_gra_buff_addr16( pGB, nX, nY );
	for( nI = 0; nI < nLength; nI++ )
	{
		pW[0] = wRGB;
		pW  = &pW[ pGB->pR->nH ];
	}

	return( 0 );
}

int k_fill_rect( GraBuffStt *pGB, RectStt *pR, UINT16 wRGB )
{
	UINT16		*pW;
	int			nXIndex, nYIndex;	

	if( pGB == NULL || pGB->pW == NULL || pR == NULL )
		return( -1 );
	
	if( pR->nH <= 0 || pR->nV <= 0 || pR->nX < 0 || pR->nY < 0 )
		return( 0 );

	pW  = get_gra_buff_addr16( pGB, pR->nX, pR->nY );
	
	for( nYIndex = 0; nYIndex < pR->nV; nYIndex++ )
	{
		for( nXIndex = 0; nXIndex < pR->nH; nXIndex++ )
			pW[ nXIndex ] = wRGB;

		pW  = (UINT16*)&pW[ pGB->pR->nH ];
	}

	return( 0 );
}

// 반투명 효과를 주기 위한 것.
int k_fill_rect_or( GraBuffStt *pGB, RectStt *pR, UINT16 wRGB )
{
	UINT16		*pW, *pWX;
	int 		nXIndex, nYIndex;	

	if( pGB == NULL || pGB->pW == NULL || pR == NULL )
		return( -1 );
	
	if( pR->nH <= 0 || pR->nV <= 0 || pR->nX < 0 || pR->nY < 0 )
		return( 0 );

	pW	= get_gra_buff_addr16( pGB, pR->nX, pR->nY );

	wRGB = (UINT16)( wRGB >> 1 );
	wRGB = (UINT16)( wRGB & (UINT16)0x7BEF );	
	for( nYIndex = 0; nYIndex < pR->nV; nYIndex++ )
	{
		pWX = pW;
		for( nXIndex = 0; nXIndex < pR->nH; nXIndex++, pWX++ )
		{
			pWX[0] = (UINT16)( pWX[0] >> 1 );
			pWX[0] = (UINT16)( pWX[0] & (UINT16)0x7BEF );	
			pWX[0] += wRGB;
		}

		pW	= (UINT16*)&pW[ pGB->pR->nH ];
	}

	return( 0 );
}

int k_fill_rect2( GraBuffStt *pGB, RectStt *pR, UINT16 wRGB )
{
	RGBQUAD			rgb;
	UINT16			*pW;
	int				nXIndex, nYIndex;	
	
	if( pGB == NULL || pGB->pW == NULL || pR == NULL )
		return( -1 );
	if( pR->nH <= 0 || pR->nV <= 0 || pR->nX < 0 || pR->nY < 0 )
		return( 0 );

	pW  = get_gra_buff_addr16( pGB, pR->nX, pR->nY );

	// RGB16을 RGBQUAD로 바꾼다. 
	rgb16_to_rgbquad( wRGB, &rgb );
	for( nYIndex = 0; nYIndex < pR->nV; nYIndex++ )
	{
		wRGB = rgbquad_to_rgb16( &rgb );
		for( nXIndex = 0; nXIndex < pR->nH; nXIndex++ )
			pW[nXIndex] = wRGB;
		pW  = (UINT16*)&pW[ pGB->pR->nH ];

		// 색상을 엹게 한다. 
		if( nYIndex < (pR->nV*3)/4 )
			darken_rgbquad( &rgb );
		else
			brighten_rgbquad( &rgb );
	}

	return( 0 );
}

typedef struct {
	UINT16 	wBit;
	int		nDirection;
	int		nLine;
	int		nCurLine;
} ColorDeltaStt;

typedef struct {
	ColorDeltaStt	r, g, b;
} ColorRangeStt;

static void init_color_range( ColorRangeStt *pCR, UINT16 wColor1, UINT16 wColor2, int nLine )
{
	UINT16 	wX, wY, wZ;

	memset( pCR, 0, sizeof( ColorRangeStt ) );

	// RED	
	wX = wColor1 >> 11;
	wY = wColor2 >> 11;
	pCR->r.wBit = wX;
	if( wX > wY )
	{
	    pCR->r.nDirection = -1;
		wZ = wX - wY;
	 }   
	 else 
	 	wZ = wY - wX;
	pCR->r.nLine = (int)( nLine / (int)wZ ); 	
	
	// GREEN
	wX = (UINT16)(wColor1 >> 5) & (UINT16)0x3F;
	wY = (UINT16)(wColor2 >> 5) & (UINT16)0x3F;
	pCR->g.wBit = wX;
	if( wX > wY )
	{
	    pCR->g.nDirection = -1;
		wZ = wX - wY;
	 }   
	 else 
	 	wZ = wY - wX;
	pCR->g.nLine = (int)( nLine / (int)wZ ); 	
	
	// BLUE
	wX = wColor1 & (UINT16)0x1F;
	wY = wColor2 & (UINT16)0x1F;
	pCR->b.wBit = wX;
	if( wX > wY )
	{
	    pCR->b.nDirection = -1;
		wZ = wX - wY;
	 }   
	 else 
	 	wZ = wY - wX;
	pCR->b.nLine = (int)( nLine / (int)wZ ); 	
}

static UINT16 get_next_color( ColorRangeStt *pCR )
{
	UINT16 wColor;
	
	pCR->r.nCurLine++;
	pCR->g.nCurLine++;
	pCR->b.nCurLine++;

	if( pCR->r.nCurLine >= pCR->r.nLine )
	{
		pCR->r.nCurLine = 0;
		if( pCR->r.nDirection < 0 )
		    pCR->r.wBit--;
		else
			pCR->r.wBit++;    
	}

	if( pCR->g.nCurLine >= pCR->g.nLine )
	{
		pCR->g.nCurLine = 0;
		if( pCR->g.nDirection < 0 )
		    pCR->g.wBit--;
		else
			pCR->g.wBit++;    
	}

	if( pCR->b.nCurLine >= pCR->b.nLine )
	{
		pCR->b.nCurLine = 0;
		if( pCR->b.nDirection < 0 )
		    pCR->b.wBit--;
		else
			pCR->b.wBit++;    
	}
	
	wColor = (UINT16)( pCR->r.wBit << 11 ) + (UINT16)( pCR->g.wBit << 5 ) + (UINT16)( pCR->b.wBit );
	
	return( wColor );	
}

int get_mask_bit( MaskStt *pMask, int nX, int nY )
{
	BYTE	byBit;
	int		nIndex, nMod;

	if( pMask == NULL || pMask->pB == NULL )
		return( 1 );

	nIndex = ( nY * pMask->dwLineBytes ) + ( nX / 8 );
	nMod = nX % 8;
	byBit = 0x80;
	if( nMod > 0 )
		byBit = byBit >> nMod;
			
	if( pMask->pB[ nIndex ] & byBit )
		return( 1 );

	return( 0 );
}

int set_mask_bit( MaskStt *pMask, int nX, int nY, int nBit )
{
	BYTE	byBit;
	int		nIndex, nMod;

	if( pMask == NULL || pMask->pB == NULL )
		return( 1 );

	nIndex = ( nY * pMask->dwLineBytes ) + ( nX / 8 );
	nMod = nX % 8;
	byBit = 0x80;
	if( nMod > 0 )
		byBit = byBit >> nMod;
			
	pMask->pB[ nIndex ] &= (BYTE)(~byBit);
	if( nBit != 0 )
		pMask->pB[ nIndex ] |= byBit;

	return( 0 );
}

// 기준이 되는 BaseRect와 실제로 그려지는 DrawRect가 있어야 한다.
// Vertical incremental 효과를 준다.
int k_incremental_fill_rect( GraBuffStt *pGB, RectStt *pBaseRect, RectStt *pDrawRect, DWORD dwLinesByte, 
							 UINT16 wRGB1, UINT16 wRGB2, MaskStt *pMask )
{
	ColorRangeStt	cr;
	UINT16			*pW;
	UINT16 			wColor;
	int				nXIndex, nYIndex;	

	init_color_range( &cr, wRGB1, wRGB2, pBaseRect->nV );
	
	if( pGB == NULL )
		pW = get_video_mem_addr16( pBaseRect->nX, pBaseRect->nY );
	else
		pW = get_gra_buff_addr16( pGB, pBaseRect->nX, pBaseRect->nY );
	
	wColor = wRGB1;
	for( nYIndex = 0; nYIndex < pBaseRect->nV; nYIndex++ )
	{
		if( pDrawRect->nY <= nYIndex )
		{	
			if( pDrawRect->nY + pDrawRect->nV <= nYIndex )
			    break;	 // DrawRect 영역만큼 다 그렸다.
			
 			for( nXIndex = pDrawRect->nX; nXIndex < pDrawRect->nX + pDrawRect->nH; nXIndex++ )
			{
				if( get_mask_bit( pMask, nXIndex, nYIndex ) != 0 )
					pW[nXIndex] = wColor;
			}
		}
		wColor = get_next_color( &cr );		
		
		pW  = (UINT16*)( (DWORD)pW + dwLinesByte );
	}

	return( 0 );
}

// 두 사각형의 겹치는 부분 구하기.
int get_overlapped_rect( RectStt *pResult, RectStt *pA, RectStt *pB )
{
	// X 좌표
	if( pA->nX <= pB->nX && pB->nX < pA->nX + pA->nH )
	{
		pResult->nX = pB->nX;
		// H
		if( pA->nX + pA->nH < pB->nX + pB->nH )
			pResult->nH = pA->nX + pA->nH - pResult->nX;
		else
			pResult->nH = pB->nH;
	}
	else if( pB->nX <= pA->nX && pA->nX < pB->nX + pB->nH )
	{
		pResult->nX = pA->nX;
		// H
		if( pB->nX + pB->nH < pA->nX + pB->nH )
			pResult->nH = pB->nX + pB->nH - pResult->nX;
		else
			pResult->nH = pA->nH;
	}
	else
		return( 0 );	// 겹치는 부분이 없다.

	// Y 좌표
	if( pA->nY <= pB->nY && pB->nY < pA->nY + pA->nV )
	{
		pResult->nY = pB->nY;
		// V
		if( pA->nY+ pA->nV < pB->nY + pB->nV )
			pResult->nV = pA->nY + pA->nV - pResult->nY;
		else
			pResult->nV = pB->nV;
	}
	else if( pB->nY <= pA->nY && pA->nY < pB->nY + pB->nV )
	{
		pResult->nY = pA->nY;
		// V
		if( pB->nY+ pB->nV < pA->nY + pA->nV )
			pResult->nV = pB->nY + pB->nV - pResult->nY;
		else
			pResult->nV = pA->nV;
	}
	else
		return( 0 );	// 겹치는 부분이 없다.

	// 겹치는 부분이 있다.
	return( 1);
}

// Rect 영역이 올바른 것인지 확인한다.
int is_valid_rect( RectStt *pR, RectStt *pBaseR )
{
	GuiStt *pGui;
	
	if( pR->nX < 0 || pR->nY < 0 || pR->nH < 0 || pR->nV < 0 )
		return( 0 );

	if( pBaseR == NULL )
	{
		pGui = get_gui_stt();
		pBaseR = &pGui->wall.obj.r;
	}

	if( pR->nH > pBaseR->nH ||	pR->nV > pBaseR->nV )
		return( 0 );
	
	return( 1 );
}	

// 비트 벡터의 해당 영역을 0또는 1로 설정한다.
// pMask->r과 pR 모두 Screen에 대한 절대 좌표다.  
// 이들 둘의 겹치는 부분의 mask를 조작하면 된다.
int	modify_bit_vect( MaskStt *pMask, RectStt *pR, DWORD dwBit )
{
	GuiStt		*pGui;
	BYTE		*pB, byBit;
	DWORD		dwMaskWord;
	RectStt		overlap_r, target_r;
	int			nH, nV, nLinesByte, nR, nDiv, nMod;
	
	pGui = get_gui_stt();

	// 겹치는 부분의 Rect를 얻는다.
	nR = get_overlapped_rect( &overlap_r, pMask->pR, pR );
	if( nR == 0 )	   // RECT가 겹치는 부분이 없으면 그냥 돌아간다.
		return( 0 );
	screen_to_win( &target_r, pMask->pR, &overlap_r ); // 좌표만 바뀐다.

	// target 영역이 올바른 것인지 확인한다.
	if( is_valid_rect( &target_r, pMask->pR ) == 0 )
		return( -1 );
		 
	nDiv = target_r.nX / 8;
	nMod = target_r.nX % 8;
	pB   = &pMask->pB[ nDiv + target_r.nY * pMask->dwLineBytes ];
	byBit = (BYTE)0x80;
	if( nMod > 0 ) 
		byBit = (BYTE)( byBit >> nMod );

	nH = target_r.nH;
	nV = target_r.nV;
	nLinesByte = pMask->dwLineBytes; 

	dwBit &= 1;
	if( dwBit == 0 )
		dwMaskWord = 0;
	else
		dwMaskWord = 0xFFFFffff;

	_asm {
        MOV EAX, dwMaskWord
		MOV EBX, dwBit;
		MOV ECX, nV
		MOV DL,  byBit
		MOV EDI, pB	   // 비트 조작을 위한 시작 위치.
V_LINES:	
		PUSH ECX
		PUSH EDX
		PUSH EDI

             MOV ECX, nH

H_BITS:		 CMP DL, 0x80		 // 바이트의 시작인지 확인한다.
			 JNE H_BITS2

CMP_32:		 CMP ECX, 32		 // DWORD의 시작이고 32비트 이상 남았으면 DWORD 전체를 1로 설정.
			 JB  CMP_16
				 MOV [EDI], EAX
				 ADD EDI, 4
				 SUB ECX, 32
				 JZ  V_LOOP
				     JMP CMP_32		

CMP_16:		 CMP ECX, 16
			 JB  CMP_8
				 MOV [EDI], AX
				 ADD EDI, 2
				 SUB ECX, 16
				 JZ  V_LOOP
				     JMP CMP_32
					 
CMP_8:		 CMP ECX, 8
			 JB  H_BITS2
				 MOV [EDI], AL
				 INC EDI
				 SUB ECX, 8
				 JZ  V_LOOP
				     JMP CMP_32		
		
H_BITS2:	 AND EBX,1
			 JZ  CLEAR_BIT
				 OR [EDI], DL			   // 해당 비트를 1로 설정.
				 JMP ROTATE_RIGHT
CLEAR_BIT:		
			 MOV DH, DL					   // 해당 비트를 0으로 클리어.
			 NOT DH
			 AND [EDI], DH

ROTATE_RIGHT:
			 SHR DL, 1
			 OR  DL, DL
			 JNZ H_LOOP
			     MOV DL, 0x80
				 INC EDI
				 LOOP H_BITS			   // 바이트의 시작인지 비교한다.
				 JMP  V_LOOP
				 				 
H_LOOP:		 LOOP H_BITS2				   // 바이트의 시작이 아니다.
					 
V_LOOP:	POP  EDI
		POP  EDX
		POP  ECX
		ADD  EDI, nLinesByte

		LOOP V_LINES
	}
			
	return( 0 );
}

// 윈도우의 사각형 영역의 상상을 반전시킨다.
int k_invert_rect( GraBuffStt *pGB, RectStt *pR )
{
	UINT16		*pW;
	int			nXIndex, nYIndex;	

	if( pGB == NULL || pGB->pW == NULL || pR == NULL )
		return( -1 );
	
	if( pR->nH <= 0 || pR->nV <= 0 || pR->nX < 0 || pR->nY < 0 )
		return( 0 );

	pW  = get_gra_buff_addr16( pGB, pR->nX, pR->nY );
	
	for( nYIndex = 0; nYIndex < pR->nV; nYIndex++ )
	{
		for( nXIndex = 0; nXIndex < pR->nH; nXIndex++ )
			pW[ nXIndex ] = (UINT16)~(UINT16)pW[ nXIndex ];

		pW  = (UINT16*)&pW[ pGB->pR->nH ];
	}

	return( 0 );
}

static int kabs( int nValue )
{
	if( nValue < 0 )
		return( nValue * (-1) );

	return( nValue );
}

typedef int (*PIXEL_FUNC)( GraBuffStt *pGB, int nX, int nY, UINT16 col );
static int set_pixel( GraBuffStt *pGB, int nX, int nY, UINT16 col )
{
	UINT16			*pW;
	pW	= get_gra_buff_addr16( pGB, nX, nY );
	pW[0] = col;
	return( 0 );
}

static int or_pixel( GraBuffStt *pGB, int nX, int nY, UINT16 col )
{
	UINT16			*pW;
	pW	= get_gra_buff_addr16( pGB, nX, nY );

	pW[0] = (UINT16)( pW[0] >> 1 );
	pW[0] = (UINT16)( pW[0] & (UINT16)0x7BEF );

	col = (UINT16)( col >> 1 );
	col = (UINT16)( col & (UINT16)0x7BEF );
	
	pW[0] += col;
	
	return( 0 );
}

UINT16 k_rgb32_to_rgb16( DWORD dwColor )
{
	BYTE	*pB;
	UINT16	wColor;

	pB = (BYTE*)&dwColor;
	wColor = RGB16( pB[0], pB[1], pB[2] ); // r, g, b
	
	return( wColor );
}

int k_bresenhem_line16( GraBuffStt *pGB, int nX1, int nY1, int nX2, int nY2, UINT16 col, int nOrFlag )
{
	int 		nTemp ;
	int 		nX, nY ;
	PIXEL_FUNC	pPixelFunc;
	int 		nA, nB, nXInc, nYInc ;
	int 		nDelta, nDeltaX, nDeltaY;

	if( nOrFlag == 0 )
		pPixelFunc = set_pixel;
	else
		pPixelFunc = or_pixel;

	//kdbg_printf( "k_bresenhem_line: (%d, %d), %d, %d)\n", nX1, nY1, nX2, nY2 );

	if( kabs(nX2-nX1) > kabs(nY2-nY1) ) 
	{	 // X의 변위가 Y 변위보다 길다.
		if( nX1 > nX2 )
		{		   
			nTemp = nX1;
			nX1   = nX2;
			nX2   = nTemp;

			nTemp = nY1 ;
			nY1   = nY2 ;
			nY2   = nTemp ;
		}
		// 기울기
		if( nY2 > nY1 )
			nYInc = 1 ;
		else 
			nYInc = -1 ;

		// X의 변위와 Y의 변위.
		nDeltaX = nX2 - nX1;		 
		nDeltaY = kabs( nY2 - nY1 );

		nDelta = 2 * nDeltaY - nDeltaX;
		nA = 2 * ( nDeltaY - nDeltaX );
		nB = 2 * nDeltaY;
		
		nX = nX1;
		nY = nY1;
		
		pPixelFunc( pGB, nX, nY, col );
		for( nX++; nX <= nX2; nX++)
		{
			if( nDelta >= 0 )
			{
				nY += nYInc ;
				nDelta += nA ;
			}
			else 
				nDelta += nB ;
			pPixelFunc( pGB, nX, nY, col);
		}			 
	}	 
	else 
	{
		if( nY1 > nY2 )
		{		   
			nTemp = nX1 ;
			nX1   = nX2 ;
			nX2   = nTemp ;
			
			nTemp = nY1 ;
			nY1   = nY2 ;
			nY2   = nTemp ;
		}
		if (nX2 > nX1)
			nXInc = 1 ;
		else 
			nXInc = -1 ;	
		nDeltaX   = kabs( nX2 - nX1 );
		nDeltaY   = nY2 - nY1 ;
		nDelta	  = 2*nDeltaX - nDeltaY;
		
		nA = 2 * (nDeltaX - nDeltaY);
		nB = 2 * nDeltaX ;
		
		nX = nX1 ;
		nY = nY1 ;
		
		pPixelFunc( pGB, nX, nY, col);
		for (nY++; nY <= nY2; nY++)
		{
			if (nDelta >= 0)
			{
				nX += nXInc;
				nDelta += nA;
			}
			else 
				nDelta += nB ;
			pPixelFunc( pGB, nX, nY, col );
		}  
	}
	return( 0 );
}

int k_bresenhem_line( GraBuffStt *pGB, int nX1, int nY1, int nX2, int nY2, DWORD dwColor )
{ 
	int 	nR;
	UINT16	col;
	
	col = k_rgb32_to_rgb16( dwColor );

	nR = k_bresenhem_line16( pGB, nX1, nY1, nX2, nY2, col, 0 );

	return( nR );
}

