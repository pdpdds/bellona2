#include "bellona2.h"

TSSStt dbg_tss, init_tss, pf_tss;

static TSSStt tlb_tss;
static DWORD  tlb_stack[64];

#define MAX_TSS_NAME	32
typedef struct {
	char	szName[MAX_TSS_NAME];
	DWORD	dwAddr;
} TSSTblStt;

static int nTotalTSS = 0;
#define MAX_TSS_TBL		128
static TSSTblStt tt[MAX_TSS_TBL];

void tlb_task_main()
{
	for( ;; )
	{
		_asm IRETD;
	}
}
void make_tlb_tss()
{
	// GDT에 TASK Descriptor를 만든다.
	vMakeGDTDescriptor( GSEL_TLBTSS32, (DWORD)&tlb_tss, (DWORD)sizeof( tlb_tss ), (UCHAR)0x89, (UCHAR)0x00 );
	vMakeTaskGate_in_IDT( ISEL_TLB, GSEL_TLBTSS32 );
	// make init tss
	vMakeTSS( &tlb_tss, (DWORD)tlb_task_main, (DWORD)tlb_stack + sizeof(tlb_stack) -4 );

	// TSS의 이름과 주소를 등록한다.
	nAppendTSSTbl( "tlb",  (DWORD)&tlb_tss  );
}	

TSSStt *get_current_tss()
{
	UINT16	wI;
	TSSStt	*pTSS; 

	_asm STR wI;

	//kdbg_printf( "get_current_tss(): wI = %d\n", wI );
	
	pTSS = (TSSStt*)dwGetDescriptorAddr( &gdt[wI/8] );

	//kdbg_printf( "get_current_tss(): pTSS = 0x%X\n", (DWORD)pTSS );


	return( pTSS );
}

// TSS를 TSS테이블에 추가한다.
int nAppendTSSTbl( char *pName, DWORD dwTSSAddr )
{
	int nI;

	if( nTotalTSS >= MAX_TSS_TBL )
		return( nTotalTSS );

	for( nI = 0; nI < MAX_TSS_TBL; nI++ )
	{
		if( tt[nI].szName[0] == 0 )
			break;
	}  
	if( nI >= MAX_TSS_TBL )
		return( nTotalTSS );

	strcpy( tt[nI].szName, pName );
	tt[nI].dwAddr = dwTSSAddr;
	nTotalTSS;

	return( nTotalTSS );
}

static void disp_tss()
{
	int		nI;
	TSSStt	*pTSS;

	for( nI = 0;  nI < MAX_TSS_TBL; nI++ )
	{
		if( tt[nI].szName[0] != 0 )
		{
			pTSS = (TSSStt*)tt[nI].dwAddr;
			kdbg_printf( "[%2d] %-10s 0x%08X, CR3=0x%08X, EIP=0x%08X\n", 
				nI, tt[nI].szName, tt[nI].dwAddr, pTSS->dwCR3, pTSS->dwEIP );
		}
	}
}

static void make_tss( TSSStt *pTSS, DWORD dwEIP, DWORD dwESP )
{
	DWORD   dwCR3, dwEAX, dwEBX, dwECX, dwEDX, dwESI, dwEDI, dwEBP, dwEFLAG;
	UINT16  wCS,  wDS,  wES,  wFS,  wGS,  wSS, wLDT;

	_asm {
        PUSH EAX
		MOV	dwEAX, EAX
        MOV dwEBX, EBX
        MOV dwECX, ECX
        MOV dwEDX, EDX
        MOV dwESI, ESI
        MOV dwEDI, EDI
        MOV dwEBP, EBP
        
		PUSHFD
		POP EAX
		MOV dwEFLAG, EAX

		MOV EAX,CR3
		MOV dwCR3, EAX

        MOV AX,  CS
        MOV wCS, AX
        MOV AX,  DS
        MOV wDS, AX
        MOV AX,  ES
        MOV wES, AX
        MOV AX,  FS
        MOV wFS, AX
        MOV AX,  GS
        MOV wGS, AX
        MOV AX,  SS
        MOV wSS, AX
		
		SLDT AX
		MOV  wLDT,AX

        POP EAX
	}
	 
    pTSS->dwEAX   = dwEAX;
    pTSS->dwEBX   = dwEBX;
    pTSS->dwECX   = dwECX;
    pTSS->dwEDX   = dwEDX;
    pTSS->dwESI   = dwESI;
    pTSS->dwEDI   = dwEDI;
    pTSS->dwEBP   = dwEBP;
    pTSS->dwESP   = dwESP;
    pTSS->dwCR3   = dwCR3;
    pTSS->dwEIP   = dwEIP;
    pTSS->wCS     = wCS;
    pTSS->wDS     = wDS;
    pTSS->wES     = wES;
    pTSS->wFS     = wFS;
    pTSS->wGS     = wGS;
    pTSS->wSS     = wSS;
    pTSS->wLDT    = wLDT;
	pTSS->wSS0    = wSS;
	pTSS->dwESP0  = dwESP;
    pTSS->dwEFLAG = dwEFLAG;

	pTSS->wBitField = (UINT16)0x68;
}

void vMakeTSS( TSSStt *pTSS, DWORD dwEIP, DWORD dwESP )
{
	memset( pTSS, 0, sizeof( TSSStt ) );
	make_tss( pTSS, dwEIP, dwESP );
	pTSS->byEndMark = (UCHAR)0xFF;
}

void vMakeV86TSS( V86TSSStt *pTSS, DWORD dwEIP, DWORD dwESP )
{
	memset( pTSS, 0, sizeof( V86TSSStt ) );
	make_tss( (TSSStt*)pTSS, dwEIP, dwESP );
	pTSS->byEndMark = (UCHAR)0xFF;
}

static char *pGetFlagStr( DWORD dwFlag, char *pS )
{
	DWORD	dwT;
	char	szT[32];
    
	pS[0] = 0;

    if( dwFlag & MASK_CF  ) strcat( pS, " CF"  ); else strcat( pS, " cf"  );
    if( dwFlag & MASK_PF  ) strcat( pS, " PF"  ); else strcat( pS, " pf"  );
    if( dwFlag & MASK_AF  ) strcat( pS, " AF"  ); else strcat( pS, " af"  );
    if( dwFlag & MASK_ZF  ) strcat( pS, " ZF"  ); else strcat( pS, " zf"  );
    if( dwFlag & MASK_SF  ) strcat( pS, " SF"  ); else strcat( pS, " sf"  );
    if( dwFlag & MASK_TF  ) strcat( pS, " TF"  ); else strcat( pS, " tf"  );
    if( dwFlag & MASK_IF  ) strcat( pS, " IF"  ); else strcat( pS, " if"  );
    if( dwFlag & MASK_DF  ) strcat( pS, " DF"  ); else strcat( pS, " df"  );
    if( dwFlag & MASK_OF  ) strcat( pS, " OF"  ); else strcat( pS, " of"  );
    if( dwFlag & MASK_NT  ) strcat( pS, " NT"  ); else strcat( pS, " nt"  );
    if( dwFlag & MASK_RF  ) strcat( pS, " RF"  ); else strcat( pS, " rf"  );
    if( dwFlag & MASK_VM  ) strcat( pS, " VM"  ); else strcat( pS, " vm"  );
    if( dwFlag & MASK_AC  ) strcat( pS, " AC"  ); else strcat( pS, " ac"  );
    if( dwFlag & MASK_VIF ) strcat( pS, " VIF" ); else strcat( pS, " vif" );
    if( dwFlag & MASK_VIP ) strcat( pS, " VIP" ); else strcat( pS, " vip" );
    if( dwFlag & MASK_ID  ) strcat( pS, " ID"  ); else strcat( pS, " id"  );

	dwT = (DWORD)( (DWORD)( dwFlag >> 12 ) & (DWORD)3 );
	sprintf( szT, " IOPL(%d)", dwT );
	strcat( pS, szT );

	return( pS );								
}

int nDispTSS( int argc, char *argv[] )
{
	char	*pName;
	int		nI;
	TSSStt	*pT;
	char	szFlagStr[128];

	if( argc < 2 )
	{	// 엔트리만 출력한다.
		disp_tss();
		return( 0 );
	}

	pName = argv[1];
	
	// TSS 번호로 입력되었다.
	if( '0' <= pName[0] && pName[0] <= '9' )
	{
		nI = dwDecValue( pName );
		if( tt[nI].szName[0] == 0 )
			return( -1 );
	}
	else
	{	// 테이블에서 이름을 찾는다.
		for( nI = 0; nI < MAX_TSS_TBL; nI++ )
		{
			if( strcmpi( tt[nI].szName, pName ) == 0 )
				goto DISP_CONTENT;
		}
		return( -1 );
	}
	
DISP_CONTENT:	// 내용을 출력한다.
	pT = (TSSStt*)tt[nI].dwAddr;
    kdbg_printf( "TSS <%s> 0x%08X\n", tt[nI].szName, tt[nI].dwAddr );
    kdbg_printf( "BL=%4X CR3=%08X T-Bit=%d LDT=%04X IO-Bit-Field=%04X\n", 
		          pT->dwBackLink, pT->dwCR3, pT->wTBit, pT->wLDT, pT->wBitField );
    kdbg_printf( "S[0]=%04X:%08X S[1]=%04X:%08X S[2]=%04X:%08X\n",
                  pT->wSS0, pT->dwESP0,pT->wSS1, pT->dwESP1,pT->wSS2, pT->dwESP2 );
	kdbg_printf( "EAX=%08X EBX=%08X ECX=%08X EDX=%08X ESI=%08X EDI=%08X\n", 
				  pT->dwEAX, pT->dwEBX, pT->dwECX, pT->dwEDX, pT->dwESI, pT->dwEDI );
    kdbg_printf( "EBP=%08X ESP=%08X EIP=%08X \n", pT->dwEBP, pT->dwESP, pT->dwEIP );
    kdbg_printf( "CS=%04X DS=%04X ES=%04X FS=%04X GS=%04X SS=%04X\n", pT->wCS, pT->wDS, pT->wES, pT->wFS, pT->wGS, pT->wSS );
	kdbg_printf( "%s\n", pGetFlagStr( pT->dwEFLAG, szFlagStr ) );

	return( 0 );
}




