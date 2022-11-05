#include "lib.h"

// wait for signal event forever
int pause()
{
	return( system_call( SCTYPE_PAUSE, 0 ) );
}

// ����� ���� �ñ׳� �ڵ鸵.
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

	// �ñ׳� �ڵ鷯�� ȣ���Ѵ�.
	dwT = 1;
	for( nI = 0; nI < 32; nI++, dwT = dwT << 1 )
	{
		if( ( pSig->dwSignal & dwT ) == 0 )
			continue;
	
		// �ñ׳� �ڵ鷯�� ���ų� Ŀ�� ������ �����ϸ� ��ŵ.
		if( pSig->func[nI] == 0 || ( (DWORD)pSig->func[nI] & (DWORD)0x80000000 ) == 0 )
			continue;

		pFunc = (SIG_FUNC)pSig->func[nI];
	
		// ����� ������ �����ϴ� �ñ׳� �ڵ鷯�� �����Ѵ�.
		nR = pFunc( pSig->param[nI] );
	
		// �ش� ��ũ�� ��Ʈ�� Ŭ���� �Ѵ�.
		pSig->dwSignal ^= dwT;
	}

	return( 0 );
}














