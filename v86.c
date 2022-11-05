#include "bellona2.h"

// IO Bit Field 값이 모두 0이어야 R3에서 I/O의 수행이 가능해 진다.
static V86TSSStt v86_tss;
static ThreadStt *G_pV86Thread = NULL;

int is_v86_lib( BYTE *pBuff )
{
	if( memcmp( &pBuff[2], V86LIB_ID, strlen( V86LIB_ID ) ) != 0 && memcmp( &pBuff[3], V86LIB_ID, strlen( V86LIB_ID ) ) != 0 )
		return( 0 );
	else
		return( 1 );
}

// 단순히 파일만 지정된 위치에 로드한다.
int load_v86_lib( char *pFileName )
{
	int		nHandle;
	long	lSize;
	char	*pBuff;

	// 파일을 오픈한다.
	nHandle = kopen( pFileName, FM_READ );
	if( nHandle < 0 )
	{
		kdbg_printf( "load_v86_module() - file open error!\n" );
		return( -1 );   
	}

	// 파일의 크기를 알아낸다.
	lSize = klseek( nHandle, 0, FSEEK_END );
    klseek( nHandle, 0, FSEEK_SET );
	if( lSize < 0 )
	{
		kdbg_printf( "load_v86_module() - get file size failed!\n" );
		kclose( nHandle );
		return( -1 );
	}	

	// 파일을 읽고 닫는다.
	pBuff = (char*)V86LIB_ADDR;
	kread( nHandle, (char*)( (DWORD)pBuff), lSize );
	kclose( nHandle );

	kdbg_printf( "V86Lib : 0x%08X (%d bytes)\n", pBuff, lSize );

	if( is_v86_lib( pBuff ) == 0 )
		kdbg_printf( "Invalid V86Lib ID.\n" );
	else
		kdbg_printf( "ok\n" );

	return( 0 );
}

// v86 thread는 bellona2_main에서 생성된다.
int v86lib_thread( void *pPram )
{
	char			*pB;
	V86TSSStt		*pT;
    DWORD			addr[2];
	V86ParamStt		*pParam;
	ThreadStt		*pThread;

	// v86 TASK를 생성하고 테이블에 추가한다.
	pT = &v86_tss;
	vMakeV86TSS( pT, 0, 0 );
	nAppendTSSTbl( "V86_TSS", (DWORD)pT );

	// VM-ON, IF-OFF, IOPL-3
	pT->dwEFLAG |= (DWORD)MASK_VM | (DWORD)MASK_IF | (DWORD)MASK_IOPL;
	//pT->dwEFLAG ^= (DWORD)MASK_IF;
	pT->dwESP = 0xFFFE;
	pT->dwEIP = 0x100;
	pT->wSS = pT->wCS	=  0x1000;
	pT->wDS = pT->wES = pT->wFS = pT->wGS = 0;

	// G_pV86Thread는 나중에 시그널을 보내는 용도로 사용한다
	G_pV86Thread = pThread = get_current_thread();
	if( pThread == NULL )
	{
		kdbg_printf( "v86lib_thread() - invalid current thread!\n" );
		return( -1 );
	}

	// Ring 0의 스택(SS0, ESP0)을 설정한다.
	pT->wSS0   = pThread->pTSS->wSS0;
	pT->dwESP0 = pThread->pTSS->dwESP0 - (DWORD)(64 * 1024);

	// 파러메터 구조체를 초기화한다.
	pParam = (V86ParamStt*)V86LIB_PARAM_ADDR;
	memset( pParam, 0, sizeof( V86ParamStt ) );

	for( ;; )
	{	// SIGNAL이 들어올 때까지 대기한다.
		kpause();

		// V86Lib가 존재하는지 확인해 본다.
		pB = (char*)(V86LIB_ADDR + 2);
		if( memcmp( pB, "V86LIB", 6 ) != 0 && memcmp( &pB[1], "V86LIB", 6 ) != 0 )
		{	// 아직 로드되어 있지 않은 모양이다.
			kdbg_printf( "No V86 Library!\n\n" );
			continue;
		}

		// PARAMETER가 세팅되어 있는지 확인한다.
		if( pParam->wMagic != V86PARAM_MAGIC )
		{
			kdbg_printf( "Invalid V86PARAM_MAGIC!\n" );
			continue;		
		}

		// v86TASK로 점프해 들어간다.
		dwSetDescriptorAddr( &gdt[GSEL_DUMMY_V86TSS/8], (DWORD)&v86_tss );
		addr[0] = 0;
		addr[1] = (DWORD)GSEL_DUMMY_V86TSS;
		vSetDescriptorBusyBit( &gdt[GSEL_DUMMY_V86TSS/8], 0 );// clear busy bit to jump
		_asm JMP FWORD PTR addr;   // => 최초 0x10100번지로 갔다가 다음부터 <2>로 간다.
	}	

	return( 0 );
}


// 현재 컨텍스트는 V86_TSS이다.
// Thread는 V86 Thread이다.
static void internal_v86_system_call()
{
	DWORD		addr[2];
	ThreadStt	*pThread;

	// V86Thread.V86_TSS -> V86Thread.Flat32_TSS로 전환한다.
	pThread = G_pV86Thread;

	// 32Bit Protected Mode Task로 잠시 건너갔다 온다.  (scheduling을 위해)
	dwSetDescriptorAddr( &gdt[GSEL_DUMMY_TSS32/8], (DWORD)pThread->pTSS );
	addr[0] = 0;
	addr[1] = (DWORD)GSEL_DUMMY_TSS32;
	vSetDescriptorBusyBit( &gdt[GSEL_DUMMY_TSS32/8], 0 );// clear busy bit to jump
	_asm JMP FWORD PTR addr; 
}

// v86용 시스템 콜 (INT 51H)
_declspec(naked) void v86_system_call()
{
	_asm {
		PUSHAD
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
		STI
	}

 	internal_v86_system_call();

	_asm {
		POPAD
		IRETD  // V86 태스크로 돌아간다.
	}
}

// stack frame
/*						   
	EIP, CS
	EFLAG
	ESP, SS, ES, DS, FS, GS
*/
_declspec(naked) void int_10h_emulator()
{
	static DWORD	dwESP;
	static UINT16	*pESP3, *pIVT;
	static DWORD	*pESP, dwEFLAG;

	_asm {
		PUSHAD				// 8 * 4 = 32
		PUSHFD				// 1 * 4 = 4
		CLI
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
		MOV EAX, ESP
		ADD EAX, 32 + 4
		MOV dwESP, EAX
	}

	pESP = (DWORD*)dwESP;
	dwEFLAG = pESP[2];
	if( dwEFLAG & (DWORD)MASK_VM )
	{	// interrupted from v86 mode

		// manipulate v86 stack
		pESP[3] -= 6;  // 미리 6을 뺀다.
		pESP3 = (UINT16*)( (DWORD)pESP[3] + (DWORD)( (DWORD)((DWORD)pESP[4] & (DWORD)0xFFFF) << 4 ) );
		pESP3[0] = (UINT16)pESP[0];		// IP
		pESP3[1] = (UINT16)pESP[1];		// CS
		pESP3[2] = (UINT16)pESP[2];		// FLAG
							  		
		// return to int 10h entry point
		pIVT    = (UINT16*)( 0x10 * 4 );
		pESP[0] = (DWORD)pIVT[0];		// IP
		pESP[1] = (DWORD)pIVT[1];		// CS
	}	
	_asm {
		POPFD
		POPAD
		IRETD  // V86 태스크로 돌아간다.
	}
}

_declspec(naked) void int_1Ah_emulator()
{
	static DWORD	dwESP;
	static UINT16	*pESP3, *pIVT;
	static DWORD	*pESP, dwEFLAG;

	_asm {
		PUSHAD				// 8 * 4 = 32
		PUSHFD				// 1 * 4 = 4
		CLI
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
		MOV EAX, ESP
		ADD EAX, 32 + 4
		MOV dwESP, EAX
	}	 

	pESP = (DWORD*)dwESP;
	dwEFLAG = pESP[2];
	if( dwEFLAG & (DWORD)MASK_VM )
	{	// interrupted from v86 mode

		// manipulate v86 stack
		pESP[3] -= 6;  // 미리 6을 뺀다.
		pESP3 = (UINT16*)( (DWORD)pESP[3] + (DWORD)( (DWORD)((DWORD)pESP[4] & (DWORD)0xFFFF) << 4 ) );
		pESP3[0] = (UINT16)pESP[0];		// IP
		pESP3[1] = (UINT16)pESP[1];		// CS
		pESP3[2] = (UINT16)pESP[2];		// FLAG
							  		
		// return to int 10h entry point
		pIVT    = (UINT16*)( 0x1A * 4 );
		pESP[0] = (DWORD)pIVT[0];		// IP
		pESP[1] = (DWORD)pIVT[1];		// CS
	}	
	_asm {
		POPFD
		POPAD
		IRETD  // V86 태스크로 돌아간다.
	}
}

// VMWare에서는 INT 10H를 Call하면 INT 6Dh가 호출된다.
_declspec(naked) void int_6Dh_emulator()
{
	static DWORD	dwESP;
	static UINT16	*pESP3, *pIVT;
	static DWORD	*pESP, dwEFLAG;

	_asm {
		PUSHAD				// 8 * 4 = 32
		PUSHFD				// 1 * 4 = 4
		CLI
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
		MOV EAX, ESP
		ADD EAX, 32 + 4
		MOV dwESP, EAX  
	}

	pESP = (DWORD*)dwESP;
	dwEFLAG = pESP[2];
	if( dwEFLAG & (DWORD)MASK_VM )
	{	// interrupted from v86 mode

		// manipulate v86 stack
		pESP[3] -= 6;  // 미리 6을 뺀다.
		pESP3 = (UINT16*)( (DWORD)pESP[3] + (DWORD)( (DWORD)((DWORD)pESP[4] & (DWORD)0xFFFF) << 4 ) );
		pESP3[0] = (UINT16)pESP[0];		// IP
		pESP3[1] = (UINT16)pESP[1];		// CS
		pESP3[2] = (UINT16)pESP[2];		// FLAG
							  		
		// return to int 10h entry point
		pIVT    = (UINT16*)( 0x6D * 4 );
		pESP[0] = (DWORD)pIVT[0];		// IP
		pESP[1] = (DWORD)pIVT[1];		// CS
	}	
	_asm {
		POPFD
		POPAD
		IRETD  // V86 태스크로 돌아간다.
	}
}

// V86 파러메터를 설정한다.
int set_v86_param( UINT16 wFunc, DWORD dwParam )
{
	BYTE		*pBuff;
	V86ParamStt	*pParam;

	pBuff = (BYTE*)V86LIB_ADDR;
	if( is_v86_lib( pBuff ) == 0 )
	{	// V86LIB이 로드되지 않다.
		kdbg_printf( "set_v86_param() - invalid V86LIB_ID!\n" );
		return( -1 );
	}

	pParam = (V86ParamStt*)V86LIB_PARAM_ADDR;

	for( ; pParam->wMagic != 0; )
		kernel_scheduler();			// 플래그가 "0"이될 때까지 대기한다.

	memset( pParam, 0, sizeof( V86ParamStt ) );
	pParam->wFunc   = wFunc;
	pParam->dwParam = dwParam;
	pParam->wMagic  = V86PARAM_MAGIC;
	
	return( 0 );
}

DWORD call_to_v86_thread()
{	
	DWORD		dwR;
	V86ParamStt	*pParam;

	pParam = (V86ParamStt*)V86LIB_PARAM_ADDR;

	// 일단 시그널을 보내 놓는다.
	send_signal_to_thread( G_pV86Thread->dwID, SIG_CONT );
	
	// 해당 쓰레드로 전환한다.
	// 현재 쓰레드는 일반 커널 쓰레드이다.
	kernel_thread_switching( G_pV86Thread );

	dwR = (DWORD)pParam->wResult;

	// 파러메터 블록을 초기화한다.
	memset( pParam, 0, sizeof( V86ParamStt ) );

	// 파러메터의 결과값을 리턴한다.
	return( dwR );
}

// 화면의 라인 수를 변경한다. 
DWORD lines_xx( int nLine )
{
	DWORD			dwR;
	BYTE			buff[80*25*2];
	int				nR, nPrevLine;

	// 파러메터를 설정한다.
	if( nLine == 25 )
		nR = set_v86_param( V86FUNC_LINES25, 0 );
	else if( nLine == 50 )
		nR = set_v86_param( V86FUNC_LINES50, 0 );
	else
		return( -1 );
	if( nR < 0 )
		return( 0 );

	// 80 * 25 화면의 데이터를 복사해 둔다.
	nPrevLine = get_vertical_line_size();
	if( nPrevLine == 25 && nLine == 50 )
		memcpy( buff, (BYTE*)0xB8000, sizeof( buff ) );

	// 바로 V86 쓰레드를 활설화하여 실행한다.
	dwR = call_to_v86_thread();
	if( dwR == 0 )
	{	// 내부 데이터를 변경한다.
		set_vertical_line_size( nLine );
		
		// 화면을 지운다.
		if( nLine != 50 )
			kdbg_clearscreen();
		else if( nPrevLine == 25 )
			memcpy( (BYTE*)0xB8000, buff, sizeof( buff ) );

	}

	return( dwR );
}

void *get_v86_buff()
{
	int	 nI;
	BYTE *pBuff;

	pBuff = (BYTE*)V86LIB_ADDR;
	for( nI = 0; nI < 64; nI++ )
	{
		if( pBuff[nI] == 'B' && pBuff[nI+1] == 'U' && pBuff[nI+2] == 'F' && pBuff[nI+3] == 'F' )
			return( &pBuff[nI+4] );
	}
	return( NULL );
}

