#include "lib.h"

// wait for signal event forever
int pause()
{
	return( system_call( SCTYPE_PAUSE, 0 ) );
}

// 사용자 레벨 시그널 핸들링.
int user_signal_handling( R3ExportTblStt *pR3Exp )
{
	UAreaStt	*pU;
	SignalStt 	*pSig;
	SIG_FUNC	pFunc;
	int 		nI, nR;
	DWORD		dwNewSigBits, dwT;

	if( pR3Exp == NULL )
		return( -1 );

	pU = (UAreaStt*)pR3Exp->func[R3EXI_UAREA];
	if( pU == NULL )
		return( -1 );

	pSig = &pU->sig;
	dwNewSigBits = pSig->dwSignal;

	// 시그널 핸들러를 호출한다.
	dwT = 1;
	for( nI = 0; nI < 32; nI++, dwT = dwT << 1 )
	{
		if( ( pSig->dwSignal & dwT ) == 0 )
			continue;
	
		// 시그널 핸들러가 없거나 커널 영역에 존재하면 스킵.
		if( pSig->func[nI] == 0 || ( (DWORD)pSig->func[nI] & (DWORD)0x80000000 ) == 0 )
			continue;

		pFunc = (SIG_FUNC)pSig->func[nI];
	
		// 사용자 영역에 존재하는 시그널 핸들러를 수행한다.
		nR = pFunc( pSig->param[nI] );
	
		// 해당 시크널 비트를 클리어 한다.
		pSig->dwSignal ^= dwT;
	}

	return( 0 );
}














