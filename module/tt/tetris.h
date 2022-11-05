#ifndef TT_GLOBAL_HEADER_jj
#define TT_GLOBAL_HEADER_jj

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32
	#include <types.h>
	#include <stdio.h>
	#include <io.h>
	#include <stdlib.h>
	#include <conio.h>
	#include <grx.h>
	#include <process.h>
	#include <funckey.h>
	#include "..\gui\resource.h"

	#define TT_INTERVAL		3		// B2OS 용에서만 사용한다.
	#define TT_WIN_H		150
	#define TT_WIN_V		320
	#define TT_INFO_V		 40
	#define TT_FONT_ID		IDR_BF_BASE11
#endif
#ifdef WIN32
	typedef struct {
		int nX, nY, nH, nV;
	} RectStt; 
	#define RGB32 RGB
	#define TT_WIN_H		150
	#define TT_WIN_V		380
	#define TT_INFO_V		 40
	#define TT_FONT_ID		  0
#endif


/////////////////////////////////////////////
#define RGB_BROWN16 		RGB16(155,135,150)
#define RGB_BROWN			RGB32(105, 90,100)
#define COLOR_BLOCK 		RGB32(165,95,240)
#define COLOR_STACK 		RGB32(90,90,217)
#define COLOR_SLOT          RGB32(222,208,224)//RGB32(203,179,214)
#define COLOR_INFO          RGB32(212,168,230)
#define COLOR_WHITE			RGB32(255,255,255)
/////////////////////////////////////////////

#define TT_LINE_H		12
#define TT_LINE_V		20

#define TT_BLOCK_H		4
#define TT_BLOCK_V		4

#define TT_MAX_LEVEL	5
#define TT_TIMER_INTERVAL	100

#define TETRIS_GUI_TIMER_ID	5554

typedef struct TTBlockArrayTag {
	struct TTBlockArrayTag		*pNext;
	BYTE				b[TT_BLOCK_V][TT_BLOCK_H];
} TTBlockArrayStt;

typedef struct TTBlockTag {
	int				nRealX,		nRealY;
	int				nModifiedX, nModifiedY;
	TTBlockArrayStt	*pBlk;	
} TTBlockStt;

#define TT_STATE_GAME_OVER		0
#define TT_STATE_RUNNING		1
#define TT_STATE_PAUSED			2


typedef struct TTCfgTag {
	RectStt			view_r;
	int				nState;			// 슬롯의 현재 상태.
	int				nLevel;			// 레벨
	int				nDeletedLine;	// 처리된 라인 수
	int				nTimerCount;
	unsigned short 	wHLine;			// H 셀의 개수		 // H, V 셀의 개수에 의해 Pixel 크기가 결정됨.
	unsigned short 	wVLine;			// V 셀의 개수
	unsigned short 	wCellH;			// 셀의 H Pixel
	unsigned short 	wCellV;			// 셀의 V Pixel
	
	unsigned short	wSlotX;			//
	unsigned short	wSlotY;			// 슬롯의 X, Y, H, V
	unsigned short	wSlotH;			//
	unsigned short  wSlotV;			//
	unsigned short  wClientH;
	unsigned short  wClientV;

	unsigned short  wInfoX;
	unsigned short  wInfoY;
	unsigned short  wInfoH;
	unsigned short  wInfoV;

	struct TTBlockArrayTag	*pNextBlk;

	TTBlockStt		cur_blk;
	BYTE			s[TT_LINE_V][TT_LINE_H];
} TTCfgStt;

extern int			eat_line			();
extern int			init_tt_cfg			( TTCfgStt *pCfg, RectStt *pR );
extern int			stack_block			( TTBlockStt *pBlock, int nXPos, int nYPos );
extern int			is_stack_overlap	( TTBlockArrayStt *pBlk, int nXPos, int nYPos );
extern int			is_right_overlap	( TTBlockStt *pBlock, int nXPos );
extern int			is_left_overlap		( TTBlockStt *pBlock, int nXPos );
extern int			is_down_overlap		( TTBlockArrayStt *pBlk, int nXPos, int nYPos );;
extern int			fill_cell			( DWORD dwHandle, int nX, int nY, DWORD dwColor );
extern int			fill_cell_ex		( DWORD dwHandle, int nXPoint, int nYPoint, DWORD dwColor );
extern int			remake_slot			( DWORD dwHandle );
extern int			block_rotate		( DWORD dwHandle );
extern int			block_left			( DWORD dwHandle );
extern int			block_right			( DWORD dwHandle );
extern int			block_down			( DWORD dwHandle, int nStep );
extern int			block_start			( DWORD dwHandle );
extern int			block_pause			( DWORD dwHandle );
extern int			slot_feed_timer		( DWORD dwHandle );
extern int			get_state			();

extern void			set_cur_block		( TTBlockArrayStt *pBlock );
extern void			set_state			( int nState );

extern struct TTBlockArrayTag	*get_block			( int nIndex );

extern ImageStt *G_pBkImg;

#ifdef __cplusplus
}
#endif


#endif
