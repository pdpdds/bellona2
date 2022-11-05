#include "bellona2.h"

// 보호모드용 PIC 컨트롤러 초기화 데이타 IRQ0 = 0x20, IRQ8 = 0x28
UCHAR pm_pic_data[] = {
	0xA1, 0xFF, 0x21, 0xFF, 0xA0, 0x11, 0x20, 0x11, 0xA1, 0x28,
	0x21, 0x20, 0xA1, 0x02, 0x21, 0x04, 0xA1, 0x01, 0x21, 0x01,
	0xA1, 0x00, 0x21, 0x00, 0x00  // 마지막임을 나타내는 0
};

// 리얼모드용 PIC 컨트롤러 초기화 데이타 IRQ0 = 0x20, IRQ8 = 0x70
UCHAR real_pic_data[] = {
	0xA1, 0xFF, 0x21, 0xFF, 0xA0, 0x11, 0x20, 0x11, 0xA1, 0x70,
	0x21, 0x08, 0xA1, 0x02, 0x21, 0x04, 0xA1, 0x01, 0x21, 0x01,
	0xA1, 0x00, 0x21, 0x00, 0x00  // 마지막임을 나타내는 0
};

typedef void (*USER_DEF_INT_HANDLER)( void );

static USER_DEF_INT_HANDLER user_def_int[16];

// IDT Tanle Entry의 오프셋을 구한다.
DWORD dwGetIDTOffset( IDTStt *pIdt )
{
	DWORD dwR, dwTemp;

	dwR    = (DWORD)pIdt->wOffs0;
	dwTemp = (DWORD)pIdt->wOffs1;

	dwTemp = ( dwTemp << 16 );
	dwR += dwTemp;

	return( dwR );
}

// IDT Table Entry의 오프셋을 설정하고 설정하기 전 값을 리턴한다.
DWORD dwSetIDTOffset( IDTStt *pIdt, DWORD dwOffs )
{
	DWORD dwR, dwTemp0, dwTemp1;

	dwR = dwGetIDTOffset( pIdt );

	dwTemp0 = dwOffs;
	dwTemp0 = dwTemp0 & 0xFFFF;

	dwTemp1 = dwOffs;
	dwTemp1 = (dwTemp1 >> 16);
	// 오프셋 하위와 상위를 설정한다.
	pIdt->wOffs0 = (UINT16)dwTemp0;
	pIdt->wOffs1 = (UINT16)dwTemp1;
	// 기존의 값을 리턴한다.
	return( dwR);
}

_declspec(naked) static void double_fault_handler( DWORD dwAddr )
{
    _asm {
        CLI
        CLD
XXX:    MOV EDI, 0xB8000;
        MOV ECX, 80*25
        MOV AL, '2'
        MOV AH, 7
            REP STOSW
        JMP XXX
    }
}

// PAGE FAULT
_declspec(naked) void vDefaultExceptionHandler( DWORD dwAddr )
{
    _asm CLI;
    kdbg_printf( "Exception!!" );
    _asm int 1
    for( ;; )
        ;
}

_declspec(naked) void invalid_op_handler( DWORD dwErr )
{
    _asm CLI;
    kdbg_printf( "Invalid Instruction!!!(0x%X)\n", dwErr );
    _asm  {
		int 1
		ADD ESP, 4
	}
    
}

_declspec(naked) void no_seg_handler( DWORD dwErr )
{
    _asm CLI;
    kdbg_printf( "Segment not present!!!(0x%X)", dwErr );
    _asm int 1
    for( ;; )
        ;
}

_declspec(naked) void stack_fault( DWORD dwErr )
{
    _asm CLI;
    kdbg_printf( "Stack fault!!!(0x%X)", dwErr );
    _asm int 1
    for( ;; )
        ;
}

_declspec(naked) static void gp_fault_handler( DWORD dwAddr )
{
    _asm CLI;
    kdbg_printf( "GP Fault : 0x%08X\n", dwAddr );
    for( ;; )
        ;
}

static int page_fault( DWORD dwLinAddr, DWORD dwEIP, DWORD dwErrCode )
{
	int			nR;
	DWORD		dwCR3;
	ThreadStt	*pThread;

	kdbg_printf( "\n[PAGE FAULT] Addr(%08X) EIP(0x%08X) ErrCode(0x%08X)\n", dwLinAddr, dwEIP, dwErrCode );
	pThread = get_current_thread();

	dwCR3 = pThread->pTSS->dwCR3;
	pf_tss.dwCR3 = dwCR3;
	// CR3를 교체한다.
	_asm {
		MOV EAX, dwCR3
		MOV CR3, EAX
		FLUSH_TLB
	}

	// 사용자 영역의 RDOnly 매핑에 대한 폴트인가?
	if( ( dwLinAddr & (DWORD)0x80000000 ) && (( dwErrCode & (DWORD)0x07 ) & PAGE_FAULT_W) )
	{
		if( pThread != NULL )
		{
			nR = copy_on_write( (DWORD*)pThread->pTSS->dwCR3, dwLinAddr );
			if( nR < 0 )
				kdbg_printf( "copy_on_write: 0x%X (error!)\n", dwLinAddr );
			else
				kdbg_printf( "copy_on_write: 0x%X (ok)\n", dwLinAddr );
			_asm int 1
		}
		return( 0 );
	}

	// user thread
	if( dwEIP & (DWORD)0x80000000 )
	{
		//change_thread_state( NULL, pThread, TS_ERROR );
		//kernel_scheduler();
	}

	// 커널 디버거를 띄운다.
	return( 1 );
}

// PAGE FAULT
// 인터럽트 핸들러로 되어 있던 page_fault_handler를 page fault task로 변경 2003-08-30
_declspec(naked) void pagefault_task_main()
{
	TSSStt	*pTSS;
	DWORD	dwLinAddr, dwErrCode;

REWIND:
	// Page Fault를 발생한 EIP를 구한다.
	_asm {
		PUSH EAX
		MOV  AX,GSEL_DATA32	   //
		MOV  DS,AX             //
		MOV  ES,AX             // 세그먼트 레지스터 설정.
		MOV  FS,AX             //
		MOV  GS,AX             //

		MOV  EAX, [ESP+4]
		MOV  dwErrCode, EAX		// error code 는 스택에 확실하게 들어간다.
		//MOV  EAX, [ESP+8]		//
		//MOV  dwRetAddr, EAX	// 2003-09-06 인터럽트 핸들러가 아니므로 더이상 이렇게 사용할 수 없다.

		MOV  EAX, CR2
		MOV  dwLinAddr, EAX		// linear address
		POP  EAX
		PUSHAD
	}

	//backlink의 tss를 조사해서 구해야 한다. 2003-09-06
	pTSS = (TSSStt*)pGetBackLinkTSS( &pf_tss );
	if( pTSS == NULL )
	{	// 이게 NULL ??!!
		kdbg_printf( "page_fault_task_main: pTSS = NULL!\n" );
		for( ;; );
	}

	// 실제 Page Fault 핸들러를 호출한다.
	page_fault( dwLinAddr, pTSS->dwEIP, dwErrCode );

	_asm
	{
		POPAD
		ADD ESP,4	// 스택에 에러 코드가 들어가므로 리턴하기 전에 4를 더해야 한다.
		INT 1  		// 디버거를 띄운다.
		IRETD
	}

	goto REWIND;
}

static int tflag = 1;

// 이게 정상적으로 작동하지 않는다.
void get_clk( __int64 *pClk )
{
	_asm {
		PUSHFD
		CLI
		MOV EBX, pClk
		RDTSC
		MOV [EBX],   EAX
		MOV [EBX+4], EDX
		POPFD
	}
}

static __int64 clk_prev = 0;
static int nClkCounter  = 0;
static DWORD G_dwCounterPerSec = 0;			// 초당 RDTSC Counter 값.

DWORD get_rdtsc_per_millis()
{
	return( G_dwCounterPerSec );
}

static void detect_frequency()
{
	__int64 clk_cur;	
	DWORD	dwDelta;
	
	if( nClkCounter == 0 )
		get_clk( &clk_prev );

	nClkCounter++;
	
	if( nClkCounter == (int)bell.dwTimerIntPerSecond )
	{
		__int64 clk_t;
		
		get_clk( &clk_cur );
		clk_t = clk_cur - clk_prev;
		i64_shr( &clk_t, 10);
		dwDelta = (DWORD)( clk_t ); 

		if( G_dwCounterPerSec == 0 )
			G_dwCounterPerSec = dwDelta;
		else
		{	// 평균을 구한다.
			G_dwCounterPerSec += dwDelta;
			G_dwCounterPerSec = G_dwCounterPerSec >> 1;
		}

		//kdbg_printf_ex( get_kernel_vconsole(), "freq/sec = %d\n", get_rdtsc_per_millis() );		

		nClkCounter = 0;
	}
}

// TIMER
// _declspec(naked)에서 지역변수를 static으로 해야만 한다.
// static을 빼 먹으면 스택 프레임을 따로 만들지 않기 때문에 만들지 않은 지역변수를
// 참조하여 스택이 깨진다.
_declspec(naked) static void timer_handler()
{
	static DWORD dwESP;

	_asm{
		PUSHAD				// 8 * 4 = 32
		PUSHFD				// 1 * 4 = 4
		CLI
		PUSH DS				//
		PUSH ES				//
		PUSH FS				//
		PUSH GS				//	4 * 4 = 16

		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX

		MOV EAX, ESP
		ADD EAX, 32 + 4 + 16
		MOV dwESP, EAX
	}

	{
		static char *pV;
		pV = (UCHAR*)(0xB8000 + 158);
		pV[0]++;
	}

	detect_frequency();
	
	// send eoi signal
	if( down_cond_value( &tflag ) < 0 )
		goto BACK;

	ktimer_scheduler( dwESP );

	tflag = 1;

BACK:
	vSendEOI( 0 );

	_asm {
		POP GS
		POP FS
		POP ES
		POP DS

		POPFD
		POPAD
		IRETD
	}
}

_declspec(naked) void fpu_exception()
{
	_asm PUSHFD
	_asm PUSHAD;

	kdbg_printf( "FPU Exception\n" );
	for(;;);

	_asm POPAD;
	_asm POPFD
	_asm IRETD;
}

_declspec(naked) void invalid_tss()
{
	_asm PUSHFD
	_asm PUSHAD;

	kdbg_printf( "Invalid TSS\n" );
	for(;;);

	_asm POPAD;
	_asm POPFD
	_asm IRETD;
}

static _declspec(naked) void irq3_handler()
{
	_asm {
		PUSHAD
	    PUSHFD
	    CLI
		PUSH DS
		PUSH ES
		PUSH FS
		PUSH GS
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}

	serial_irq( 3 );

	// send EOI signal
	vSendEOI( 3 );

	_asm {
		POP GS
		POP FS
		POP ES
		POP DS
		POPFD
		POPAD
		IRETD
	}
}

static _declspec(naked) void irq4_handler()
{
	_asm {
		PUSHAD
	    PUSHFD
	    CLI
		PUSH DS
		PUSH ES
		PUSH FS
		PUSH GS
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}

	serial_irq( 4 );

	// send EOI signal
	vSendEOI( 4 );

	_asm {
		POP GS
		POP FS
		POP ES
		POP DS
		POPFD
		POPAD
		IRETD
	}
}

static _declspec(naked) void irq5_handler()
{
	_asm {
		PUSHAD
	    PUSHFD
	    CLI
		PUSH DS
		PUSH ES
		PUSH FS
		PUSH GS
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}

	if(	user_def_int[5] != NULL )
		user_def_int[5]();

	// send EOI signal
	vSendEOI( 5 );

	_asm {
		POP GS
		POP FS
		POP ES
		POP DS
		POPFD
		POPAD
		IRETD
	}
}

static _declspec(naked) void irq7_handler()
{
	_asm {
		PUSHAD
	    PUSHFD
	    CLI
		PUSH DS
		PUSH ES
		PUSH FS
		PUSH GS
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}

	if(	user_def_int[7] != NULL )
		user_def_int[7]();


	// send EOI signal
	vSendEOI( 7 );

	_asm {
		POP GS
		POP FS
		POP ES
		POP DS
		POPFD
		POPAD
		IRETD
	}
}

static _declspec(naked) void irq8_handler()
{
	_asm {
		PUSHAD
	    PUSHFD
	    CLI
		PUSH DS
		PUSH ES
		PUSH FS
		PUSH GS
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}

	if(	user_def_int[8] != NULL )
		user_def_int[8]();


	// send EOI signal
	vSendEOI( 8 );

	_asm {
		POP GS
		POP FS
		POP ES
		POP DS
		POPFD
		POPAD
		IRETD
	}
}
static _declspec(naked) void irq9_handler()
{
	_asm {
		PUSHAD
	    PUSHFD
	    CLI
		PUSH DS
		PUSH ES
		PUSH FS
		PUSH GS
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}

	if(	user_def_int[9] != NULL )
		user_def_int[9]();

	// send EOI signal
	vSendEOI( 9 );

	_asm {
		POP GS
		POP FS
		POP ES
		POP DS
		POPFD
		POPAD
		IRETD
	}
}
static _declspec(naked) void irq10_handler()
{
	_asm {
		PUSHAD
	    PUSHFD
	    CLI
		PUSH DS
		PUSH ES
		PUSH FS
		PUSH GS
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}

	if(	user_def_int[10] != NULL )
		user_def_int[10]();

	// send EOI signal
	vSendEOI( 10 );

	_asm {
		POP GS
		POP FS
		POP ES
		POP DS
		POPFD
		POPAD
		IRETD
	}
}
static _declspec(naked) void irq11_handler()
{
	_asm {
		PUSHAD
	    PUSHFD
	    CLI
		PUSH DS
		PUSH ES
		PUSH FS
		PUSH GS
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}

	if(	user_def_int[11] != NULL )
		user_def_int[11]();

	// send EOI signal
	vSendEOI( 11 );

	kdbg_printf( "irq 11\n" );
	for( ;; );



	_asm {
		POP GS
		POP FS
		POP ES
		POP DS
		POPFD
		POPAD
		IRETD
	}
}

static _declspec(naked) void irq13_handler()
{
	_asm {
		PUSHAD
	    PUSHFD
	    CLI
		PUSH DS
		PUSH ES
		PUSH FS
		PUSH GS
		MOV AX, GSEL_DATA32
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}

	if(	user_def_int[13] != NULL )
		user_def_int[13]();

	// send EOI signal
	vSendEOI( 13 );

	_asm {
		POP GS
		POP FS
		POP ES
		POP DS
		POPFD
		POPAD
		IRETD
	}
}

/*
_declspec(naked) static void vINT1Handler()
{
    static DWORD addr[2];

    _asm PUSHFD;
    _asm PUSHAD;

    addr[0] = 0;
    //addr[1] = (DWORD)GSEL_DBGTSS32;       바로 TSS 셀렉터를 써도 되고 TASK GATE를 써도 된다.
    addr[1] = (DWORD)GSEL_DBG_TASK_GATE;

    _asm CALL FWORD PTR addr;

//  기계어 코드 FWORD PTR과 DWORD PTR의 차이점.
//  00016 ff 1d 00 00 00 00               call    FWORD PTR _
//  0001c ff 15 00 00 00 00               call    DWORD PTR _

    _asm POPAD;
    _asm POPFD;
    _asm IRETD;
}
*/
//-------------------------------------------------------------//
typedef struct {
	UINT16  wIntNo;  // 설정할 인터럽트 번호
	UINT16  wType;	 // 타입 값. 0이면 무시하고 넘어간다.
	DWORD   dwAddr;  // 주소.    0이면 배열의 끝으로 인식한다.
} IntExceptStt;

static IntExceptStt privateHandler[] = {
	{ 0x06, 0, (DWORD)invalid_op_handler	},	// Invalid Instruction
	{ 0x07, 0, (DWORD)fpu_exception			},	// FPU Exception
	{ 0x08, 0, (DWORD)double_fault_handler	},	// double fault (abort)
	{ 0x0A, 0, (DWORD)invalid_tss			},	// Invalid TSS Exception
	{ 0x0B, 0, (DWORD)no_seg_handler		},	// Segment not present
	{ 0x0C, 0, (DWORD)stack_fault			},	// Stack Segment Fault
	{ 0x0D, 0, (DWORD)gp_fault_handler		},	// gp fault handler
	{ 0x10, 0, (DWORD)int_10h_emulator		},	// v86 int 10h emulator
	{ 0x1A, 0, (DWORD)int_1Ah_emulator		},	// v86 int 1Ah emulator
	{ 0x20, 0, (DWORD)timer_handler			},  // IRO 0 Timer Handler
	{ 0x21, 0, (DWORD)kbd_handler			},  // IRO 1 Keyboard Handler
	{ 0x23, 0, (DWORD)irq3_handler			},	// COM 2, 4
	{ 0x24, 0, (DWORD)irq4_handler			},	// COM 1, 3
	{ 0x25, 0, (DWORD)irq5_handler			},	// NIC ??
	{ 0x26, 0, (DWORD)fdd_handler			},  // IRQ 6 FDD Handler
	{ 0x27, 0, (DWORD)irq7_handler			},  // IRQ 7 Handler
	{ 0x28, 0, (DWORD)irq8_handler			},  // IRQ 8 Handler
	{ 0x29, 0, (DWORD)irq8_handler			},  // IRQ 9 Handler
	{ 0x2A, 0, (DWORD)irq10_handler			},  // IRQ 10 Handler
	{ 0x2B, 0, (DWORD)irq11_handler			},  // IRQ 11 Handler
	{ 0x2C, 0, (DWORD)ps2_mouse_handler		},  // PS/2 Mouse Handler
	{ 0x2D, 0, (DWORD)irq13_handler			},  // IRQ 13 Handler
	{ 0x2E, 0, (DWORD)primary_hdd_irq14		},  // IRQ 14 Primary HDD Handler
	{ 0x2F, 0, (DWORD)secondary_hdd_irq15	},  // IRQ 15 Secondary HDD Handler
	{ 0x50, 0, (DWORD)system_call_wrapper	},  // System Call
	{ 0x51, 0, (DWORD)v86_system_call		},  // V86 System Call
    { 0x52, 0, (DWORD)ksyscall_handler		},  // New System Call
	// { 0x53 }	예약. 0x53번은 TLB Task Gate가 사용한다.
    { 0x54, 0, (DWORD)kgrxcall_handler		},  // Graphic System Call
    { 0x6D, 0, (DWORD)int_6Dh_emulator		},  // v86 int 6Dh emulator

	{ 0, 0, 0}  // 배열의 끝이다.
};
// bell내의 IDT를 설정한다.
void vSetDefaultIDT( IDTStt *pIDT, UINT16 wSize )
{
	int nI;

	memset( user_def_int, 0, sizeof( user_def_int ) );

	// assign the default int handler to all the interrupt vectors.
	for( nI = 0; nI < MAX_IDT; nI++ )
	{
		pIDT[nI].wSel = GSEL_CODE32;
		if( nI < 0x32 )
			pIDT[nI].wType = 0xEF00;		// trap gate
	    else
			pIDT[nI].wType = 0xEE00;		// interrupt gate

		dwSetIDTOffset( &pIDT[nI], (DWORD)vDefaultExceptionHandler );
	}

	// 개별적인 핸들러가 있는 것들을 설정해 준다.
	for( nI = 0; privateHandler[nI].dwAddr != 0; nI++ )
	{	// IDT Descriptor의 Type은 직접 설정해 줄 수 있다.
		if( privateHandler[nI].wType != 0 )  // 0이면 건너뛴다.
			pIDT[privateHandler[nI].wIntNo].wType = privateHandler[nI].wType;

		dwSetIDTOffset( &pIDT[privateHandler[nI].wIntNo], privateHandler[nI].dwAddr );
	}
}

// display idt
int display_idt( IDTStt *pIDT )
{
	MyCoffDbg2FuncStt	*pFunc;
	DWORD				dwOffset;
	int					nI, nTotal;
	char				*pFuncName, szT[10];

	kdbg_printf( "IDT table\n" );

	nTotal = 0;
	for( nI = 0; nI < 256; nI++ )
	{
		dwOffset = dwGetIDTOffset( &pIDT[nI] );
		if( dwOffset != (DWORD)vDefaultExceptionHandler )
		{
			kdbg_printf( "[0x%02X] %04X ", nI, pIDT[nI].wSel );

			if( pIDT[nI].wType == 0xE500 )
				kdbg_printf( "TASK   " );
			else if( pIDT[nI].wType == 0xEF00 )
				kdbg_printf( "TRAP   " );
			else if( pIDT[nI].wType == 0xEE00 )
				kdbg_printf( "INT    " );
			else
				kdbg_printf( "UNKNOWN" );

			// get func entry by address
			pFunc = get_func_ent_by_addr( pMy, dwOffset - (DWORD)RELOC_BASE );
			if( pFunc == NULL )
			{
				sprintf( szT, "0x%08X", dwOffset );
				pFuncName = szT;
			}
			else
			{
				pFuncName = &pMy->pStrTbl[ pFunc->nNameIndex ] ;
				pFuncName++;
			}

			kdbg_printf( " %s\n", pFuncName );

			nTotal++;
		}
	}

	kdbg_printf( "total %d user defined idt entries.\n", nTotal );

	return( nTotal );
}

// set user defined interrupt handler
// This function is used in the 3C905B interrupt handler.
void set_int_handler( int nInt, void *pHandler )
{
	kdbg_printf( "set_int_handler( %d ) = 0x08X\n", nInt, (DWORD)pHandler );

	user_def_int[nInt] = pHandler;
}
