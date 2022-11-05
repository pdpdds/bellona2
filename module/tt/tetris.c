#ifdef WIN32
	#include <windows.h>
	// Win32 쪽에서만 만들어 주면 되는 함수.
	extern int gx_line( DWORD dwHandle, int nX1, int nY1, int nX2, int nY2, DWORD dwColor );
	extern int gx_drawtext_xy( DWORD dwHandle, int nX, int nY, DWORD dwFontID, char *pStr, unsigned short wColor, DWORD dwEffect );
#endif

#include "tetris.h"

// Win32, B2OS에서 각각 만들어 줘야 하는 함수.
extern int flush_slot( DWORD dwWinHandle );
extern int fill_rect32( DWORD dwHandle, RectStt *pR, DWORD dwColor );
extern int fill_rect32_or( DWORD dwHandle, RectStt *pR, DWORD dwColor );

////////// 블록의 종류//////////
extern TTBlockArrayStt tbtype_T0;
extern TTBlockArrayStt tbtype_L0;
extern TTBlockArrayStt tbtype_R0;
extern TTBlockArrayStt tbtype_X0;
extern TTBlockArrayStt tbtype_I0;
extern TTBlockArrayStt tbtype_H0;
extern TTBlockArrayStt tbtype_Z0;
////////////////////////////////

TTCfgStt tt_cfg;

static TTBlockArrayStt *t_blocks[] = {
	&tbtype_I0,
	&tbtype_L0,
	&tbtype_T0,
	&tbtype_R0,
	&tbtype_X0,
	&tbtype_H0,
	&tbtype_Z0
};

TTBlockArrayStt *get_block( int nIndex )
{
	int				nMax;
	TTBlockArrayStt *pNextBlk;

	nMax = sizeof( t_blocks ) / sizeof( 4 );
	if( nIndex >= nMax )
		nIndex %= nMax; 

	pNextBlk = tt_cfg.pNextBlk;
	tt_cfg.pNextBlk = t_blocks[nIndex];

	return( pNextBlk );
}

////////////////////////////////////////////////////////////
static TTBlockArrayStt tbtype_T3 = {
	&tbtype_T0,
	{   0 ,  1 ,  0,  0 ,
	    1 ,  1 ,  0,  0 ,
	    0 ,  1 ,  0,  0 ,
	    0 ,  0 ,  0,  0  },
};

static TTBlockArrayStt tbtype_T2 = {
	&tbtype_T3,
	{  0 ,  1 ,  0 ,  0,
	   1 ,  1 ,  1 ,  0,
	   0 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};

static TTBlockArrayStt tbtype_T1 = {
	&tbtype_T2,
	{  1 ,  0,   0 ,  0,
	   1 ,  1 ,  0 ,  0,
	   1 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};

TTBlockArrayStt tbtype_T0 = {
	&tbtype_T1,
	{  1 ,  1 ,  1 ,  0,
	   0 ,  1 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};
////////////////////////////////////////////////////////////
static TTBlockArrayStt tbtype_L3 = {
	&tbtype_L0,
	{  1 ,  1 ,  0 ,  0,
	   1 ,  0 ,  0 ,  0,
	   1 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};
static TTBlockArrayStt tbtype_L2 = {
	&tbtype_L3,
	{  1 ,  1 ,  1 ,  0,
	   0 ,  0 ,  1 ,  0,
	   0 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0 }
};
static TTBlockArrayStt tbtype_L1 = {
	&tbtype_L2,
	{  0 ,  1 ,  0 ,  0,
	   0 ,  1 ,  0 ,  0,
	   1 ,  1 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};
TTBlockArrayStt tbtype_L0 = {
	&tbtype_L1,
	{  1 ,  0 ,  0 ,  0,
	   1 ,  1 ,  1 ,  0,
	   0 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0 }
};
////////////////////////////////////////////////////////////
static TTBlockArrayStt tbtype_R3 = {
	&tbtype_R0,
	{  1 ,  0 ,  0 ,  0,
	   1 ,  0 ,  0 ,  0,
	   1 ,  1 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};
static TTBlockArrayStt tbtype_R2 = {
	&tbtype_R3,
	{  1 ,  1 ,  1 ,  0,
	   1 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0 }
};
static TTBlockArrayStt tbtype_R1 = {
	&tbtype_R2,
	{  1 ,  1 ,  0 ,  0,
	   0 ,  1 ,  0 ,  0,
	   0 ,  1 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};
TTBlockArrayStt tbtype_R0 = {
	&tbtype_R1,
	{  0 ,  0 ,  1 ,  0,
	   1 ,  1 ,  1 ,  0,
	   0 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0 }
};
////////////////////////////////////////////////////////////
TTBlockArrayStt tbtype_X0 = {
	&tbtype_X0,
	{  1 ,  1 ,  0 ,  0,
	   1 ,  1 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};
////////////////////////////////////////////////////////////
static TTBlockArrayStt tbtype_I1 = {
	&tbtype_I0,
	{  0 ,  1 ,  0 ,  0,
	   0 ,  1 ,  0 ,  0,
	   0 ,  1 ,  0 ,  0,
	   0 ,  1 ,  0 ,  0  },
};
TTBlockArrayStt tbtype_I0 = {
	&tbtype_I1,
	{  1 ,  1 ,  1 ,  1,
	   0 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};
////////////////////////////////////////////////////////////
static TTBlockArrayStt tbtype_H1 = {
	&tbtype_H0,
	{  0 ,  1 ,  0 ,  0,
	   1 ,  1 ,  0 ,  0,
	   1 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};
TTBlockArrayStt tbtype_H0 = {
	&tbtype_H1,
	{  1 ,  1 ,  0 ,  0,
	   0 ,  1 ,  1 ,  0,
	   0 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};
////////////////////////////////////////////////////////////
static TTBlockArrayStt tbtype_Z1 = {
	&tbtype_Z0,
	{  1 ,  0 ,  0 ,  0,
	   1 ,  1 ,  0 ,  0,
	   0 ,  1 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};
TTBlockArrayStt tbtype_Z0 = {
	&tbtype_Z1,
	{  0 ,  1 ,  1 ,  0,
	   1 ,  1 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0,
	   0 ,  0 ,  0 ,  0  },
};
////////////////////////////////////////////////////////////
int init_tt_cfg( TTCfgStt *pCfg, RectStt *pR )
{
	int nSlotV, nInterval;
	int nClientH, nClientV;

	nClientH = pR->nH;
	nClientV = pR->nV;

	nSlotV = nClientV - TT_INFO_V;
	memset( pCfg, 0, sizeof( TTCfgStt ) );

	memcpy( &tt_cfg.view_r, pR, sizeof( RectStt ) );

	pCfg->wHLine = TT_LINE_H;
	pCfg->wVLine = TT_LINE_V;

	pCfg->wCellH = nClientH / pCfg->wHLine;
	pCfg->wCellV = nSlotV / pCfg->wVLine;

	pCfg->wSlotH = pCfg->wCellH * TT_LINE_H;
	pCfg->wSlotV = pCfg->wCellV * TT_LINE_V;

	pCfg->wClientH = nClientH;
	pCfg->wClientV = nClientV;

	nInterval = (nClientH - pCfg->wSlotH) /2;
	pCfg->wSlotX = pR->nX + nInterval;

#ifdef WIN32
	pCfg->wSlotY = pR->nY + nInterval;
#endif
#ifndef WIN32
	pCfg->wSlotY = pR->nY + nInterval;
#endif

	pCfg->wInfoX = pR->nX + nInterval;
	pCfg->wInfoY = pR->nY + (nInterval*2) + pCfg->wSlotV;
	pCfg->wInfoH = pCfg->wSlotH + 1;
	pCfg->wInfoV = nClientV - ( (nInterval*3) + pCfg->wSlotV );

	pCfg->pNextBlk = get_block( rand() );

	return( 0 );
}

void set_cur_block( TTBlockArrayStt *pBlock )
{
	tt_cfg.cur_blk.pBlk = pBlock;
}

void set_state( int nState )
{ 
   tt_cfg.nState = nState;
}

int get_state( int nState )
{ 
   return( tt_cfg.nState );
}

int is_stack_overlap( TTBlockArrayStt *pBlk, int nXPos, int nYPos )
{
	int nY, nX;

	for( nY = 0; nY < TT_BLOCK_V; nY++ )
	{
		for( nX = 0; nX < TT_BLOCK_H; nX++ )
		{
			if( tt_cfg.s[ nY + nYPos ][ nX + nXPos ] != 0 && pBlk->b[nY][nX] != 0 )
				return( 1 );	// 겹친다.
		}
	}
	// 겹치지 않는다.
	return( 0 );
}

int is_left_overlap( TTBlockStt *pBlock, int nXPos )
{
	int nX, nY, nH, nResult;

	if( pBlock == NULL || pBlock->pBlk == NULL )
		return( 0 );

	// stack된 다른 블록과 겹치는지 확인한다.
	if( is_stack_overlap( pBlock->pBlk, nXPos, pBlock->nRealY ) != 0 )
		return( 1 );

	nH = nXPos * (-1);

	nResult = 0;
	for( nY = 0; nY < TT_BLOCK_V; nY++ )
	{
		for( nX = 0; nX < nH; nX++ )
			nResult += pBlock->pBlk->b[nY][nX];
	}
	if( nResult != 0 )
		return( 1 );		// 겹친다.
	
	return( 0 );
}

// 블록이 오른쪽으로 겹치는지 확인한다.
int is_right_overlap( TTBlockStt *pBlock, int nXPos )
{
	int nX, nY, nLength;

	if( pBlock == NULL || pBlock->pBlk == NULL )
		return( 0 );

	// stack된 다른 블록과 겹치는지 확인한다.
	if( is_stack_overlap( pBlock->pBlk, nXPos, pBlock->nRealY ) != 0 )
		return( 1 );

	// H Length를 구한다.
	nLength = 1;
	for( nY = 0; nY < TT_BLOCK_V; nY++ )
	{
		for( nX = TT_BLOCK_H-1; nX >= 0; nX-- )
		{
			if( pBlock->pBlk->b[nY][nX] != 0 )
				break;
		}
		if( nX > nLength )
			nLength = nX;
	}

	// Length가 Slot의 H 길이를 벗어나는가?
	if( nXPos + nLength >= (int)tt_cfg.wHLine )
		return( 1 );		// 겹친다.

	return( 0 );
}

int stack_block( TTBlockStt *pBlock, int nXPos, int nYPos )
{
	int nX, nY;

	if( pBlock == NULL || pBlock->pBlk == NULL )
		return( -1 );

	// 블록을 쌓는다.
	for( nY = 0; nY < TT_BLOCK_V; nY++ )
	{
		if( nY + nYPos >= TT_LINE_V )
			break;

		for( nX = 0; nX < TT_BLOCK_H; nX++ )
		{
			if( nX + nXPos < 0 || nX + nXPos >= TT_LINE_H )
				continue;

			if( pBlock->pBlk->b[nY][nX] != 0 )
				tt_cfg.s[ nY + nYPos ][ nX + nXPos ] = pBlock->pBlk->b[nY][nX];
		}
	}

	return( 0 );
}

int is_down_overlap( TTBlockArrayStt *pBlk, int nXPos, int nYPos )
{
	int nX, nY, nLength;

	if( pBlk == NULL )
		return( 0 );

	// 블록의 V Length를 구한다.
	nLength = 0;
	for( nX = 0; nX < TT_BLOCK_H; nX++ )
	{
		for( nY = TT_BLOCK_V-1; nY >= 0; nY-- )
		{
			if( pBlk->b[nY][nX] != 0 )
				break;
		}
		if( nY > nLength )
			nLength = nY;
	}

	// Length가 Slot의 V 길이를 벗어나는가?
	if( nYPos + nLength >= (int)tt_cfg.wVLine )
		return( 1 );   // 겹친다.
		
	// 슬롯에 쌓인 블록들과 겹치는지 확인한다.
	if( is_stack_overlap( pBlk, nXPos, nYPos ) != 0 )
		return( 1 );

	// 겹치지 않는다.
	return( 0 );
}

static int move_line( int nDestY, int nSrcY )
{
	int nX;

	for( nX = 0; nX < TT_LINE_H; nX++ )
		tt_cfg.s[nDestY][nX] = tt_cfg.s[nSrcY][nX];

	return( 0 );
}	

// 연결된 하나의 라인을 지운다.
int eat_line()
{
	int nY, nX, nK;

	for( nY = TT_LINE_V-1; nY >= 0; nY-- )
	{
		for( nX = 0; ; nX++ )
		{
			if( nX >= TT_LINE_H )
			{	// 한 라인 전체가 1이다.
				for( nK = nY; nK > 0; nK-- )
					move_line( nK, nK-1 );
				
				// 가장 위쪽 라인을 0으로 만든다.
				for( nK = 0; nK < TT_LINE_H; nK++ )
					tt_cfg.s[0][nK] = 0;

				nY++;
				break;
			}

			if( tt_cfg.s[nY][nX] == 0 )
				break;
		}
	}

	return( 0 );
}

int fill_cell( DWORD dwHandle, int nX, int nY, DWORD dwColor )
{
	RectStt	r;
			
	r.nX = tt_cfg.wSlotX + ( nX * tt_cfg.wCellH );
	r.nY = tt_cfg.wSlotY + ( nY * tt_cfg.wCellV );
	r.nH = tt_cfg.wCellH;
	r.nV = tt_cfg.wCellV;
	if( G_pBkImg == NULL )
		fill_rect32( dwHandle,	(RectStt*)&r, dwColor );
	else
		fill_rect32_or( dwHandle,  (RectStt*)&r, dwColor );

	return( 0 );
}

int fill_cell_ex( DWORD dwHandle, int nXPoint, int nYPoint, DWORD dwColor )
{
	RectStt	r;
			
	r.nX = nXPoint;
	r.nY = nYPoint;
	r.nH = tt_cfg.wCellH;
	r.nV = tt_cfg.wCellV;
	if( G_pBkImg == NULL )
		fill_rect32( dwHandle,  (RectStt*)&r, dwColor );
	else
		fill_rect32_or( dwHandle,  (RectStt*)&r, dwColor );

	return( 0 );
}

void disp_block( DWORD dwHandle, TTBlockStt *pBlock )
{
	int nH, nV, nX, nY;

	if( pBlock == NULL || pBlock->pBlk == NULL )
		return;

	nX = pBlock->nModifiedX;
	nY = pBlock->nModifiedY;
	for( nV = 0; nV < TT_BLOCK_V; nV++ )
	{
		for( nH = 0; nH < TT_BLOCK_V; nH++ )
		{
			if( pBlock->pBlk->b[nV][nH] == 0 )
				continue;

			if( nX + nH >= 0 )
				fill_cell( dwHandle, nX+nH, nY+nV, COLOR_BLOCK );
		}
	}
}

void disp_block_ex( DWORD dwHandle, TTBlockArrayStt *pBlk, int nXPoint, int nYPoint )
{
	int nH, nV, nX, nY;

	if( pBlk == NULL )
		return;

	nX = nXPoint;
	nY = nYPoint;

	for( nV = 0; nV < TT_BLOCK_V; nV++ )
	{
		for( nH = 0; nH < TT_BLOCK_V; nH++ )
		{
			if( pBlk->b[nV][nH] == 0 )
				continue;

			fill_cell_ex( dwHandle, nX+(nH*tt_cfg.wCellH), nY+(nV*tt_cfg.wCellV), COLOR_BLOCK );
		}
	}
}

int remake_slot( DWORD dwHandle )
{
	RectStt		r;
	TTBlockStt	*pBlock;
	UINT16		wTextColor;
	int 		nX, nY, nOrFlag;

	// dc 배경을 흰색으로 지운다.
	if( G_pBkImg == NULL )
	{
		fill_rect32( dwHandle, &tt_cfg.view_r, COLOR_WHITE );

		// info 배경을 녹색으로 지운다.
		r.nX = tt_cfg.wInfoX;
		r.nY = tt_cfg.wInfoY;
		r.nH = tt_cfg.wInfoH;
		r.nV = tt_cfg.wInfoV;
		fill_rect32( dwHandle, (RectStt*)&r, COLOR_INFO );
	
		// 슬롯을 그린다.
		r.nX = tt_cfg.wSlotX;
		r.nY = tt_cfg.wSlotY;
		r.nH = tt_cfg.wSlotH;
		r.nV = tt_cfg.wSlotV;
		fill_rect32( dwHandle, (RectStt*)&r, COLOR_SLOT );

		wTextColor = 0;			// 문자색 = 검정색.
		nOrFlag    = 0;
	}
	else
	{
		gx_copy_image16( dwHandle, G_pBkImg, tt_cfg.view_r.nX, tt_cfg.view_r.nY, tt_cfg.view_r.nH, tt_cfg.view_r.nV,
			0, 0 );
		wTextColor = 0xFFFF;	// 문자색 = 흰색.
		nOrFlag    = 1;
	}

	pBlock = &tt_cfg.cur_blk;

	// 세로 선을 그린다
	nY = tt_cfg.wSlotY;
	for( nX = tt_cfg.wSlotX; nX <= tt_cfg.wSlotH+tt_cfg.wSlotX; nX+= tt_cfg.wCellH )
	{
		gx_draw_line( dwHandle, nX, nY, nX, nY+ tt_cfg.wSlotV, RGB_BROWN16, nOrFlag );
	}  

	// 가로 선을 그린다. (가장 위와 가장 아래의 선만 그린다.)
	nX = tt_cfg.wSlotX;
	for( nY = tt_cfg.wSlotY; nY <= tt_cfg.wSlotV+tt_cfg.wSlotY; nY+= tt_cfg.wCellV )
	{
		gx_draw_line( dwHandle, nX, nY, nX+tt_cfg.wSlotH, nY, RGB_BROWN16, nOrFlag );
	}

	// right overlap을 체크한다.
	pBlock->nModifiedX = pBlock->nRealX;
	pBlock->nModifiedY = pBlock->nRealY;

	for( ; is_left_overlap( pBlock, pBlock->nModifiedX ); )
		pBlock->nModifiedX++;

	for( ; is_right_overlap( pBlock, pBlock->nModifiedX ); )
		pBlock->nModifiedX--;

	// current block을 그린다.
	disp_block( dwHandle, pBlock );

	// next block을 그린다.
	if( tt_cfg.pNextBlk != NULL )
		disp_block_ex( dwHandle, tt_cfg.pNextBlk, tt_cfg.wInfoX+5, tt_cfg.wInfoY+5 );
	
	// stack된 블록들을 그린다.
	for( nY = 0; nY < TT_LINE_V; nY++ )
	{
		for( nX = 0; nX < TT_LINE_H; nX++ )
		{
			if( tt_cfg.s[nY][nX] != 0 )
				fill_cell( dwHandle, nX, nY, COLOR_STACK );
		}
	}

	// GAME OVER 상태이면 GAME OVER를 출력한다.
	if( tt_cfg.nState == TT_STATE_GAME_OVER )
	{
		gx_drawtext_xy( dwHandle, tt_cfg.view_r.nX+10, tt_cfg.view_r.nY+10, TT_FONT_ID, "GAME OVER", wTextColor, 0 );
		gx_drawtext_xy( dwHandle, tt_cfg.view_r.nX+10, tt_cfg.view_r.nY+30, TT_FONT_ID, "[ENTER] to Start", wTextColor, 0 );
	}
	else if( tt_cfg.nState == TT_STATE_PAUSED )
	{
		gx_drawtext_xy( dwHandle, tt_cfg.view_r.nX+10, tt_cfg.view_r.nY+10, TT_FONT_ID, "PAUSED", wTextColor, 0 );
		gx_drawtext_xy( dwHandle, tt_cfg.view_r.nX+10, tt_cfg.view_r.nY+30, TT_FONT_ID, "[ENTER] to Restart", wTextColor, 0 );
	}

	return( 0 );
}

int block_rotate( DWORD dwHandle )
{
	TTBlockStt	*pBlock;

	pBlock = &tt_cfg.cur_blk;
	if( pBlock->pBlk == NULL )
		return( 0 );

	if( is_down_overlap( pBlock->pBlk->pNext, pBlock->nModifiedX, pBlock->nModifiedY ) )
		return( 0 );	// overlap되므로 rotate할 수 없다.

	pBlock->pBlk = pBlock->pBlk->pNext;

	// 슬롯을 재구성한 후 다시 그린다.
	remake_slot( dwHandle );
	flush_slot( dwHandle );

	return( 0 );
}

int block_left( DWORD dwHandle )
{
	TTBlockStt	*pBlock;

	pBlock = &tt_cfg.cur_blk;
	if( pBlock->pBlk == NULL )
		return( 0 );

	if( pBlock->nModifiedX > 0 )
		pBlock->nRealX = pBlock->nModifiedX -1;
	else
	{
		if( is_left_overlap( pBlock, pBlock->nModifiedX -1 ) != 0 )
			return( 0 );

		pBlock->nRealX = pBlock->nModifiedX -1;
	}

	// 슬롯을 재구성한 후 다시 그린다.
	remake_slot( dwHandle );
	flush_slot( dwHandle );

	return( 0 );
}

int block_right( DWORD dwHandle )
{
	TTBlockStt	*pBlock;

	pBlock = &tt_cfg.cur_blk;
	if( pBlock->pBlk == NULL )
		return( 0 );

	if( is_right_overlap( pBlock, pBlock->nModifiedX + 1 ) != 0 )
		return( 0 );	// 겹친다.

	pBlock->nRealX = pBlock->nModifiedX + 1;

	// 슬롯을 재구성한 후 다시 그린다.
	remake_slot( dwHandle );
	flush_slot( dwHandle );

	return( 0 );
}

int block_down( DWORD dwHandle, int nStep )
{
	int			nR, nY, nOverlapped;
	TTBlockStt	*pBlock;

	pBlock = &tt_cfg.cur_blk;
	if( pBlock->pBlk == NULL )
		return( 0 );

	nOverlapped = 0;
	for( nY = 0; nY < nStep; nY++ )
	{
		if( is_down_overlap( pBlock->pBlk, pBlock->nModifiedX, pBlock->nModifiedY + 1) != 0 )
		{	// overlap 되었다.
			nOverlapped = 1;
			break;
		}

		pBlock->nModifiedY++;
	}

	pBlock->nRealX = pBlock->nModifiedX;
	pBlock->nRealY = pBlock->nModifiedY;
	
	// 블록을 쌓는다.
	if( nOverlapped != 0 )
	{
		stack_block( pBlock, pBlock->nRealX, pBlock->nRealY );

		// 라인 전체가 이어진 것은 제거한다.
		nR = eat_line();
		if( nR > 0 )
			tt_cfg.nDeletedLine += nR;

		pBlock->pBlk = NULL;

		// 블록을 쌍은 후에는 즉시 다음 블록이 등장하도록 한다.
		tt_cfg.nTimerCount = 100000;
	}

	// 슬롯을 재구성한 후 다시 그린다.
	remake_slot( dwHandle );
	flush_slot( dwHandle );

	return( 0 );
}

// 일정 시간 간격으로 타이머 핸들러에서 호출된다.
int slot_feed_timer( DWORD dwHandle )
{
	TTBlockStt *pBlock;

	pBlock = &tt_cfg.cur_blk;

	if( pBlock->pBlk == NULL )
	{	// 새로운 블록을 생성한다.
		pBlock->nModifiedX = (TT_LINE_H - TT_BLOCK_H) / 2;
		pBlock->nModifiedY = 0;
		pBlock->nRealX = (TT_LINE_H - TT_BLOCK_H) / 2;
		pBlock->nRealY = 0;
		pBlock->pBlk = get_block( rand() );

		// 생성하자 마자 overlap되는지 확인한다.
		if( is_down_overlap( pBlock->pBlk, pBlock->nRealX, pBlock->nRealY ) != 0 )
		{	// Game Over로 처리한다.
			tt_cfg.nState = TT_STATE_GAME_OVER;
		}
	}
	else
	{	// 블록을 한 칸 내린다.
		block_down( dwHandle, 1 );
	}

	// 슬롯을 재구성한 후 다시 그린다.
	remake_slot( dwHandle );
	flush_slot( dwHandle );

	return( 0 );
}	

int block_start( DWORD dwHandle )
{
	if( tt_cfg.nState != TT_STATE_GAME_OVER && tt_cfg.nState != TT_STATE_PAUSED )
		return( 0 );
	
	set_state( TT_STATE_RUNNING );
	
	tt_cfg.nLevel		= 0;
	tt_cfg.cur_blk.pBlk = NULL;
	tt_cfg.nDeletedLine = 0;
	memset( tt_cfg.s, 0, sizeof( tt_cfg.s ) );

	// 슬롯을 재구성한 후 다시 그린다.
	remake_slot( dwHandle );
	flush_slot( dwHandle );

	return( 0 );
}

int block_pause( DWORD dwHandle )
{
	if( tt_cfg.nState != TT_STATE_RUNNING )
		return( 0 );
	
	set_state( TT_STATE_PAUSED );

	// 슬롯을 재구성한 후 다시 그린다.
	remake_slot( dwHandle );
	flush_slot( dwHandle );

	return( 0 );
}
