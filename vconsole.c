#include "bellona2.h"

static VConsoleStt	*G_pKernelVCon = NULL;
static SysVConsoleStt sys_vcon;

// 커널이 사용하는 가상 콘솔을 리턴한다.
void *get_kernel_vconsole()
{
	return( G_pKernelVCon );
}

// 시스템의 가상 콘솔 자료구조를 초기화 한다.
int init_sys_vconsole()
{
	memset( &sys_vcon, 0, sizeof( sys_vcon ) );

	sys_vcon.pp_vcon = (VConsoleStt**)kmalloc( 4 * MAX_VCONSOLE );
	if( sys_vcon.pp_vcon == NULL )
		return( -1 );
	memset( sys_vcon.pp_vcon, 0, 4 * MAX_VCONSOLE );

	sys_vcon.nMax = MAX_VCONSOLE;
	sys_vcon.nNextID = 100;

	// 커널이 사용할 vconsole 하나를 reserve 한다.
	G_pKernelVCon = make_vconsole();
	sys_vcon.pActive = G_pKernelVCon;
	
	return( 0 );
}

// 가상 콘솔을 할당한다.
void *make_vconsole()
{
	int			nI;
	VConsoleStt *pVC;

	if( sys_vcon.nTotal >= sys_vcon.nMax )
	{	// 더이상 할당할 수 없다.
		kdbg_printf( "make_vconsole: no available slot!\n" );
		return( NULL );
	}							 

	// 메모리를 할당한 후 '0'으로 초기화 한다.
	pVC = (VConsoleStt*)kmalloc( sizeof( VConsoleStt ) );
	if( pVC == NULL )
	{
		kdbg_printf( "make_vconsole: kmalloc failed!\n" );
		return( NULL );
	}
	memset( pVC, 0, sizeof( VConsoleStt ) );
	// 버퍼의 속성을 설정한다.
	for( nI = 0; nI < VCONSOLE_H * VCONSOLE_V; nI++ )
	{
		pVC->con_buff[nI].ch = ' ';
		pVC->con_buff[nI].attr = 7;
	}

	// ID를 설정한다.
	pVC->nID      = sys_vcon.nNextID;
	pVC->byLInes  = 50;						// 기본적으로 50라인으로 설정한다.
	sys_vcon.nNextID++;

	// 빈 슬롯에 포인터를 저장한다.
	for( nI = 0; nI < sys_vcon.nMax; nI++ )
	{
		if( sys_vcon.pp_vcon[nI] == NULL )
		{
			sys_vcon.pp_vcon[nI] = pVC;
			break;
		}
	}

	// 가상 콘솔의 개수를 증가시킨다.
	sys_vcon.nTotal++;

	return( pVC );
}

// 가상 콘솔을 제거한다.
int close_vconsole( void *pVConsole )
{
	int		nI;

	if( pVConsole == NULL )
		return( -1 );

	for( nI = 0; ; nI++ )
	{
		if( nI >= sys_vcon.nMax )
			return( -1 );

		if( sys_vcon.pp_vcon[nI] == pVConsole )
		{
			sys_vcon.pp_vcon[nI] = NULL;
			break;
		}
	}

	// active console이면 NULL로 설정한다.
	if( sys_vcon.pActive == pVConsole )
		sys_vcon.pActive = NULL;

	kfree( pVConsole );
	sys_vcon.nTotal--;

	return( 0 );
}

// 프로세스의 현재 가상 콘솔을 구한다.
void *get_current_vconsole()
{
	ProcessStt *pP;

	pP = k_get_current_process();
	if( pP == NULL )
		return( NULL );

	return( pP->pVConsole );
}

// 프로세스의 현재 가상 콘솔을 설정한다.
int set_current_vconsole( void *pVConsole )
{
	ProcessStt *pP;

	pP = k_get_current_process();
	if( pP == NULL )
		return( -1 );

	pP->pVConsole = pVConsole;
	return( 0 );
}

// 현재 활성화된 가상 콘솔을 구한다.
void *get_active_vconsole()
{
	return( sys_vcon.pActive );
}

// 가상 콘솔의 커서를 움직인다.
void move_vconsole_cursor( DWORD dwOffs, void *pVC )
{
	VConsoleStt *pVCon;

	if( pVC == NULL )
		return;

	pVCon = (VConsoleStt*)pVC;
	pVCon->wCursorX = (UINT16)( dwOffs % 80 );
	pVCon->wCursorY = (UINT16)( dwOffs / 80 );
}

int is_active_vconsole( void *pVC )
{
	if( pVC != NULL && pVC == sys_vcon.pActive )
		return( 1 );

	return( 0 );
}

// 지정된 가상 콘솔을 활성화 시킨다.
int set_active_vconsole( void *pVConsole )
{
	BYTE		*pSrc, *pDest;
	VConsoleStt *pVC;
	int			nI, nLines;

	sys_vcon.pActive = pVConsole;
	if( pVConsole == NULL )
		return( 0 );

	pVC = (VConsoleStt*)pVConsole;

	// 라인의 개수가 다르면 화면 모드를 변환한다.  (V86Lib가 로드되어 있어야 한다.)
	nLines = get_vertical_line_size();
	if( nLines != (int)pVC->byLInes )
		lines_xx( (int)pVC->byLInes );
	
	// 커서 위치를 새로 설정한다.
	set_cursor_xy( pVC->wCursorX, pVC->wCursorY );

	if( is_gui_mode() != 0 )
	{
		// 커서 위치를 설정한다.
		gui_set_cursor_xy( pVC->wCursorX, pVC->wCursorY );

		// GUI 쪽으로 플러싱 한다.
		gui_flushing( pVC );
	}
	else
	{	// 비디오 메모리를 복사한다.
		pDest = (BYTE*)0xB8000;
		pSrc  = (BYTE*)pVC->con_buff;
		for( nI = 0; nI < (int)pVC->byLInes; nI++ )
		{
			memcpy( pDest, pSrc, 160 );

			pSrc  = (BYTE*)( (DWORD)pSrc  + 160 );
			pDest = (BYTE*)( (DWORD)pDest + 160 );
		}
	}
	
	return( 0 );
}

// 가상 콘솔의 목록을 출력한다.
int disp_vconsole()
{
	int				nPID;
	VConsoleStt		*pVC;
	int				nI, nTotal;

	if( sys_vcon.pp_vcon == NULL )
		return( -1 );

	kdbg_printf( "     VConID  FG-PID   X   Y   Lines\n" );
	nTotal = 0;
	for( nI = 0; nI < sys_vcon.nMax; nI++ )
	{
		pVC = sys_vcon.pp_vcon[nI];
		if( pVC == NULL )
			continue;

		if( pVC->pStartFg == NULL )
			nPID = 0;
		else
			nPID = (int)pVC->pStartFg->dwID;

		if( pVC == sys_vcon.pActive )
			kdbg_printf( "[%2d] *%3d     %3d   %3d %3d    %3d\n", nTotal, pVC->nID, nPID, pVC->wCursorX, pVC->wCursorY, pVC->byLInes  );
		else
			kdbg_printf( "[%2d]  %3d     %3d   %3d %3d    %3d\n", nTotal, pVC->nID, nPID, pVC->wCursorX, pVC->wCursorX, pVC->byLInes );

		nTotal++;
	}

	return( 0 );
}

// 지정 프로세스가 자신의 가상 콘솔에서 FG로 설정되어 있는가?
int is_fg_process( ProcessStt *pP )
{
	if( pP == NULL || pP->pVConsole == NULL )
		return( 0 );
	
	if( pP == pP->pVConsole->pStartFg )
		return( 1 );
	
	return( 0 );
}

// FG 링크에서 프로세스가 존재하는지 찾는다.
static int find_fg_link( VConsoleStt *pVC, ProcessStt *pP )
{
	ProcessStt *pProc;

	if( pP == NULL || pVC == NULL )
		return( 0 );

	for( pProc = pVC->pStartFg; pProc != NULL; pProc = pProc->pNextFG )
	{
		if( pProc == pP )
			return( 1 );		// 링크에서 찾았다.
	}					

	return( 0 );
}

// 프로세스를 FG 링크에서 제거한다.
int del_fg_link( VConsoleStt *pVC, ProcessStt *pP )
{
	if( pP == NULL || pVC == NULL )
	{
		//kdbg_printf( "del_fg_link: pP == NULL || pVC == NULL\n" );
		return( 0 );
	}

	// 존재하지 않는지 확인한다.
	if( find_fg_link( pVC, pP ) == 0 )
	{
		//kdbg_printf( "del_fg_link(VC=%d): PID(%d) not found\n", pVC->nID, pP->dwID );
		return( 0 );	
	}
	
	if( pP->pPreFG != NULL )
		pP->pPreFG->pNextFG = pP->pNextFG;
	else
		pVC->pStartFg = pP->pNextFG;

	if( pP->pNextFG != NULL )
		pP->pNextFG->pPreFG = pP->pPreFG;
	else
		pVC->pEndFg = pP->pPreFG;

	pP->pPreFG = pP->pNextFG = NULL;

	//kdbg_printf( "del_fg_link(VC=%d): PID(%d) ok.\n", pVC->nID, pP->dwID );

	return( 0 );
}				

// 가상 콘솔의 FG 프로세스를 설정한다.
int set_fg_process( VConsoleStt *pVC, struct ProcessTag *pP )
{
	if( pP == NULL || pP->pVConsole == NULL )
		return( -1 );
	
	if( pVC == NULL )
		pVC = pP->pVConsole;

	// 이미 FG 링크에 포함되어 있던 것이면 링크에서 제거한다.
	del_fg_link( pVC, pP );

	// 링크의 가장 앞쪽에 연결한다.
	pP->pPreFG  = NULL;
	pP->pNextFG = pVC->pStartFg;
	if( pVC->pStartFg == NULL )
		pVC->pEndFg = pP;
	else
		pVC->pStartFg->pPreFG = pP;
	pVC->pStartFg = pP;

	/*
	{ // 디버깅을 위해 현재 FG 링크의 PID를 출력한다.
		ProcessStt *pPF;

		kdbg_printf( "FG Link(VC=%d): ", pVC->nID );
		for( pPF = pVC->pStartFg; pPF != NULL; pPF = pPF->pNextFG )
		{
			kdbg_printf( "(%d) ", pPF->dwID );
		}
		kdbg_printf( "\n" );
	}
	*/

	return( 0 );
}

// 가상 콘솔을 전환한다.
int change_next_vconsole()
{
	VConsoleStt		*pVC;
	int				nI, nTotal;

	if( sys_vcon.pp_vcon == NULL )
		return( -1 );

	nI = 0;
	// 현재 프로세스의 가상 콘솔을 구한다.
	pVC = get_active_vconsole();
	if( pVC != NULL )
	{
		for( nI = 0; ; nI++ )
		{
			if( nI >= sys_vcon.nMax )
				break;	 // 현재 VConsole의 인덱스를 찾을 수 없다.
			if( sys_vcon.pp_vcon[nI] == pVC )
			{
				nI++;	 // 현재 VConsole의 인덱스를 찾았다.
				break;
			}
		}
	}

	if( nI >= sys_vcon.nMax )
		nI = 0;
		
	for( nTotal = 0; nTotal < sys_vcon.nMax; nTotal++, nI++ )
	{
		if( nI >= sys_vcon.nMax )
			nI = 0;

		if( sys_vcon.pp_vcon[nI] != NULL )
			break;
	}

	// 새로 Active VConsole을 설정한다.
	if( sys_vcon.pp_vcon[nI] != NULL && pVC != sys_vcon.pp_vcon[nI] )
		set_active_vconsole( sys_vcon.pp_vcon[nI] );

	return( 0 );
}

static GuiConsoleFuncStt *G_pGCTbl = NULL;

void set_gui_console_ftbl( void *pTbl )
{
	G_pGCTbl = pTbl;
}		  

int gui_write( char *pS )
{
	if( G_pGCTbl == NULL )
		return( -1 );
		
	if( G_pGCTbl->pWrite == NULL )
		return( 0 );

	G_pGCTbl->pWrite( G_pGCTbl->pWin, pS );

	return( 0 );
}

int gui_direct_write( char *pS, int nX )
{
	if( G_pGCTbl == NULL )
		return( -1 );
		
	if( G_pGCTbl->pDirectWrite == NULL )
		return( 0 );

	G_pGCTbl->pDirectWrite( G_pGCTbl->pWin, pS, nX );

	return( 0 );
}

int gui_set_cursor_xy( int nX, int nY )
{
	if( G_pGCTbl == NULL )
		return( -1 );
		
	if( G_pGCTbl->pCursorXY == NULL )
		return( 0 );

	G_pGCTbl->pCursorXY( G_pGCTbl->pWin, nX, nY );

	return( 0 );
}

int gui_cls( int nY )
{
	if( G_pGCTbl == NULL )
		return( -1 );
		
	if( G_pGCTbl->pCls == NULL )
		return( 0 );

	G_pGCTbl->pCls( G_pGCTbl->pWin, nY );

	return( 0 );
}

int gui_flushing( VConsoleStt *pVC )
{
	if( G_pGCTbl == NULL )
		return( -1 );
		
	if( G_pGCTbl->pFlushing == NULL )
		return( 0 );

	G_pGCTbl->pFlushing( G_pGCTbl->pWin, pVC );

	return( 0 );
}
