/*
<스택>-------------------------------------------------------------------------------------;
; 5M - 4Byte 위치부터 위쪽으로 128K가 스택으로 쓰인다.									   ;
; 디버그 스택 64K + INIT 스택 64K를 합쳐 128K를 잡는다.									   ;
; INIT's ESP = 0x500000 - 4;															   ;
; DBG's  ESP = 0x500000 - 64K;															   ;
;------------------------------------------------------------------------------------------;
*/

#include "bellona2.h"


DescriptorStt   gdt[MAX_GDT];	// GDT
DescriptorStt	ldt[MAX_LDT];	// LDT
IDTStt          idt[MAX_IDT];	// IDT
IDTRStt         idtr;			// IDTR
GDTRStt			gdtr;			// GDTR의 값

int				nTotalGDTEnt = TOTAL_GSEL;	// GDT에 들어있는 엔트리의 개수

BellonaStt		bell;
// my debugging symbol
MyCoffDbg2Stt	*pMy = NULL;

static UCHAR szDebugPrompt[2] = { '!', 0 };  //{ 175, 0 };
static UCHAR szInitPrompt[2]  = { '$', 0 };
static int nTraceRepeat = 0;

_declspec(naked) void bellona2_main()
{
	int							nI;
	PeImgStt					pe;
	UCHAR						*pVideo;
	MY_IMAGE_DEBUG_DIRECTORY	*pDbgDir;
	UINT16						wBackLink;
	int							nVideoLine;
	DWORD						dwDestAddr;			// 점프해간 주소를 출력하기 위한 변수.
	DWORD						*pT, dwTemp;
	DWORD						dwLastImageByte;
	DWORD						dwDebugPosition;
	LONG						lBase, lDestBase;
	DWORD						dwBuiltInV86Lib, dwBuiltInV86LibSize;	
	
	// 세그먼트들을 초기화 한다.
	_asm {
		MOV AX,DS
		MOV ES,AX
		MOV FS,AX
		MOV GS,AX
	 	MOV SS,AX

		MOV EAX, KERNEL_INIT_STACK_BASE-4	
		MOV EBP, EAX
		SUB EAX, __LOCAL_SIZE;//KERNEL_MAIN_LOCAL_VAR_SIZE	
		MOV ESP, EAX
	}

	pVideo		= (UCHAR*)0xB8000;
	nVideoLine	= 8;
	pDbgDir		= NULL;

	// BLOAD에서 로드한 베이스를 설정한다.
	pe.pBuff = (UCHAR*)IMAGE_BASE;
	pe.pDosHd = (MY_IMAGE_DOS_HEADER*)( pe.pBuff ); 
	// 기타 베이스 오프셋을 설정한다.
	pe.lPeBase = pe.lIfBase = pe.pDosHd->e_lfanew;
	pe.lIoBase = pe.lPeBase + sizeof( MY_IMAGE_FILE_HEADER );
	// 각 HEADER의 주소를 계산한다.
	pe.pPeHd = (MY_IMAGE_PE_HEADER*)&pe.pBuff[ pe.pDosHd->e_lfanew ];
	pe.pIfHd = (MY_IMAGE_FILE_HEADER*)pe.pPeHd;
	pe.pIoHd = (MY_IMAGE_OPTIONAL_HEADER*)&pe.pBuff[ pe.pDosHd->e_lfanew + sizeof(MY_IMAGE_FILE_HEADER)];

	// 첫번째 섹션의 헤더위치를 구한다.
	lBase = pe.lIoBase + sizeof( MY_IMAGE_OPTIONAL_HEADER );

    // 옮겨갈 곳 계산.
	lDestBase = 0;
	pe.pBase = (UCHAR*)RELOC_BASE;  // 옮겨갈 곳의 주소.
    
    // 2002-12-13 커널 이미지 들어갈 곳을 0으로 클리어.
    memset( pe.pBase, 0, 512 ); 
	
    // 섹션 바로 윗부분까지는 그냥 옮겨버린다.
	memcpy( pe.pBase, pe.pBuff, lBase );

	for( nI = 0; nI < pe.pIfHd->NumberOfSections && nI < BDF_MAX_PE_SECTION -1; nI++ )
	{
		// 섹션 헤더의 위치	 계산
		pe.sect[nI].lBase = lBase;                         
		// 섹션 헤더의 포인터 계산
		pe.sect[nI].pPtr = (MY_IMAGE_SECTION_HEADER*)&pe.pBuff[ lBase ];  
		// 섹션명 복사
		memset( pe.sect[nI].szName, 0, 8 );
		memcpy( pe.sect[nI].szName, pe.sect[nI].pPtr, 8 );
		pT = (DWORD*)pe.sect[nI].szName;
		
		// 섹션명을 화면에 복사한다.
		nWriteToVideoMem_Len( 0, nVideoLine++, pe.sect[nI].szName, 8 );

		// 필요에 따라 섹션의 타입을 결정한다.
		pe.sect[nI].nType = nI;

		// 섹션의 바디만 정해진 위치에 옮겨 놓으면 된다.
		dwTemp = (DWORD)&pe.pBuff[pe.sect[nI].pPtr->PointerToRawData];
        memset( &pe.pBase[ pe.sect[nI].pPtr->VirtualAddress ], 0, pe.sect[nI].pPtr->VirtualSize );  // 0으로 클리어.
		memcpy( &pe.pBase[ pe.sect[nI].pPtr->VirtualAddress ], (char*)dwTemp, pe.sect[nI].pPtr->SizeOfRawData );
		dwTemp += pe.sect[nI].pPtr->SizeOfRawData;
		dwTemp = (DWORD)( ( (dwTemp + 511) / 512 ) * 512 );
														
		lBase += sizeof( MY_IMAGE_SECTION_HEADER );
		
		// 이미지의 마지막 바이트 위치
		dwLastImageByte = (DWORD)&pe.pBase[ pe.sect[nI].pPtr->VirtualAddress ] + pe.sect[nI].pPtr->VirtualSize;
	}
	
	// 디버깅 정보를 옮긴다. (VC6 with CODEMAP utility)
	if( pe.pIoHd->dd_Debug_dwVAddr != 0 )
	{
		char	*pS;
		DWORD   *pX;
		
	 	// dd_Debug_dwVAddr은 그냥 오프셋으로 사용한다.
		pS = (char*)&pe.pBuff[ pe.pIoHd->dd_Debug_dwVAddr ];
		pX = (DWORD*)pS; 
		
		if( pX[0] == (DWORD)0x46464F43 )
		{
			dwDebugPosition = dwLastImageByte;
			dwLastImageByte += pe.pIoHd->dd_Debug_dwSize;
			
			// 디버깅 정보를 복사한다.
			memcpy( (UCHAR*)dwDebugPosition, pS, pe.pIoHd->dd_Debug_dwSize );
		}
		else
			dwDebugPosition = 0;
	}

	// 도스 헤더 바로 다음 위치에 MAIGIC값과 V86Lib 오프셋이 있는지 확인.
	dwBuiltInV86Lib = 0;
	dwBuiltInV86LibSize = 0;
	pT = (DWORD*)( (DWORD)pe.pDosHd + sizeof( MY_IMAGE_DOS_HEADER ) );
	if( pT[0] == V86PARAM_MAGIC )
	{
		dwBuiltInV86Lib	    = dwLastImageByte;
		dwBuiltInV86LibSize = pT[2]; // 사이즈
		// DOS STUB를 옮긴다.
		memcpy( (BYTE*)dwBuiltInV86Lib, (BYTE*)pT[1] + (DWORD)pe.pDosHd, dwBuiltInV86LibSize );
		dwLastImageByte += dwBuiltInV86LibSize;
	}	

	// 이미지의 마지막 바이트 오프셋을 4096으로 올림한다.
	dwLastImageByte = (DWORD)( ( ( dwLastImageByte + 4095  ) / 4096 ) * 4096 );
 
	// 마지막 섹션임을 표시한다.
	pe.sect[nI].nType = -1;

 	// 모든 섹션을 재배치 하였으니 이제 전역변수를 사용할 수 있다.
	nWriteToVideoMem( 0, nVideoLine++, "Image Relocation - ok."  );

	// 재배치한 4메가 영역으로 점프해 가 버리자.
	_asm {
			// 첫번째 섹션이 Text 섹션이라고 가정한다.
			MOV EAX,OFFSET JUMP_DEST  // 실제 재배치된 상태에서의 주소가 구해진다.
			MOV dwDestAddr, EAX       // 바로 점프하면 된다.
			JMP EAX
JUMP_DEST:
			NOP
	} 
///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// 여기서 부터 전역변수를 사용할 수 있다.  //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////	

    // get coff debug information
	pMy = get_coff_dbg2( (char*)dwDebugPosition );
	if( pMy == NULL )
		nWriteToVideoMem( 0, nVideoLine++, "no debug information." );
	else
		nWriteToVideoMem( 0, nVideoLine++, "Debug information - ok." );

	// initial the FPU(?)  (Is this right?)
	vAsmInitFPU();

    // Copy the GDT table from DOS area.
	vResetGDT( gdt, &gdtr, MAX_GDT * sizeof(DescriptorStt) );	
	
	// reprogramming the PIC (Programmable Interrupt Controller)
    vReprogramPIC( pm_pic_data );
	nWriteToVideoMem_Len( 0, nVideoLine++, "PIC - ok", strlen("PIC - ok.") );

	// Make the IDT table
	vSetDefaultIDT( idt, MAX_IDT * sizeof( IDTStt ) );
	idtr.dwAddr = (DWORD)&idt;
	idtr.wSize  = MAX_IDT * sizeof( IDTStt );

	// Set the Keyboard and timer handler
	vEnableInterrupt( &idtr );  

	// IRQ0 타이머 인터럽트 발생빈도를 조정한다.
	bell.dwTimerIntPerSecond = 50; // 2002-12-22 
	// 1193180 / 초당 발생 회수 = Timer Interval 값.
	bell.dwTimerInterval = (DWORD)1193180 / bell.dwTimerIntPerSecond;  
	bell.dwTickCarry = 0;
	bell.dwTick = 0;
	bell.pExp = (MY_IMAGE_EXPORT_DIRECTORY*)( pe.pIoHd->dd_Export_dwVAddr + RELOC_BASE );
	vSetTimerInterval( bell.dwTimerInterval );  

	// Get the physical memory size
	bell.nPhysSize = nGetPhysMemSize();
	{
		char szT[260];
		sprintf( szT, "Physical Memory Size : %d", bell.nPhysSize );
		nWriteToVideoMem_Len( 0, nVideoLine++, szT, strlen( szT )  );
		// Last Image Byte
		sprintf( szT, "dwLastImageByte : 0x%X", dwLastImageByte );
		nWriteToVideoMem( 0, nVideoLine++, szT  );
	}	    

	// 커널의 페이징 시스템을 초기화한다.
	bell.nPhysRefSize = bell.nPhysSize / 4096;	// 한 페이지 당 1바이트를 할당한다.
	bell.pPhysRefTbl  = (UCHAR*)dwLastImageByte;
	dwLastImageByte   += bell.nPhysRefSize;
	dwLastImageByte   = (DWORD)( ( ( dwLastImageByte + 4095  ) / 4096 ) * 4096 );	// 4096 올림
	bell.pPD		  = (DWORD*)dwLastImageByte;
	dwLastImageByte	  += 4096 * 3;		// 페이지 디렉토리 1개, 페이지 테이블 2개
	bell.dwLastImageByte = dwLastImageByte;

	// PhysRefTbl을 0으로 클리어한다.
	memset( bell.pPhysRefTbl, 0, bell.nPhysRefSize ); // 메모리 512M일 경우 128K 필요.
	memset( bell.pPD, 0, 4096 * 3);

	// 디폴트 매핑
	vInitKernelPage( bell.pPD, RELOC_BASE, dwLastImageByte - RELOC_BASE );
	
	// 페이징을 가동한다.
	vEnablePaging( bell.pPD );

    // set iopl to 3
	set_iopl( 3 );

	// V86라이브러리의 크기와 사이즈를 설정한 후 이동한다.
	if( dwBuiltInV86Lib != 0 )
	{
		memcpy( (BYTE*)V86LIB_ADDR, (BYTE*)dwBuiltInV86Lib, dwBuiltInV86LibSize );
		bell.dwBuiltInV86Lib    = dwBuiltInV86Lib;
		bell.nBuiltInV86LibSize = dwBuiltInV86LibSize;
	}

	// initialize system event structure and register events
	init_system_event();

	// turn off fdd motor
	fdd_motor_off();
	
    // initialize keyboard led
	init_kbd_and_ps2_mouse();
    vInitKBDLed();		
    
    // initialize kernel memory pool
	init_memory_pool( bell.pPD, &kmp, (DWORD)KERNEL_MEM_POOL_ADDR, (DWORD)KERNEL_MEM_POOL_SIZE );

	// initialize kernel scheduler
	kinit_scheduler();
	
	// LDT Descriptor를 만든다.  (LDT를 만들지 않아도 TSS는 작동한다.)
	//----------------------------------------------------//
	memset( ldt, 0, sizeof( ldt ) );
	vMakeGDTDescriptor( GSEL_LDT, (DWORD)ldt, (DWORD)sizeof( ldt ), (UCHAR)0x82, (UCHAR)0x00 );
	_asm { 
		MOV  AX, GSEL_LDT
		LLDT AX
	}				  

	// initialize debugger register
	vInitDebugRegister();
        
	// tlb_tss를 만든다.
	make_tlb_tss();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	{	// init, 디버거 TSS를 생성한다.
		static DWORD dwESP, dwEIP;
		
		//############[ Debug TSS를 만든다 ]#################//		
		_asm MOV DWORD PTR dwEIP, OFFSET DEBUG_ENTRANCE;
		_asm MOV dwESP, KERNEL_DBG_STACK_BASE;
		// GDT에 TASK Descriptor를 만든다. (주의 !! GSEL_xx를 gdt의 배열 첨자로 쓰면 죽는다.)
		vMakeGDTDescriptor( GSEL_DBGTSS32, (DWORD)&dbg_tss, (DWORD)sizeof( dbg_tss ), (UCHAR)0x89, (UCHAR)0x00 );
		// make task gate in IDT (INT 1, 3)
		vMakeTaskGate_in_IDT( ISEL_INT_1, GSEL_DBGTSS32 );
		vMakeTaskGate_in_IDT( ISEL_INT_3, GSEL_DBGTSS32 );
		// Debug TSS를 만든다.
		vMakeTSS( &dbg_tss, dwEIP, dwESP );
		
		//############[ Init TSS를 만든다 ]##################//
		_asm MOV DWORD PTR dwEIP, OFFSET init_task_main;
		_asm MOV dwESP, ESP;
		// GDT에 TASK Descriptor를 만든다.
		vMakeGDTDescriptor( GSEL_INITTSS32, (DWORD)&init_tss, (DWORD)sizeof( init_tss ), (UCHAR)0x89, (UCHAR)0x00 );
		// make init tss
		vMakeTSS( &init_tss, dwEIP, dwESP );

		//############[ Page Fault TSS를 만든다 ]###########//
		_asm MOV DWORD PTR dwEIP, OFFSET pagefault_task_main;
		_asm MOV dwESP, PAGE_FAULT_STACK_BASE;
		// GDT에 TASK Descriptor를 만든다.
		vMakeGDTDescriptor( GSEL_PFTSS32, (DWORD)&pf_tss, (DWORD)sizeof( pf_tss ), (UCHAR)0x89, (UCHAR)0x00 );
		vMakeTaskGate_in_IDT( ISEL_PF, GSEL_PFTSS32 );
		// make init tss
		vMakeTSS( &pf_tss, dwEIP, dwESP );
		//---------------------------------------------------//
		
		// Debugger의 Task Gate를 GDT에 생성한다.
		vMakeTaskGate_in_GDT( GSEL_DBG_TASK_GATE,  GSEL_DBGTSS32 );

		// TSS의 이름과 주소를 등록한다.
		nAppendTSSTbl( "debugger",  (DWORD)&dbg_tss  );
		nAppendTSSTbl( "itask",      (DWORD)&init_tss );
		nAppendTSSTbl( "pagefault", (DWORD)&pf_tss   );
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
						 	
	// TR을 초기화시키기 위해 임시로 하나 만들어 둔다.
	vMakeGDTDescriptor( GSEL_DUMMY_TSS32, (DWORD)&init_tss, (DWORD)sizeof( init_tss ), (UCHAR)0x89, (UCHAR)0x00 );
	vMakeGDTDescriptor( GSEL_DUMMY_V86TSS, 0, (DWORD)sizeof( V86TSSStt ), (UCHAR)0x89, (UCHAR)0x00 );

	//TR을 초기화한다. ( 먼저 init context로 실행된다. )
	_asm {
		//MOV AX, GSEL_DUMMY_TSS32
		MOV AX, GSEL_INITTSS32
		LTR AX	// TR을 로드할 때 해당 디스크립터가 TSS가 아니면 예외가 발생한다.
	}
	set_init_phase( INIT_PHASE_TR_SET );
    
	{// 기본적인 프로세스와 쓰레드를 생성한다.
		ProcessStt	*pP;
		ThreadStt   *pInit, *pSniper, *pRShell, *pV86;
			
		// init process
		pP = kcreate_process( NULL ); 
		k_set_process_alias( pP,   "init"	  );

		pInit   = kcreate_thread( pP, 0, (DWORD)init_thread,   0, TS_READY_NORMAL );	// Owner Process, StackSize, Entry Function, Parameter
		pSniper = kcreate_thread( pP, 0, (DWORD)sniper_thread, 0, TS_READY_TIME_CRITICAL );	// Owner Process, StackSize, Entry Function, Parameter
		pRShell = kcreate_thread( pP, 0, (DWORD)rshell_thread, 0, TS_READY_NORMAL );	// Owner Process, StackSize, Entry Function, Parameter
		pV86	   = kcreate_thread( pP, (128*1024), (DWORD)v86lib_thread, 0, TS_READY_NORMAL );	// Owner Process, StackSize, Entry Function, Parameter
		k_set_thread_alias( pInit,   "init_main"    	);
		k_set_thread_alias( pSniper, "init_sniper"   	);
		k_set_thread_alias( pRShell, "init_rshell"   	);
		k_set_thread_alias( pV86,    "init_v86"      	);
	}

	// 프로세스 쓰레드 ID를 초기화 한다.
	bell.dwNextProcessThreadID = 99;

	// thread switch to init_thread
	kernel_scheduler();
		
	// call init task main
	init_task_main();

//************************************************************************//
//						Interrup 1 debugger handler						  //  
//************************************************************************//
DEBUG_ENTRANCE:

DBG_LOOP:
	// 커널 메모리 매핑이 변경된 것이 있으면 복사해 온다. 
	if( bell.dwDbgMappingFlag != get_kernel_mapping_flag() )
	{	// 하위 2GB 매핑을 복사한다. 
		DWORD *pPD;
		_asm {
			MOV EAX,CR3
			MOV pPD,EAX
		}
		memcpy( pPD, bell.pPD, 4096 / 2 );
		bell.dwDbgMappingFlag = get_kernel_mapping_flag();
	}

	// 키보드를 디버거쪽으로 끌어온다.
	set_debugger_active( 1 );

	// debugger의 CR3를 debugee의 CR3로 교체해야 그쪽 메모리에 접근(덤프, 역어셈블등...)할 수 있다.
	kdbg_change_CR3();
	
	// INT3에 의해 디버거로 들어온 것인지 확인하고 원래의 코드를 복구한다.
	swap_cc_by_org_code();

	// 바로 다음에 실행할 코드를 보여준다.
	kdbg_disp_next_code();

	// 디버거 쉘을 실행한다.
	if( nTraceRepeat > 0 )
	{
		kdbg_printf( "TRACE_REPEAT : %d\n", nTraceRepeat );
		nTraceRepeat--;
	}
	else
		kdbg_shell(  (char*)szDebugPrompt ); 

	// 디버거에서 리턴할 때 TSS Descriptor의 BUSY Bit가 1로 설정되어 있어야 한다.
	wBackLink = wGetBackLink( &dbg_tss );	// 현재 TSS의 BackLink를 구한다.
	vSetDescriptorBusyBit( &gdt[ wBackLink/8 ], 1 );

	set_debugger_active( 0 );

	// set rf flag
	kdbg_set_debugee_rf( 1 );

	_asm IRETD;		// 쉘에서 리턴되면 원래의 IRETD에 의해 TASK가 실행된다.

	goto DBG_LOOP;
}

int kdbg_set_trace_repeat( int nRepeat )
{
	nTraceRepeat = nRepeat;
	return( 0 );
}

// 지정된 길이 이상은 출력하지 않는다.
int nWriteToVideoMem_Len( int x, int y, char *pS, int nLen )
{
	int				nI;
	VConsoleStt		*pKVC;
	char			*pVideo;
	
	pVideo = (char*)0xB8000;
	pVideo += (x * 2) + (y * 160);

	pKVC = get_current_vconsole();

	for( nI = 0; nI < nLen; nI++ )
	{
		if( pS[nI] == 0 )
			break;
		pVideo[nI*2] = pS[nI];
		if( pKVC != NULL )
			pKVC->con_buff[ nI + x + (y*80) ].ch = pS[nI];
	}

	return(0);

}

// 0을 만날때까지 비디오 메모리에 출력한다.
int nWriteToVideoMem( int x, int y, char *pS )
{
	int   nI;
	VConsoleStt		*pKVC;
	char			*pVideo;
	
	pVideo = (char*)0xB8000;
	pVideo += (x * 2) + (y * 160);

	pKVC = get_current_vconsole();

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		pVideo[nI*2] = pS[nI];
		if( pKVC != NULL )
			pKVC->con_buff[ nI + x + (y*80) ].ch = pS[nI];
	}

	return(0);
}







