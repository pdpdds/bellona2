#include "bellona2.h"

//extern int asm_printf( char *pFmt,...);
#define MYASM_PRINTF kdbg_printf


typedef struct {
	// ONEBYTE
	int	nOneByteIndex;	// ONEBYTE TABLE�� ���� �Ϲݸ�� �ε���.
	int	oneExtIndex[2];	// Ȯ�� �׷쿡 ���� �ε���.
	// TWOBYTE
	int	nTwoByteIndex;	// TWOBYTE TABLE�� ���� �Ϲݸ�� �ε���.
	int twoExtIndex[2];	// Ȯ�� �׷쿡 ���� �ε���.
} OpIndexStt;

int			nTotalLex;
LexStt		lex[MAX_LEX];
OpIndexStt 	opindex[ TOTAL_ot ];

static OpDataStt  *basicOneTbl[ONEBYTE_OPTBL_SIZE+1];
static OpData2Stt *basicTwoTbl[TWOBYTE_OPTBL_SIZE+1];
char   szMyAsmError[260];	// ������ �߻����� �� ��Ʈ���� ����.

#define MAX_ODATA 5
typedef struct {
	int			nTbl;				// Max Weight�� ���� ������ OneByteTbl���� TwoByteTbl������ ��Ÿ����. 
	int			nWeight;			// Weight��.
	int			nIndex;				// ���� �ε��� ��.

	int			nTotal1, nTotal2;
	int			one[ MAX_ODATA ];
	int			two[ MAX_ODATA ];
} ODStt;

/////////////////////////////////////////////////////////////
UINT16	wAsmDefault = 1;		// ������� �ϱ� ���� Default
/////////////////////////////////////////////////////////////

// pGrp �׷쿡 �ִ� nSize��ŭ�� Ȯ��׷� ������ ����� �ش�.
int nSetExtGrp( UINT16 *pGrp, int nSize, int nGrpIndex, int nTbl )
{
	int nI;

	if( nTbl == 1 )
	{
		for( nI = 0; nI < nSize; nI++ )
		{
			if( opindex[ pGrp[nI] ].oneExtIndex[0] == -1 )
				opindex[ pGrp[nI] ].oneExtIndex[0] = nGrpIndex;
			else
				opindex[ pGrp[nI] ].oneExtIndex[1] = nGrpIndex;
		}
	}
	else
	{
		for( nI = 0; nI < nSize; nI++ )
		{
			if( opindex[ pGrp[nI] ].twoExtIndex[0] == -1 )
				opindex[ pGrp[nI] ].twoExtIndex[0] = nGrpIndex;
			else
				opindex[ pGrp[nI] ].twoExtIndex[1] = nGrpIndex;
		}
	}

	return( 0 );
}

// Ȯ�� �׷��� ó���Ѵ�.
static int nProcessExtGrp( int nGrpType, int nGrpIndex, int nTbl )
{
	switch( nGrpType )
	{
    case ot_ESC8 : nSetExtGrp( swEscD8Tbl,  8, nGrpIndex, nTbl ); break;
    case ot_ESC9 : nSetExtGrp( swEscD9Tbl1, 8, nGrpIndex, nTbl ); nSetExtGrp( swEscD9Tbl2, 8*4, nGrpIndex, nTbl ); break;
    case ot_ESCa : nSetExtGrp( swEscDaTbl,  8, nGrpIndex, nTbl ); break;
    case ot_ESCb : nSetExtGrp( swEscDbTbl,  8, nGrpIndex, nTbl ); break;
    case ot_ESCc : nSetExtGrp( swEscDcTbl,  8, nGrpIndex, nTbl ); break;
    case ot_ESCd : nSetExtGrp( swEscDdTbl1, 8, nGrpIndex, nTbl ); nSetExtGrp( swEscDdTbl2, 8, nGrpIndex, nTbl); break;
    case ot_ESCe : nSetExtGrp( swEscDeTbl,  8, nGrpIndex, nTbl ); break;
    case ot_ESCf : nSetExtGrp( swEscDfTbl,  8, nGrpIndex, nTbl ); break;
    case ot_GRP1 : nSetExtGrp( swGrp01Tbl,  8, nGrpIndex, nTbl ); break;
    case ot_GRP2 : nSetExtGrp( swGrp02Tbl,  8, nGrpIndex, nTbl ); break;
    case ot_GRP3 : nSetExtGrp( swGrp03Tbl,  8, nGrpIndex, nTbl ); break;
    case ot_GRP4 : nSetExtGrp( swGrp04Tbl,  8, nGrpIndex, nTbl ); break;
    case ot_GRP5 : nSetExtGrp( swGrp05Tbl,  8, nGrpIndex, nTbl ); break;
    case ot_GRP6 : nSetExtGrp( swGrp06Tbl,  8, nGrpIndex, nTbl ); break;
    case ot_GRP7 : nSetExtGrp( swGrp07Tbl,  8, nGrpIndex, nTbl ); break;
    case ot_GRP8 : nSetExtGrp( swGrp08Tbl,  8, nGrpIndex, nTbl ); break;
    }

	return( 0 );
}

static int nInitTwoByteTbl()
{
	int			nPrevExt, nI, nK, nMin;
	OpData2Stt	*pT;

	// TWOBYTE TABLE�� ���������Ѵ�.
    for( nI = 0; nI < TWOBYTE_OPTBL_SIZE-1; nI++ )
	{
		nMin = nI;
        for( nK = nI+1; nK < TWOBYTE_OPTBL_SIZE; nK++ )
		{
			if( basicTwoTbl[nMin]->wType > basicTwoTbl[nK]->wType )
				nMin = nK;
		}
		if( nMin != nI )
		{
			pT = basicTwoTbl[nI];
			basicTwoTbl[nI]   = basicTwoTbl[nMin];
			basicTwoTbl[nMin] = pT;
		}
	}

	// TWOBYTE INDEX�� �����Ѵ�.	(Ȯ�� �׷쿡 ���� �����ʹ� �����Ǿ� ���� �ʴ�. )
	nPrevExt = -1;
    for( nI = 0; nI < TWOBYTE_OPTBL_SIZE; nI++ )
	{
		nK = (int)basicTwoTbl[nI]->wType;
		
		if( ot_ESC8 <= nK && nK <= ot_GRP9 )
		{		// Ȯ�� �׷�
			if( nPrevExt != nK )
			{
				nPrevExt = nK;		// �ѹ� ó���ߴ� Ȯ�� �׷��� �ٽ� ó���� �ʿ䰡 ����.
				nProcessExtGrp( nK, nI, 2 );  // 2 <- TwoByteTbl
			}
		}
		else	// �Ϲݸ��
		{
			if( opindex[nK].nTwoByteIndex == -1 )
				opindex[nK].nTwoByteIndex	= nI;
		}
	}								 
	// ���� ������ ��Ʈ���� ���̺��� ���� ó���� ����Ű���� �Ѵ�.
	basicTwoTbl[ ONEBYTE_OPTBL_SIZE ] = &TwoByteTbl[0];
	
	return( 0 );
}

static int nInitOneByteTbl()
{
	int			nPrevExt, nI, nK, nMin;
	OpDataStt	*pT;

	// ONEBYTE TABLE�� ���������Ѵ�.
    for( nI = 0; nI < ONEBYTE_OPTBL_SIZE-1; nI++ )
	{
		nMin = nI;
        for( nK = nI+1; nK < ONEBYTE_OPTBL_SIZE; nK++ )
		{
			if( basicOneTbl[nMin]->wType > basicOneTbl[nK]->wType )
				nMin = nK;
		}
		if( nMin != nI )
		{
			pT = basicOneTbl[nI];
			basicOneTbl[nI]   = basicOneTbl[nMin];
			basicOneTbl[nMin] = pT;
		}
	}

	// ONEBYTE INDEX�� �����Ѵ�.	(Ȯ�� �׷쿡 ���� �����ʹ� �����Ǿ� ���� �ʴ�. )
	nPrevExt = -1;
    for( nI = 0; nI < ONEBYTE_OPTBL_SIZE; nI++ )
	{
		nK = (int)basicOneTbl[nI]->wType;
		
		if( ot_ESC8 <= nK && nK <= ot_GRP9 )
		{		// Ȯ�� �׷�
			if( nPrevExt != nK )
			{
				nPrevExt = nK;		// �ѹ� ó���ߴ� Ȯ�� �׷��� �ٽ� ó���� �ʿ䰡 ����.
				nProcessExtGrp( nK, nI, 1 ); // 1 <- OneByteTbl
			}
		}
		else	// �Ϲݸ��
		{
			if( opindex[nK].nOneByteIndex == -1 )
				opindex[nK].nOneByteIndex = nI;
		}
	}			
	
	// ���� ������ ��Ʈ���� ���̺��� ���� ó���� ������ ot_DB�� ����Ű���� �Ѵ�.
	basicOneTbl[ ONEBYTE_OPTBL_SIZE ] = &OneByteTbl[0];
	
	return( 0 );
}

// ������� ���� ������ ���̺��� �����Ѵ�.
int nInitOpCodeTbl()
{
	int nI;

	// �⺻���� �����Ѵ�.
    for( nI = 0; nI < ONEBYTE_OPTBL_SIZE; nI++ )
		basicOneTbl[nI] = &OneByteTbl[nI];
    for( nI = 0; nI < TWOBYTE_OPTBL_SIZE; nI++ )
		basicTwoTbl[nI] = &TwoByteTbl[nI];

	// INDEX TABLE�� �ʱ�ȭ�Ѵ�. 
	memset( opindex, 0, sizeof( opindex ) );
	for( nI = 0; nI < TOTAL_ot; nI++ )
	{
		opindex[nI].nOneByteIndex  = -1;
		opindex[nI].oneExtIndex[0] = -1;
		opindex[nI].oneExtIndex[1] = -1;
		
		opindex[nI].nTwoByteIndex  = -1;
		opindex[nI].twoExtIndex[0] = -1;
		opindex[nI].twoExtIndex[1] = -1;
	}

	// OneByte Table�� �ʱ�ȭ
	nInitOneByteTbl();

	// TwoByte Table�� �ʱ�ȭ
	nInitTwoByteTbl();

	return( 0 );
}

// ����� TAB�� �ǳʶڴ�. 
static char *pSkipSpace( char *pS )
{
	int nI;

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( pS[nI] == 0 || ( pS[nI] != ' ' && pS[nI] != '\t' ) )
			return( &pS[nI] );
	}
	return( &pS[nI] );
}

static char *pGetToken( char *pToken, char *pS )
{
	char *pNext;
	int  nI, nStringFlag;

	pToken[0] = 0;
	pNext = pSkipSpace( pS );
	if( pNext[0] == 0 )
		return( pNext );

	nStringFlag = 0;

	for( nI = 0; pS[nI] != 0; )
	{
		pToken[nI] = pNext[nI];
		nI++;
		pToken[nI] = 0;

		if( pToken[nI-1] == '\'' )
		{
			if( nStringFlag == 0 )
				nStringFlag = 1;		// ��Ʈ�� ���κκ��� '
			else
				nStringFlag = 0;		// ��Ʈ�� �� �κ��� '
		}

		if( nStringFlag == 1 )
			continue;

		if( pNext[nI-1] == ':' || pNext[nI-1] == ',' ||
			pNext[nI-1] == '+' || pNext[nI-1] == '-' || pNext[nI-1] == '*' || pNext[nI-1] == '/' ||
			pNext[nI-1] == '(' || pNext[nI-1] == ')' || pNext[nI-1] == '[' || pNext[nI-1] == ']' )
			break;

		if( pNext[nI] == 0 || pNext[nI] == ' ' || pNext[nI] == '\t' || pNext[nI] == ',' ||
			pNext[nI] == '+' || pNext[nI] == '-' || pNext[nI] == '*' || pNext[nI] == '/' ||
			pNext[nI] == '(' || pNext[nI] == ')' || pNext[nI] == '[' || pNext[nI] == ']' )
			break;
	}	

	pNext = pSkipSpace( &pNext[nI] );

	return( pNext );
}

static int nNumberBase = 16;

/*
static DWORD dwHexValue( char *pS )
{
	DWORD dwR;
	int   nI;

	dwR = 0;
	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( '0' <= pS[nI] && pS[nI] <= '9' )
		{
			dwR *= 16;
			dwR += (DWORD)( pS[nI] - '0' );
		}
		else if( 'A' <= pS[nI] && pS[nI] <= 'F' )
		{
			dwR *= 16;
			dwR += (DWORD)( pS[nI] - 'A' + 10 );
		}
		else if( 'a' <= pS[nI] && pS[nI] <= 'f' )
		{
			dwR *= 16;
			dwR += (DWORD)( pS[nI] - 'a' + 10 );
		}
	}

	return( dwR );
}

static DWORD dwDecValue( char *pS )
{
	DWORD dwR;
	int   nI;

	dwR = 0;
	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( '0' <= pS[nI] && pS[nI] <= '9' )
		{
			dwR *= 10;
			dwR += (DWORD)( pS[nI] - '0' );
		}
	}

	return( dwR );
}
*/

// ��Ʈ���� ���ڷ� �����Ѵ�.
static DWORD dwGetValue( char *pS )
{
	DWORD dwR;
	int   nI;

	dwR = 0;
	nI = strlen( pS );

	if( nI == 0 )				// ���� ��Ʈ��
		return( 0 );
	else if( nI == 1 )			// �� ����.
	{
		dwR = (DWORD)( pS[0] - '0' );
		return( dwR );
	}

	if( nNumberBase == 16 || pS[1] == 'x' || pS[nI-1] == 'h' || pS[nI-1] == 'H' )
		dwR = dwHexValue( pS );
	else
		dwR = dwDecValue( pS );

	return( dwR );
}


static int nUppercase( char *pS )
{
	int nI;

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( 'a' <= pS[nI] && pS[nI] <= 'z' )
			pS[nI] = pS[nI] - 'a' + 'A';
	}

	return( nI );
}

// �� ��Ʈ���� ���Ѵ�.
static int nCmpStr( char *pY, char* pX )
{
	int nI;

	for( nI = 0; ; nI++ )
	{
		if( pY[nI] == pX[nI] )
		{
			if( pX[nI] == 0 )
				return(0 );
		}
		else if( pY[nI] > pX[nI] )
			return( 1 );
		else
			return( -1 );
	}

	return( 0 );
}

// ���̺��� �־��� ��ū�� ã�´�. 
int nSearchTbl( RsvSymStt *pRTbl, int nMin, int nMax, char *pToken )
{
	int			nR, nI, nK;
	RsvSymStt	*pT;

	nI = ( nMin + nMax ) / 2;
	pT = &pRTbl[nI];

	nK = nCmpStr( pT->pStr, pToken );
	if( nK == 0 )
		return( nI );

	if( nI == nMin || nI == nMax )		// ���̻� ã�ƺ� ��Ʈ���� ����.
		return( -1 );

	if( nK > 0 )
		nR = nSearchTbl( pRTbl, nMin, nI, pToken );
	else
		nR = nSearchTbl( pRTbl, nI, nMax, pToken );
	
	return( nR );
}

static int nIsOpCodeStr( int nMin, int nMax, char *pS )
{
	int		nI, nK;

	nI = ( nMin + nMax ) / 2;
					   
	nK = nCmpStr( spMnemonic[nI], pS );
	if( nK == 0 )
		return( nI );

	if( nI == nMin || nI == nMax )		// ���̻� ã�ƺ� ��Ʈ���� ����.
		return( -1 );

	if( nK > 0 )
		nK = nIsOpCodeStr( nMin, nI, pS );
	else
		nK = nIsOpCodeStr( nI, nMax, pS );

	return( nK );
}

// OpCode Alias (JC�� JB�� ������ CodeTable���� JB�� ��ϵǾ� �ִ�.
// JC�� JB�� �ٲپ� �־�� �Ѵ�.)
typedef struct {
	char *pOrg;
	char *pAlias;
} AliasOpStt;
static AliasOpStt aliasOp[] = {
	{ "JC",		"JB"	},
	{ "JNC",	"JNB"	},
	{ NULL,		NULL	}
};
static int nOpCodeAlias( char *pS )
{
	int nI;
	for( nI = 0; aliasOp[nI].pOrg != NULL; nI++ )
	{
		if( nCmpStr( aliasOp[nI].pOrg, pS ) == 0 )
		{
			strcpy( pS, aliasOp[nI].pAlias );
			return( 0 );
		}
	}

	return( -1 );
}

static int nChkTokenType( LexStt *pLex, char *pToken )
{
	int  nI;
	char szT[TOKEN_SIZE];

	if( pToken[0] == 0 )
		return( 0 );

	memset( pLex, 0, sizeof( LexStt ) );

	// ���ڸ� �ν��Ѵ�.
	if( '0' <= pToken[0] && pToken[0] <= '9' )
	{
		pLex->dwValue = dwGetValue( pToken );
		// ũ�⸦ �����Ѵ�.
		//if( pLex->dwValue >= (DWORD)0x01000000 )
		//	pLex->nSubType = 4;   (1, 2, 4����Ʈ�� �ν��Ѵ�.)
		if( pLex->dwValue >= (DWORD)0x00010000 )
			pLex->nSubType = 4;
		else if( pLex->dwValue >= (DWORD)0x00000100 )
			pLex->nSubType = 2;
		else
			pLex->nSubType = 1;

		// pToken�� :���� ������ ���׸�Ʈ ��巹���̴�.
		nI = strlen( pToken );
		if( pToken[nI-1] == ':' )
			pLex->nType = LT_ABSADDR;
		else
			pLex->nType = LT_NUMBER;
		return( 0 );
	}

	// �빮�ڷ� ��ȯ�Ѵ�.
	strcpy( szT, pToken );
	nUppercase( szT );	

	// OpCode�� ��� Aliasó�� (JC -> JB )
	nOpCodeAlias( szT );

	// ����ɺ� �ν��Ѵ�.
	nI = nSearchTbl( rsvTbl, 0, MAX_RSVSYM, szT );
	if( nI != -1 )		// �ɺ��� ã�Ҵ�.  
	{
		RsvSymStt *pR;

		pR				= &rsvTbl[nI];
		pLex->nType		= pR->nType;
		pLex->nSubType	= pR->nSubType;
		return(0);
	}			  

	// OpCode�� �ν��Ѵ�.
	nI = nIsOpCodeStr( 1, TOTAL_ot, szT );		// -1 �� �����ϸ� OpCode�� �ƴϴ�.
	if( nI > 0 )
	{
		pLex->nType		= LT_OPCODE;
		pLex->nSubType	= nI;
		return(0);
	}

	// ���׸�Ʈ �����Ƚ� �ν�.
	if( szT[1] == 'S' && szT[2] == ':' )
	{
		pLex->nType		= LT_SEGPRX;
		switch( szT[0] )
		{
		case 'C' : pLex->nSubType = rCS; break;
		case 'D' : pLex->nSubType = rDS; break;
		case 'E' : pLex->nSubType = rES; break;
		case 'F' : pLex->nSubType = rFS; break;
		case 'G' : pLex->nSubType = rGS; break;
		case 'S' : pLex->nSubType = rSS; break;
		default  : 			
			strcpy( szMyAsmError, "Syntax error on segment prefix." );
			return( -1 );	// �Ƹ��� Syntax ������ ���̴�. 
		}
		return( 0 );
	}

	// ������ �����Ƚ��� �ν�.
	if( nCmpStr( szT, "BYTE" ) == 0 )
	{
		pLex->nType		= LT_SIZEPRX;
		pLex->nSubType	= 1;
		return( 0 );
	}
	else if( nCmpStr( szT, "WORD" ) == 0 )
	{
		pLex->nType		= LT_SIZEPRX;
		pLex->nSubType	= 2;
		return( 0 );
	}
	else if( nCmpStr( szT, "DWORD" ) == 0 )
	{
		pLex->nType		= LT_SIZEPRX;
		pLex->nSubType	= 4;
		return( 0 );
	}
	else if( nCmpStr( szT, "FAR" ) == 0 )
	{
		pLex->nType		= LT_FAR;
		if( wAsmDefault == 1 )
			pLex->nSubType = 6;
		else
			pLex->nSubType = 4;
		return( 0 );
	}
	else if( nCmpStr( szT, "PTR" ) == 0 )
	{
		pLex->nType		= LT_PTR;
		return( 0 );
	}

	// String �ν� 'ABCD'...
	if( szT[0] == '\'' )
	{
		int nK;

		nI = strlen( szT );
		if( nI == 1 )		// ���� üŷ.
		{
			strcpy( szMyAsmError, "Another ' is required." );
			return( -1 );	// '�� �ϳ� �޶������� ������.
		}
		else if( nI > 6 )
		{
			strcpy( szMyAsmError, "String is too long." );
			return( -1 );
		}
		if( szT[nI-1] != '\'' )
		{
			strcpy( szMyAsmError, "Another ' is required." );
			return( -1 );	// ���� '�� �����Ǿ���.
		}

		pLex->nType    = LT_STRING;
		pLex->nSubType = nI-2;	// ���� ���ڱ���.
		for( nK = 1; nK < nI-1; nK++ )
		{
			pLex->dwValue =  (DWORD)( pLex->dwValue << 8 );
			pLex->dwValue += (DWORD)( (UCHAR)szT[nK] );
		}
		return( 0 );
	}

	// ���Ϲ��� �ν�.
	if( strlen( szT ) == 1 )
	{
		pLex->nType		= LT_CHAR;
		pLex->nSubType	= (int)szT[0];
		return( 0 );
	}								  

	return( 0 );
}	

// �����м��� �����Ѵ�.
static int nLexer( LexStt *pLex, char *pS )
{
	int  nTotal, nR;
	char *pNext;
	char szToken[TOKEN_SIZE];

	nTotal = 0;
	pNext = pS;
	for( ; ; )
	{	// ��ū�� �����´�. 
		pNext = pGetToken( szToken, pNext );
		if( szToken[0] == 0 )
			break;

		// ��ū Ÿ�԰� ���� Ȯ���Ѵ�.
		nR = nChkTokenType( &pLex[nTotal], szToken );
		MYASM_PRINTF( "%-10s : (Type=%d,SubType=%d) (value = 0x%X)\n", szToken, pLex[nTotal].nType, pLex[nTotal].nSubType, pLex[nTotal].dwValue );
		if( nR == 0 )
			nTotal++;
		else
			return( -1 );	// ������ ���� �׳� �ٷ� ���ư���.
	}	

	return( nTotal );
}

// ���������� ũ�⸦ ���Ѵ�.
static int nGetRegisterSize( int nReg )
{
	if( rAH <= nReg && nReg <= rDL )
		return( 1 );
	else if( rAX <= nReg && nReg <= rSP )
		return( 2 );
	else if( rEAX <= nReg && nReg <= rESP )
		return( 4 );
	else if( rCS <= nReg && nReg <= rSS )
		return( 2 );
	else if( rC0 <= nReg && nReg <= rT7 )
		return( 4 );
	else if( rST <= nReg && nReg <= rST7 )
		return( 8 );

	return( 0 );
}

// ������ DWORD������ �����Ѵ�.
static LexStt *pExpression( LexStt *pLex, DWORD *pValue, int *pErr )
{
	LexStt	*pN;
	DWORD	dwR;
	int		nI, nR;
	SEntStt s[2];

	pN	= pLex;
	*pValue = dwR = 0;
	*pErr   = 0;
	
	// ����ó��
	if( pN == NULL || ( pN->nType != LT_NUMBER && pN->nSubType != '(' ) )
	{
		*pErr = -1;
		return( pN );
	}

	//  ������ �� ǥ�ø� �� �д�.
	s[0].nType	 = SENT_END;
	s[0].dwValue = 0;
	nR = nPushSEnt( &s[0] );
	if( nR != 0 )
		goto ERROR_X;

	for( nI = 0; ; )
	{	// ����.
		if( pN[nI].nType == LT_NUMBER )
		{
			s[0].nType	 = SENT_NUMBER;
			s[0].dwValue = pN[nI].dwValue;
			nR = nPushSEnt( &s[0] );
			if( nR != 0 )
				goto ERROR_X;
			pN++;
		}
		else if( pN[nI].nType == LT_CHAR )
		{	// ��ȣ�� ���Դ�.
			if( pN[nI].nSubType == '(' )
			{
				pN = pExpression( &pN[nI+1], &s[0].dwValue, &nR );
				if( nR != 0 )
					goto ERROR_X;
				s[0].nType	 = SENT_NUMBER;
				nR = nPushSEnt( &s[0] );
				if( nR != 0 )
					goto ERROR_X;
			}
			else if( pN[nI].nSubType == ')' )
			{	// ��ȣ�� ����Ǿ���.  ��꿡 ����.
				pN++;
				goto CALC;
			}	// ��Ģ������
			else if( pN[nI].nSubType == '+'	|| pN[nI].nSubType == '-' || 
			/**/pN[nI].nSubType == '*' || pN[nI].nSubType == '/' )
			{
				s[0].nType	 = SENT_OPERATOR;
				s[0].dwValue = (DWORD)pN[nI].nSubType;
				s[1].nType	 = SENT_NUMBER;
				// ������ ������ ���ڰ� ������ ���
				if( pN[nI+1].nType == LT_NUMBER )
				{
					s[1].dwValue = pN[nI+1].dwValue;
					pN += 2;
				}  // �Ǵٸ� ��ȣ�� �����̴�.
				else if( pN[nI+1].nType == LT_CHAR && pN[nI+1].nSubType == '(' )			
				{	
					pN = pExpression( &pN[nI+2], &dwR, &nR );
					if( nR != 0 )
						goto ERROR_X;
					s[1].dwValue = dwR;
				}
				else 
					goto CALC;
			
				nR = nPushSEntOperator( s );
				if( nR != 0 )
					goto ERROR_X;
			}	
		}
		else	
			goto CALC;
	}

CALC:	 
	nR = nCalcStk( &dwR );
	MYASM_PRINTF( "Expression = %d(0x%-X)\n", dwR, dwR );

	// ������ �� ǥ�ø� ���Ѵ�.
	nPopSEnt( &s[0] );

	*pValue = dwR;
	return( pN );

ERROR_X:
	*pErr = -1;
	MYASM_PRINTF( "Expression error.\n" );
	return( pN );
}

// [�� �����ϴ� �޸� ���۷��带 ó���Ѵ�.
static LexStt *pMemoryOperand( OpStt *pOp, int nOperand, LexStt *pLex, int *pErr )
{
	LexStt		*pN;
	OperandStt	*pOpnd;

	pOpnd = &pOp->Oprnd[nOperand];
	pN    = pLex;
	*pErr = 0;
	// [�� �������� ������ �޸� ���۷��尡 �ƴϴ�.
	if( pN->nType != LT_CHAR || pN->nSubType != '[' )
		goto ERROR_X;
	else
		pN++;

	if( pN->nType == LT_REG32 || pN->nType == LT_REG16 )
	{
		if( pN[1].nType != LT_CHAR )	// �������� ������ ] + * �� �ϳ��� ���;� �Ѵ�.
			goto ERROR_X;
		else
		{
			if( pN[1].nSubType == ']' || pN[1].nSubType == '+')
				goto GET_BASE;		// ���̽��� �ִ� ��� �Ǵ� ���̽��� �ε����� ������ �ٴ� ���
			else if( pN[1].nSubType == '*' )
				goto GET_INDEX;		// �ٷ� �ε����� ���� ���.
			else
				goto ERROR_X;
		}
	}	  
	else if( pN->nType == LT_NUMBER )
		goto GET_DISP;				// �ٷ� ������ ���� ���.

GET_BASE:	// ���̽� �������͸� �ν��Ͽ���.
	pOpnd->wRegBase = (UINT16)pN->nSubType;
	if( pN->nType == LT_REG32 )
		pOpnd->wWidth = 4;
	else
		pOpnd->wWidth = 2;
	pN++;				   
	if( pN->nType == LT_CHAR )
	{
		if( pN->nSubType == ']' )	
			goto END;		// ���̽��� �޶� ���Դ�.
		if( pN->nSubType == '+' )
		{
			if( pN[1].nType == LT_NUMBER )
			{
				pN++;		// ���̽��� ���� (�ε����� ����)
				goto GET_DISP;
			}
			else if( pN[1].nType == LT_REG32 || pN[1].nType == LT_REG16 )
			{	// �ε��� �������Ͱ� ���Դ�.
				pN++;
				goto GET_INDEX;
			}
			else
				goto ERROR_X;
		}
		else 
			goto ERROR_X;
	}
	else
		goto ERROR_X;

GET_INDEX:	// �ε��� �������͸� ���Ѵ�. 
	pOpnd->wIndex = (UINT16)pN->nSubType;
	if( pN->nType == LT_REG32 )
		pOpnd->wWidth = 4;
	else
		pOpnd->wWidth = 2;
	pN++;		
	if( pN->nType == LT_CHAR )
	{
		if( pN->nSubType == ']' )	
			goto END;		// ���̽� + �ε����� ���
		if( pN->nSubType == '+' )
		{
			if( pN[1].nType == LT_NUMBER )
			{
				pN++;		// ���̽��� ���� (�ε����� ����)
				goto GET_DISP;
			}
			else 
				goto ERROR_X;
		}
		else if( pN->nSubType == '*' )
		{
			pN++;
			goto GET_SCALE;

		}
		else
			goto ERROR_X;
	}
	else
		goto ERROR_X;

GET_SCALE:	// �������� ���Ѵ�.
	if( pN->nType != LT_NUMBER )
		goto ERROR_X;
	pOpnd->wScale = (UINT16)pN->dwValue;
	pN++;
	if( pN->nType == LT_CHAR )
	{
		if( pN->nSubType == ']' )	
			goto END;		// ���̽� + �ε����� ���
		if( pN->nSubType != '+' || pN[1].nType != LT_NUMBER )
			goto ERROR_X;
		pN++;		// ���̽��� ���� (�ε����� ����)
		goto GET_DISP;
	}
	else
		goto ERROR_X;

GET_DISP:
	if( pN->nType == LT_NUMBER )
	{
		pOpnd->dwValue = pN->dwValue;
		pOpnd->wValueSize	= (UINT16)pN->nSubType;
		pN++;
	}
	else if( pN->nType == LT_CHAR && pN->nSubType == '(' )
	{
		int nErr;
		pN = pExpression( pN, &pOpnd->dwValue, &nErr );
		if( nErr != 0 )		// ���� ������ ������ �߻��Ͽ���.
			goto ERROR_X;

		// ����� ���� �������� �ʾ����Ƿ� ����� �����Ѵ�.
		if( pOpnd->dwValue >= (DWORD)0x01000000 )
			pOpnd->wValueSize = 4;
		else if( pOpnd->dwValue >= (DWORD)0x00010000 )
			pOpnd->wValueSize = 3;
		else if( pOpnd->dwValue >= (DWORD)0x00000100 )
			pOpnd->wValueSize = 2;
		else
			pOpnd->wValueSize = 1;
	}		
	else
		goto ERROR_X;

	// �ݵ�� ]�� ���;� �Ѵ�.
	if( pN->nType != LT_CHAR || pN->nSubType != ']' )
		goto ERROR_X;
		
END:
	pOpnd->wType = LT_MEMORY;
	pN++;
	return( pN ); 

ERROR_X:
	*pErr = -1;
	return( pN );
}

// ���۷��尡 � Ÿ���� ���������� �ν��Ѵ�.
static LexStt *pSetOperand( OpStt *pOp, int nOperand, LexStt *pLex )
{
	LexStt		*pN;
	OperandStt	*pOpnd;
	int			nFar;

	pN		= pLex;
	pOpnd	= &pOp->Oprnd[nOperand];

	// ��������
	if( LT_REG8 <= pN->nType && pN->nType <= LT_STKREG )
	{
		pOpnd->wType    = (UINT16)pN->nType;
		pOpnd->wRegBase = (UINT16)pN->nSubType;
		pOpnd->wSize    = (UINT16)nGetRegisterSize( pN->nSubType );
		return( &pN[1] );
	}

	// ������ 10:0000
	if( pN->nType == LT_ABSADDR )
	{
		if( pN[1].nType != LT_NUMBER )
			return( NULL );
		pOpnd->wType		= LT_ABSADDR;
		pOpnd->wSize		= (UINT16)pN->nSubType;
		pOpnd->wRegBase		= (UINT16)pN->dwValue;
		pOpnd->wValueSize   = (UINT16)pN[1].nSubType;
		pOpnd->dwValue		= pN[1].dwValue;
		return( &pN[1] );
	}

	// ������ �����Ƚ��� �پ��°� Ȯ���� ����.
	if( pN->nType == LT_SIZEPRX )
	{
		if( pN[1].nType != LT_PTR )
			return( NULL );			// PTR�� ������.
		// ����� ũ�Ⱑ �����Ǿ���.
		pOpnd->wSize = (UINT16)pN->nSubType;
		pN += 2;
	}

	// FAR PTR
	if( pN->nType == LT_FAR )
	{
		if( pN[1].nType != LT_PTR )
			return( NULL );			// PTR�� ������.
		// �տ� FAR�� �پ���.
		pOpnd->wSize = (UINT16)pN->nSubType;
		nFar = 1;
		pN += 2;
	}
	else 
		nFar = 0;

	// ���׸�Ʈ �����Ƚ��� �پ��ִ��� Ȯ���� ����.
	if( pN->nType == LT_SEGPRX )
	{
		pOp->wSegPrx = (UINT16)pN->nSubType;
		pN++;		// ���׸�Ʈ �����Ƚ��� ������ Ȯ���� �޸� ���۷���.
	}

	// �޸� ���۷���
	if( pN->nType == LT_CHAR && pN->nSubType == '[' )
	{
		int nErr;
		// pOpnd->wType = LT_MEMORY (�ȿ��� ���õǾ� ���´�.)
		pN = pMemoryOperand( pOp, nOperand, pN, &nErr );
		if( nErr != 0 )		// �޸� ���۷��� ���� ������ ������ �߻��ߴ�.
			return( NULL );
		// FAR PTR�� ���� ��� LT_FARMEMORY�� ������ �ش�.
		if( nFar == 1 )
			pOp->Oprnd[nOperand].wType = LT_FARMEMORY;
	}// ����. ( IMMEDIATE DATA )
	else if( pN->nType == LT_NUMBER || ( pN->nType == LT_CHAR && pN->nSubType == '(' ) )	
	{
		int nErr;
		pN = pExpression( pN, &pOpnd->dwValue, &nErr );
		if( nErr != 0 )		// ���� ������ ������ �߻��Ͽ���.
			return( NULL );

		// ����� ���� �������� �ʾ����Ƿ� ����� �����Ѵ�.
		if( pOpnd->wSize == 0 )
		{	// ���۷��� ������� 1,2,4�� �ν��Ѵ�.
			//if( pOpnd->dwValue >= (DWORD)0x01000000 )
			//	pOpnd->wSize = 4;
			if( pOpnd->dwValue >= (DWORD)0x00010000 )
				pOpnd->wSize = 4;
				//pOpnd->wSize = 3;
			else if( pOpnd->dwValue >= (DWORD)0x00000100 )
				pOpnd->wSize = 2;
			else
				pOpnd->wSize = 1;
		}	
		pOpnd->wType = (UINT16)LT_NUMBER;
  	}

	return( pN );
}	

// Lexical Analysis�� ���� ������ pLex�� ������� Parsing�� �����Ѵ�. 
static int nParsing( OpStt *pOp, LexStt *pLex )
{
	LexStt	*pNext;

	// ������ �ʱ�ȭ�� �ش�. 
	nInitStk();

	memset( pOp, 0, sizeof( OpStt ) );

	// �׻� OpCode�� ���� ���;� �Ѵ�.
	if( pLex[0].nType == LT_OPCODE )
	{	// OpCode ot_???�� �����Ѵ�.
		pOp->wType = (UINT16)pLex[0].nSubType;
	}
	else
	{	// OpCode�� �����Ƿ� �ٷ� ���������Ѵ�.
		strcpy( szMyAsmError, "OPCODE not found." ); 
		return( -1 );
	}				

	pNext = &pLex[1];
	// Operand 3������ ó���Ѵ�.
	pNext = pSetOperand( pOp, 0, pNext );
	if( pNext == NULL )					goto PARSING_ERROR;
	if( pNext->nType == LT_UNKNOWN )	goto END_PARSING;
	if( pNext->nType != LT_CHAR || pNext->nSubType != ',' ) goto END_PARSING;
	pNext++;

	pNext = pSetOperand( pOp, 1, pNext );
	if( pNext == NULL )					goto PARSING_ERROR;
	if( pNext->nType == LT_UNKNOWN )	goto END_PARSING;
	if( pNext->nType != LT_CHAR || pNext->nSubType != ',' ) goto END_PARSING;
	pNext++;

	pNext = pSetOperand( pOp, 2, pNext );
	if( pNext == NULL )					goto PARSING_ERROR;
	if( pNext->nType == LT_UNKNOWN )	goto END_PARSING;	
	if( pNext->nType != LT_UNKNOWN ) goto END_PARSING;

END_PARSING:	
	return( 0 );
PARSING_ERROR:
	return( -1 );
}

// ������� ���� ���۷��带 ����Ѵ�.
static void vDispOperand( char *pS )
{
	if( pS[0] == 0 )
		return;
	else if( pS[0] == '!' )
		MYASM_PRINTF( "!(%s,%d)   ", pRegStr( pS[1] ), pS[1] );
	else if( pS[0] == '%' )
		MYASM_PRINTF( "%(Implicit-%d)   ", pS[1] );
	else
		MYASM_PRINTF( "%s   ", pS );
}	

// �޸� ���۷��带 �ν��� �� �ִ� �������� �Ǵ��Ѵ�. 
static int nIsMemory( OperandStt *pOprnd, char *pS )
{
	if( pS[0] == 'E' || pS[0] == 'M' )
		return( 0 );
	else if( pS[0] == 'O' && pOprnd->wRegBase == 0 && pOprnd->wIndex == 0 )
		return( 0 );			// O�� ������ �����ϴ� ���.
	else
		return( -1 );
}

// �������� ���۷��带 �ν��� �� �ִ� �������� �Ǵ��Ѵ�.
static int nIsReg( char *pS, UINT16 wReg, UINT16 wType )
{
	char ch1;

	ch1 = pS[1];

	//if( eAX <= ch1 && ch1 <= eSP )
	//	ch1 = ( ch1 - eAX );

	switch( pS[0] )
	{
	case '!' :	// ������ �������Ͱ� ���;߸� �Ѵ�.
		if( eAX <= ch1 && ch1 <= eSP )  // eAX�� ��� AX�Ǵ� EAX�� ��ġ�� �� �ִ�.
		{
			if( wType == LT_REG32 )
			{	// 32��Ʈ ���������� ��� EAX, EBX...��� ���� ����.
				if( wReg == (ch1 - eAX) + rEAX )
					return( 0 );
				else
					return( -1 );
			}
			else if( wType == LT_REG16 )
			{	// 16��Ʈ ���������� ��� AX, BX...��� ���� ����.
				if( wReg == (ch1 - eAX) + rAX )
					return( 0 );
				else
					return( -1 );
			}
			else
				return( -1 );				
		}
		else
		{
			if( wReg == (UINT16)ch1 ) 
				return( 0 );
			else 
				return( -1 );
		}
		break;

	case 'E' :
	case 'G' :
	case 'R' :
		if( wType == LT_REG8 || wType == LT_REG16 || wType == LT_REG32 )
			return( 0 );
		else
			return( -1 );
		break;

	case 'S' :
		if( wType == LT_SEGREG )
			return( 0 );
		else
			return( -1 );
		break;

	case 'D' :
		if( wType == LT_DBGREG )
			return( 0 );
		else
			return( -1 );
		break;

	case 'C' :
		if( wType == LT_CTRLREG )
			return( 0 );
		else
			return( -1 );
		break;

	case 'T' :
		if( wType == LT_TESTREG )
			return( 0 );
		else
			return( -1 );
		break;

	}	   
	return( -1 );
}

// ���ڿ� �޸��� ��� ���۷����� ũ�Ⱑ �ش� �ڵ� ���̺� ��Ʈ���� �����ϴ��� ���Ѵ�.
static int nChkNumMemSize( UINT16 wSize, char ch )
{
	switch( ch )
	{
	case 'b' :
		if( wSize == 1 )
			return( 0 );
		else
			return( -1 );
		break;

	case 'w' :
		if( 0 < wSize && wSize <= 2 )
			return( 0 );
		else
			return( -1 );
		break;

	case 's' :
	case 'v' :
		if( 0 < wSize && wSize <= 4 )
			return( 0 );
		else
			return( -1 );
		break;

	case 'p' :
		if( wSize == 6 )
			return( 0 );
		else
			return( -1 );

	case '?' :
		if( wSize == 2 || wSize == 6 )
			return( 0 );
		else
			return( -1 );

	default :
		return( -1 );
		break;
	}
	return( -1 );
}

// ���������� ��� ���۷����� ũ�Ⱑ �ش� �ڵ� ���̺� ��Ʈ���� �����ϴ��� ���Ѵ�.
static int nChkRegSize( UINT16 wSize, char ch )
{
	switch( ch )
	{
	case 'b' :
		if( wSize == 1 )
			return( 0 );
		else
			return( -1 );
		break;

	case 'w' :
		if( wSize == 2 )
			return( 0 );
		else
			return( -1 );
		break;

	case 'd' :
		if( wSize == 4 )
			return( 0 );
		else
			return( -1 );
		break;
		break;

	case 'v' :
		if( wSize == 2 || wSize == 4 )
			return( 0 );
		else
			return( -1 );
		break;

	default :
		return( -1 );
		break;
	}
	return( -1 );
}

// �ν��� ���۷��尡 ���� �ڵ� ���̺��� ��Ʈ���� �ִ� ���۷���� ��ġ�ϴ��� Ȯ���Ѵ�.
static int nCompareOperand( OperandStt *pOprnd, char *pS )
{
	int nR;

	if( pOprnd->wType == 0 )
	{
		if( pS[0] == 0 )
			return( 0 );		// ���۷��尡 �Ѵ� �������� �ʴ´�.
		else if( pS[0] == '^' )	// ^�� �����Ѵ�. 
			return( 0 );
		else
			return( -1 );		// �־�� �� ���۷��尡 ����.
	}
	else if( pS[0] == 0 )
		return(-1 );			// ������� ���۷��尡 �ִ�.

	nR = 0;
	switch( pOprnd->wType )		// �� �� 0�� �ƴ� ���.
	{
	case LT_ABSADDR :			// ���� ��巹�� 10:1100	<- 4����Ʈ �Ǵ� 6����Ʈ
		if( pS[0] != 'A' )
			return( -1 );
		break;

	case LT_NUMBER :
		if( pS[0] != 'I' && pS[0] != 'J' )
		{
			if( pS[0] == '%' )		// �Ͻ������� ���� ���ڸ� �� ���. ROR AX,1
			{
				if( pOprnd->dwValue == (DWORD)pS[1] )
					return( 0 );	// ũ�⸦ ���� �ʿ���� �׳� ���ư���.
			}
			return( -1 );
		}
		nR = nChkNumMemSize( pOprnd->wSize, pS[1] );
		break;

	case LT_FARMEMORY :
	case LT_MEMORY :
		nR = nIsMemory( pOprnd, pS );
		if( nR == -1 )
			return( -1 );
		nR = nChkNumMemSize( pOprnd->wSize, pS[1] );
		//nR = nChkNumMemSize( pOprnd->wValueSize, pS[1] );
		break;

	case LT_REG8	:
	case LT_REG16	:
	case LT_REG32	:
	case LT_SEGREG	:
	case LT_DBGREG	:
	case LT_CTRLREG	:
	case LT_TESTREG	:
	case LT_STKREG	:
		nR = nIsReg( pS, pOprnd->wRegBase, pOprnd->wType );
		if( nR == -1 )
			return( -1 );
		if( pS[0] != '!' )	// !�� ũ�⸦ ���� �ʿ䰡 ����.
			nR = nChkRegSize( pOprnd->wSize, pS[1] );
		break;

	default :
		return( -1 );
		break;
	}	

	return( nR );
}

static int n_xx_GetOperandWeight( char *pS )
{
	int nR;

	nR = 0;

	if( pS[0] == '!' )		nR += 20;
	if( pS[0] == '%' )		nR += 20;
	if( pS[0] == 'O' )		nR += 7;
	if( pS[1] == 'b' )		nR += 10;
	if( pS[1] == 'w' )		nR += 5;

	return( nR );
}

// Oprand ������ Weight�� ���Ѵ�.
static int nGetOperandWeight( char *pOp1, char *pOp2, char *pOp3 )
{
	int nR;

	nR = 0;

	nR += n_xx_GetOperandWeight( pOp1 );
	nR += n_xx_GetOperandWeight( pOp2 );
	nR += n_xx_GetOperandWeight( pOp3 );

	return( nR );
}

static void vDispCodeEntry( char *pTitle, UINT16 wType, char *pOp0, char *pOp1, char *pOp2 )
{
	MYASM_PRINTF( "\t%s(%d)   ", pTitle, wType );
	vDispOperand( pOp0 );
	vDispOperand( pOp1 );
	vDispOperand( pOp2 );
	MYASM_PRINTF( "\n" );
}

// OneByteTbl�� ������ ��Ʈ���� pOp�� Oprnd[]�� ��ġ�ϴ��� Ȯ���Ѵ�.
static int nCmpOperand1( OpStt *pOp, OpDataStt *pOne )
{
	int	result[3];
	int	nR;

	nR = -1;
	result[0] = nCompareOperand( &pOp->Oprnd[0], (char*)pOne->szOperand[0] );
	result[1] = nCompareOperand( &pOp->Oprnd[1], (char*)pOne->szOperand[1] );
	result[2] = nCompareOperand( &pOp->Oprnd[2], (char*)pOne->szOperand[2] );

	// �ķ����Ͱ� ��ġ�Ѵ�.
	if( result[0] == 0 && result[1] == 0 && result[2] == 0 )
	{	//����ġ�� �ε����� �����Ѵ�.
		nR = nGetOperandWeight( (char*)pOne->szOperand[0], (char*)pOne->szOperand[1], (char*)pOne->szOperand[2] );
		MYASM_PRINTF( "*" );
		// Code Entry�� ����Ѵ�.
		vDispCodeEntry( "1-OPCODE", pOne->wType, (char*)pOne->szOperand[0], (char*)pOne->szOperand[1], (char*)pOne->szOperand[2] );
	}
														
	// ����ġ�� ���ϵȴ�. 
	return( nR );
}	

// TwoByteTbl�� ������ ��Ʈ���� pOp�� Oprnd[]�� ��ġ�ϴ��� Ȯ���Ѵ�.
static int nCmpOperand2( OpStt *pOp, OpData2Stt *pTwo )
{
	int	result[3];
	int	nR;

	nR= -1;
	result[0] = nCompareOperand( &pOp->Oprnd[0], (char*)pTwo->szOperand[0] );
	result[1] = nCompareOperand( &pOp->Oprnd[1], (char*)pTwo->szOperand[1] );
	result[2] = nCompareOperand( &pOp->Oprnd[2], (char*)pTwo->szOperand[2] );

	// �ķ����Ͱ� ��ġ�Ѵ�.
	if( result[0] == 0 && result[1] == 0 && result[2] == 0 )
	{	//����ġ�� �ε����� �����Ѵ�.
		nR = nGetOperandWeight( (char*)pTwo->szOperand[0], (char*)pTwo->szOperand[1], (char*)pTwo->szOperand[2] );
		MYASM_PRINTF( "*" );
		// Code Entry�� ����Ѵ�.
		vDispCodeEntry( "2-OPCODE", pTwo->wType, (char*)pTwo->szOperand[0], (char*)pTwo->szOperand[1], (char*)pTwo->szOperand[2] );
	}	


	// ����ġ�� ���ϵȴ�. 
	return( nR );
}	

// pOData�� �ִ� ����ġ ���� �����Ѵ�.  (����ġ�� ���� ���� ������ �ڵ带 �����ϰ� �ȴ�. )
// nTbl�� 1, 2���� ���µ� OneByteTble���� TwoByteTbl������ �ǹ��Ѵ�.
static int nUpdateWeight( ODStt *pOData, int nWeight, int nTbl, int nIndex )
{
	if( pOData->nWeight < nWeight )
	{
		pOData->nWeight = nWeight;
		pOData->nTbl	= nTbl;
		pOData->nIndex	= nIndex;
	}	

	return( 0 );
}

static int nSearchSequence1( OpStt *pOp, ODStt *pOData, int nOneIndex ) 
{
	int	nI, nR;

	if( nOneIndex <= 0 )
		return( -1 );
		
	for( nI = nOneIndex; nI < ONEBYTE_OPTBL_SIZE; nI++ )
	{
		if( nI == 0x67 )
			nI = 0x67;

		nR = nCmpOperand1( pOp, basicOneTbl[nI] );
		if( nR >= 0 )  // ��ġ�� ���� ������ �д�.
		{
			nUpdateWeight( pOData, nR, 1, nI );
			pOData->one[ pOData->nTotal1++ ] = nI;
		}
		
		if( basicOneTbl[nI]->wType != basicOneTbl[nI+1]->wType )
			break;
	}
	return( 0 );
}

static int nSearchSequence2( OpStt *pOp, ODStt *pOData, int nTwoIndex ) 
{
	int nI, nR;

	if( nTwoIndex <= 0 )
		return( -1 );
		
	for( nI = nTwoIndex; nI < TWOBYTE_OPTBL_SIZE; nI++ )
	{
		nR = nCmpOperand2( pOp, basicTwoTbl[nI] );
		if( nR >= 0 )  // ��ġ�� ���� ������ �д�.
		{
			nUpdateWeight( pOData, nR, 2, nI );
			pOData->two[ pOData->nTotal2++ ] = nI;
		}
	
		if( basicTwoTbl[nI]->wType != basicTwoTbl[nI+1]->wType )
			break;
	}
	return( 0 );
}
//	// ONEBYTE
//	int	nOneByteIndex;	// ONEBYTE TABLE�� ���� �Ϲݸ�� �ε���.
//	int	oneExtIndex[2];	// Ȯ�� �׷쿡 ���� �ε���.
//	// TWOBYTE
//	int	nTwoByteIndex;	// TWOBYTE TABLE�� ���� �Ϲݸ�� �ε���.
//	int twoExtIndex[2];	// Ȯ�� �׷쿡 ���� �ε���.
// Codetable���� op�� ���۷��忡 �´� ������ ã�� ODStt�� �����Ѵ�.
static int nSearchCodeTable( OpStt *pOp, ODStt *pOData )
{
	OpIndexStt	*pIndex;
	int			nR;

	memset( pOData, 0, sizeof( ODStt ) );

	pIndex = &opindex[ (int)pOp->wType ];
	// �� ����Ʈ OpCode���� Operand�� ��ġ�Ǵ� ���� �ִ��� ã�´�.
	nR = nSearchSequence1( pOp, pOData, pIndex->nOneByteIndex  );
	nR = nSearchSequence1( pOp, pOData, pIndex->oneExtIndex[0] );
	nR = nSearchSequence1( pOp, pOData, pIndex->oneExtIndex[1] );

	// �� ����Ʈ OpCode���� Operand�� ��ġ�Ǵ� ���� �ִ��� ã�´�.
	nR = nSearchSequence2( pOp, pOData, pIndex->nTwoByteIndex  );
	nR = nSearchSequence2( pOp, pOData, pIndex->twoExtIndex[0] );
	nR = nSearchSequence2( pOp, pOData, pIndex->twoExtIndex[1] );

	MYASM_PRINTF( "Total (%d/%d) Matched Code Table Entries.\n", pOData->nTotal1, pOData->nTotal2 );
	
	return( 0 );
}

// ������� ���� �׳� ����� ����.
static void vDispOpnd( OperandStt *pOpnd )
{
	if( pOpnd->wType == 0 )
		return;

	if( pOpnd->wType == LT_MEMORY )
		MYASM_PRINTF( "Mem" );
	else if( pOpnd->wType == LT_FARMEMORY )
		MYASM_PRINTF( "farM" );
	else if( pOpnd->wType == LT_NUMBER )
		MYASM_PRINTF( "Imm" );
	else if( LT_REG8 <= pOpnd->wType && pOpnd->wType <= LT_STKREG )
		MYASM_PRINTF( "Reg" );
	else
		MYASM_PRINTF( "Unknown" );

	MYASM_PRINTF( "(size=%d/width=%d)   ", pOpnd->wSize, pOpnd->wWidth );
}

// ���۷����� ���տ� ������ �ִ��� Ȯ���� ����.
static int nChkOperandError( OpStt *pOp )
{
	return( 0 );
}

// �������� ���� ���۷����� ũ�⸦ ��������� �����Ѵ�.
static int nInheritSize( OpStt *pOp )
{
	OperandStt *pO0, *pO1;

	// ���۷��尡 �ϳ��� ���� ���.
	if( pOp->Oprnd[0].wType == 0 )
		return( 0 );

	// ���۷��尡 �� �� �ִ� ���.
	if( pOp->Oprnd[1].wType == 0 )
		return( 0 );

	// ���۷��尡 �� �� �ִ� ���. 
	if( pOp->Oprnd[2].wType == 0 )
	{
		pO0 = &pOp->Oprnd[0];
		pO1 = &pOp->Oprnd[1];

		if( pO0->wSize == 0 )
		{
			if( pO1->wSize == 0 )
				return( -1 );		// ���۷��尡 �� ���� Size�� 0�̴�.
			else
			{
				pO0->wSize = pO1->wSize;
				MYASM_PRINTF( "wSize0 <- wSize1 (%d)\n", pO1->wSize );
			}
		}
		else
		{
			if( pO1->wSize == 0 )
			{
				pO1->wSize = pO0->wSize;
				MYASM_PRINTF( "wSize0 -> wSize1 (%d)\n", pO0->wSize );
			}

		}			
		
		return( 0 );
	}

	// ���۷��尡 �� �� �� �ִ� ���.
	

	return( 0 );
}

// �������� ���ڵ� ���� ���Ѵ�.
static int nGerRegBitCode( UINT16 wRegType )
{
	int nR;

	if( wRegType == rAL || wRegType == rAX || wRegType == rEAX )
		nR = 0;
	else if( wRegType == rCL || wRegType == rCX || wRegType == rECX )
		nR = 1;
	else if( wRegType == rDL || wRegType == rDX || wRegType == rEDX )
		nR = 2;
	else if( wRegType == rBL || wRegType == rBX || wRegType == rEBX )
		nR = 3;
	else if( wRegType == rAH || wRegType == rSP || wRegType == rESP	)
		nR = 4;
	else if( wRegType == rCH || wRegType == rBP || wRegType == rEBP	)
		nR = 5;
	else if( wRegType == rDH || wRegType == rSI || wRegType == rESI	)
		nR = 6;
	else if( wRegType == rBH || wRegType == rDI || wRegType == rEDI	)
		nR = 7;
	else
		nR = -1;
	return( nR );
}

// Rm �ʵ尡 �޸𸮷� ���ȴ�.
static int nRmIsMem( OperandStt *pOperand, UCHAR *pModRegRm )
{
	UINT16	wT;
	int		nNoDisp, nSibling, nX;

	nNoDisp  = 0;			 
	nSibling = 0;

	// Clear Mod Field 00(Mod) 000(Reg) 000(Rm)
	pModRegRm[0] = (UCHAR)( pModRegRm[0] & (UCHAR)0x3F );
	// Displacement�� ������ ���� Mod Field�� �����Ѵ�.
	if( pOperand->wValueSize == 0 )			// No Displacement
	{
		nNoDisp = 1;
		pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x00 );
	}
	else if( pOperand->wValueSize == 1 )	// Byte Displacement
		pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x40 );
	else if( pOperand->wValueSize == 2 )	// Word Displacement
		pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x80 );
	else if( pOperand->wValueSize == 4 )	// Dword Displacement
		pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x80 );
	else
		return( -1 );

	// Clear Rm Field ( 11 111 000 )
	pModRegRm[0] = (UCHAR)( pModRegRm[0] & (UCHAR)0xF8 );
	// 16��Ʈ Addressing Mode
	if( rBX <= pOperand->wRegBase && pOperand->wRegBase <= rBP )
	{
		wT = pOperand->wRegBase + pOperand->wIndex;
		switch( wT )
		{
		case (UINT16)(rBX + rSI) :
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x00 );
			break;
		case (UINT16)(rBX + rDI) :
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x01 );
			break;
		case (UINT16)(rBP + rSI) :
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x02 );
			break;
		case (UINT16)(rBP + rDI) :
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x03 );
			break;
		case (UINT16)rSI :
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x04 );
			break;
		case (UINT16)rDI :
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x05 );
			break;
		case (UINT16)rBP :
			if( nNoDisp == 1 )		// BP�� Displacement ���� ���̸� ����
				return(0);
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x06 );
			break;
		case (UINT16)rBX :
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x07 );
			break;
		default :		// Base�� Index�� ������ Ʋ�ȴ�.
			return( -1 );
			break;
		}
	}
	// 32��Ʈ Addressing Mode
	else if( rEAX <= pOperand->wRegBase && pOperand->wRegBase <= rEBP )
	{	// �ε����� ������ ���� ��� (Sibling�� ���� �ʾƵ� �ȴ�.)
		if( pOperand->wIndex == 0 )
		{
			if( pOperand->wRegBase == rEBP && nNoDisp == 1 )
				return( -1 );		// �������� EBP�� ���Ǿ���.

			nX = nGerRegBitCode( pOperand->wRegBase );
			// Base�� �����Ѵ�. 
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)nX );

		}
		else// �ε����� ���Ǹ� Sibling������ Ȯ���� �Ͼ��. 
		{	// Sibling = 00(Scale) 000(Index) 000(Base)
			nSibling = 1;
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x04 );  // 04 = Sibling Extension

			// Clear Scale Field
			pModRegRm[1] = (UCHAR)( pModRegRm[1] & (UCHAR)0x3F );
			//Scale ���� �����Ѵ�. 
			if( pOperand->wScale == 0 )
				pModRegRm[1] = (UCHAR)( pModRegRm[1] | (UCHAR)0x00 );
			else if( pOperand->wScale == 2 )			// * 2
				pModRegRm[1] = (UCHAR)( pModRegRm[1] | (UCHAR)0x40 );
			else if( pOperand->wScale == 4 )	// * 4
				pModRegRm[1] = (UCHAR)( pModRegRm[1] | (UCHAR)0x80 );
			else if( pOperand->wScale == 8 )	// * 8
				pModRegRm[1] = (UCHAR)( pModRegRm[1] | (UCHAR)0xC0 );
			else
				return( -1 );		// �߸��� Scale ��.

			// Clear Index Field
			pModRegRm[1] = (UCHAR)( pModRegRm[1] & (UCHAR)0xC7 );
			// Index ���� �����Ѵ�.
			nX = nGerRegBitCode( pOperand->wIndex );
			nX = (int)( nX << 3 );
			// Index�� �����Ѵ�. 
			pModRegRm[1] = (UCHAR)( pModRegRm[1] | (UCHAR)nX );
			
			// Clear Base Field
			pModRegRm[1] = (UCHAR)( pModRegRm[1] & (UCHAR)0xF8 );
			// Base ���� �����Ѵ�.
			nX = nGerRegBitCode( pOperand->wRegBase );
			// Index�� �����Ѵ�. 
			pModRegRm[1] = (UCHAR)( pModRegRm[1] | (UCHAR)nX );
		}	
	}
	else
		return( -1 );

	if( nSibling == 0 )		
		return( 1 );	// Sibling�� ������ �ʾҰ� ModRegRm�� ���Ǿ���.
	else
		return( 2 );	// Sibling���� ���Ǿ���.
}

// Rm �ʵ尡 ������Ʈ�ͷ� ���ȴ�.
static int nRmIsReg( OperandStt *pOperand, UCHAR *pModRegRm )
{
	int nX;

	if( pOperand->wRegBase == 0 )
		return( -1 );

	// Clear Mod Field 00(Mod) 000(Reg) 000(Rm)
	pModRegRm[0] = (UCHAR)( pModRegRm[0] & (UCHAR)0x3F );
	pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0xC0 );	// Mod = 11

	// Clear Rm Field and Set
	pModRegRm[0] = (UCHAR)( pModRegRm[0] & (UCHAR)0xF8 );
	nX = nGerRegBitCode( pOperand->wRegBase );
	pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)nX );

	return( 1 );
}

// ModRegRm�� Reg Field
static int nRegField( OperandStt *pOperand, UCHAR *pModRegRm )
{
	int nX;

	if( pOperand->wRegBase == 0 )
		return( -1 );

	// Clear Reg Field and Set
	pModRegRm[0] = (UCHAR)( pModRegRm[0] & (UCHAR)0xC7 );
	nX = nGerRegBitCode( pOperand->wRegBase );
	nX = (int)( nX << 3 );
	pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)nX );

	return( 1 );
}						  

// ModRegRm ����Ʈ�� ���� �����Ѵ�.
// ���ϰ� 0-ModRegRm ����Ʈ�� ������� �ʾ���, 1-ModRegRm�� ����, 2-Sibling���� �������.
static int n_xx_MakeModRegRm( OperandStt *pOperand, char *pS, UCHAR *pModRegRm )
{
	int nR;

	nR = 0;			
	switch( pS[0] )
	{
	case 'E' :	//Rm �ʵ尡 �������� �Ǵ� �޸𸮷� ����.
		if( pOperand->wType == LT_MEMORY || pOperand->wType == LT_FARMEMORY )
			nR = nRmIsMem( pOperand, pModRegRm );
		else if( LT_REG8 <= pOperand->wType && pOperand->wType <= LT_REG32 )
			nR = nRmIsReg( pOperand, pModRegRm );
		break;

	case 'R' :	// Rm �ʵ尡 �׻� �������ͷ� ���ȴ�.
		if( LT_REG8 <= pOperand->wType && pOperand->wType <= LT_REG32 )
			nR = nRmIsReg( pOperand, pModRegRm );
		break;

	case 'M' :	// Rm �ʵ尡 �׻� �޸𸮷� ���ȴ�.
		if( pOperand->wType == LT_MEMORY )
			nR = nRmIsMem( pOperand, pModRegRm );
		break;

	case 'S' :
	case 'D' :
	case 'C' :
	case 'T' :
	case 'G' :	// Operand�� RegBase�� ModRegRm�� Reg �ʵ�� ���δ�. 
		if( LT_REG8 <= pOperand->wType && pOperand->wType <= LT_REG32 )
			nR = nRegField( pOperand, pModRegRm );
		break;

	}	
	
	return( nR );
}	

// ModregRm Byte�� �����Ѵ�.
static int nMakeModRegRm( OpStt *pOp, char *pOp0, char *pOp1, char *pOp2, UCHAR *pModRegRm )
{
	int nSize, nR;
		   
	nSize = 0;
	
	pModRegRm[0] = pModRegRm[1] = 0;

	// ù ��° ���۷���
	if( pOp->Oprnd[0].wType == 0 )
		return( nSize );
	nR = n_xx_MakeModRegRm( &pOp->Oprnd[0], pOp0, pModRegRm );
	if( nR == -1 )			return( -1 );
	else if( nSize < nR )	nSize = nR;

	// �� ��° ���۷���
	if( pOp->Oprnd[1].wType == 0 )
		return( nSize );
	nR = n_xx_MakeModRegRm( &pOp->Oprnd[1], pOp1, pModRegRm );
	if( nR == -1 )			return( -1 );
	else if( nSize < nR )	nSize = nR;
	
	// �� ��° ���۷���
	if( pOp->Oprnd[2].wType == 0 )
		return( nSize );
	nR = n_xx_MakeModRegRm( &pOp->Oprnd[2], pOp2, pModRegRm );
	if( nR == -1 )			return( -1 );
	else if( nSize < nR )	nSize = nR;

	return( nSize );
}

// Immediate Data�� �ش�Ǵ� ���� pCode�� �����Ѵ�.
// Immediate Data�� ũ�⸦ �����Ѵ�.
static int nImmediateData( UCHAR *pCode, char chSize, OperandStt *pOperand )
{
	if( chSize == 'v' )
	{
		if( pOperand->wSize == 4 )
			chSize = 'd';		
		else if( pOperand->wSize == 2 || pOperand->wSize == 1 )
			chSize = 'w';		
	}
	
	if( chSize == 'b' )
	{
		pCode[0] = (UCHAR)pOperand->dwValue;
		return( 1 );
	}
	else if( chSize == 'w' )
	{
		memcpy( pCode, &pOperand->dwValue, 2 );
		return( 2 );
	}
	else if( chSize == 'd' )
	{
		memcpy( pCode, &pOperand->dwValue, 4 );
		return( 4 );
	}
	return( 0 );
}

// Displacement�� �ش�Ǵ� ���� pCode�� �����Ѵ�.
static int nDisplacement( UCHAR *pCode, char chSize, OperandStt *pOperand )
{
	int nSize = 0;

	// Displacement�� ������ �ʾҴ�.
	if( pOperand->wValueSize == 0 )
		return( 0 );
	
	// Byte ���� (2002-03-27 �߰�)
	if( pOperand->wValueSize == 1 )
	{
		memcpy( pCode, &pOperand->dwValue, 1 );
		return( 1 );
	}

	if( pOperand->wRegBase == 0 )
	{	// ���̽� �������͸� ���� �ʾ����� wAsmDefault�� ���� ������ ũ�⸦ �����Ѵ�. 
		if( wAsmDefault == 1 )
			nSize = 4;
		else
			nSize = 2;
	}	// 16Bit�������͸� ��������� ������ 16��Ʈ�� ����. 
	else if( rAX <= pOperand->wRegBase && pOperand->wRegBase <= rSP )
		nSize = 2;
	else if( rEAX <= pOperand->wRegBase && pOperand->wRegBase <= rESP )
		nSize = 4;

	if( nSize == 4 )
	{	// 32Bit - DWORD Displacement
		memcpy( pCode, &pOperand->dwValue, 4 );
		return( 4 );
	}
	else if( nSize == 2 )
	{	// 16Bit Displacement
		memcpy( pCode, &pOperand->dwValue, 2 );
		return( 2 );
	}

	return( 0 );
}	

// Reg Field�� Ȯ���ڵ�� ���ȴ�.
// ��ġ�ϴ� ���� ã������ Reg Field�� �� ���� �����ϰ� �����ϸ� -1����.
static int nRegFieldIsExt( UINT16 wCodeType, UINT16 wGrp, UCHAR *pModRegRm )
{
	UINT16	*pCodeArray;
	int		nI;

	switch( wGrp )
	{
	case ot_GRP1 : pCodeArray = swGrp01Tbl; break;
	case ot_GRP2 : pCodeArray = swGrp02Tbl; break;
	case ot_GRP3 : pCodeArray = swGrp03Tbl; break;
	case ot_GRP4 : pCodeArray = swGrp04Tbl; break;
	case ot_GRP5 : pCodeArray = swGrp05Tbl; break;
	case ot_GRP6 : pCodeArray = swGrp06Tbl; break;
	case ot_GRP7 : pCodeArray = swGrp07Tbl; break;
	case ot_GRP8 : pCodeArray = swGrp08Tbl; break;
	default: return( -1 );
	}

	for( nI = 0; nI < 8; nI++ )
	{
		if( pCodeArray[nI] == wCodeType )
			return( nI );
	}
	
	return( -1 );
}

// JUMP ����� ���۷��带 �����Ѵ�.
static int nMakeJumpOperand( BYTE *pCodeBuff, char cSizeChar, OperandStt *pOperand )
{
	if( pOperand->wType != LT_NUMBER )
		return( 0 );

	// BYTE OFFSET
	if( cSizeChar == 'b' )
	{
		memcpy( pCodeBuff, &pOperand->dwValue, 1 );
		return( 1 );
	}
	else if( cSizeChar == 'v' )	// 2����Ʈ �Ǵ� 4 ����Ʈ  (�ϴ� 4 ����Ʈ�� �Ի��Ѵ�.)
	{
		memcpy( pCodeBuff, &pOperand->dwValue, 4 );
		return( 4 );
	}				

	return( 0 );
}

// ������ Assemble�� �����Ѵ�.
static int nAssemble( OpStt *pOp, int nTbl, int nIndex, UCHAR *pCode )
{
	int		nR, nSize;
	UINT16	wOpCodeType;
	char	*pOp1, *pOp2, *pOp3;
	UCHAR   ModRegRm[2];
	char	nullStr[3];

	nullStr[0] = 0;
	pOp1 = pOp2 = pOp3 = nullStr;
	
	nSize	= 0;
	// Segment Override Prefix
	if( pOp->wSegPrx != 0 )
	{
		UCHAR byTe;
		switch( pOp->wSegPrx )
		{
		case rCS : byTe = (UCHAR)0x2E;	break;
		case rDS : byTe = (UCHAR)0x3E;	break;
		case rES : byTe = (UCHAR)0x26;	break;
		case rFS : byTe = (UCHAR)0x64;	break;
		case rGS : byTe = (UCHAR)0x65;	break;
		case rSS : byTe = (UCHAR)0x36;	break;
		}
		pCode[ nSize++ ] = byTe;
	}

	// OpCode
	if( nTbl == 1 )	
	{	// 1 Byte OpCode
		pCode[ nSize++ ] = (UCHAR)( ( (DWORD)basicOneTbl[nIndex] - (DWORD)OneByteTbl ) / sizeof( OpDataStt ) );
		pOp1 = (char*)basicOneTbl[nIndex]->szOperand[0];
		pOp2 = (char*)basicOneTbl[nIndex]->szOperand[1];
		pOp3 = (char*)basicOneTbl[nIndex]->szOperand[2];
		wOpCodeType = basicOneTbl[nIndex]->wType;
	}
	else
	{	// 0x0F�� Ȯ��Ǵ� 2 Byte OpCode
		pCode[ nSize++ ] = (UCHAR)0x0F;
		pCode[ nSize++ ] = (UCHAR)basicTwoTbl[nIndex]->wNo;
		pOp1 = (char*)basicTwoTbl[nIndex]->szOperand[0];
		pOp2 = (char*)basicTwoTbl[nIndex]->szOperand[1];
		pOp3 = (char*)basicTwoTbl[nIndex]->szOperand[2];
		wOpCodeType = basicTwoTbl[nIndex]->wType;
	}						  

	// GRP7�� ��� pOp0, 1, 2�� �����μ����� �־�� �Ѵ�. (���̺��� �������� �ʴ�.)
	if( wOpCodeType == ot_GRP7 )
	{
		if( pOp->wType == ot_SMSW || pOp->wType == ot_LMSW )
			pOp1 = "Ew"; 
		else if( pOp->wType != ot_INVLPG )
			pOp1 = "Ms" ;
	}
	else if( wOpCodeType == ot_GRP5 && pOp->Oprnd[0].wType == LT_FARMEMORY )
	{	// GRP5�� CALL, JMP�� FAR�� �پ Ev�� �ƴϰ� Ep�� �ȴ�.
		if( pOp->wType == ot_JMP || pOp->wType == ot_CALL )
			pOp1 = "Ep";
	}

	// ModRegRm, Sibling Byte�� �����Ѵ�.
	nR = nMakeModRegRm( pOp, pOp1, pOp2, pOp3, ModRegRm );
	if( nR < 0 ) // �����߻�
		return( -1 );
	// Ȯ�� �׷��� ��� Reg Field�� Code ���� ������ �־�� �Ѵ�.
	if( ot_GRP1 <= wOpCodeType && wOpCodeType <= ot_GRP8 )
	{
		int nExt;
		nExt = nRegFieldIsExt( pOp->wType, wOpCodeType, ModRegRm );
		if( nExt == -1 )
			return( -1 );

		if( wOpCodeType == ot_GRP5 && pOp1[1] == 'p' )
			nExt++;

		// Reg Field Clear
		ModRegRm[0] = (UCHAR)( ModRegRm[0] & (UCHAR)0xC7 );
		nExt = (int)( nExt << 3 );
		ModRegRm[0] = (UCHAR)( ModRegRm[0] | (UCHAR)nExt );
	}

	//////////////////////////////////////////////////////////////////
	//				FPU Ȯ�� �׷��� ���� ó���� �ȵȴ�.				//
	//////////////////////////////////////////////////////////////////

	// ModRegRm, Sibling�� Code�� �����Ѵ�.
	if( nR == 1 )
		pCode[nSize++] = ModRegRm[0];
	else if( nR == 2 )
	{
		pCode[nSize++] = ModRegRm[0];
		pCode[nSize++] = ModRegRm[1];
	}					
	
	// Displacement  (������ 1, 2, 4����Ʈ)
	if( pOp->Oprnd[0].wType == LT_MEMORY )
		nSize += nDisplacement( &pCode[nSize], pOp1[1], &pOp->Oprnd[0] );
	else if( pOp->Oprnd[1].wType == LT_MEMORY )
		nSize += nDisplacement( &pCode[nSize], pOp2[1], &pOp->Oprnd[1] );
	else if( pOp->Oprnd[2].wType == LT_MEMORY )
		nSize += nDisplacement( &pCode[nSize], pOp3[1], &pOp->Oprnd[2] );

	// Iv�� ��� ¦�� �̷�� ���۷����� ũ�⿡ ������Ų��.
	// Immediate Data
	if( pOp1[0] == 'I' )
	{
		if( pOp1[1] == 'v' && pOp->Oprnd[0].wSize < pOp->Oprnd[1].wSize )
			pOp->Oprnd[0].wSize = pOp->Oprnd[1].wSize;
		nSize += nImmediateData( &pCode[nSize], pOp1[1], &pOp->Oprnd[0] );
	}
	else if( pOp2[0] == 'I' )
	{
		if( pOp2[1] == 'v' && pOp->Oprnd[1].wSize < pOp->Oprnd[0].wSize )
			pOp->Oprnd[1].wSize = pOp->Oprnd[0].wSize;
		nSize += nImmediateData( &pCode[nSize], pOp2[1], &pOp->Oprnd[1] );
	}
	else if( pOp3[0] == 'I' )
		nSize += nImmediateData( &pCode[nSize], pOp3[1], &pOp->Oprnd[2] );

	// Address Pointer Ap
	if( pOp1[0] == 'A' )
	{
		memcpy( &pCode[nSize], &pOp->Oprnd[0].wRegBase, 2 );
		nSize+= 2;
		if( wAsmDefault == 1 )
		{
			memcpy( &pCode[nSize], &pOp->Oprnd[0].dwValue, 4 );
			nSize += 4;
		}
		else
		{
			memcpy( &pCode[nSize], &pOp->Oprnd[0].dwValue, 2 );
			nSize += 2;
		}		
	} // JUMP ����� ��� Jb, jv
	else if( pOp1[0] == 'J' )
	{
		nSize += nMakeJumpOperand( &pCode[nSize], pOp1[1], &pOp->Oprnd[0] );
	}

	return( nSize );
}

static int nGetMaxWeight( ODStt *pOData, int *pTbl, int *pIndex )
{
	if( pOData->nIndex != 0 )
	{
		*pTbl   = pOData->nTbl;
		*pIndex = pOData->nIndex;
	}
	else if( pOData->nTotal1 != 0 )
	{
		*pTbl   = 1;
		*pIndex = pOData->one[0];
	}
	else if( pOData->nTotal2 != 0 )
	{
		*pTbl   = 2;
		*pIndex = pOData->two[0];
	}
	else
		return( -1 );	

	// Max Weight�� ����� ����.
	MYASM_PRINTF( "ASM ->" );
	if( *pTbl == 1 )
		vDispCodeEntry( "1-OPCODE", basicOneTbl[*pIndex]->wType, 
			(char*)basicOneTbl[*pIndex]->szOperand[0], 
			(char*)basicOneTbl[*pIndex]->szOperand[1], 
			(char*)basicOneTbl[*pIndex]->szOperand[2] );
	else
		vDispCodeEntry( "2-OPCODE", basicTwoTbl[*pIndex]->wType, 
			(char*)basicTwoTbl[*pIndex]->szOperand[0], 
			(char*)basicTwoTbl[*pIndex]->szOperand[1], 
			(char*)basicTwoTbl[*pIndex]->szOperand[2] );

	return( 1 );
}

static void vDispAssembledCode( UCHAR *pCode, int nSize )
{
	int nI;

	MYASM_PRINTF( "Assembled Code(%d) : ", nSize );
	for( nI = 0; nI < nSize; nI++ )
	{
		MYASM_PRINTF( "%02X ", pCode[nI] );
	}

	MYASM_PRINTF( "\n" );
}

typedef struct {
	UINT16 wOpCode;
	UINT16 wWarp;
	UINT16 wUp;
} WarpUpStt;

static WarpUpStt warpup[] = {
    { 0x60, ot_PUSHA,   ot_PUSHAD },
    { 0x61, ot_POPA,    ot_POPAD  },
    { 0x6D, ot_INSW,    ot_INSD   },
    { 0x6F, ot_OUTSW,   ot_OUTSD  },
    { 0x98, ot_CBW,     ot_CWDE   },
    { 0x99, ot_CWD,     ot_CDQ    },
    { 0x9C, ot_PUSHF,   ot_PUSHFD },
    { 0x9D, ot_POPF,    ot_POPFD  },
    { 0xA5, ot_MOVSW,   ot_MOVSD  },
    { 0xA7, ot_CMPSW,   ot_CMPSD  },
    { 0xAB, ot_STOSW,   ot_STOSD  },
    { 0xAD, ot_LODSW,   ot_LODSD  },
    { 0xAF, ot_SCASW,   ot_SCASD  },
    { 0xCF, ot_IRET,    ot_IRETD  },

    { 0, 0, 0 }
};

// ^�� ���� Warp Up Instruction (return ���)
// �ƴϸ� 0
static int nWarpUpCode( OpStt *pOp, UCHAR *pCode )
{
	int nI;

	for( nI = 0; warpup[nI].wOpCode != 0; nI++ )
	{	// 32 Bit Mode
		if( wAsmDefault == 1 )
		{
			if( pOp->wType == warpup[nI].wUp )
			{
				pCode[0] = (UCHAR)warpup[nI].wOpCode;
				return( 1 );
			}
			else if( pOp->wType == warpup[nI].wWarp )
			{
				pCode[0] = (UCHAR)0x66;
				pCode[1] = (UCHAR)warpup[nI].wOpCode;
				return( 2 );
			}	 
		}
		else
		{
			if( pOp->wType == warpup[nI].wWarp )
			{
				pCode[0] = (UCHAR)warpup[nI].wOpCode;
				return( 1 );
			}
			else if( pOp->wType == warpup[nI].wUp )
			{
				pCode[0] = (UCHAR)0x66;
				pCode[1] = (UCHAR)warpup[nI].wOpCode;
				return( 2 );
			}	 
		}
	}
	return( 0 );
}	

// JUMP addr���� addr ���� EIP�� ���� �����Ѵ�.
static int nRenderJumpAddr( OpStt *pOp, DWORD dwEIP )
{
	int nSize;

	nSize = 0;

	// �������� ����
	if( pOp->Oprnd[0].dwValue > dwEIP )
	{	// �� ����Ʈ�δ� ó���� �ȵȴ�.  (4����Ʈ�� ���� �Ѵ�.)
		if( pOp->Oprnd[0].dwValue - dwEIP > 0x7F )  // ������ ���ؼ� 7F�� �ƴϴ�.
		{	// ��ɾ� ���̰� 5����Ʈ�� �ȴ�.
			pOp->Oprnd[0].wValueSize = 4;
			pOp->Oprnd[0].wSize = 4;
			pOp->Oprnd[0].dwValue = pOp->Oprnd[0].dwValue - dwEIP - 5;
		}
		else
		{	// ��ɾ� ���̰� 2 ����Ʈ�� �ȴ�.
			pOp->Oprnd[0].wValueSize = 1;
			pOp->Oprnd[0].wSize = 1;
			pOp->Oprnd[0].dwValue = pOp->Oprnd[0].dwValue - dwEIP - 2;
		}		
	}
	else  // �������� ����
	{	// �� ����Ʈ�δ� ó���� �ȵȴ�.  (4����Ʈ�� ���� �Ѵ�.)
		if( dwEIP - pOp->Oprnd[0].dwValue > 0x7F )  // ������ ���ؼ� 7F�� �ƴϴ�.
		{	// ��ɾ� ���̰� 5����Ʈ�� �ȴ�.
			pOp->Oprnd[0].wValueSize = 4;
			pOp->Oprnd[0].wSize = 4;
			pOp->Oprnd[0].dwValue = pOp->Oprnd[0].dwValue - dwEIP - 5;
		}
		else
		{	// ��ɾ� ���̰� 2 ����Ʈ�� �ȴ�.
			pOp->Oprnd[0].wValueSize = 1;
			pOp->Oprnd[0].wSize = 1;
			pOp->Oprnd[0].dwValue = pOp->Oprnd[0].dwValue - dwEIP - 2;
		}		
	}

	return( 0 );
}

static int nRenderCallAddr( OpStt *pOp, DWORD dwEIP )
{
	// �յ� ������ 4����Ʈ�� �����Ѵ�.
	pOp->Oprnd[0].wValueSize = 4;
	pOp->Oprnd[0].wSize = 4;
	pOp->Oprnd[0].dwValue = pOp->Oprnd[0].dwValue - dwEIP - 5;

	return( 0 );
}

// �־��� pStr�� ������Ͽ� pBinCode�� ���̳ʸ� �ڵ带 �����ϰ� 
// ���̳ʸ� �ڵ��� ���̸� �����Ѵ�. -1�� �����ϸ� ����.	
// ( 0�� ������ �ǹ������� �ʴ´�. )
// JMP xxxx�� ��� Operand�� Jb, Jv�ε� �� ���� EIP�� �־�� �Ѵ�.
int nMyAsm( char *pStr, unsigned char *pBinCode, DWORD dwEIP )
{
	int		nR;
	OpStt	op;
	ODStt	odata;

	nR = 0;

	// Lexical Analysis
	memset( lex, 0, sizeof( lex ) );
	nTotalLex = nLexer( lex, pStr );
	if( nTotalLex < 0 )		// ������ ���� �ٷ� ���ư���.
		return( -1 );

	// Parsing
	nR = nParsing( &op, lex );
	if( nR != 0 )
		return( -1 );		// ������ ���� �ٷ� ���ư���.

	{// ���۷��� �м��� ���� ����� ����.
		MYASM_PRINTF( "OpCode( 0x%2X, %d)   ", op.wType, op.wType );
		if( op.Oprnd[0].wType != 0 )
			vDispOpnd( &op.Oprnd[0] );
		if( op.Oprnd[1].wType != 0 )
			vDispOpnd( &op.Oprnd[1] );
		if( op.Oprnd[2].wType != 0 )
			vDispOpnd( &op.Oprnd[2] );
		MYASM_PRINTF( "\n" );
	}

	// ^�� �ش�Ǵ� ���
	nR = nWarpUpCode( &op, pBinCode );
	if( nR > 0 )
		goto MYASM_OK;
	
	// ���۷��忡 ������ �ִ��� Ȯ���� ���ƾ� �Ѵ�.
	nR = nChkOperandError( &op );
	if( nR == -1 )
		return( -1 );

	// �������� ���� ���۷����� ũ�� wSize�� ��������� ������ �ش�.
	nR = nInheritSize( &op );

	// JUMP ����� ��� Operand EIP�� ���� ���� �����Ѵ�.
	if( op.wType == ot_JMP || op.wType == ot_LOOP )
		nRenderJumpAddr( &op, dwEIP );
	else if( op.wType == ot_CALL )
	{  // CALL ����̸� �ϴ� ��� ������ �����Ѵ�.
		nRenderCallAddr( &op, dwEIP );
	}
	
	// Searching CodeTable
	nR = nSearchCodeTable( &op, &odata );
	if( nR == -1 )
	{
		MYASM_PRINTF( "Code table search failed!\n" );
		return( -1 );		// ������ ���� �ٷ� ���ư���.
	}
	else 
	{
	}
	
	{////////////////////////////////////////////////////////////
		int nTbl, nIndex;

		// ������ ���� ��Ʈ���� ���Ѵ�. 
		nR = nGetMaxWeight( &odata, &nTbl, &nIndex );
		if( nR > 0 )
		{	// �ڵ带 �����Ѵ�. (Assemble)
			nR = nAssemble( &op, nTbl, nIndex, pBinCode );
			if( nR > 0 )
			{
			}
			else
			{
				MYASM_PRINTF( "Assemble Failed.\n" );
			}
		}
		else
		{
			MYASM_PRINTF( "Max Weight Index Not found.\n" );
		}
	}

MYASM_OK:	// nR�� ��� ���̰� �� �ִ�.
	vDispAssembledCode( pBinCode, nR );

	return( nR );
}



