#include "lib.h"

// create new thread
DWORD create_thread( DWORD dwFunc, DWORD dwParam, int nState, DWORD dwStackSize )
{
	DWORD dwTID;

	if( nState < 0 )
		nState = TS_READY_NORMAL;

	dwTID = (DWORD)syscall_stub( SCTYPE_CREATE_THREAD, dwStackSize, dwFunc, dwParam, nState );
	
	return( dwTID );
}

int remove_thread( DWORD dwTID )
{
	int nR;
	nR = syscall_stub( SCTYPE_EXIT_USER_THREAD, dwTID );
	return( nR );	
}

int set_thread_alias( DWORD dwTID, char *pAlias )
{
	int nR;

	nR = syscall_stub( SCTYPE_SET_THREAD_ALIAS, dwTID, pAlias );

	return( nR );
}

int set_process_alias( DWORD dwTID, char *pAlias )
{
	int nR;

	nR = syscall_stub( SCTYPE_SET_PROCESS_ALIAS, dwTID, pAlias );

	return( nR );
}
  
int set_fg_tid( DWORD dwTID )
{
	int nR;
	nR = syscall_stub( SCTYPE_SET_FG_TID, dwTID );
	return( nR );	
}

DWORD get_cur_thread_id()
{
	DWORD dwID;

	dwID = (DWORD)system_call( SCTYPE_GET_CUR_TID, 0 );

	return( dwID );
}

DWORD get_cur_process_id()
{
	DWORD dwID;

	dwID = (DWORD)syscall_stub( SCTYPE_GET_CUR_PID );

	return( dwID );
}

int set_fg_proc( DWORD dwID )
{
	int nR;

	// 현재 프로세스를 FG로 설정한다.
	nR = syscall_stub( SCTYPE_SET_FG_PROCESS, dwID );
	return( nR );
}

int fork()
{
	int		nID;
	DWORD	dwESP3;

	// 현재 사용중인 ESP3 값을 전달한다.
	_asm MOV dwESP3, ESP

	nID = syscall_stub( SCTYPE_FORK, dwESP3 );

	return( nID );
}

