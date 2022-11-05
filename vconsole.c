#include "bellona2.h"

static VConsoleStt	*G_pKernelVCon = NULL;
static SysVConsoleStt sys_vcon;

// Ŀ���� ����ϴ� ���� �ܼ��� �����Ѵ�.
void *get_kernel_vconsole()
{
	return( G_pKernelVCon );
}

// �ý����� ���� �ܼ� �ڷᱸ���� �ʱ�ȭ �Ѵ�.
int init_sys_vconsole()
{
	memset( &sys_vcon, 0, sizeof( sys_vcon ) );

	sys_vcon.pp_vcon = (VConsoleStt**)kmalloc( 4 * MAX_VCONSOLE );
	if( sys_vcon.pp_vcon == NULL )
		return( -1 );
	memset( sys_vcon.pp_vcon, 0, 4 * MAX_VCONSOLE );

	sys_vcon.nMax = MAX_VCONSOLE;
	sys_vcon.nNextID = 100;

	// Ŀ���� ����� vconsole �ϳ��� reserve �Ѵ�.
	G_pKernelVCon = make_vconsole();
	sys_vcon.pActive = G_pKernelVCon;
	
	return( 0 );
}

// ���� �ܼ��� �Ҵ��Ѵ�.
void *make_vconsole()
{
	int			nI;
	VConsoleStt *pVC;

	if( sys_vcon.nTotal >= sys_vcon.nMax )
	{	// ���̻� �Ҵ��� �� ����.
		kdbg_printf( "make_vconsole: no available slot!\n" );
		return( NULL );
	}							 

	// �޸𸮸� �Ҵ��� �� '0'���� �ʱ�ȭ �Ѵ�.
	pVC = (VConsoleStt*)kmalloc( sizeof( VConsoleStt ) );
	if( pVC == NULL )
	{
		kdbg_printf( "make_vconsole: kmalloc failed!\n" );
		return( NULL );
	}
	memset( pVC, 0, sizeof( VConsoleStt ) );
	// ������ �Ӽ��� �����Ѵ�.
	for( nI = 0; nI < VCONSOLE_H * VCONSOLE_V; nI++ )
	{
		pVC->con_buff[nI].ch = ' ';
		pVC->con_buff[nI].attr = 7;
	}

	// ID�� �����Ѵ�.
	pVC->nID      = sys_vcon.nNextID;
	pVC->byLInes  = 50;						// �⺻������ 50�������� �����Ѵ�.
	sys_vcon.nNextID++;

	// �� ���Կ� �����͸� �����Ѵ�.
	for( nI = 0; nI < sys_vcon.nMax; nI++ )
	{
		if( sys_vcon.pp_vcon[nI] == NULL )
		{
			sys_vcon.pp_vcon[nI] = pVC;
			break;
		}
	}

	// ���� �ܼ��� ������ ������Ų��.
	sys_vcon.nTotal++;

	return( pVC );
}

// ���� �ܼ��� �����Ѵ�.
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

	// active console�̸� NULL�� �����Ѵ�.
	if( sys_vcon.pActive == pVConsole )
		sys_vcon.pActive = NULL;

	kfree( pVConsole );
	sys_vcon.nTotal--;

	return( 0 );
}

// ���μ����� ���� ���� �ܼ��� ���Ѵ�.
void *get_current_vconsole()
{
	ProcessStt *pP;

	pP = k_get_current_process();
	if( pP == NULL )
		return( NULL );

	return( pP->pVConsole );
}

// ���μ����� ���� ���� �ܼ��� �����Ѵ�.
int set_current_vconsole( void *pVConsole )
{
	ProcessStt *pP;

	pP = k_get_current_process();
	if( pP == NULL )
		return( -1 );

	pP->pVConsole = pVConsole;
	return( 0 );
}

// ���� Ȱ��ȭ�� ���� �ܼ��� ���Ѵ�.
void *get_active_vconsole()
{
	return( sys_vcon.pActive );
}

// ���� �ܼ��� Ŀ���� �����δ�.
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

// ������ ���� �ܼ��� Ȱ��ȭ ��Ų��.
int set_active_vconsole( void *pVConsole )
{
	BYTE		*pSrc, *pDest;
	VConsoleStt *pVC;
	int			nI, nLines;

	sys_vcon.pActive = pVConsole;
	if( pVConsole == NULL )
		return( 0 );

	pVC = (VConsoleStt*)pVConsole;

	// ������ ������ �ٸ��� ȭ�� ��带 ��ȯ�Ѵ�.  (V86Lib�� �ε�Ǿ� �־�� �Ѵ�.)
	nLines = get_vertical_line_size();
	if( nLines != (int)pVC->byLInes )
		lines_xx( (int)pVC->byLInes );
	
	// Ŀ�� ��ġ�� ���� �����Ѵ�.
	set_cursor_xy( pVC->wCursorX, pVC->wCursorY );

	if( is_gui_mode() != 0 )
	{
		// Ŀ�� ��ġ�� �����Ѵ�.
		gui_set_cursor_xy( pVC->wCursorX, pVC->wCursorY );

		// GUI ������ �÷��� �Ѵ�.
		gui_flushing( pVC );
	}
	else
	{	// ���� �޸𸮸� �����Ѵ�.
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

// ���� �ܼ��� ����� ����Ѵ�.
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

// ���� ���μ����� �ڽ��� ���� �ֿܼ��� FG�� �����Ǿ� �ִ°�?
int is_fg_process( ProcessStt *pP )
{
	if( pP == NULL || pP->pVConsole == NULL )
		return( 0 );
	
	if( pP == pP->pVConsole->pStartFg )
		return( 1 );
	
	return( 0 );
}

// FG ��ũ���� ���μ����� �����ϴ��� ã�´�.
static int find_fg_link( VConsoleStt *pVC, ProcessStt *pP )
{
	ProcessStt *pProc;

	if( pP == NULL || pVC == NULL )
		return( 0 );

	for( pProc = pVC->pStartFg; pProc != NULL; pProc = pProc->pNextFG )
	{
		if( pProc == pP )
			return( 1 );		// ��ũ���� ã�Ҵ�.
	}					

	return( 0 );
}

// ���μ����� FG ��ũ���� �����Ѵ�.
int del_fg_link( VConsoleStt *pVC, ProcessStt *pP )
{
	if( pP == NULL || pVC == NULL )
	{
		//kdbg_printf( "del_fg_link: pP == NULL || pVC == NULL\n" );
		return( 0 );
	}

	// �������� �ʴ��� Ȯ���Ѵ�.
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

// ���� �ܼ��� FG ���μ����� �����Ѵ�.
int set_fg_process( VConsoleStt *pVC, struct ProcessTag *pP )
{
	if( pP == NULL || pP->pVConsole == NULL )
		return( -1 );
	
	if( pVC == NULL )
		pVC = pP->pVConsole;

	// �̹� FG ��ũ�� ���ԵǾ� �ִ� ���̸� ��ũ���� �����Ѵ�.
	del_fg_link( pVC, pP );

	// ��ũ�� ���� ���ʿ� �����Ѵ�.
	pP->pPreFG  = NULL;
	pP->pNextFG = pVC->pStartFg;
	if( pVC->pStartFg == NULL )
		pVC->pEndFg = pP;
	else
		pVC->pStartFg->pPreFG = pP;
	pVC->pStartFg = pP;

	/*
	{ // ������� ���� ���� FG ��ũ�� PID�� ����Ѵ�.
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

// ���� �ܼ��� ��ȯ�Ѵ�.
int change_next_vconsole()
{
	VConsoleStt		*pVC;
	int				nI, nTotal;

	if( sys_vcon.pp_vcon == NULL )
		return( -1 );

	nI = 0;
	// ���� ���μ����� ���� �ܼ��� ���Ѵ�.
	pVC = get_active_vconsole();
	if( pVC != NULL )
	{
		for( nI = 0; ; nI++ )
		{
			if( nI >= sys_vcon.nMax )
				break;	 // ���� VConsole�� �ε����� ã�� �� ����.
			if( sys_vcon.pp_vcon[nI] == pVC )
			{
				nI++;	 // ���� VConsole�� �ε����� ã�Ҵ�.
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

	// ���� Active VConsole�� �����Ѵ�.
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
