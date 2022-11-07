#include "bellona2.h"

static char *get_function_name( DWORD dwAddr );
static int get_src_dbg_info( char *pSrc, char *pFunc, DWORD dwAddr );


//KDBGStt kdbg;
Int3EntStt int3_ent[MAX_INT3_ENT];

#define VCONSOLE_BUFF_SIZE ( 160*50 )

static int G_nVideoLine = 25;

int set_vertical_line_size( int nLine )
{
	G_nVideoLine = nLine;
	return( G_nVideoLine );
}

int get_vertical_line_size()
{
	return( G_nVideoLine );
}

void kdbg_clearscreen()
{
	UCHAR			*pV;
	VConsoleStt		*pVC;
	int				nI, nJ;

	pVC = get_current_vconsole();

	if( pVC != NULL )
	{
		// 가상 콘솔의 버퍼를 지운다.
		pV = (UCHAR*)pVC->con_buff;
		for( nI = 0; nI < G_nVideoLine; nI++ )
		{
			for( nJ = 0; nJ < 80; nJ++ )
			{
				pV[ nI*160 + nJ*2 ] = ' ';
				pV[ nI*160 + nJ*2 +1 ] = 7;
			}
		}
	
		// 가상 콘솔의 커서 위치를 지정한다.
		pVC->wCursorX = 0;
		pVC->wCursorY = 0;
	
		if( is_active_vconsole( pVC ) == 0 )
			return;
	}

	// GUI Console을 지운다.
	gui_cls( 0 );

	// 비디오 메모리를 지운다.
	pV = (UCHAR*)0xB8000;
	for( nI = 0; nI < G_nVideoLine; nI++ )
	{
		for( nJ = 0; nJ < 80; nJ++ )
		{
			pV[ nI*160 + nJ*2 ] = ' ';
			pV[ nI*160 + nJ*2 +1 ] = 7;
		}
	}								  
	
	{// REMOTE SHELL을 클리어 한다.
		char	szT[16];
		sprintf( szT, "%c[2J%c[1;1H", 0x1B,0x1B );
		send_string_to_remote_shell( szT );
	}
}

static int nTTYOutChar( char *pB, DWORD dwLimit, char byCh, char  byAttr )
{
	if( (DWORD)pB >= dwLimit )  // out of range
		return( -1 );

	pB[0] = byCh;	  // char 
	pB[1] = byAttr;	  // attr
	return( 0 );
}

DWORD dwTTYOut( char *pB, int nTabSize, int x, int y, char *pS )
{
	char    *pBuff;
	int     nI, nK, nR = 0;
	DWORD   dwLimit;
	DWORD   dwNewOffs;
	UINT16 	*pW, *pWX;

	// send string to remote shell
	if( 0xB8000 <= (DWORD)pB && (DWORD)pB < (DWORD)0xB8000 + (DWORD)(160*50) )
		send_string_to_remote_shell( pS );

	dwLimit = (DWORD)pB + VCONSOLE_BUFF_SIZE;

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		pBuff = &pB[x*2+y*160];	

		if( pS[nI] == 0x0A )		// CRLF
		{
			x = 0;
			y++;
		}
		else if( pS[nI] == 0x0D )	// CR
			x = 0;
		else if( pS[nI] == 0x09 )	// TAB
		{
			int nNewX;

			nNewX = x + (nTabSize-1);
			x = (nNewX / nTabSize)*nTabSize;
		}
		else
		{
			nR = nTTYOutChar( pBuff, dwLimit, pS[nI], 0x07 );  // 한 문자를 출력한다.
			x++;
		}

		// x좌표가 화면 끝에 다다랐으면 다름 라인으로 넘어간다. 
		if( x >= 80 )
		{
			x = 0;
			y++;
		}

		// 범위 넘어갔음?
	    if( nR < 0 || y >= G_nVideoLine )  // 스크롤한다.
		{
			x = 0; 
			y = G_nVideoLine - 1;  // 마지막 라인에서 계속 왔다갔다 한다.
			// 스크롤
			  
			pW  = (UINT16*)pB;
			pWX = (UINT16*)&pW[80];

			for( nK = 0; nK < (G_nVideoLine-1) * 80 ; nK++ )							  
				pW[nK] = pWX[nK];														  
																						  
			for( nK = (G_nVideoLine-1) * 80 ; nK < (G_nVideoLine) * 80; nK++ )	  
				pW[nK] = 0X0700;														  
		}
	}

	// calculate new offset by x,y location
	dwNewOffs = x * 2 + y * 160;

	return( dwNewOffs );
}

/*
int kdbg_printf( char *pFmt, ... )
{
	va_list		va;
	int			nI;
	VConsole	pVC;
	DWORD		dwNewOffs;
	char		szTX[1024];

	va_start( va, pFmt );
	nI = vsprintf( szTX, pFmt, va );
	va_end( va ); 

	// kcmdwin 쪽으로 문자열을 전달한다.
	gui_redirect( szTX );
	// send string to the video memory
	dwNewOffs = dwTTYOut( (UCHAR*)0xB8000, DEFAULT_TAB_SIZE, kdbg.nX, kdbg.nY, szTX );

	// move cursor
	vSetCursorOffs( dwNewOffs/2 );
	vCursorOffsetToXY( dwNewOffs );

	// 콘솔을 구한다.
	pVC = (VConsoleStt*)get_kernel_vconsole();
	if( pVC != NULL )
	{
	}	
	
	return( nI );
}
*/

static int internal_vcon_write( VConsoleStt *pVC, char *pS )
{
	DWORD dwNewOffs;
	
	_asm {
		PUSHFD 
		CLI
	}

	// 이 과정에서 KBD Interrupt가 걸리면 안된다.  CTRL-TAB
	///////////////////////////////////////////////////////////////////////////////////
	dwNewOffs = dwTTYOut( (UCHAR*)pVC->con_buff, DEFAULT_TAB_SIZE, pVC->wCursorX, pVC->wCursorY, pS );
	if( is_active_vconsole( pVC ) != 0 )
	{
		dwTTYOut( (UCHAR*)0xB8000, DEFAULT_TAB_SIZE, pVC->wCursorX, pVC->wCursorY, pS );

		// 화면에 보이는 실제 커서를 움직인다.
		move_cursor( dwNewOffs/2 );

		// kcmdwin 쪽으로 문자열을 전달한다.
		gui_write( pS );
	
	}
	else// 가상 콘솔
		move_vconsole_cursor( dwNewOffs/2, pVC );
	///////////////////////////////////////////////////////////////////////////////////
	_asm POPFD
}

int kdbg_printf_ex( void *pVoidVC, char *pFmt, ... )
{
	va_list		va;
	int			nI;
	char		szTX[1024];
	
	if( pVoidVC == NULL )
		return( -1 );
	
	va_start( va, pFmt );
	nI = vsprintf( szTX, pFmt, va );
	va_end( va ); 

	internal_vcon_write( (VConsoleStt*)pVoidVC, szTX );

	return( nI );
}

int kdbg_printf( char *pFmt, ... )
{
	va_list 	va;
	int 		nI;
	VConsoleStt *pVC;
	char		szTX[1024];
	
	va_start( va, pFmt );
	nI = vsprintf( szTX, pFmt, va );
	va_end( va ); 
	
	// 콘솔을 구한다.
	pVC = (VConsoleStt*)get_current_vconsole();
	if( pVC == NULL )
		return( -1 );
	
	internal_vcon_write( pVC, szTX );

	return( nI );
}

int kxy_printf( int nX, int nY, char *pFmt, ... )
{
	char    szTX[1024];

	va_list va;
	int		nI;

	va_start( va, pFmt );
	nI = vsprintf( szTX, pFmt, va );
	va_end( va ); 
		
	dwTTYOut( (UCHAR*)0xB8000, DEFAULT_TAB_SIZE, nX, nY, szTX );
	
	return( nI );
}

////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_DBG_KEY		16
static int nTotalDbgKey = 0, nKI = 0, nKO = 0;
static BKeyStt kdbg_keybuff[ MAX_DBG_KEY ];
static int nDebuggerActive = 0;

int kdbg_key_input( BKeyStt *pKey )
{
	if( nTotalDbgKey >= MAX_DBG_KEY )
		return( -1 );

	memcpy( &kdbg_keybuff[nKI++], pKey, sizeof( BKeyStt ) );
	if( nKI == MAX_DBG_KEY )
		nKI = 0;
	nTotalDbgKey++;

	return( 0 );
}

// get one character
int debugger_getchar()
{
	BKeyStt key;

	if( nTotalDbgKey == 0 )
		return( -1 );
		
	memcpy( &key, &kdbg_keybuff[nKO++], sizeof( BKeyStt ) );

	if( nKO == MAX_DBG_KEY )
		nKO = 0;
	nTotalDbgKey--;

	return( (int)key.byCode );
}

int set_debugger_active( int nActive )
{
	int nPrev;

	nPrev = nDebuggerActive;
	nDebuggerActive = nActive;

	return( nPrev );
}

int is_debugger_active()
{
	return( nDebuggerActive );
}


_declspec(naked) void kdbg_breakpoint( DWORD dwT )
{
	_asm INT 1;

	_asm RETN;
}

// get register value in tss
static DWORD get_register_value( unsigned short wReg, TSSStt *pTSS )
{
	switch( wReg )
	{
    case rEAX : return( pTSS->dwEAX );
    case rEBX : return( pTSS->dwEBX );
    case rECX : return( pTSS->dwECX );
    case rEDX : return( pTSS->dwEDX );
    case rESI : return( pTSS->dwESI );
    case rEDI : return( pTSS->dwEDI );
    case rEBP : return( pTSS->dwEBP );
    case rESP : return( pTSS->dwESP );
    default   : return( 0 );
    }
	return(0);
}	

// get indirect memory reference address
static DWORD get_indirect_mem_addr( OpStt *pOp, OperandStt *pOperand, TSSStt *pTSS )
{
	DWORD	dwAddr, dwIndex, dwDisp, dwBase;

	if( pOperand->wType != oc_MEM )
		return( 0xFFFFFFFF );

	// displacement ( signed char, signed short int, long )
	if( pOp->wDispSize == 0 )
		dwDisp = 0;
	else if( pOp->wDispSize == 1 )
	{
		dwDisp = pOp->dwDisp;
		if( pOp->dwDisp & 0x80 )
			dwDisp += (DWORD)0xFFFFFF00;
	}
	else if( pOp->wDispSize == 2 )
	{
		dwDisp = pOp->dwDisp;
		if( pOp->dwDisp & 0x8000 )
			dwDisp += (DWORD)0xFFFF0000;
	}
	else if( pOp->wDispSize == 4 )
		dwDisp = pOp->dwDisp;
			 
	// base
	dwBase = get_register_value( pOperand->wRegBase, pTSS );

	// index
	dwIndex = get_register_value( pOperand->wIndex, pTSS );
	if( pOperand->wScale > 0 )
		dwIndex *= (DWORD)pOperand->wScale;

	dwAddr = dwBase + dwIndex + dwDisp;
	
	return( dwAddr );
}	

// 다음에 실행할 명령을 출력한다.
static char _strOffset[32], _strIP[12], _strOpCode[32], _strOperand[100], _strSize[12], _strDump[64];
static char *strArray[] = {_strOffset, _strIP, _strOpCode, _strOperand, _strSize, _strDump };
void kdbg_disp_next_code()
{
	OpStt				Op;
	int					nSize;
	TSSStt				*pTSS;
	char				*pFuncName;
	DWORD				dwAddr, dwCurAddr;

	pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
	if( pTSS == NULL )
		return;

	memset( &Op, 0, sizeof( Op ) );

	// check if the VM flag in EFLAG is set.
	if( pTSS->dwEFLAG & MASK_VM )
	{   // V86 모드인가?
		wSetDefaultBit( 0 ); // 16비트 역어셈블
		dwCurAddr = pTSS->dwEIP + (DWORD)( (DWORD)pTSS->wCS << 4 );
	}
	else
	{
		wSetDefaultBit( 1 ); // 32비트 역어셈블
		dwCurAddr = pTSS->dwEIP;
	}

	// 한 라인을 역어셈블한다.
	nSize = nDisAssembleOneCode( dwCurAddr, &Op, (UCHAR*)dwCurAddr, strArray );
				   
	// CALL 명령이면 주소를 통해 함수 이름을 찾아 주소 대신 출력한다.
	pFuncName = NULL;
	if( Op.wType == ot_CALL )
	{
		dwAddr = dwHexValue( strArray[3] );
		// get function name
		pFuncName = get_function_name( dwAddr );
	}

	if( pFuncName == NULL )		  
		pFuncName = strArray[3];

	// 소스와 레지스터 값을 출력한다.
	disp_src_and_regs( pTSS, dwCurAddr );

	// print unassembled line
	kdbg_printf( "0x%08X %-6s %-30s %s\n", dwCurAddr, strArray[2], pFuncName, strArray[5]);

	// calc indirect memory reference
//	{
//		DWORD	dwAddr;
//
//		if( Op.Oprnd[0].wType == oc_MEM )
//		{
//			dwAddr = get_indirect_mem_addr( &Op, &Op.Oprnd[0], pTSS );
//			kdbg_printf( ":> 0x%08X\n", dwAddr );
//		}
//		else if( Op.Oprnd[1].wType == oc_MEM )
//		{
//			dwAddr = get_indirect_mem_addr( &Op, &Op.Oprnd[1], pTSS );
//			kdbg_printf( ":> 0x%08X\n", dwAddr );
//		}
//	}

}

// 지정된 TSS의 BackLink가 가리키는 TSS의 주소를 구한다.
void *pGetBackLinkTSS( void *pV )
{
	TSSStt *pT, *pTSS;
	UINT16 wBackLink;

	pTSS = (TSSStt*)pV;
	wBackLink = (UINT16)pTSS->dwBackLink;
	if( wBackLink == 0 )		// no BackLink
		return( NULL );

	pT = (TSSStt*)dwGetDescriptorAddr( &gdt[wBackLink/8] );  // 8로 나누는 것을 잊지말자.
	
	return( (void*)pT );
}

UINT16 wGetBackLink( void *pV )
{
	TSSStt *pTSS;

	pTSS = (TSSStt*)pV;

	return( (UINT16)pTSS->dwBackLink );
}

// Debugee 프로그램의 TSS의 EFLAG 의 TF비트를 nTF로 설정한다. 
void kdbg_set_debugee_tf( DWORD dwTF )
{
	TSSStt *pTSS;

	// TSS의 주소를 구한다.
	pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
	if( pTSS == NULL )
		return;

	pTSS->dwEFLAG |= (DWORD)MASK_TF;

	// TF Flag를 Clear한다.
	if( dwTF == 0 )
		pTSS->dwEFLAG ^= (DWORD)MASK_TF;
}

// set debugee's tf flag bit to skip h/w breakpoint this time
void kdbg_set_debugee_rf( DWORD dwRF )
{
	TSSStt *pTSS;

	dwRF = (DWORD)dwRF & (DWORD)1;
	dwRF = (DWORD)( dwRF << 16 );

	// get tss
	pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
	if( pTSS == NULL )
		return;

	// clear rf
	pTSS->dwEFLAG |= (DWORD)MASK_RF;
	pTSS->dwEFLAG ^= (DWORD)MASK_RF;
	
	// set new rf bit
	pTSS->dwEFLAG |= (DWORD)dwRF;
}

// TSS의 BackLink로부터 debugee의 CR3를 얻어 변경한다.
// 이것을 백링크의 CR3로 변경해 주어야 debugee task의 주소공간에 접근할 수 있다.
static DWORD	dwDebugeeTID = 0;
DWORD kdbg_change_CR3()
{
	ThreadStt	*pT;
	TSSStt		*pTSS;
	DWORD		dwNewCR3;

	pT = NULL;
	if( dwDebugeeTID != 0 )
	{
		pT = find_thread_by_id( dwDebugeeTID );
		if( pT == NULL )
		{
			kdbg_printf( "debugee thread id(%d) set to zero.\n" );
			dwDebugeeTID = 0;
		}
	}

	// 현재 Thread의 CR3를 이용한다.
	if( pT == NULL )
		pT = get_current_thread();
	if( pT == NULL )
	{
		// TSS의 주소를 구한다.
		pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
		if( pTSS == NULL )
		{
			kdbg_printf( "kdbg_change_CR3: Back link TSS is NULL!\n" );
			return( 0 );
		}
		dwNewCR3 = pTSS->dwCR3;
	}
	else
		dwNewCR3 = pT->pTSS->dwCR3;	

	// CR3를 변경한다.
	dbg_tss.dwCR3 = dwNewCR3;
	_asm {
		MOV EAX, dwNewCR3;
		MOV CR3, EAX
		FLUSH_TLB2(dwNewCR3)
	}
	return( dwNewCR3 );
}

DWORD kdbg_get_debugee_tid()
{
	return( dwDebugeeTID );
}

void kdbg_set_debugee_tid( DWORD dwTID )
{
	DWORD dwCR3;
	
	dwDebugeeTID = dwTID;

	if( is_debugger_active() == 0 )
		return;

	dwCR3 = kdbg_change_CR3();

	kdbg_printf( "debugger address space: TID=%d, CR3=0x%X\n", dwTID, dwCR3 );
}

// 다른 CR3 컨텍스트 내의 특정 주소에 있는 값을 구한다.
static int get_dword_in_other_context( DWORD dwCR3, DWORD dwAddr, DWORD *pResult )
{
	int 			nR;
	static DWORD	dwOrgCR3;

	// dwAddr이 valid한 곳인지 먼저 검사한다.
	nR = check_memory_validity( dwCR3, dwAddr );
	if( nR < 0 )
		return( -1 );

	// 원본 CR3를 보관해 둔다.
	_asm {
		MOV EAX, CR3
		MOV dwOrgCR3, EAX
	}

	if( is_debugger_active() == 0 )
		set_cr3_in_tss( &dbg_tss, dwCR3 );
	else
		set_cur_cr3_in_tss( dwCR3 );
	
	_asm {
		MOV EBX, dwCR3
		MOV ECX, dwAddr
		MOV EDX, pResult

		PUSHFD
		CLI

		// CR3를 보관한다.
		MOV  CR3,   EBX
		FLUSH_TLB2(dwCR3)

		// 값을 읽어서 pResult에 저장한다.
		MOV  EAX,  [ECX]
		MOV  [EDX], EAX
	}

	if( is_debugger_active() == 0 )
		set_cr3_in_tss( &dbg_tss, dwOrgCR3 );
	else
		set_cur_cr3_in_tss( dwOrgCR3 );

	_asm {
		// CR3를 복원한다.
		MOV  EAX, dwOrgCR3
		MOV  CR3, EAX
		POPFD
	}	

	return( 0 );
}	

// TSS의 BackLink에 보관된 EBP 값을 토대로 CALL STACK을 출력한다.
void disp_call_stack( TSSStt *pTSS, int nLocalFlag )
{
	ThreadStt			*pT;
	MyCoffDbg2FuncStt	*pFunc;
	int					nR, nI;
	MyCoffDbg2LocalStt	*pLocal;
	char				*pFuncName, *pLocalName;
	DWORD				dwValue, dwAddr, dwEBP, dwStackAddr, dwStackSize, dwNextEBP, dwRetAddr;

	// get debugee's tss
	if( pTSS == NULL )
	{
		pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
		if( pTSS == NULL )
		{
			kdbg_printf( "disp_call_stack: TSS is NULL!\n" );
			return;
		}
	}

	// pTSS를 소유하고 있는 쓰레드를 찾는다.
	pT = find_thread_by_tss( pTSS );
	if( pT == NULL )
	{
		kdbg_printf( "disp_call_stack: TSS(0x%X)' owner thread not found\n", (DWORD)pTSS );
		return;
	}

	// 쓰레드가 갖고 있는 스택의 크기를 구한다..
	nR = get_dword_in_other_context( pTSS->dwCR3, (DWORD)&pT->pStack->dwBaseAddr, &dwStackAddr );
	kdbg_printf( "disp_call_stack: Thread(%d).dwStack(0x%X)\n", pT->dwID, pT->pStack->dwBaseAddr );
	nR = get_dword_in_other_context( pTSS->dwCR3, (DWORD)&pT->pStack->dwSize, &dwStackSize );
	if( nR < 0 )
	{	// 스택의 크기를 구할 수 없다.
		kdbg_printf( "disp_call_stack: unable to get stack size\n" );
		return;	  
	}
	
	dwAddr = pTSS->dwEIP;
	dwEBP  = pTSS->dwEBP;

	for( ; dwAddr != 0 ; )
	{	
		ModuleStt		*pM;
		MyCoffDbg2Stt	*pMyDbg;
		DWORD			dwRelocBase;

		pFunc = NULL;
		pMyDbg = NULL;

		// 디버그 정보를 찾는다.
		pM = find_module_by_addr( dwAddr );
		if( pM != NULL )
		{
			pMyDbg = pM->pMyDbg;
			dwRelocBase = pM->dwLoadAddr;
			if( pMyDbg != NULL )
				pFunc = get_nearest_func_ent_by_addr( pMyDbg, dwAddr - dwRelocBase );
		}

		if( pFunc == NULL || pMyDbg == NULL )
		{	// 함수를 찾지 못했으면 주소를 출력한다.
			kdbg_printf( "0x%08X:  ---\n", dwAddr );
		}
		else
		{	// 함수를 찾았다.
			pFuncName = &pMyDbg->pStrTbl[ pFunc->nNameIndex ];
			if( pFuncName[0] =='_' )
				pFuncName++;
			
			// 주소와 함수 이름을 출력한다.
			kdbg_printf( "0x%08X:  %s();\n", dwAddr, pFuncName );

			// Local List를 출력해 본다.
			if( nLocalFlag != 0 )
			{
				pLocal = &pMyDbg->pLocalTbl[ pFunc->nLocalIndex ];
				for( nI = 0; nI < pFunc->nTotalLocal; nI++, pLocal++ )
				{	// 지역 엔트리 이름을 구한다.
					pLocalName = &pMyDbg->pStrTbl[ pLocal->nNameIndex ];
					if( pLocalName[0] == '_' )
						pLocalName++;
				
					// 지역 엔트리의 값을 구한다.
					nR = get_dword_in_other_context( pTSS->dwCR3, (DWORD)(dwEBP+(DWORD)pLocal->nEBPAdder), &dwValue );
					if( pLocal->nEBPAdder < 0 )
						kdbg_printf( "   * %-12s = 0x%X\n", pLocalName, dwValue ); 
					else
						kdbg_printf( "     %-12s = 0x%X\n", pLocalName, dwValue );
				}
			}	
		}

		// 다음 체인을 따라간다.
		nR = get_dword_in_other_context( pTSS->dwCR3, dwEBP, &dwNextEBP );
		if( nR < 0 )
			break;
		nR = get_dword_in_other_context( pTSS->dwCR3, dwEBP+4, &dwRetAddr );
		if( nR < 0 )
			break;
		
		// Next EBP가 스택의 범위를 벗어난다.
		if( dwNextEBP < dwStackAddr || dwStackAddr + dwStackSize <= dwNextEBP )
			break;

		// Return Address가 Valid한지 검사한다.
		nR = check_memory_validity( pTSS->dwCR3, dwRetAddr );
		if( nR < 0 )
			break;

		// 계속 진행한다.
		dwAddr = dwRetAddr;
		dwEBP  = dwNextEBP;
	}	
}

// TSS의 BackLink에 보관된 레지스터 값을 출력한다.
void disp_src_and_regs( TSSStt *pTSS, DWORD dwCurAddr )
{
	DWORD	dwCurCR3;
	int		nLinenumber;
	char	szFunc[48];
	char	szSrcFile[48];

	// get debugee's tss
	if( pTSS == NULL )
	{
		pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
		if( pTSS == NULL )
		{
			kdbg_printf( "TSS is NULL!\n" );
			return;
		}
		dwCurAddr = pTSS->dwEIP;
	}

	// 소스 파일, 함수명, 라인번호를 출력한다.
	nLinenumber = get_src_dbg_info( szSrcFile, szFunc, dwCurAddr );
	if( nLinenumber >= 0 )
		kdbg_printf( "%s: %s (%d)\n", szSrcFile, &szFunc[1], nLinenumber );

	// EIP, CR3를 출력한다.
	_asm {
		MOV EAX, CR3
		MOV dwCurCR3, EAX
	}
	kdbg_printf( "EIP=%08X, CR3=0x%X (DBGCR3=0x%X)\n", dwCurAddr, pTSS->dwCR3, dwCurCR3 ); 

	// 레지스터를 출력한다.
	kdbg_printf( "EAX=%08X EBX=%08X ECX=%08X EDX=%08X ESI=%08X, EDI=%08X\n", 
		pTSS->dwEAX, pTSS->dwEBX, pTSS->dwECX, pTSS->dwEDX, pTSS->dwESI, pTSS->dwEDI ); 
	kdbg_printf( "EBP=%08X ESP=%08X CS=%04X SS=%04X DS=%04X ES=%04X FS=%04X GS=%04X\n", 
		pTSS->dwEBP, pTSS->dwESP, pTSS->wCS, pTSS->wSS, pTSS->wDS, pTSS->wES, pTSS->wFS, pTSS->wGS ); 
}

// dump debugee's stack
/*
int kdbg_dump_stack()
{
	DWORD	*pX;
	TSSStt  *pTSS;

	// get debugee's tss
	pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
	if( pTSS == NULL )
		return(-1);

	pX = (DWORD*)( pTSS->dwEBP - 0x20 );

	kdbg_printf( "%08X>%08X %08X-%08X %08X=%08X %08X-%08X %08X\n", 
		pX, pX[0], pX[1], pX[2], pX[3],	pX[4], pX[5], pX[6], pX[7] );	
	
	pX += 8;
	
	kdbg_printf( "%08X>%08X %08X-%08X %08X=%08X %08X-%08X %08X\n", 
		pX, pX[0], pX[1], pX[2], pX[3],	pX[4], pX[5], pX[6], pX[7] );	

	return( 0 );
}
*/

// unassembling code
static char _xstrOffset[32], _xstrIP[12], _xstrOpCode[32], _xstrOperand[100], _xstrSize[12], _xstrDump[64];
static char *xstrArray[] = {_xstrOffset, _xstrIP, _xstrOpCode, _xstrOperand, _xstrSize, _xstrDump };
#define MAX_ADDRHISTORY 64
static DWORD addrHistory[MAX_ADDRHISTORY];
static int	 nAddrHistory = 0;
int unassembling( DWORD dwEIP, int nMaxDispLine )
{
	int					nK;
	OpStt				Op;
	int					nSize;          // unassembled code size of this time
	int					nLine;			// total number of unassembled lines
	UCHAR				*pBuff;			// location to start unassemble
	DWORD				dwAddr;
	char				*pFuncName;
	int					nReturnFlag;
	LONG				lDisAssembled;  // unassembled size
	int					nFuncNameFlag;

RESTART:
	nLine			= 0;          
	lDisAssembled	= 0;
	nAddrHistory    = 0;
	nFuncNameFlag	= 1;	// CALL 명령에 대해 디폴트로 함수 이름을 출력한다.
	nReturnFlag		= 1;	// RETF, RETN, IRET, IRETD 등의 다음 명령에 대한 주소를 심볼 테이블에서 비교한다.

	if( ( V86LIB_ADDR <= dwEIP && dwEIP <= V86LIB_ADDR + V86LIB_SIZE ) ||	// v86lib 영역
		( (DWORD)0xA0000 <= dwEIP && dwEIP <= 0x100000 ) )					// ROM-BIOS 영역
		wSetDefaultBit( 0 );
	else
		wSetDefaultBit( 1 );

	// save the first history
	addrHistory[nAddrHistory++] = dwEIP;

	for( ; ; )
	{
		// 별도의 주소를 지정하지 않으면 2/3페이지 정도만 출력한다.
		if( nLine > nMaxDispLine )
			break;

		if( nLine >= 23 )
		{
			nLine = 0;

			// 한 페이지의 출력이 끝났다.
			kdbg_printf( "[ESC|S|s] Stop, [B|b] Back, [A|a] Toggle Fn Addr, [HOME] Start address." );
GET_CHAR:
			// 한 글자를 입력받는다.
			for( nK = -1; nK == -1; )
				nK = getchar();		

			kdbg_printf( "\r                                                                  \r" );
			// 이전에 출력했던 주소로 돌아간다.
			if( nK == BK_PGUP || nK == 'b' || nK == 'B' )	
			{
				if( nAddrHistory >= 2 )
				{
					nAddrHistory -= 2;
					dwEIP = addrHistory[nAddrHistory];
					kdbg_clearscreen();		// 스크롤되지 않도록 화면을 지워버린다.
				}
				else
					goto GET_CHAR;
			}  
			else if( nK == 'a' || nK == 'A' )
			{
				if( nFuncNameFlag == 0 )
					nFuncNameFlag = 1;
				else
					nFuncNameFlag = 0;

				nAddrHistory -= 1;
				dwEIP = addrHistory[nAddrHistory];
				kdbg_clearscreen();		// 스크롤되지 않도록 화면을 지워버린다.
			} // 처음부터 다시 출력한다.
			else if( nK == BK_HOME )		
				goto RESTART;
			else if( nK == BK_ESC || nK == 's' || nK == 'S' )	// ESC를 누르면 역어셈블 출력을 종료한다.
				break;

			// 출력 주소를 History에 넣는다.
			addrHistory[nAddrHistory] = dwEIP;
			if( nAddrHistory < MAX_ADDRHISTORY -1 ) 
				nAddrHistory++;
		} 		
		
		pBuff = (UCHAR*)dwEIP;

		// 한 라인을 역어셈블 한다.
		nSize = nDisAssembleOneCode( dwEIP, &Op, pBuff, xstrArray );

		// 함수의 시작 부분인지 비교해 본다.
		if( nReturnFlag != 0 )
		{	// get function name
			pFuncName = get_function_name( dwEIP );
			if( pFuncName != NULL )		  
			{
				kdbg_printf( "========[ %s ]========\n", pFuncName );
				nLine++;
			}
		}

		// 이번 명령이 RETURN 명령이면 다음 명령이 함수의 시작인지 확인해야 할 것이다.
		if( Op.wType == ot_IRET || Op.wType == ot_IRETD || Op.wType == ot_RETN || Op.wType == ot_RETF )
			nReturnFlag = 1;
		else
			nReturnFlag = 0;

		// CALL 0x????????이면 심볼을 찾아서 대체한다.
		if( nFuncNameFlag && Op.wType == (UINT16)ot_CALL && Op.Oprnd[0].wType == oc_REL && Op.Oprnd[0].wSize == 4 && Op.Oprnd[0].wWidth == 4 )
		{
			dwAddr = dwHexValue( xstrArray[3] );

			// get function name
			pFuncName = get_function_name( dwAddr );
			if( pFuncName == NULL )		  // display hex operand if function symbol not found
				pFuncName = xstrArray[3];
			kdbg_printf( "%-8s %-6s %-30s %s\n", xstrArray[1], xstrArray[2], pFuncName, xstrArray[5]);
		}	
		else	// 역어셈블한 내용을 예쁘게 출력한다.
			kdbg_printf( "%-8s %-6s %-30s %s\n", xstrArray[1], xstrArray[2], xstrArray[3], xstrArray[5]);

		dwEIP += (DWORD)nSize;
		lDisAssembled += (LONG)nSize;
		nLine++;
	}

	return(0);
}

// get function name by address
static char *get_function_name( DWORD dwAddr )
{
	ModuleStt			*pM;
	MyCoffDbg2FuncStt	*pFunc;
	MyCoffDbg2Stt		*pMyDbg;
	char				*pFuncName;

	// get dbginfo struct
	pM = find_module_by_addr( dwAddr );
	if( pM == NULL || pM->pMyDbg == NULL )
		return( NULL );

	// get function entry by address
	pFunc = get_func_ent_by_addr( pM->pMyDbg, dwAddr - pM->dwLoadAddr );
	if( pFunc == NULL )
		return( NULL );

	pMyDbg = pM->pMyDbg;
	pFuncName = &pMyDbg->pStrTbl[ pFunc->nNameIndex ];
	
	if( pFuncName[0] == '_' )
		pFuncName++;
			 
	return( pFuncName );
}

// 소스명, 함수명을 구하고 라인번호를 리턴한다.
static int get_src_dbg_info( char *pSrc, char *pFunc, DWORD dwAddr )
{
	ModuleStt		*pM;
	int				nLinenumber;

	pSrc[0] = pFunc[0] = 0;

	pM = find_module_by_addr( dwAddr );
	if( pM == NULL || pM->pMyDbg == NULL )
		return( -1 );

	nLinenumber = get_file_func_lineno( pM->pMyDbg, pSrc, pFunc, dwAddr - pM->dwLoadAddr );

	return( nLinenumber );
}

// 메모리의 내용을 편집한다.
int kedit_memory( DWORD dwAddr )
{
	UCHAR*	pX;
	int		nTotal;
	DWORD	dwValue;
	char	szT[128];

	for( nTotal = 0; ; nTotal++ )
	{
		kdbg_printf( "%08X ", dwAddr );

		gets( szT );
		if( szT[0] == 0 )
			break;

		dwValue = dwHexValue( szT );

		kdbg_printf( "(%02X)\n", (UCHAR)dwValue );

		pX = (UCHAR*)dwAddr;
		pX[0] = (UCHAR)dwValue;
		dwAddr++;
	}

	return( nTotal );
}
	
int kassemble( DWORD dwAddr )
{
	UCHAR	bin[32];
	char	szT[128];
	int		nTotal, nSize;

	for( nTotal = 0; ; nTotal++ )
	{
		kdbg_printf( "%08X ", dwAddr );

		gets( szT );
		if( szT[0] == 0 )
			break;

		nSize = nMyAsm( szT, bin, dwAddr );
		if( nSize <= 0 )
			break;
		
		memcpy( (char*)dwAddr, bin, nSize );
		
		dwAddr += (DWORD)nSize;
	}

	return(0);
}
	
// 해당 주소의 원래 코드를 보관하고 0xCC를 심는다.
int overwrite_cc( UCHAR *pAddr )
{
	int		nI;

	for( nI = 0; nI < MAX_INT3_ENT; nI++ )
	{
		if( int3_ent[nI].dwAddr == 0 )
		{	// 빈 슬롯을 찾았다.
			int3_ent[nI].dwAddr = (DWORD)pAddr;
			int3_ent[nI].byCode = (UCHAR)pAddr[0];
			// 분명히 값을 저장하고 있는데 저장이 안된다. 
			pAddr[0] = (UCHAR)0xCC;		
			
			if( pAddr[0] != (UCHAR)0xCC )
			{
				int3_ent[nI].dwAddr = 0;
				kdbg_printf( "0xCC not available in ROM!\n" );
				return( 0 );
			}

			return( 1 );
		}
	}

	// 빈 슬롯을 찾을 수 없다.
	return( 0 );
}

int recover_cc( UCHAR *pAddr )
{
	int nI;

	for( nI = 0; nI < MAX_INT3_ENT; nI++ )
	{
		if( int3_ent[nI].dwAddr == (DWORD)pAddr )
		{	// 동일한 주소를 찾았다.
			int3_ent[nI].dwAddr = 0;
			pAddr[0] = int3_ent[nI].byCode;
			return( 1 );
		}
	}

	return( 0);
}		   

// 리턴될 어드레스와 int3_ent를 비교하여 같으면 원래 코드를 복구하고 엔트리를 지운다.
int swap_cc_by_org_code()
{
	int		nI;
	UCHAR	*pX;
	TSSStt	*pTSS;
	DWORD	dwAddr; 
				  	
	pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
	if( pTSS == NULL )
		return( 0 );

	if( pTSS->dwEFLAG & MASK_VM )   // V86 모드인가?
		dwAddr = pTSS->dwEIP - 1 + (DWORD)( (DWORD)pTSS->wCS << 4 );
	else
		dwAddr = (DWORD)(pTSS->dwEIP -1);

	for( nI = 0; nI < MAX_INT3_ENT; nI++ )
	{
		if( int3_ent[nI].dwAddr == dwAddr )
		{
			pX    = (UCHAR*)dwAddr;
			pX[0] = int3_ent[nI].byCode;	// 원래의 코드를 복구한다.
			int3_ent[nI].dwAddr = 0;		// 엔트리를 지운다.

			// EIP에서 1을 빼야한다.
			pTSS->dwEIP = dwAddr;
			
			return( 1 );					// 주소를 성공적으로 복구했다. 
		}
	}

	return( 0 );							// INT3에 의해 디버거로 들어온 것이 아니다.
}

static BreakEntStt	hw_break[4];

int kdbg_set_dr( DWORD dwAddr, int nOption, int nLength )
{
	int nI;

	// 빈 슬롯을 찾는다.
	for( nI = 0; ; nI++ )
	{
		if( nI >= 4 )
		{	// 더이상 하드웨어 브레이크 포인트를 설정할 수 없다.
			kdbg_printf( "kdbg_set_dr: no available h/w break register.\n" );
			return( -1 );	
		}

		// 설정되지 않은 슬롯을 찾는다.
		if( hw_break[nI].dwAddr == 0 )
		{
			hw_break[nI].dwAddr   = dwAddr;
			hw_break[nI].byOption = (UCHAR)nOption;
			hw_break[nI].byLen	  = (UCHAR)nLength;
			hw_break[nI].byEnable = 1;
			hw_break[nI].byGlobal = 1;
			hw_break[nI].byLocal  = 0;
			hw_break[nI].wTask	  = 0;
			break;
		}	
	}		

	// 디버그 레지스터 값을 변경한다. 
	nSetBreakpoint( nI, dwAddr, nLength, nOption, 0, 1 );		// GLOBAL

	return( 0 );
}

// clear break point
int kdbg_clear_dr( char ch )
{
	switch( ch )
	{
	case '0' :	nResetBreakpoint( 0 ); break;
	case '1' :	nResetBreakpoint( 1 ); break;
	case '2' :	nResetBreakpoint( 2 ); break;
	case '3' :	nResetBreakpoint( 3 ); break;
	case '*' :	// clear all breakpoint
		nResetBreakpoint( 0 ); 
		nResetBreakpoint( 1 ); 
		nResetBreakpoint( 2 ); 
		nResetBreakpoint( 3 ); 
		break;

	default : return( -1 );
	}

	return( 0 );
}

static char *hw_break_option_gto_str( BYTE byOption, char *pS )
{
	pS[0] = 0;
	
	if( byOption == BREAK_EXEC )
		strcpy( pS, "x" );
	else if( byOption == BREAK_WRITE )
		strcpy( pS, "w" );
	else if( byOption == BREAK_READWRITE )
		strcpy( pS, "rw" );
	else if( byOption == BREAK_IO )
		strcpy( pS, "io" );

	return( pS );	
}

int disp_hw_breaks()
{
	int		nI;
	char	*pEnabledStr, szT[64];

	for( nI = 0; nI < 4; nI++ )
	{

		// 설정되지 않은 슬롯은 SKIP 한다.
		if( hw_break[nI].dwAddr == 0 )
		{
			kdbg_printf( "[%d] empty.\n", nI );
			continue;
		}
		if( hw_break[nI].byEnable != 0 )
			pEnabledStr = "Enabled";
		else
			pEnabledStr = "Disabled";
			
		kdbg_printf( "[%d] 0x%08X (%d) %2s %s\n", nI, hw_break[nI].dwAddr,
												hw_break[nI].byLen,
												hw_break_option_gto_str( hw_break[nI].byOption, szT ),
												pEnabledStr );
	}

	return( 0 );
}















