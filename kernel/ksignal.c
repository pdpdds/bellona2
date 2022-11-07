#include "bellona2.h"

typedef struct {
	char	*pS;
	DWORD	dwBit;
} SignalStringStt;

static SignalStringStt sig_str_tbl[] = {
	{  "SIG_USER_0",	SIG_USER_0	},
	{  "SIG_USER_1",	SIG_USER_1	},
	{  "SIG_USER_2",	SIG_USER_2	},
	{  "SIG_STOP"	 ,	SIG_STOP	}, 
	{  "SIG_CONT"	 ,	SIG_CONT	},
	{  "SIG_KILL"	 ,	SIG_KILL	},

	{ NULL, 0 }
};

DWORD get_signal_bit( char *pS )
{
	int nI;

	for( nI = 0; sig_str_tbl[nI].pS != NULL; nI++ )
	{
		if( strcmpi( sig_str_tbl[nI].pS, pS ) == 0 )
			return( sig_str_tbl[nI].dwBit );
	}

	return( 0 );
}

void disp_signal_str()
{
	int nI;
	for( nI = 0; sig_str_tbl[nI].pS != NULL; nI++ )
		kdbg_printf( "    %-12s 0x%X\n", sig_str_tbl[nI].pS, sig_str_tbl[nI].dwBit );
}	 

// Ư�� ������� �ñ׳��� ������.
int send_signal_to_thread( DWORD dwTID, DWORD dwSignal )
{
	WaitObjStt	*pW;
	ThreadStt	*pThread;
	int			nR, nPrevState;

	// find thread
	pThread = find_thread_by_id( dwTID );
	if( pThread == NULL )
	{
		kdbg_printf( "send_signal_to_thread() - thread %08X not found!\n", dwTID );
		return( -1 );
	}

	pThread->dwNewSigBits |= dwSignal;

	//  �ñ׳� ��� ���¶�� ���� ���·� �����Ѵ�.
	for( pW = pThread->pStartWaitObj; pW != NULL; )
	{
		if( pW->pE == &signal_event )
		{	// unlink wait object
			nR = unlink_waitobj_from_event( pW->pE, pW );

			nPrevState = pThread->nPrevThreadState;
			
			// set free wait object
			set_wait_obj_free( pW );

			// change thread state
			nR = change_thread_state( NULL, pThread, nPrevState );
			
			break;
		}				

		pW = pW->pNextObj;
	}

	return( nR );
}

// Thread�� ���޵� �ñ׳� ��Ʈ�� ������ �� Ŀ�� ������ �����ϴ� �ñ׳� �ڵ鷯�� ȣ���Ѵ�.
int copy_thread_sig_bits()
{
	UAreaStt	*pU;
	ThreadStt 	*pT;
	SignalStt	*pSig;
	SIG_FUNC	pFunc;
	int 		nI, nR;
	DWORD 		dwNewSigBits, dwT;

	pT = get_current_thread();
	if( pT == NULL || pT->dwNewSigBits == 0 || pT->pProcess == NULL )
		return( -1 );

	_asm {
		PUSHFD
		CLI
	}
	dwNewSigBits = pT->dwNewSigBits;
	pT->dwNewSigBits = 0;
	_asm {	
		POPFD
	}

	pU = (UAreaStt*)pT->pProcess->e.func[R3EXI_UAREA];
	pSig = &pU->sig;
	if( pSig == NULL )
	{	
		kdbg_printf( "copy_thread_sig_bits: PID(%d) has not signal struct!\n", pT->pProcess->dwID );
		return( -1 );
	}

	pSig->dwMask &=(DWORD)~(DWORD)SIG_KILL;

	pSig->dwSignal |= (dwNewSigBits | pSig->dwMask);
	pSig->dwSignal ^= pSig->dwMask;

	// �ñ׳� �ڵ鷯�� ȣ���Ѵ�.
	dwT = 1;
	for( nI = 0; nI < 32; nI++, dwT = dwT << 1 )
	{
		if( ( pSig->dwSignal & dwT ) == 0 )
			continue;

		// �ñ׳� �ڵ鷯�� ���ų� ����� ������ �����ϸ� ��ŵ.
		if( pSig->func[nI] == 0 || (DWORD)pSig->func[nI] & (DWORD)0x80000000 )
			continue;

		pFunc = (SIG_FUNC)pSig->func[nI];

		// Ŀ�� ������ �����ϴ� �ñ׳� �ڵ鷯�� �����Ѵ�.
		nR = pFunc( pSig->param[nI] );

		// �ش� ��ũ�� ��Ʈ�� Ŭ���� �Ѵ�.
		pSig->dwSignal ^= dwT;
	}
	
	return( 0 );
}


