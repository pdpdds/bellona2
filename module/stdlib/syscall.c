#include "lib.h"

static int sys_call( SYSCallStt *pSC )
{
	_asm {
		PUSHFD
		PUSH DS
		PUSH ES
		PUSH FS
		PUSH GS
		PUSH EAX
		
		MOV  EAX,pSC
		int  50h

		POP  EAX
		POP  GS
		POP  FS
		POP  ES
		POP  DS
		POPFD
	}	

	return( pSC->nReturn );
}

static int system_call_x( int nType, DWORD dwParam )
{
	SYSCallStt	sc;

	memset( &sc, 0, sizeof( sc ) );

	sc.nType    = nType;
	sc.dwParam  = dwParam;
	sys_call( &sc );		

	return( sc.nReturn );
}

// 구버전 시스템 콜
int system_call( int nType, DWORD dwParam )
{
	int			nR;
   	SYSCallStt	sc;

	memset( &sc, 0, sizeof( sc ) );

	sc.nType    = nType;
	sc.dwParam  = dwParam;
	
    // 시스템 콜을 호출한다.
    sys_call( &sc );		
    nR = sc.nReturn;

	//user_signal_handling();

	return( nR );
}    

//===========================================================//
//  New System Call                                          //
//===========================================================//

// 아래 함수를 직접 호출하지 말고 syscall_stub을 호출할 것.
static int syscall( int nTotalParam, DWORD *pParam, R3ExportTblStt **ppR3Exp )
{
    int nR;

	_asm {
		PUSHFD
		PUSH DS
		PUSH ES
		PUSH FS
		PUSH GS
		
		MOV  EAX, nTotalParam
        MOV  EBX, pParam
		int  52h

		POP  GS
		POP  FS
		POP  ES
		POP  DS
		POPFD
		
        MOV   nR, EAX
        MOV   EAX, ppR3Exp
		MOV  [EAX], EBX

	}	

	return( nR );
}

int _declspec( naked ) syscall_stub( int nType, ... )
{
    static unsigned char    *pX;
	static R3ExportTblStt	*pR3Exp;
    static int              nR, nI, nTotalParam, *pValue;

    _asm {
        MOV  EAX,    ESP
        MOV  pValue, EAX
        MOV  EAX,    [EAX]
        MOV  pX,     EAX
        PUSH ESI
        PUSH EDI
    }   
    pValue++;
    if( pX[0] != 0x83 || pX[1] != 0xC4 )
        goto ERR_RETURN;

    nTotalParam = ((int)pX[2] /4);

    nR = syscall( nTotalParam, pValue, &pR3Exp );

	//user_signal_handling( pR3Exp );

    _asm {
        POP EDI
        POP ESI
        MOV EAX, nR
        RETN
    } 

ERR_RETURN:
    _asm {
        POP EDI
        POP ESI
        MOV EAX, 0xFFFFFFFF
        RETN
    }   
}










