#include "bellona2.h"

// IO Bit Field ���� ��� 0�̾�� R3���� I/O�� ������ ������ ����.
static V86TSSStt v86_tss;
static ThreadStt *G_pV86Thread = NULL;

int is_v86_lib( BYTE *pBuff )
{
	if( memcmp( &pBuff[2], V86LIB_ID, strlen( V86LIB_ID ) ) != 0 && memcmp( &pBuff[3], V86LIB_ID, strlen( V86LIB_ID ) ) != 0 )
		return( 0 );
	else
		return( 1 );
}

// �ܼ��� ���ϸ� ������ ��ġ�� �ε��Ѵ�.
int load_v86_lib( char *pFileName )
{
	int		nHandle;
	long	lSize;
	char	*pBuff;

	// ������ �����Ѵ�.
	nHandle = kopen( pFileName, FM_READ );
	if( nHandle < 0 )
	{
		kdbg_printf( "load_v86_module() - file open error!\n" );
		return( -1 );   
	}

	// ������ ũ�⸦ �˾Ƴ���.
	lSize = klseek( nHandle, 0, FSEEK_END );
    klseek( nHandle, 0, FSEEK_SET );
	if( lSize < 0 )
	{
		kdbg_printf( "load_v86_module() - get file size failed!\n" );
		kclose( nHandle );
		return( -1 );
	}	

	// ������ �а� �ݴ´�.
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

// v86 thread�� bellona2_main���� �����ȴ�.
int v86lib_thread( void *pPram )
{
	char			*pB;
	V86TSSStt		*pT;
    DWORD			addr[2];
	V86ParamStt		*pParam;
	ThreadStt		*pThread;

	// v86 TASK�� �����ϰ� ���̺� �߰��Ѵ�.
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

	// G_pV86Thread�� ���߿� �ñ׳��� ������ �뵵�� ����Ѵ�
	G_pV86Thread = pThread = get_current_thread();
	if( pThread == NULL )
	{
		kdbg_printf( "v86lib_thread() - invalid current thread!\n" );
		return( -1 );
	}

	// Ring 0�� ����(SS0, ESP0)�� �����Ѵ�.
	pT->wSS0   = pThread->pTSS->wSS0;
	pT->dwESP0 = pThread->pTSS->dwESP0 - (DWORD)(64 * 1024);

	// �ķ����� ����ü�� �ʱ�ȭ�Ѵ�.
	pParam = (V86ParamStt*)V86LIB_PARAM_ADDR;
	memset( pParam, 0, sizeof( V86ParamStt ) );

	for( ;; )
	{	// SIGNAL�� ���� ������ ����Ѵ�.
		kpause();

		// V86Lib�� �����ϴ��� Ȯ���� ����.
		pB = (char*)(V86LIB_ADDR + 2);
		if( memcmp( pB, "V86LIB", 6 ) != 0 && memcmp( &pB[1], "V86LIB", 6 ) != 0 )
		{	// ���� �ε�Ǿ� ���� ���� ����̴�.
			kdbg_printf( "No V86 Library!\n\n" );
			continue;
		}

		// PARAMETER�� ���õǾ� �ִ��� Ȯ���Ѵ�.
		if( pParam->wMagic != V86PARAM_MAGIC )
		{
			kdbg_printf( "Invalid V86PARAM_MAGIC!\n" );
			continue;		
		}

		// v86TASK�� ������ ����.
		dwSetDescriptorAddr( &gdt[GSEL_DUMMY_V86TSS/8], (DWORD)&v86_tss );
		addr[0] = 0;
		addr[1] = (DWORD)GSEL_DUMMY_V86TSS;
		vSetDescriptorBusyBit( &gdt[GSEL_DUMMY_V86TSS/8], 0 );// clear busy bit to jump
		_asm JMP FWORD PTR addr;   // => ���� 0x10100������ ���ٰ� �������� <2>�� ����.
	}	

	return( 0 );
}


// ���� ���ؽ�Ʈ�� V86_TSS�̴�.
// Thread�� V86 Thread�̴�.
static void internal_v86_system_call()
{
	DWORD		addr[2];
	ThreadStt	*pThread;

	// V86Thread.V86_TSS -> V86Thread.Flat32_TSS�� ��ȯ�Ѵ�.
	pThread = G_pV86Thread;

	// 32Bit Protected Mode Task�� ��� �ǳʰ��� �´�.  (scheduling�� ����)
	dwSetDescriptorAddr( &gdt[GSEL_DUMMY_TSS32/8], (DWORD)pThread->pTSS );
	addr[0] = 0;
	addr[1] = (DWORD)GSEL_DUMMY_TSS32;
	vSetDescriptorBusyBit( &gdt[GSEL_DUMMY_TSS32/8], 0 );// clear busy bit to jump
	_asm JMP FWORD PTR addr; 
}

// v86�� �ý��� �� (INT 51H)
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
		IRETD  // V86 �½�ũ�� ���ư���.
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
		pESP[3] -= 6;  // �̸� 6�� ����.
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
		IRETD  // V86 �½�ũ�� ���ư���.
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
		pESP[3] -= 6;  // �̸� 6�� ����.
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
		IRETD  // V86 �½�ũ�� ���ư���.
	}
}

// VMWare������ INT 10H�� Call�ϸ� INT 6Dh�� ȣ��ȴ�.
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
		pESP[3] -= 6;  // �̸� 6�� ����.
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
		IRETD  // V86 �½�ũ�� ���ư���.
	}
}

// V86 �ķ����͸� �����Ѵ�.
int set_v86_param( UINT16 wFunc, DWORD dwParam )
{
	BYTE		*pBuff;
	V86ParamStt	*pParam;

	pBuff = (BYTE*)V86LIB_ADDR;
	if( is_v86_lib( pBuff ) == 0 )
	{	// V86LIB�� �ε���� �ʴ�.
		kdbg_printf( "set_v86_param() - invalid V86LIB_ID!\n" );
		return( -1 );
	}

	pParam = (V86ParamStt*)V86LIB_PARAM_ADDR;

	for( ; pParam->wMagic != 0; )
		kernel_scheduler();			// �÷��װ� "0"�̵� ������ ����Ѵ�.

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

	// �ϴ� �ñ׳��� ���� ���´�.
	send_signal_to_thread( G_pV86Thread->dwID, SIG_CONT );
	
	// �ش� ������� ��ȯ�Ѵ�.
	// ���� ������� �Ϲ� Ŀ�� �������̴�.
	kernel_thread_switching( G_pV86Thread );

	dwR = (DWORD)pParam->wResult;

	// �ķ����� ����� �ʱ�ȭ�Ѵ�.
	memset( pParam, 0, sizeof( V86ParamStt ) );

	// �ķ������� ������� �����Ѵ�.
	return( dwR );
}

// ȭ���� ���� ���� �����Ѵ�. 
DWORD lines_xx( int nLine )
{
	DWORD			dwR;
	BYTE			buff[80*25*2];
	int				nR, nPrevLine;

	// �ķ����͸� �����Ѵ�.
	if( nLine == 25 )
		nR = set_v86_param( V86FUNC_LINES25, 0 );
	else if( nLine == 50 )
		nR = set_v86_param( V86FUNC_LINES50, 0 );
	else
		return( -1 );
	if( nR < 0 )
		return( 0 );

	// 80 * 25 ȭ���� �����͸� ������ �д�.
	nPrevLine = get_vertical_line_size();
	if( nPrevLine == 25 && nLine == 50 )
		memcpy( buff, (BYTE*)0xB8000, sizeof( buff ) );

	// �ٷ� V86 �����带 Ȱ��ȭ�Ͽ� �����Ѵ�.
	dwR = call_to_v86_thread();
	if( dwR == 0 )
	{	// ���� �����͸� �����Ѵ�.
		set_vertical_line_size( nLine );
		
		// ȭ���� �����.
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

