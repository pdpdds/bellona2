#include "bellona2.h"

static StkStt stk;

int nInitStk()
{
	memset( &stk, 0, sizeof( stk ) );
	return( 0 );
}

int nPopSEnt( SEntStt *pSEnt )
{
	if( stk.nS == 0 )		//  더이상 팝할 것이 없다.
		return( -1 );

	stk.nS--;
	memcpy( pSEnt, &stk.s[ stk.nS ], sizeof( SEntStt ) );	

	return( 0 );
}

int nPushSEnt( SEntStt *pSEnt )
{
	if( stk.nS >= MAX_SENT )		//  더이상 푸시할 수 없다.
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
		return( -1 );	// 괄호 다음에 바로 숫자가 나오는 수는 없다.

	// 스택탑에 숫자가 있으면 두번째 오퍼랜드, 오퍼레이터 순으로 푸쉬해주면 된다.
	pS = &stk.s[stk.nS-1];
	if( pS->nType != SENT_OPERATOR )
	{
		nPushSEnt( &pSEnt[1] );
		nPushSEnt( &pSEnt[0] );
		return( 0 );
	}

	// 스택탑에 어퍼레이터가 있으면 우선순위를 비교해야 한다.
	if( nGetOperatorValue( pS->dwValue ) >= nGetOperatorValue( pSEnt[0].dwValue ) )
	{
		nPushSEnt( &pSEnt[1] );
		nPushSEnt( &pSEnt[0] );
		return( 0 );
	}

	// 일단 스택탑의 오퍼레이터를 꺼낸다.
	nR = nPopSEnt( &s );
	if( nR == -1 ) return(  -1 );

	// 재귀적으로 오퍼랜드와 오퍼레이터를 푸쉬한다.
	nR = nPushSEntOperator( pSEnt );
	if( nR == -1 ) return(  -1 );

	// 꺼냈던 오퍼레이터를 푸쉬한다.
	nR = nPushSEnt( &s );
	if( nR == -1 ) return(  -1 );
	
	return( 0 );
}

// 스택을 계산한다.
// SENT_END를 만났을 때 계산을 끝마친다. 
int nCalcStk( DWORD *pR )
{
	DWORD	dwV1, dwV2;
	int		nR;
	SEntStt	s[3];

	pR[0] = 0;
	// 먼저 Operator를 하나 꺼낸다.
	nR = nPopSEnt( &s[0] );
	if( nR != 0 ) goto ERROR;

	// 오퍼레이터가 안나오고 숫자가 바로 나오면 그냥 리턴한다.
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
	
	// 이제서야 진짜 계산을 하게 되누만
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
	
	
	// 에러 없이 성공적으로 계산을 하였다.
	return( 0 );

ERROR:
	return( -1 );
}

