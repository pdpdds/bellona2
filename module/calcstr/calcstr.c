#include <types.h>
#include <string.h>

#include "calcstr.h"

static char _white_space[] = { 32, 9, 10, 13, 0 };
static char _operator[]    = { '-', '+', '%', '/', '*', '^', 0 };
static char _nest[]   = { '(', ')', 0 };

// pS�� ch�� ���ԵǾ��ִ��� Ȯ���� ����.
static int nChkChar( char *pS, char ch )
{
	int nI;

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( pS[nI] == ch )
			return( 1 );
	}

	return( 0 );	// ch�� pS�� ���ԵǾ����� ����.
}

// SPACE, TAB, CR, LF�� ��ŵ�Ѵ�.
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
	// ������ �ǳʶڴ�.
	pNext = pSkipSpace( pS );
	if( pNext[0] == 0 )
		return( pNext );
	
	for( nI = 0; pNext[nI] != 0; )
	{	// ���鹮���ΰ�?
		if( nChkChar( _white_space, pNext[nI] ) )
			break;

		// �� ���ڸ� �����Ѵ�.
		pWord[nI] = pNext[nI];
		pWord[nI+1] = 0;
			
		// operator, nest�ΰ�? 
		if( nChkChar( _operator, pNext[nI] ) || nChkChar( _nest, pNext[nI] ) )
		{
			nI++;
			break;
		}

		nI++;

		// operator, nest�ΰ�? 
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

// �Էµ� 16���� ��Ʈ���� unsigned long ������ �����Ѵ�.
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

// �Էµ� 10���� ��Ʈ���� unsigned long ������ �����Ѵ�.
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
	TOKEN_NUMBER,			// ����
	TOKEN_OPERATOR,			// ������
	TOKEN_NEST,				// ��ȣ
	TOKEN_SYMBOL,			// �̻��� �͵��� �ƴ� ��
	END_OF_TOKEN_TYPE
} TOKEN_TYPE_TAG;

typedef struct {
	int				nType;
	char			szStr[128];
	unsigned long	dwValue;
} TOKENStt;

// 10�������� 16�������� Ȯ���Ѵ�.
static int nIsNumber( char *pS )
{
	int nI;

	nI = strlen( pS );
	if( nI == 0 )
		return( 0 );	// �ƹ��͵� �ƴϴ�.

	if( pS[0] == '0' && ( pS[1] == 'x' || pS[1] == 'X' ) )
		return( 16 );	// 16����
	else if( pS[nI-1] == 'h' || pS[nI-1] == 'H' )
		return( 16 );	// 16����

	if( '0' <= pS[0] && pS[0] <= '9' )
		return( 10 );	// 10����

	return(0);
}

// ��ū�� ���Ѵ�.
static char *pGetToken( TOKENStt *pTOKEN, char *pS )
{
	char	*pNext, szWord[128];
	int		nI;
	
	pNext = pS;
	memset( pTOKEN, 0, sizeof( TOKENStt ) );
	if( pNext[0] == 0 )
		return( pNext );

	// �� �ܾ ���Ѵ�.
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
	
	// �����ΰ�?
	nI = nIsNumber( szWord );

	if( nI == 10 )			// 10����
	{
		pTOKEN->nType   = TOKEN_NUMBER;
		pTOKEN->dwValue = dwDecValue( szWord );
		return( pNext );

	}
	else if( nI == 16 )		// 16����
	{
		pTOKEN->nType = TOKEN_NUMBER;
		pTOKEN->dwValue = dwHexValue( szWord );
		return( pNext );
	}

	// �ɺ��� ó���Ѵ�.
	pTOKEN->nType = TOKEN_SYMBOL;
	
	return( pNext );
}					  

#define MAX_VALUE	16
#define MAX_OP		16

// ������ �켱�������� ���Ѵ�.
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

// nIndex Operator�� ����Ѵ�.
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
	default : return( -1 );		// �� �� ���� ������.
	}

	// operator�� packing�Ѵ�.
	for( nI = nIndex; nI < *pTotalOp; nI++ )
	{
		pOp[nI]   = pOp[nI+1];
		pOp[nI+1] = 0;
	}

	// value packing�Ѵ�.
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

// operator�� ó���Ѵ�.
static int _nOperator( unsigned long *pValue, char *pOp, int *pTotalOp, int *pTotalValue, int nContinue )
{
	int nI, nV;

	for( nI = 0; nI < *pTotalOp;  )
	{
		// ���� �����ڴ� �ϳ� ���̴�.
		if( nI +1 == *pTotalOp )
		{
			if( nContinue == 1 )
				nCalc( pValue, pOp, pTotalOp, pTotalValue, nI );

			break;
		}

		nV = nOpPriority( pOp[nI] ) - nOpPriority( pOp[nI+1] );

		// ���ʿ������� �켱���� ���� �� ũ��.
		if( nV > 0 )
			nCalc( pValue, pOp, pTotalOp, pTotalValue, nI );		// ����Ѵ�.
		else if( nV == 0 )	// �켱������ ����.
		{	// ������ �����ڰ� �ƴϸ� ����ص��ȴ�.
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

// pS�� �Էµ� ������ ����Ͽ� pRValue�� �����Ѵ�.  (0:����, -1:����)
int _nCalcStr( char *pS, unsigned long *pRValue, char **ppNext )
{
	int				nI;
	char			*pNext;
	TOKENStt		token;
	char			op[MAX_OP];			// ���� 5���� ������ �ȴ�.
	unsigned long	value[MAX_VALUE];		// ���� 6���� ������ �ȴ�.
	int				nTotalOp;
	int				nTotalValue;

	pRValue[0] = 0;
	if( pS == NULL || pS[0] == 0 )
		return( 0 );

	nTotalOp = nTotalValue = 0;
	for( pNext = pS; pNext[0] != 0; )
	{	// ��ū�� ���Ѵ�.
		pNext = pGetToken( &token, pNext );

		// ��ū�� ���� �� ������ �����Ѵ�.
		if( token.nType == TOKEN_UNKNOWN )
			break;

		// ��ȣó��
		if( token.nType == TOKEN_NEST )
		{	// ���� ��ȣ?
			if( token.szStr[0] == '(' )
			{	
				nI = _nCalcStr( pNext, pRValue, &pNext );
				if( nI == -1 )		
					return( -1 );		// ����
				// ��ūŸ���� ���ڷ� �����Ѵ�.				
				token.nType   = TOKEN_NUMBER;
				token.dwValue = pRValue[0];
			} // �� ��ȣ ?
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
		
		// ���ڰ� ���� �������� operator�� ó���Ѵ�.
		if( token.nType == TOKEN_NUMBER && nTotalOp > 1 )
		{	
			nI = nOperator( value, op, &nTotalOp, &nTotalValue, 0 ); // continue = 0
			if( nI == -1 )
				return( -1 );
		}					 
	}	

	// ���� operator�� value���� ó���Ѵ�.
	nI = nOperator( value, op, &nTotalOp, &nTotalValue, 1 ); // continue = 1
	if( nI == -1 )
		return( -1 );

	// operator�� ����� ������ �ƴϴ�.
	if( nTotalOp > 0 || nTotalValue > 1 )
		return( -1 );

	pRValue[0] = value[0];

	if( ppNext != NULL )
		ppNext[0] = pNext;
	
	return( 0 );	// ���� 
}

int nCalcStr( char *pS, unsigned long *pRValue )
{
	int nR;
	
	nR = _nCalcStr( pS, pRValue, NULL );

	return( nR );
}
