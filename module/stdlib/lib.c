#include "lib.h"

//extern int main( int argc,... );

// parse command string
/*
static int arg_parsing( char *argv[], char *pS )
{
	int  nTotal, nI, nX;
	
	nTotal = 0;

	if( pS == NULL || pS[0] == 0 )
		return( 0 );

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( nI == 0 && pS[0] != ' ')
			argv[nTotal++] = pS;
		else
		{
			if( pS[nI-1] == ' ' && pS[nI] != ' ' )
				argv[nTotal++] = &pS[nI];
		}	
	}

	for( nX = 1; nX < nI; nX++ )
	{
		if( pS[nX-1] != ' ' && pS[nX] == ' ' )
			pS[nX] = 0;
	} 

	return( nTotal );
}
*/

_declspec( naked ) static void preempt_r3_thread()
{
	_asm {
		PUSHFD
		PUSHAD
		MOV AX, GSEL_DATA32_R3
		MOV DS, AX
		MOV ES, AX
		MOV FS, AX
		MOV GS, AX
	}			 

	system_call( SCTYPE_CALL_SCHEDULER, 0 );

	_asm {
		POPAD
		POPFD
		RETN
	}
}

// exit user thread
_declspec(naked) static void user_thread_entry_point()
{
	// set segment registers
	_asm {	
		MOV AX,SS
		MOV DS,AX
		MOV ES,AX
		MOV FS,AX
		MOV GS,AX
	}

	// call real thread function
	_asm ADD ESP, 4
	_asm POP  EAX
	_asm CALL EAX

	// call exit thread system call
	syscall_stub( SCTYPE_EXIT_USER_THREAD, 0 );
}

// get path, arg, evn
static KExecveParamStt *copy_kexecve_param()
{
	KExecveParamStt	*pEP;

	pEP = (KExecveParamStt*)syscall_stub( SCTYPE_COPY_KEXECVE_PARAM );
	
	return( pEP );
}

// export r3 functions
static int export_r3_function( DWORD dwUAreaAddr )
{
	R3ExportTblStt	x;
	int				nR;

	memset( &x, 0, sizeof( x ) );

	x.func[R3EXI_PREEMPT_R3_THREAD] = (DWORD)preempt_r3_thread;
	x.func[R3EXI_USER_THREAD_ENTRY] = (DWORD)user_thread_entry_point;
	x.func[R3EXI_UAREA] 			= dwUAreaAddr;
					
	// register export table system call
	nR = system_call( SCTYPE_REG_EXPORT, (DWORD)&x );		

	return( nR );
}

typedef int (*MAIN_ENTRY_FUNC)( int argc, char *argv[], char *env[] );
void main_startup( DWORD dwMain, DWORD dwUArea )
{
	int					nR;
	MAIN_ENTRY_FUNC		pMain;
	KExecveParamStt		*pEP;
	
	// export functions
	export_r3_function( dwUArea );

	// initialize uarea
	nR = init_uarea( (UAreaStt*)dwUArea );

	// get path, arg, evn (메모리가 할당된다.)
	pEP = copy_kexecve_param();

	/*
	argc = 1;
	argv[0] = ex_arg.szPath;

	// arsing argument
	argc += arg_parsing( &argv[1], ex_arg.szArg );
	*/

	// call main function
	pMain = (MAIN_ENTRY_FUNC)dwMain;
	nR = pMain( pEP->nArgc, pEP->ppArgv, pEP->ppEnv );

    // uarea의 해제는 exit() 라이브러리 함수 내에서 이루어진다.
	exit( nR );
}

