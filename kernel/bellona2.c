/*
<����>-------------------------------------------------------------------------------------;
; 5M - 4Byte ��ġ���� �������� 128K�� �������� ���δ�.									   ;
; ����� ���� 64K + INIT ���� 64K�� ���� 128K�� ��´�.									   ;
; INIT's ESP = 0x500000 - 4;															   ;
; DBG's  ESP = 0x500000 - 64K;															   ;
;------------------------------------------------------------------------------------------;
*/

#include "bellona2.h"


DescriptorStt   gdt[MAX_GDT];	// GDT
DescriptorStt	ldt[MAX_LDT];	// LDT
IDTStt          idt[MAX_IDT];	// IDT
IDTRStt         idtr;			// IDTR
GDTRStt			gdtr;			// GDTR�� ��

int				nTotalGDTEnt = TOTAL_GSEL;	// GDT�� ����ִ� ��Ʈ���� ����

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
	DWORD						dwDestAddr;			// �����ذ� �ּҸ� ����ϱ� ���� ����.
	DWORD						*pT, dwTemp;
	DWORD						dwLastImageByte;
	DWORD						dwDebugPosition;
	LONG						lBase, lDestBase;
	DWORD						dwBuiltInV86Lib, dwBuiltInV86LibSize;	
	
	// ���׸�Ʈ���� �ʱ�ȭ �Ѵ�.
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

	// BLOAD���� �ε��� ���̽��� �����Ѵ�.
	pe.pBuff = (UCHAR*)IMAGE_BASE;
	pe.pDosHd = (MY_IMAGE_DOS_HEADER*)( pe.pBuff ); 
	// ��Ÿ ���̽� �������� �����Ѵ�.
	pe.lPeBase = pe.lIfBase = pe.pDosHd->e_lfanew;
	pe.lIoBase = pe.lPeBase + sizeof( MY_IMAGE_FILE_HEADER );
	// �� HEADER�� �ּҸ� ����Ѵ�.
	pe.pPeHd = (MY_IMAGE_PE_HEADER*)&pe.pBuff[ pe.pDosHd->e_lfanew ];
	pe.pIfHd = (MY_IMAGE_FILE_HEADER*)pe.pPeHd;
	pe.pIoHd = (MY_IMAGE_OPTIONAL_HEADER*)&pe.pBuff[ pe.pDosHd->e_lfanew + sizeof(MY_IMAGE_FILE_HEADER)];

	// ù��° ������ �����ġ�� ���Ѵ�.
	lBase = pe.lIoBase + sizeof( MY_IMAGE_OPTIONAL_HEADER );

    // �Űܰ� �� ���.
	lDestBase = 0;
	pe.pBase = (UCHAR*)RELOC_BASE;  // �Űܰ� ���� �ּ�.
    
    // 2002-12-13 Ŀ�� �̹��� �� ���� 0���� Ŭ����.
    memset( pe.pBase, 0, 512 ); 
	
    // ���� �ٷ� ���κб����� �׳� �Űܹ�����.
	memcpy( pe.pBase, pe.pBuff, lBase );

	for( nI = 0; nI < pe.pIfHd->NumberOfSections && nI < BDF_MAX_PE_SECTION -1; nI++ )
	{
		// ���� ����� ��ġ	 ���
		pe.sect[nI].lBase = lBase;                         
		// ���� ����� ������ ���
		pe.sect[nI].pPtr = (MY_IMAGE_SECTION_HEADER*)&pe.pBuff[ lBase ];  
		// ���Ǹ� ����
		memset( pe.sect[nI].szName, 0, 8 );
		memcpy( pe.sect[nI].szName, pe.sect[nI].pPtr, 8 );
		pT = (DWORD*)pe.sect[nI].szName;
		
		// ���Ǹ��� ȭ�鿡 �����Ѵ�.
		nWriteToVideoMem_Len( 0, nVideoLine++, pe.sect[nI].szName, 8 );

		// �ʿ信 ���� ������ Ÿ���� �����Ѵ�.
		pe.sect[nI].nType = nI;

		// ������ �ٵ� ������ ��ġ�� �Ű� ������ �ȴ�.
		dwTemp = (DWORD)&pe.pBuff[pe.sect[nI].pPtr->PointerToRawData];
        memset( &pe.pBase[ pe.sect[nI].pPtr->VirtualAddress ], 0, pe.sect[nI].pPtr->VirtualSize );  // 0���� Ŭ����.
		memcpy( &pe.pBase[ pe.sect[nI].pPtr->VirtualAddress ], (char*)dwTemp, pe.sect[nI].pPtr->SizeOfRawData );
		dwTemp += pe.sect[nI].pPtr->SizeOfRawData;
		dwTemp = (DWORD)( ( (dwTemp + 511) / 512 ) * 512 );
														
		lBase += sizeof( MY_IMAGE_SECTION_HEADER );
		
		// �̹����� ������ ����Ʈ ��ġ
		dwLastImageByte = (DWORD)&pe.pBase[ pe.sect[nI].pPtr->VirtualAddress ] + pe.sect[nI].pPtr->VirtualSize;
	}
	
	// ����� ������ �ű��. (VC6 with CODEMAP utility)
	if( pe.pIoHd->dd_Debug_dwVAddr != 0 )
	{
		char	*pS;
		DWORD   *pX;
		
	 	// dd_Debug_dwVAddr�� �׳� ���������� ����Ѵ�.
		pS = (char*)&pe.pBuff[ pe.pIoHd->dd_Debug_dwVAddr ];
		pX = (DWORD*)pS; 
		
		if( pX[0] == (DWORD)0x46464F43 )
		{
			dwDebugPosition = dwLastImageByte;
			dwLastImageByte += pe.pIoHd->dd_Debug_dwSize;
			
			// ����� ������ �����Ѵ�.
			memcpy( (UCHAR*)dwDebugPosition, pS, pe.pIoHd->dd_Debug_dwSize );
		}
		else
			dwDebugPosition = 0;
	}

	// ���� ��� �ٷ� ���� ��ġ�� MAIGIC���� V86Lib �������� �ִ��� Ȯ��.
	dwBuiltInV86Lib = 0;
	dwBuiltInV86LibSize = 0;
	pT = (DWORD*)( (DWORD)pe.pDosHd + sizeof( MY_IMAGE_DOS_HEADER ) );
	if( pT[0] == V86PARAM_MAGIC )
	{
		dwBuiltInV86Lib	    = dwLastImageByte;
		dwBuiltInV86LibSize = pT[2]; // ������
		// DOS STUB�� �ű��.
		memcpy( (BYTE*)dwBuiltInV86Lib, (BYTE*)pT[1] + (DWORD)pe.pDosHd, dwBuiltInV86LibSize );
		dwLastImageByte += dwBuiltInV86LibSize;
	}	

	// �̹����� ������ ����Ʈ �������� 4096���� �ø��Ѵ�.
	dwLastImageByte = (DWORD)( ( ( dwLastImageByte + 4095  ) / 4096 ) * 4096 );
 
	// ������ �������� ǥ���Ѵ�.
	pe.sect[nI].nType = -1;

 	// ��� ������ ���ġ �Ͽ����� ���� ���������� ����� �� �ִ�.
	nWriteToVideoMem( 0, nVideoLine++, "Image Relocation - ok."  );

	// ���ġ�� 4�ް� �������� ������ �� ������.
	_asm {
			// ù��° ������ Text �����̶�� �����Ѵ�.
			MOV EAX,OFFSET JUMP_DEST  // ���� ���ġ�� ���¿����� �ּҰ� ��������.
			MOV dwDestAddr, EAX       // �ٷ� �����ϸ� �ȴ�.
			JMP EAX
JUMP_DEST:
			NOP
	} 
///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// ���⼭ ���� ���������� ����� �� �ִ�.  //////////////////////////////////
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

	// IRQ0 Ÿ�̸� ���ͷ�Ʈ �߻��󵵸� �����Ѵ�.
	bell.dwTimerIntPerSecond = 50; // 2002-12-22 
	// 1193180 / �ʴ� �߻� ȸ�� = Timer Interval ��.
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

	// Ŀ���� ����¡ �ý����� �ʱ�ȭ�Ѵ�.
	bell.nPhysRefSize = bell.nPhysSize / 4096;	// �� ������ �� 1����Ʈ�� �Ҵ��Ѵ�.
	bell.pPhysRefTbl  = (UCHAR*)dwLastImageByte;
	dwLastImageByte   += bell.nPhysRefSize;
	dwLastImageByte   = (DWORD)( ( ( dwLastImageByte + 4095  ) / 4096 ) * 4096 );	// 4096 �ø�
	bell.pPD		  = (DWORD*)dwLastImageByte;
	dwLastImageByte	  += 4096 * 3;		// ������ ���丮 1��, ������ ���̺� 2��
	bell.dwLastImageByte = dwLastImageByte;

	// PhysRefTbl�� 0���� Ŭ�����Ѵ�.
	memset( bell.pPhysRefTbl, 0, bell.nPhysRefSize ); // �޸� 512M�� ��� 128K �ʿ�.
	memset( bell.pPD, 0, 4096 * 3);

	// ����Ʈ ����
	vInitKernelPage( bell.pPD, RELOC_BASE, dwLastImageByte - RELOC_BASE );
	
	// ����¡�� �����Ѵ�.
	vEnablePaging( bell.pPD );

    // set iopl to 3
	set_iopl( 3 );

	// V86���̺귯���� ũ��� ����� ������ �� �̵��Ѵ�.
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
	
	// LDT Descriptor�� �����.  (LDT�� ������ �ʾƵ� TSS�� �۵��Ѵ�.)
	//----------------------------------------------------//
	memset( ldt, 0, sizeof( ldt ) );
	vMakeGDTDescriptor( GSEL_LDT, (DWORD)ldt, (DWORD)sizeof( ldt ), (UCHAR)0x82, (UCHAR)0x00 );
	_asm { 
		MOV  AX, GSEL_LDT
		LLDT AX
	}				  

	// initialize debugger register
	vInitDebugRegister();
        
	// tlb_tss�� �����.
	make_tlb_tss();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	{	// init, ����� TSS�� �����Ѵ�.
		static DWORD dwESP, dwEIP;
		
		//############[ Debug TSS�� ����� ]#################//		
		_asm MOV DWORD PTR dwEIP, OFFSET DEBUG_ENTRANCE;
		_asm MOV dwESP, KERNEL_DBG_STACK_BASE;
		// GDT�� TASK Descriptor�� �����. (���� !! GSEL_xx�� gdt�� �迭 ÷�ڷ� ���� �״´�.)
		vMakeGDTDescriptor( GSEL_DBGTSS32, (DWORD)&dbg_tss, (DWORD)sizeof( dbg_tss ), (UCHAR)0x89, (UCHAR)0x00 );
		// make task gate in IDT (INT 1, 3)
		vMakeTaskGate_in_IDT( ISEL_INT_1, GSEL_DBGTSS32 );
		vMakeTaskGate_in_IDT( ISEL_INT_3, GSEL_DBGTSS32 );
		// Debug TSS�� �����.
		vMakeTSS( &dbg_tss, dwEIP, dwESP );
		
		//############[ Init TSS�� ����� ]##################//
		_asm MOV DWORD PTR dwEIP, OFFSET init_task_main;
		_asm MOV dwESP, ESP;
		// GDT�� TASK Descriptor�� �����.
		vMakeGDTDescriptor( GSEL_INITTSS32, (DWORD)&init_tss, (DWORD)sizeof( init_tss ), (UCHAR)0x89, (UCHAR)0x00 );
		// make init tss
		vMakeTSS( &init_tss, dwEIP, dwESP );

		//############[ Page Fault TSS�� ����� ]###########//
		_asm MOV DWORD PTR dwEIP, OFFSET pagefault_task_main;
		_asm MOV dwESP, PAGE_FAULT_STACK_BASE;
		// GDT�� TASK Descriptor�� �����.
		vMakeGDTDescriptor( GSEL_PFTSS32, (DWORD)&pf_tss, (DWORD)sizeof( pf_tss ), (UCHAR)0x89, (UCHAR)0x00 );
		vMakeTaskGate_in_IDT( ISEL_PF, GSEL_PFTSS32 );
		// make init tss
		vMakeTSS( &pf_tss, dwEIP, dwESP );
		//---------------------------------------------------//
		
		// Debugger�� Task Gate�� GDT�� �����Ѵ�.
		vMakeTaskGate_in_GDT( GSEL_DBG_TASK_GATE,  GSEL_DBGTSS32 );

		// TSS�� �̸��� �ּҸ� ����Ѵ�.
		nAppendTSSTbl( "debugger",  (DWORD)&dbg_tss  );
		nAppendTSSTbl( "itask",      (DWORD)&init_tss );
		nAppendTSSTbl( "pagefault", (DWORD)&pf_tss   );
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
						 	
	// TR�� �ʱ�ȭ��Ű�� ���� �ӽ÷� �ϳ� ����� �д�.
	vMakeGDTDescriptor( GSEL_DUMMY_TSS32, (DWORD)&init_tss, (DWORD)sizeof( init_tss ), (UCHAR)0x89, (UCHAR)0x00 );
	vMakeGDTDescriptor( GSEL_DUMMY_V86TSS, 0, (DWORD)sizeof( V86TSSStt ), (UCHAR)0x89, (UCHAR)0x00 );

	//TR�� �ʱ�ȭ�Ѵ�. ( ���� init context�� ����ȴ�. )
	_asm {
		//MOV AX, GSEL_DUMMY_TSS32
		MOV AX, GSEL_INITTSS32
		LTR AX	// TR�� �ε��� �� �ش� ��ũ���Ͱ� TSS�� �ƴϸ� ���ܰ� �߻��Ѵ�.
	}
	set_init_phase( INIT_PHASE_TR_SET );
    
	{// �⺻���� ���μ����� �����带 �����Ѵ�.
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

	// ���μ��� ������ ID�� �ʱ�ȭ �Ѵ�.
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
	// Ŀ�� �޸� ������ ����� ���� ������ ������ �´�. 
	if( bell.dwDbgMappingFlag != get_kernel_mapping_flag() )
	{	// ���� 2GB ������ �����Ѵ�. 
		DWORD *pPD;
		_asm {
			MOV EAX,CR3
			MOV pPD,EAX
		}
		memcpy( pPD, bell.pPD, 4096 / 2 );
		bell.dwDbgMappingFlag = get_kernel_mapping_flag();
	}

	// Ű���带 ����������� ����´�.
	set_debugger_active( 1 );

	// debugger�� CR3�� debugee�� CR3�� ��ü�ؾ� ���� �޸𸮿� ����(����, ��������...)�� �� �ִ�.
	kdbg_change_CR3();
	
	// INT3�� ���� ����ŷ� ���� ������ Ȯ���ϰ� ������ �ڵ带 �����Ѵ�.
	swap_cc_by_org_code();

	// �ٷ� ������ ������ �ڵ带 �����ش�.
	kdbg_disp_next_code();

	// ����� ���� �����Ѵ�.
	if( nTraceRepeat > 0 )
	{
		kdbg_printf( "TRACE_REPEAT : %d\n", nTraceRepeat );
		nTraceRepeat--;
	}
	else
		kdbg_shell(  (char*)szDebugPrompt ); 

	// ����ſ��� ������ �� TSS Descriptor�� BUSY Bit�� 1�� �����Ǿ� �־�� �Ѵ�.
	wBackLink = wGetBackLink( &dbg_tss );	// ���� TSS�� BackLink�� ���Ѵ�.
	vSetDescriptorBusyBit( &gdt[ wBackLink/8 ], 1 );

	set_debugger_active( 0 );

	// set rf flag
	kdbg_set_debugee_rf( 1 );

	_asm IRETD;		// ������ ���ϵǸ� ������ IRETD�� ���� TASK�� ����ȴ�.

	goto DBG_LOOP;
}

int kdbg_set_trace_repeat( int nRepeat )
{
	nTraceRepeat = nRepeat;
	return( 0 );
}

// ������ ���� �̻��� ������� �ʴ´�.
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

// 0�� ���������� ���� �޸𸮿� ����Ѵ�.
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







