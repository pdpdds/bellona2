#include <types.h>
#include <string.h>

#include "calcstr.h"

static char _white_space[] = { 32, 9, 10, 13, 0 };
static char _operator[]    = { '-', '+', '%', '/', '*', '^', 0 };
static char _nest[]   = { '(', ')', 0 };

// pS에 ch가 포함되어있는지 확인해 본다.
static int nChkChar( char *pS, char ch )
{
	int nI;

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( pS[nI] == ch )
			return( 1 );
	}

	return( 0 );	// ch가 pS에 포함되어있지 않음.
}

// SPACE, TAB, CR, LF를 스킵한다.
static char *pSkipSpace( char *pS )
{
	int nI;

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( !nChkChar( _white_space, pS[nI] ) )
			break;
	}

	return( &pS[nI] );
}

static char *pGetWord( char *pWord, char *pS )
{
	char *pNext;
	int  nI;

	pWord[0] = 0;
	// 공백을 건너뛴다.
	pNext = pSkipSpace( pS );
	if( pNext[0] == 0 )
		return( pNext );
	
	for( nI = 0; pNext[nI] != 0; )
	{	// 공백문자인가?
		if( nChkChar( _white_space, pNext[nI] ) )
			break;

		// 한 문자를 복사한다.
		pWord[nI] = pNext[nI];
		pWord[nI+1] = 0;
			
		// operator, nest인가? 
		if( nChkChar( _operator, pNext[nI] ) || nChkChar( _nest, pNext[nI] ) )
		{
			nI++;
			break;
		}

		nI++;

		// operator, nest인가? 
		if( nChkChar( _operator, pNext[nI] ) || nChkChar( _nest, pNext[nI] ) )
			break;
	}	 

	return( &pNext[nI] );
}

static void uppercase( char *pS )
{
    int nI;

    for( nI = 0; pS[nI] != 0; nI++ )
    {
        if( pS[nI] >= 'a' && pS[nI] <= 'z' )
            pS[nI] = pS[nI] - ( 'a' - 'A' );
    }
}

// 입력된 16진수 스트링을 unsigned long 값으로 변경한다.
static unsigned long dwHexValue( char *pS )
{
    unsigned long dwR = 0;
    int   nI;

    uppercase( pS );

    for( nI = 0; pS[nI] != 0; nI++ )
    {
        if( pS[nI] >= 'A' && pS[nI] <= 'F' )
        {
            dwR = (unsigned long)(dwR << 4);
            dwR += (unsigned long)( ( pS[nI] - 'A' ) + 10 );
        }
        else if( pS[nI] >= '0' && pS[nI] <= '9' )
        {
            dwR = (unsigned long)(dwR << 4);
            dwR += (unsigned long)( pS[nI] - '0' );
        }
    }
    return( dwR );
}

// 입력된 10진수 스트링을 unsigned long 값으로 변경한다.
static unsigned long dwDecValue( char *pS )
{
    unsigned long dwR = 0;
    int   nI;

    uppercase( pS );

    for( nI = 0; pS[nI] != 0; nI++ )
    {
        if( pS[nI] >= '0' && pS[nI] <= '9' )
        {
            dwR = (unsigned long)(dwR * 10);
            dwR += (unsigned long)( pS[nI] - '0' );
        }
    }
    return( dwR );
}

typedef enum {
	TOKEN_UNKNOWN = 0,
	TOKEN_NUMBER,			// 숫자
	TOKEN_OPERATOR,			// 연산자
	TOKEN_NEST,				// 괄호
	TOKEN_SYMBOL,			// 이상의 것들이 아닌 것
	END_OF_TOKEN_TYPE
} TOKEN_TYPE_TAG;

typedef struct {
	int				nType;
	char			szStr[128];
	unsigned long	dwValue;
} TOKENStt;

// 10진수인지 16진수인지 확인한다.
static int nIsNumber( char *pS )
{
	int nI;

	nI = strlen( pS );
	if( nI == 0 )
		return( 0 );	// 아무것도 아니다.

	if( pS[0] == '0' && ( pS[1] == 'x' || pS[1] == 'X' ) )
		return( 16 );	// 16진수
	else if( pS[nI-1] == 'h' || pS[nI-1] == 'H' )
		return( 16 );	// 16진수

	if( '0' <= pS[0] && pS[0] <= '9' )
		return( 10 );	// 10진수

	return(0);
}

// 토큰을 구한다.
static char *pGetToken( TOKENStt *pTOKEN, char *pS )
{
	char	*pNext, szWord[128];
	int		nI;
	
	pNext = pS;
	memset( pTOKEN, 0, sizeof( TOKENStt ) );
	if( pNext[0] == 0 )
		return( pNext );

	// 한 단어를 구한다.
	pNext = pGetWord( szWord, pNext );
	if( szWord[0] == 0 )
		return( pNext );

	strcpy( pTOKEN->szStr, szWord ); 	
	
	nI = strlen( szWord );
	if( nI == 1 )
	{	// operator ?
		if( nChkChar( _operator, szWord[0] ) )
		{
			pTOKEN->nType = TOKEN_OPERATOR;
			return( pNext );
		}// nest ?
		else if( nChkChar( _nest, szWord[0] ) )
		{
			pTOKEN->nType = TOKEN_NEST;
			return( pNext );
		}
	}
	
	// 숫자인가?
	nI = nIsNumber( szWord );

	if( nI == 10 )			// 10진수
	{
		pTOKEN->nType   = TOKEN_NUMBER;
		pTOKEN->dwValue = dwDecValue( szWord );
		return( pNext );

	}
	else if( nI == 16 )		// 16진수
	{
		pTOKEN->nType = TOKEN_NUMBER;
		pTOKEN->dwValue = dwHexValue( szWord );
		return( pNext );
	}

	// 심볼로 처리한다.
	pTOKEN->nType = TOKEN_SYMBOL;
	
	return( pNext );
}					  

#define MAX_VALUE	16
#define MAX_OP		16

// 연산자 우선순위값을 구한다.
static int nOpPriority( char ch )
{
	switch( ch )
	{
	case '-' : return( 100 );
	case '+' : return( 200 );
	case '%' : return( 300 );
	case '/' : return( 400 );
	case '*' : return( 500 );
	case '^' : return( 600 );
	}
	return(0);
}

// nIndex Operator를 계산한다.
static int nCalc( unsigned long *pValue, char *pOp, int *pTotalOp, int *pTotalValue, int nIndex )
{
	int				nI;
	char			op;
	unsigned long 	v1, v2, result;

	op = pOp[nIndex];
	v1 = pValue[nIndex];
	v2 = pValue[nIndex+1];

	switch( op )
	{
	case '+' : result = (unsigned long)( v1 + v2 ); break;	// ADD
	case '-' : result = (unsigned long)( v1 - v2 ); break;	// SUB
	case '*' : result = (unsigned long)( v1 * v2 ); break;	// MUL
	case '/' : result = (unsigned long)( v1 / v2 ); break;	// DIV
	case '%' : result = (unsigned long)( v1 % v2 ); break;	// MOD
	case '^' : result = (unsigned long)( v1 ^ v2 ); break;	// XOR
	default : return( -1 );		// 알 수 없는 연산자.
	}

	// operator를 packing한다.
	for( nI = nIndex; nI < *pTotalOp; nI++ )
	{
		pOp[nI]   = pOp[nI+1];
		pOp[nI+1] = 0;
	}

	// value packing한다.
	pValue[nIndex] = result;
	for( nI = nIndex+1; nI < *pTotalValue; nI++ )
	{
		pValue[nI]   = pValue[nI+1];
		pValue[nI+1] = 0;
	}

	pTotalValue[0]--;
	pTotalOp[0]--;

	return( 0 );
}

// operator를 처리한다.
static int _nOperator( unsigned long *pValue, char *pOp, int *pTotalOp, int *pTotalValue, int nContinue )
{
	int nI, nV;

	for( nI = 0; nI < *pTotalOp;  )
	{
		// 남은 연산자는 하나 뿐이다.
		if( nI +1 == *pTotalOp )
		{
			if( nContinue == 1 )
				nCalc( pValue, pOp, pTotalOp, pTotalValue, nI );

			break;
		}

		nV = nOpPriority( pOp[nI] ) - nOpPriority( pOp[nI+1] );

		// 앞쪽연산자의 우선순위 값이 더 크다.
		if( nV > 0 )
			nCalc( pValue, pOp, pTotalOp, pTotalValue, nI );		// 계산한다.
		else if( nV == 0 )	// 우선순위가 같다.
		{	// 마지막 연산자가 아니면 계산해도된다.
			if( nContinue == 1 || nI + 1 < *pTotalOp )
				nCalc( pValue, pOp, pTotalOp, pTotalValue, nI );
		}
		else
			nI++;
	}	

	return( 0 );
}

static int nOperator( unsigned long *pValue, char *pOp, int *pTotalOp, int *pTotalValue, int nContinue )
{
	int nR;

	for( ; *pTotalOp >= 1; )
	{
		nR = _nOperator( pValue, pOp, pTotalOp, pTotalValue, nContinue );
		if( nR == -1 )
			return( -1 );

		if( nContinue == 0 )
			break;
	}

	return( 0 );
}

// pS에 입력된 수식을 계산하여 pRValue로 리턴한다.  (0:성공, -1:실패)
int _nCalcStr( char *pS, unsigned long *pRValue, char **ppNext )
{
	int				nI;
	char			*pNext;
	TOKENStt		token;
	char			op[MAX_OP];			// 실은 5개만 있으면 된다.
	unsigned long	value[MAX_VALUE];		// 실은 6개만 있으면 된다.
	int				nTotalOp;
	int				nTotalValue;

	pRValue[0] = 0;
	if( pS == NULL || pS[0] == 0 )
		return( 0 );

	nTotalOp = nTotalValue = 0;
	for( pNext = pS; pNext[0] != 0; )
	{	// 토큰을 구한다.
		pNext = pGetToken( &token, pNext );

		// 토큰을 구할 수 없으면 리턴한다.
		if( token.nType == TOKEN_UNKNOWN )
			break;

		// 괄호처리
		if( token.nType == TOKEN_NEST )
		{	// 시작 괄호?
			if( token.szStr[0] == '(' )
			{	
				nI = _nCalcStr( pNext, pRValue, &pNext );
				if( nI == -1 )		
					return( -1 );		// 에러
				// 토큰타입을 숫자로 변경한다.				
				token.nType   = TOKEN_NUMBER;
				token.dwValue = pRValue[0];
			} // 끝 괄호 ?
			else if( token.szStr[0] == ')' )
				break;
		}
		
		// number
		if( token.nType == TOKEN_NUMBER )
		{
			if( nTotalValue >= MAX_VALUE )
				return( -1 );

			value[nTotalValue++] = token.dwValue;
		}

		// operator
		if( token.nType == TOKEN_OPERATOR )
		{
			if( nTotalOp >= MAX_OP )
				return( -1 );

			op[nTotalOp++] = token.szStr[0];
		}	
		
		// 숫자가 나온 시점에서 operator를 처리한다.
		if( token.nType == TOKEN_NUMBER && nTotalOp > 1 )
		{	
			nI = nOperator( value, op, &nTotalOp, &nTotalValue, 0 ); // continue = 0
			if( nI == -1 )
				return( -1 );
		}					 
	}	

	// 남은 operator와 value들을 처리한다.
	nI = nOperator( value, op, &nTotalOp, &nTotalValue, 1 ); // continue = 1
	if( nI == -1 )
		return( -1 );

	// operator가 없어야 에러가 아니다.
	if( nTotalOp > 0 || nTotalValue > 1 )
		return( -1 );

	pRValue[0] = value[0];

	if( ppNext != NULL )
		ppNext[0] = pNext;
	
	return( 0 );	// 성공 
}

int nCalcStr( char *pS, unsigned long *pRValue )
{
	int nR;
	
	nR = _nCalcStr( pS, pRValue, NULL );

	return( nR );
}
