#include "bellona2.h"

static StkStt stk;

int nInitStk()
{
	memset( &stk, 0, sizeof( stk ) );
	return( 0 );
}

int nPopSEnt( SEntStt *pSEnt )
{
	if( stk.nS == 0 )		//  ���̻� ���� ���� ����.
		return( -1 );

	stk.nS--;
	memcpy( pSEnt, &stk.s[ stk.nS ], sizeof( SEntStt ) );	

	return( 0 );
}

int nPushSEnt( SEntStt *pSEnt )
{
	if( stk.nS >= MAX_SENT )		//  ���̻� Ǫ���� �� ����.
		return( -1 );

	memcpy( &stk.s[ stk.nS ], pSEnt, sizeof( SEntStt ) );	
	stk.nS++;

	return( 0 );
}

static int nGetOperatorValue( DWORD dwOperator )
{
	switch( dwOperator )
	{
	case (DWORD)'/' : return( 3 );
	case (DWORD)'*' : return( 2 );
	case (DWORD)'-' : return( 1 );
	case (DWORD)'+' : return( 0 );
	}

	return( 0 );
}

int nPushSEntOperator( SEntStt *pSEnt )
{
	int		nR;
	SEntStt	s, *pS;
	
	if( stk.nS == 0 )
		return( -1 );	// ��ȣ ������ �ٷ� ���ڰ� ������ ���� ����.

	// ����ž�� ���ڰ� ������ �ι�° ���۷���, ���۷����� ������ Ǫ�����ָ� �ȴ�.
	pS = &stk.s[stk.nS-1];
	if( pS->nType != SENT_OPERATOR )
	{
		nPushSEnt( &pSEnt[1] );
		nPushSEnt( &pSEnt[0] );
		return( 0 );
	}

	// ����ž�� ���۷����Ͱ� ������ �켱������ ���ؾ� �Ѵ�.
	if( nGetOperatorValue( pS->dwValue ) >= nGetOperatorValue( pSEnt[0].dwValue ) )
	{
		nPushSEnt( &pSEnt[1] );
		nPushSEnt( &pSEnt[0] );
		return( 0 );
	}

	// �ϴ� ����ž�� ���۷����͸� ������.
	nR = nPopSEnt( &s );
	if( nR == -1 ) return(  -1 );

	// ��������� ���۷���� ���۷����͸� Ǫ���Ѵ�.
	nR = nPushSEntOperator( pSEnt );
	if( nR == -1 ) return(  -1 );

	// ���´� ���۷����͸� Ǫ���Ѵ�.
	nR = nPushSEnt( &s );
	if( nR == -1 ) return(  -1 );
	
	return( 0 );
}

// ������ ����Ѵ�.
// SENT_END�� ������ �� ����� ����ģ��. 
int nCalcStk( DWORD *pR )
{
	DWORD	dwV1, dwV2;
	int		nR;
	SEntStt	s[3];

	pR[0] = 0;
	// ���� Operator�� �ϳ� ������.
	nR = nPopSEnt( &s[0] );
	if( nR != 0 ) goto ERROR;

	// ���۷����Ͱ� �ȳ����� ���ڰ� �ٷ� ������ �׳� �����Ѵ�.
	if( s[0].nType == SENT_NUMBER )
	{
		pR[0] = s[0].dwValue;
		return( 0 );
	}

	// Operand1
	nR = nPopSEnt( &s[1] );
	if( nR != 0 ) goto ERROR;
	if( s[1].nType == SENT_NUMBER )
		dwV1 = s[1].dwValue;
	else if( s[1].nType == SENT_OPERATOR )
	{
		nPushSEnt( &s[1] );
		nR = nCalcStk( &dwV1 );
		if( nR != 0 )
			goto ERROR;
	}

	// Operand2
	nR = nPopSEnt( &s[2] );
	if( nR != 0 ) goto ERROR;
	if( s[2].nType == SENT_NUMBER )
		dwV2 = s[2].dwValue;
	else if( s[2].nType == SENT_OPERATOR )
	{
		nPushSEnt( &s[2] );
		nR = nCalcStk( &dwV2 );
		if( nR != 0 )
			goto ERROR;
	}
	
	// �������� ��¥ ����� �ϰ� �Ǵ���
	switch( s[0].dwValue )
	{
	case (DWORD)'+' :
		pR[0] = (DWORD)( dwV1 + dwV2 );
		break;
	case (DWORD)'-' :
		pR[0] = (DWORD)( dwV2 - dwV1 );
		break;
	case (DWORD)'*' :
		pR[0] = (DWORD)( dwV1 * dwV2 );
		break;
	case (DWORD)'/' :
		pR[0] = (DWORD)( dwV2 / dwV1 );
		break;
	}
	
	
	// ���� ���� ���������� ����� �Ͽ���.
	return( 0 );

ERROR:
	return( -1 );
}

