#include "bellona2.h"

//extern int asm_printf( char *pFmt,...);
#define MYASM_PRINTF kdbg_printf


typedef struct {
	// ONEBYTE
	int	nOneByteIndex;	// ONEBYTE TABLE에 대한 일반명령 인덱스.
	int	oneExtIndex[2];	// 확장 그룹에 대한 인덱스.
	// TWOBYTE
	int	nTwoByteIndex;	// TWOBYTE TABLE에 대한 일반명령 인데스.
	int twoExtIndex[2];	// 확장 그룹에 대한 인덱스.
} OpIndexStt;

int			nTotalLex;
LexStt		lex[MAX_LEX];
OpIndexStt 	opindex[ TOTAL_ot ];

static OpDataStt  *basicOneTbl[ONEBYTE_OPTBL_SIZE+1];
static OpData2Stt *basicTwoTbl[TWOBYTE_OPTBL_SIZE+1];
char   szMyAsmError[260];	// 에러가 발생했을 때 스트링이 들어간다.

#define MAX_ODATA 5
typedef struct {
	int			nTbl;				// Max Weight를 갖는 조합이 OneByteTbl인지 TwoByteTbl인지를 나타낸다. 
	int			nWeight;			// Weight값.
	int			nIndex;				// 최적 인덱스 값.

	int			nTotal1, nTotal2;
	int			one[ MAX_ODATA ];
	int			two[ MAX_ODATA ];
} ODStt;

/////////////////////////////////////////////////////////////
UINT16	wAsmDefault = 1;		// 어셈블을 하기 위한 Default
/////////////////////////////////////////////////////////////

// pGrp 그룹에 있는 nSize만큼의 확장그룹 연결을 만들어 준다.
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

// 확장 그룹을 처리한다.
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

	// TWOBYTE TABLE을 선택정렬한다.
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

	// TWOBYTE INDEX를 설정한다.	(확장 그룹에 대한 포인터는 설정되어 있지 않다. )
	nPrevExt = -1;
    for( nI = 0; nI < TWOBYTE_OPTBL_SIZE; nI++ )
	{
		nK = (int)basicTwoTbl[nI]->wType;
		
		if( ot_ESC8 <= nK && nK <= ot_GRP9 )
		{		// 확장 그룹
			if( nPrevExt != nK )
			{
				nPrevExt = nK;		// 한번 처리했던 확장 그룹은 다시 처리할 필요가 없다.
				nProcessExtGrp( nK, nI, 2 );  // 2 <- TwoByteTbl
			}
		}
		else	// 일반명령
		{
			if( opindex[nK].nTwoByteIndex == -1 )
				opindex[nK].nTwoByteIndex	= nI;
		}
	}								 
	// 가장 마지막 엔트리는 테이블의 제일 처음을 가리키도록 한다.
	basicTwoTbl[ ONEBYTE_OPTBL_SIZE ] = &TwoByteTbl[0];
	
	return( 0 );
}

static int nInitOneByteTbl()
{
	int			nPrevExt, nI, nK, nMin;
	OpDataStt	*pT;

	// ONEBYTE TABLE을 선택정렬한다.
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

	// ONEBYTE INDEX를 설정한다.	(확장 그룹에 대한 포인터는 설정되어 있지 않다. )
	nPrevExt = -1;
    for( nI = 0; nI < ONEBYTE_OPTBL_SIZE; nI++ )
	{
		nK = (int)basicOneTbl[nI]->wType;
		
		if( ot_ESC8 <= nK && nK <= ot_GRP9 )
		{		// 확장 그룹
			if( nPrevExt != nK )
			{
				nPrevExt = nK;		// 한번 처리했던 확장 그룹은 다시 처리할 필요가 없다.
				nProcessExtGrp( nK, nI, 1 ); // 1 <- OneByteTbl
			}
		}
		else	// 일반명령
		{
			if( opindex[nK].nOneByteIndex == -1 )
				opindex[nK].nOneByteIndex = nI;
		}
	}			
	
	// 가장 마지막 엔트리는 테이블의 제일 처음에 나오는 ot_DB를 가리키도록 한다.
	basicOneTbl[ ONEBYTE_OPTBL_SIZE ] = &OneByteTbl[0];
	
	return( 0 );
}

// 어셈블을 위한 포인터 테이블을 구축한다.
int nInitOpCodeTbl()
{
	int nI;

	// 기본값을 설정한다.
    for( nI = 0; nI < ONEBYTE_OPTBL_SIZE; nI++ )
		basicOneTbl[nI] = &OneByteTbl[nI];
    for( nI = 0; nI < TWOBYTE_OPTBL_SIZE; nI++ )
		basicTwoTbl[nI] = &TwoByteTbl[nI];

	// INDEX TABLE을 초기화한다. 
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

	// OneByte Table을 초기화
	nInitOneByteTbl();

	// TwoByte Table을 초기화
	nInitTwoByteTbl();

	return( 0 );
}

// 공백과 TAB을 건너뛴다. 
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
				nStringFlag = 1;		// 스트링 선두부분의 '
			else
				nStringFlag = 0;		// 스트링 끝 부분의 '
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

// 스트링을 숫자로 변경한다.
static DWORD dwGetValue( char *pS )
{
	DWORD dwR;
	int   nI;

	dwR = 0;
	nI = strlen( pS );

	if( nI == 0 )				// 공백 스트링
		return( 0 );
	else if( nI == 1 )			// 한 글자.
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

// 두 스트링을 비교한다.
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

// 테이블에서 주어진 토큰을 찾는다. 
int nSearchTbl( RsvSymStt *pRTbl, int nMin, int nMax, char *pToken )
{
	int			nR, nI, nK;
	RsvSymStt	*pT;

	nI = ( nMin + nMax ) / 2;
	pT = &pRTbl[nI];

	nK = nCmpStr( pT->pStr, pToken );
	if( nK == 0 )
		return( nI );

	if( nI == nMin || nI == nMax )		// 더이상 찾아볼 엔트리가 없다.
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

	if( nI == nMin || nI == nMax )		// 더이상 찾아볼 엔트리가 없다.
		return( -1 );

	if( nK > 0 )
		nK = nIsOpCodeStr( nMin, nI, pS );
	else
		nK = nIsOpCodeStr( nI, nMax, pS );

	return( nK );
}

// OpCode Alias (JC는 JB와 같지만 CodeTable에는 JB만 등록되어 있다.
// JC를 JB로 바꾸어 주어야 한다.)
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

	// 숫자를 인식한다.
	if( '0' <= pToken[0] && pToken[0] <= '9' )
	{
		pLex->dwValue = dwGetValue( pToken );
		// 크기를 결정한다.
		//if( pLex->dwValue >= (DWORD)0x01000000 )
		//	pLex->nSubType = 4;   (1, 2, 4바이트로 인식한다.)
		if( pLex->dwValue >= (DWORD)0x00010000 )
			pLex->nSubType = 4;
		else if( pLex->dwValue >= (DWORD)0x00000100 )
			pLex->nSubType = 2;
		else
			pLex->nSubType = 1;

		// pToken이 :으로 끝나면 세그먼트 어드레스이다.
		nI = strlen( pToken );
		if( pToken[nI-1] == ':' )
			pLex->nType = LT_ABSADDR;
		else
			pLex->nType = LT_NUMBER;
		return( 0 );
	}

	// 대문자로 전환한다.
	strcpy( szT, pToken );
	nUppercase( szT );	

	// OpCode일 경우 Alias처리 (JC -> JB )
	nOpCodeAlias( szT );

	// 예약심볼 인식한다.
	nI = nSearchTbl( rsvTbl, 0, MAX_RSVSYM, szT );
	if( nI != -1 )		// 심볼을 찾았다.  
	{
		RsvSymStt *pR;

		pR				= &rsvTbl[nI];
		pLex->nType		= pR->nType;
		pLex->nSubType	= pR->nSubType;
		return(0);
	}			  

	// OpCode를 인식한다.
	nI = nIsOpCodeStr( 1, TOTAL_ot, szT );		// -1 을 리턴하면 OpCode가 아니다.
	if( nI > 0 )
	{
		pLex->nType		= LT_OPCODE;
		pLex->nSubType	= nI;
		return(0);
	}

	// 세그먼트 프리픽스 인식.
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
			return( -1 );	// 아마도 Syntax 에러일 것이다. 
		}
		return( 0 );
	}

	// 사이즈 프리픽스를 인식.
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

	// String 인식 'ABCD'...
	if( szT[0] == '\'' )
	{
		int nK;

		nI = strlen( szT );
		if( nI == 1 )		// 에러 체킹.
		{
			strcpy( szMyAsmError, "Another ' is required." );
			return( -1 );	// '만 하나 달랑있으면 에러다.
		}
		else if( nI > 6 )
		{
			strcpy( szMyAsmError, "String is too long." );
			return( -1 );
		}
		if( szT[nI-1] != '\'' )
		{
			strcpy( szMyAsmError, "Another ' is required." );
			return( -1 );	// 끝에 '가 생략되었다.
		}

		pLex->nType    = LT_STRING;
		pLex->nSubType = nI-2;	// 실제 문자길이.
		for( nK = 1; nK < nI-1; nK++ )
		{
			pLex->dwValue =  (DWORD)( pLex->dwValue << 8 );
			pLex->dwValue += (DWORD)( (UCHAR)szT[nK] );
		}
		return( 0 );
	}

	// 단일문자 인식.
	if( strlen( szT ) == 1 )
	{
		pLex->nType		= LT_CHAR;
		pLex->nSubType	= (int)szT[0];
		return( 0 );
	}								  

	return( 0 );
}	

// 구문분석을 수행한다.
static int nLexer( LexStt *pLex, char *pS )
{
	int  nTotal, nR;
	char *pNext;
	char szToken[TOKEN_SIZE];

	nTotal = 0;
	pNext = pS;
	for( ; ; )
	{	// 토큰을 가져온다. 
		pNext = pGetToken( szToken, pNext );
		if( szToken[0] == 0 )
			break;

		// 토큰 타입과 값을 확인한다.
		nR = nChkTokenType( &pLex[nTotal], szToken );
		MYASM_PRINTF( "%-10s : (Type=%d,SubType=%d) (value = 0x%X)\n", szToken, pLex[nTotal].nType, pLex[nTotal].nSubType, pLex[nTotal].dwValue );
		if( nR == 0 )
			nTotal++;
		else
			return( -1 );	// 에러가 나면 그냥 바로 돌아간다.
	}	

	return( nTotal );
}

// 레지스터의 크기를 구한다.
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

// 수식을 DWORD값으로 변경한다.
static LexStt *pExpression( LexStt *pLex, DWORD *pValue, int *pErr )
{
	LexStt	*pN;
	DWORD	dwR;
	int		nI, nR;
	SEntStt s[2];

	pN	= pLex;
	*pValue = dwR = 0;
	*pErr   = 0;
	
	// 에러처리
	if( pN == NULL || ( pN->nType != LT_NUMBER && pN->nSubType != '(' ) )
	{
		*pErr = -1;
		return( pN );
	}

	//  스택의 끝 표시를 해 둔다.
	s[0].nType	 = SENT_END;
	s[0].dwValue = 0;
	nR = nPushSEnt( &s[0] );
	if( nR != 0 )
		goto ERROR_X;

	for( nI = 0; ; )
	{	// 숫자.
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
		{	// 괄호가 나왔다.
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
			{	// 괄호가 종료되었다.  계산에 들어간다.
				pN++;
				goto CALC;
			}	// 사칙연산자
			else if( pN[nI].nSubType == '+'	|| pN[nI].nSubType == '-' || 
			/**/pN[nI].nSubType == '*' || pN[nI].nSubType == '/' )
			{
				s[0].nType	 = SENT_OPERATOR;
				s[0].dwValue = (DWORD)pN[nI].nSubType;
				s[1].nType	 = SENT_NUMBER;
				// 연산자 다음에 숫자가 나오는 경우
				if( pN[nI+1].nType == LT_NUMBER )
				{
					s[1].dwValue = pN[nI+1].dwValue;
					pN += 2;
				}  // 또다른 괄호의 시작이다.
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

	// 스택의 끝 표시를 팝한다.
	nPopSEnt( &s[0] );

	*pValue = dwR;
	return( pN );

ERROR_X:
	*pErr = -1;
	MYASM_PRINTF( "Expression error.\n" );
	return( pN );
}

// [로 시작하는 메모리 오퍼랜드를 처리한다.
static LexStt *pMemoryOperand( OpStt *pOp, int nOperand, LexStt *pLex, int *pErr )
{
	LexStt		*pN;
	OperandStt	*pOpnd;

	pOpnd = &pOp->Oprnd[nOperand];
	pN    = pLex;
	*pErr = 0;
	// [로 시작하지 않으면 메모리 오퍼랜드가 아니다.
	if( pN->nType != LT_CHAR || pN->nSubType != '[' )
		goto ERROR_X;
	else
		pN++;

	if( pN->nType == LT_REG32 || pN->nType == LT_REG16 )
	{
		if( pN[1].nType != LT_CHAR )	// 레지스터 다음에 ] + * 중 하나는 나와야 한다.
			goto ERROR_X;
		else
		{
			if( pN[1].nSubType == ']' || pN[1].nSubType == '+')
				goto GET_BASE;		// 베이스만 있는 경우 또는 베이스에 인덱스나 변위가 붙는 경우
			else if( pN[1].nSubType == '*' )
				goto GET_INDEX;		// 바로 인덱스가 나온 경우.
			else
				goto ERROR_X;
		}
	}	  
	else if( pN->nType == LT_NUMBER )
		goto GET_DISP;				// 바로 변위가 나온 경우.

GET_BASE:	// 베이스 레지스터를 인식하였다.
	pOpnd->wRegBase = (UINT16)pN->nSubType;
	if( pN->nType == LT_REG32 )
		pOpnd->wWidth = 4;
	else
		pOpnd->wWidth = 2;
	pN++;				   
	if( pN->nType == LT_CHAR )
	{
		if( pN->nSubType == ']' )	
			goto END;		// 베이스만 달랑 나왔다.
		if( pN->nSubType == '+' )
		{
			if( pN[1].nType == LT_NUMBER )
			{
				pN++;		// 베이스와 변위 (인덱스는 생략)
				goto GET_DISP;
			}
			else if( pN[1].nType == LT_REG32 || pN[1].nType == LT_REG16 )
			{	// 인덱스 레지스터가 나왔다.
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

GET_INDEX:	// 인덱스 레지스터를 구한다. 
	pOpnd->wIndex = (UINT16)pN->nSubType;
	if( pN->nType == LT_REG32 )
		pOpnd->wWidth = 4;
	else
		pOpnd->wWidth = 2;
	pN++;		
	if( pN->nType == LT_CHAR )
	{
		if( pN->nSubType == ']' )	
			goto END;		// 베이스 + 인덱스의 경우
		if( pN->nSubType == '+' )
		{
			if( pN[1].nType == LT_NUMBER )
			{
				pN++;		// 베이스와 변위 (인덱스는 생략)
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

GET_SCALE:	// 스케일을 구한다.
	if( pN->nType != LT_NUMBER )
		goto ERROR_X;
	pOpnd->wScale = (UINT16)pN->dwValue;
	pN++;
	if( pN->nType == LT_CHAR )
	{
		if( pN->nSubType == ']' )	
			goto END;		// 베이스 + 인덱스의 경우
		if( pN->nSubType != '+' || pN[1].nType != LT_NUMBER )
			goto ERROR_X;
		pN++;		// 베이스와 변위 (인덱스는 생략)
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
		if( nErr != 0 )		// 수식 내에서 에러가 발생하였다.
			goto ERROR_X;

		// 사이즈가 아직 결정되지 않았으므로 사이즈를 결정한다.
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

	// 반드시 ]가 나와야 한다.
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

// 오퍼랜드가 어떤 타입의 무엇인지를 인식한다.
static LexStt *pSetOperand( OpStt *pOp, int nOperand, LexStt *pLex )
{
	LexStt		*pN;
	OperandStt	*pOpnd;
	int			nFar;

	pN		= pLex;
	pOpnd	= &pOp->Oprnd[nOperand];

	// 레지스터
	if( LT_REG8 <= pN->nType && pN->nType <= LT_STKREG )
	{
		pOpnd->wType    = (UINT16)pN->nType;
		pOpnd->wRegBase = (UINT16)pN->nSubType;
		pOpnd->wSize    = (UINT16)nGetRegisterSize( pN->nSubType );
		return( &pN[1] );
	}

	// 섹렉터 10:0000
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

	// 사이즈 프리픽스가 붙었는가 확인해 본다.
	if( pN->nType == LT_SIZEPRX )
	{
		if( pN[1].nType != LT_PTR )
			return( NULL );			// PTR이 빠졌다.
		// 명시적 크기가 지정되었다.
		pOpnd->wSize = (UINT16)pN->nSubType;
		pN += 2;
	}

	// FAR PTR
	if( pN->nType == LT_FAR )
	{
		if( pN[1].nType != LT_PTR )
			return( NULL );			// PTR이 빠졌다.
		// 앞에 FAR가 붙었다.
		pOpnd->wSize = (UINT16)pN->nSubType;
		nFar = 1;
		pN += 2;
	}
	else 
		nFar = 0;

	// 세그먼트 프리픽스가 붙어있는지 확인해 본다.
	if( pN->nType == LT_SEGPRX )
	{
		pOp->wSegPrx = (UINT16)pN->nSubType;
		pN++;		// 세그먼트 프리픽스가 붙으면 확실히 메모리 오퍼랜드.
	}

	// 메모리 오퍼랜드
	if( pN->nType == LT_CHAR && pN->nSubType == '[' )
	{
		int nErr;
		// pOpnd->wType = LT_MEMORY (안에서 세팅되어 나온다.)
		pN = pMemoryOperand( pOp, nOperand, pN, &nErr );
		if( nErr != 0 )		// 메모리 오퍼랜드 조합 내에서 에러가 발생했다.
			return( NULL );
		// FAR PTR이 붙은 경우 LT_FARMEMORY로 변경해 준다.
		if( nFar == 1 )
			pOp->Oprnd[nOperand].wType = LT_FARMEMORY;
	}// 숫자. ( IMMEDIATE DATA )
	else if( pN->nType == LT_NUMBER || ( pN->nType == LT_CHAR && pN->nSubType == '(' ) )	
	{
		int nErr;
		pN = pExpression( pN, &pOpnd->dwValue, &nErr );
		if( nErr != 0 )		// 수식 내에서 에러가 발생하였다.
			return( NULL );

		// 사이즈가 아직 결정되지 않았으므로 사이즈를 결정한다.
		if( pOpnd->wSize == 0 )
		{	// 오퍼랜드 사이즈는 1,2,4만 인식한다.
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

// Lexical Analysis에 의해 생성된 pLex를 대상으로 Parsing을 수행한다. 
static int nParsing( OpStt *pOp, LexStt *pLex )
{
	LexStt	*pNext;

	// 스택을 초기화해 준다. 
	nInitStk();

	memset( pOp, 0, sizeof( OpStt ) );

	// 항상 OpCode가 먼저 나와야 한다.
	if( pLex[0].nType == LT_OPCODE )
	{	// OpCode ot_???를 복사한다.
		pOp->wType = (UINT16)pLex[0].nSubType;
	}
	else
	{	// OpCode가 없으므로 바로 에러리턴한다.
		strcpy( szMyAsmError, "OPCODE not found." ); 
		return( -1 );
	}				

	pNext = &pLex[1];
	// Operand 3개까지 처리한다.
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

// 디버깅을 위해 오퍼랜드를 출력한다.
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

// 메모리 오퍼랜드를 인식할 수 있는 조합인지 판단한다. 
static int nIsMemory( OperandStt *pOprnd, char *pS )
{
	if( pS[0] == 'E' || pS[0] == 'M' )
		return( 0 );
	else if( pS[0] == 'O' && pOprnd->wRegBase == 0 && pOprnd->wIndex == 0 )
		return( 0 );			// O는 변위만 존재하는 경우.
	else
		return( -1 );
}

// 레지스터 오퍼랜드를 인식할 수 있는 조합인지 판단한다.
static int nIsReg( char *pS, UINT16 wReg, UINT16 wType )
{
	char ch1;

	ch1 = pS[1];

	//if( eAX <= ch1 && ch1 <= eSP )
	//	ch1 = ( ch1 - eAX );

	switch( pS[0] )
	{
	case '!' :	// 지정된 레지스터가 나와야만 한다.
		if( eAX <= ch1 && ch1 <= eSP )  // eAX일 경우 AX또는 EAX와 매치될 수 있다.
		{
			if( wType == LT_REG32 )
			{	// 32비트 레지스터일 경우 EAX, EBX...등과 비교해 본다.
				if( wReg == (ch1 - eAX) + rEAX )
					return( 0 );
				else
					return( -1 );
			}
			else if( wType == LT_REG16 )
			{	// 16비트 레지스터일 경우 AX, BX...등과 비교해 본다.
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

// 숫자와 메모리의 경우 오퍼랜드의 크기가 해당 코드 테이블 엔트리와 부합하는지 비교한다.
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

// 레지스터의 경우 오퍼랜드의 크기가 해당 코드 테이블 엔트리와 부합하는지 비교한다.
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

// 인식한 오퍼랜드가 현재 코드 테이블의 엔트리에 있는 오퍼랜드와 일치하는지 확인한다.
static int nCompareOperand( OperandStt *pOprnd, char *pS )
{
	int nR;

	if( pOprnd->wType == 0 )
	{
		if( pS[0] == 0 )
			return( 0 );		// 오퍼랜드가 둘다 존재하지 않는다.
		else if( pS[0] == '^' )	// ^은 무시한다. 
			return( 0 );
		else
			return( -1 );		// 있어야 할 오퍼랜드가 없다.
	}
	else if( pS[0] == 0 )
		return(-1 );			// 없어야할 오퍼랜드가 있다.

	nR = 0;
	switch( pOprnd->wType )		// 둘 다 0이 아닌 경우.
	{
	case LT_ABSADDR :			// 절대 어드레스 10:1100	<- 4바이트 또는 6바이트
		if( pS[0] != 'A' )
			return( -1 );
		break;

	case LT_NUMBER :
		if( pS[0] != 'I' && pS[0] != 'J' )
		{
			if( pS[0] == '%' )		// 암시적으로 직접 숫자를 쓴 경우. ROR AX,1
			{
				if( pOprnd->dwValue == (DWORD)pS[1] )
					return( 0 );	// 크기를 비교할 필요없이 그냥 돌아간다.
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
		if( pS[0] != '!' )	// !은 크기를 비교할 필요가 없다.
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

// Oprand 조합의 Weight를 구한다.
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

// OneByteTbl의 지정된 엔트리와 pOp의 Oprnd[]가 일치하는지 확인한다.
static int nCmpOperand1( OpStt *pOp, OpDataStt *pOne )
{
	int	result[3];
	int	nR;

	nR = -1;
	result[0] = nCompareOperand( &pOp->Oprnd[0], (char*)pOne->szOperand[0] );
	result[1] = nCompareOperand( &pOp->Oprnd[1], (char*)pOne->szOperand[1] );
	result[2] = nCompareOperand( &pOp->Oprnd[2], (char*)pOne->szOperand[2] );

	// 파러메터가 일치한다.
	if( result[0] == 0 && result[1] == 0 && result[2] == 0 )
	{	//가중치와 인덱스를 저장한다.
		nR = nGetOperandWeight( (char*)pOne->szOperand[0], (char*)pOne->szOperand[1], (char*)pOne->szOperand[2] );
		MYASM_PRINTF( "*" );
		// Code Entry를 출력한다.
		vDispCodeEntry( "1-OPCODE", pOne->wType, (char*)pOne->szOperand[0], (char*)pOne->szOperand[1], (char*)pOne->szOperand[2] );
	}
														
	// 가중치가 리턴된다. 
	return( nR );
}	

// TwoByteTbl의 지정된 엔트리와 pOp의 Oprnd[]가 일치하는지 확인한다.
static int nCmpOperand2( OpStt *pOp, OpData2Stt *pTwo )
{
	int	result[3];
	int	nR;

	nR= -1;
	result[0] = nCompareOperand( &pOp->Oprnd[0], (char*)pTwo->szOperand[0] );
	result[1] = nCompareOperand( &pOp->Oprnd[1], (char*)pTwo->szOperand[1] );
	result[2] = nCompareOperand( &pOp->Oprnd[2], (char*)pTwo->szOperand[2] );

	// 파러메터가 일치한다.
	if( result[0] == 0 && result[1] == 0 && result[2] == 0 )
	{	//가중치와 인덱스를 저장한다.
		nR = nGetOperandWeight( (char*)pTwo->szOperand[0], (char*)pTwo->szOperand[1], (char*)pTwo->szOperand[2] );
		MYASM_PRINTF( "*" );
		// Code Entry를 출력한다.
		vDispCodeEntry( "2-OPCODE", pTwo->wType, (char*)pTwo->szOperand[0], (char*)pTwo->szOperand[1], (char*)pTwo->szOperand[2] );
	}	


	// 가중치가 리턴된다. 
	return( nR );
}	

// pOData의 최대 가중치 값을 수정한다.  (가중치가 가장 높은 것으로 코드를 조합하게 된다. )
// nTbl은 1, 2값을 갖는데 OneByteTble인지 TwoByteTbl인지를 의미한다.
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
		if( nR >= 0 )  // 매치된 것을 저장해 둔다.
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
		if( nR >= 0 )  // 매치된 것을 저장해 둔다.
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
//	int	nOneByteIndex;	// ONEBYTE TABLE에 대한 일반명령 인덱스.
//	int	oneExtIndex[2];	// 확장 그룹에 대한 인덱스.
//	// TWOBYTE
//	int	nTwoByteIndex;	// TWOBYTE TABLE에 대한 일반명령 인데스.
//	int twoExtIndex[2];	// 확장 그룹에 대한 인덱스.
// Codetable에서 op의 오퍼랜드에 맞는 조합을 찾아 ODStt에 저장한다.
static int nSearchCodeTable( OpStt *pOp, ODStt *pOData )
{
	OpIndexStt	*pIndex;
	int			nR;

	memset( pOData, 0, sizeof( ODStt ) );

	pIndex = &opindex[ (int)pOp->wType ];
	// 한 바이트 OpCode에서 Operand가 매치되는 것이 있는지 찾는다.
	nR = nSearchSequence1( pOp, pOData, pIndex->nOneByteIndex  );
	nR = nSearchSequence1( pOp, pOData, pIndex->oneExtIndex[0] );
	nR = nSearchSequence1( pOp, pOData, pIndex->oneExtIndex[1] );

	// 두 바이트 OpCode에서 Operand가 매치되는 것이 있는지 찾는다.
	nR = nSearchSequence2( pOp, pOData, pIndex->nTwoByteIndex  );
	nR = nSearchSequence2( pOp, pOData, pIndex->twoExtIndex[0] );
	nR = nSearchSequence2( pOp, pOData, pIndex->twoExtIndex[1] );

	MYASM_PRINTF( "Total (%d/%d) Matched Code Table Entries.\n", pOData->nTotal1, pOData->nTotal2 );
	
	return( 0 );
}

// 디버깅을 위해 그냥 만들어 본것.
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

// 오퍼랜드의 조합에 에러가 있는지 확인해 본다.
static int nChkOperandError( OpStt *pOp )
{
	return( 0 );
}

// 지정되지 않은 오퍼랜드의 크기를 명시적으로 변경한다.
static int nInheritSize( OpStt *pOp )
{
	OperandStt *pO0, *pO1;

	// 오퍼랜드가 하나도 없는 경우.
	if( pOp->Oprnd[0].wType == 0 )
		return( 0 );

	// 오퍼랜드가 한 개 있는 경우.
	if( pOp->Oprnd[1].wType == 0 )
		return( 0 );

	// 오퍼랜드가 두 개 있는 경우. 
	if( pOp->Oprnd[2].wType == 0 )
	{
		pO0 = &pOp->Oprnd[0];
		pO1 = &pOp->Oprnd[1];

		if( pO0->wSize == 0 )
		{
			if( pO1->wSize == 0 )
				return( -1 );		// 오퍼랜드가 두 개다 Size가 0이다.
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

	// 오퍼랜드가 세 개 다 있는 경우.
	

	return( 0 );
}

// 레지스터 인코딩 값을 구한다.
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

// Rm 필드가 메모리로 사용된다.
static int nRmIsMem( OperandStt *pOperand, UCHAR *pModRegRm )
{
	UINT16	wT;
	int		nNoDisp, nSibling, nX;

	nNoDisp  = 0;			 
	nSibling = 0;

	// Clear Mod Field 00(Mod) 000(Reg) 000(Rm)
	pModRegRm[0] = (UCHAR)( pModRegRm[0] & (UCHAR)0x3F );
	// Displacement의 유무에 따라 Mod Field를 설정한다.
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
	// 16비트 Addressing Mode
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
			if( nNoDisp == 1 )		// BP만 Displacement 없이 쓰이면 에러
				return(0);
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x06 );
			break;
		case (UINT16)rBX :
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x07 );
			break;
		default :		// Base와 Index의 조합이 틀렸다.
			return( -1 );
			break;
		}
	}
	// 32비트 Addressing Mode
	else if( rEAX <= pOperand->wRegBase && pOperand->wRegBase <= rEBP )
	{	// 인덱스가 사용되지 않은 경우 (Sibling을 쓰지 않아도 된다.)
		if( pOperand->wIndex == 0 )
		{
			if( pOperand->wRegBase == rEBP && nNoDisp == 1 )
				return( -1 );		// 변위없이 EBP만 사용되었다.

			nX = nGerRegBitCode( pOperand->wRegBase );
			// Base를 설정한다. 
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)nX );

		}
		else// 인덱스가 사용되면 Sibling으로의 확장이 일어난다. 
		{	// Sibling = 00(Scale) 000(Index) 000(Base)
			nSibling = 1;
			pModRegRm[0] = (UCHAR)( pModRegRm[0] | (UCHAR)0x04 );  // 04 = Sibling Extension

			// Clear Scale Field
			pModRegRm[1] = (UCHAR)( pModRegRm[1] & (UCHAR)0x3F );
			//Scale 값을 설정한다. 
			if( pOperand->wScale == 0 )
				pModRegRm[1] = (UCHAR)( pModRegRm[1] | (UCHAR)0x00 );
			else if( pOperand->wScale == 2 )			// * 2
				pModRegRm[1] = (UCHAR)( pModRegRm[1] | (UCHAR)0x40 );
			else if( pOperand->wScale == 4 )	// * 4
				pModRegRm[1] = (UCHAR)( pModRegRm[1] | (UCHAR)0x80 );
			else if( pOperand->wScale == 8 )	// * 8
				pModRegRm[1] = (UCHAR)( pModRegRm[1] | (UCHAR)0xC0 );
			else
				return( -1 );		// 잘못된 Scale 값.

			// Clear Index Field
			pModRegRm[1] = (UCHAR)( pModRegRm[1] & (UCHAR)0xC7 );
			// Index 값을 설정한다.
			nX = nGerRegBitCode( pOperand->wIndex );
			nX = (int)( nX << 3 );
			// Index를 설정한다. 
			pModRegRm[1] = (UCHAR)( pModRegRm[1] | (UCHAR)nX );
			
			// Clear Base Field
			pModRegRm[1] = (UCHAR)( pModRegRm[1] & (UCHAR)0xF8 );
			// Base 값을 설정한다.
			nX = nGerRegBitCode( pOperand->wRegBase );
			// Index를 설정한다. 
			pModRegRm[1] = (UCHAR)( pModRegRm[1] | (UCHAR)nX );
		}	
	}
	else
		return( -1 );

	if( nSibling == 0 )		
		return( 1 );	// Sibling은 사용되지 않았고 ModRegRm만 사용되었다.
	else
		return( 2 );	// Sibling까지 사용되었다.
}

// Rm 필드가 레지스트터로 사용된다.
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

// ModRegRm의 Reg Field
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

// ModRegRm 바이트를 실제 조합한다.
// 리턴값 0-ModRegRm 바이트를 사용하지 않았음, 1-ModRegRm만 썼음, 2-Sibling까지 사용했음.
static int n_xx_MakeModRegRm( OperandStt *pOperand, char *pS, UCHAR *pModRegRm )
{
	int nR;

	nR = 0;			
	switch( pS[0] )
	{
	case 'E' :	//Rm 필드가 레지스터 또는 메모리로 쓰임.
		if( pOperand->wType == LT_MEMORY || pOperand->wType == LT_FARMEMORY )
			nR = nRmIsMem( pOperand, pModRegRm );
		else if( LT_REG8 <= pOperand->wType && pOperand->wType <= LT_REG32 )
			nR = nRmIsReg( pOperand, pModRegRm );
		break;

	case 'R' :	// Rm 필드가 항상 레지스터로 사용된다.
		if( LT_REG8 <= pOperand->wType && pOperand->wType <= LT_REG32 )
			nR = nRmIsReg( pOperand, pModRegRm );
		break;

	case 'M' :	// Rm 필드가 항상 메모리로 사용된다.
		if( pOperand->wType == LT_MEMORY )
			nR = nRmIsMem( pOperand, pModRegRm );
		break;

	case 'S' :
	case 'D' :
	case 'C' :
	case 'T' :
	case 'G' :	// Operand의 RegBase가 ModRegRm의 Reg 필드로 쓰인다. 
		if( LT_REG8 <= pOperand->wType && pOperand->wType <= LT_REG32 )
			nR = nRegField( pOperand, pModRegRm );
		break;

	}	
	
	return( nR );
}	

// ModregRm Byte를 조합한다.
static int nMakeModRegRm( OpStt *pOp, char *pOp0, char *pOp1, char *pOp2, UCHAR *pModRegRm )
{
	int nSize, nR;
		   
	nSize = 0;
	
	pModRegRm[0] = pModRegRm[1] = 0;

	// 첫 번째 오퍼랜드
	if( pOp->Oprnd[0].wType == 0 )
		return( nSize );
	nR = n_xx_MakeModRegRm( &pOp->Oprnd[0], pOp0, pModRegRm );
	if( nR == -1 )			return( -1 );
	else if( nSize < nR )	nSize = nR;

	// 두 번째 오퍼랜드
	if( pOp->Oprnd[1].wType == 0 )
		return( nSize );
	nR = n_xx_MakeModRegRm( &pOp->Oprnd[1], pOp1, pModRegRm );
	if( nR == -1 )			return( -1 );
	else if( nSize < nR )	nSize = nR;
	
	// 세 번째 오퍼랜드
	if( pOp->Oprnd[2].wType == 0 )
		return( nSize );
	nR = n_xx_MakeModRegRm( &pOp->Oprnd[2], pOp2, pModRegRm );
	if( nR == -1 )			return( -1 );
	else if( nSize < nR )	nSize = nR;

	return( nSize );
}

// Immediate Data에 해당되는 값을 pCode에 복사한다.
// Immediate Data의 크기를 리턴한다.
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

// Displacement에 해당되는 값을 pCode에 복사한다.
static int nDisplacement( UCHAR *pCode, char chSize, OperandStt *pOperand )
{
	int nSize = 0;

	// Displacement가 사용되지 않았다.
	if( pOperand->wValueSize == 0 )
		return( 0 );
	
	// Byte 변위 (2002-03-27 추가)
	if( pOperand->wValueSize == 1 )
	{
		memcpy( pCode, &pOperand->dwValue, 1 );
		return( 1 );
	}

	if( pOperand->wRegBase == 0 )
	{	// 베이스 레지스터를 쓰지 않았으면 wAsmDefault에 따라 변위의 크기를 결정한다. 
		if( wAsmDefault == 1 )
			nSize = 4;
		else
			nSize = 2;
	}	// 16Bit레지스터를 사용했으면 변위도 16비트로 간주. 
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

// Reg Field가 확장코드로 사용된다.
// 일치하는 것을 찾았으면 Reg Field에 들어갈 값을 리턴하고 실패하면 -1리턴.
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

// JUMP 명령의 오퍼랜드를 생성한다.
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
	else if( cSizeChar == 'v' )	// 2바이트 또는 4 바이트  (일단 4 바이트로 게산한다.)
	{
		memcpy( pCodeBuff, &pOperand->dwValue, 4 );
		return( 4 );
	}				

	return( 0 );
}

// 실제로 Assemble을 수행한다.
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
	{	// 0x0F로 확장되는 2 Byte OpCode
		pCode[ nSize++ ] = (UCHAR)0x0F;
		pCode[ nSize++ ] = (UCHAR)basicTwoTbl[nIndex]->wNo;
		pOp1 = (char*)basicTwoTbl[nIndex]->szOperand[0];
		pOp2 = (char*)basicTwoTbl[nIndex]->szOperand[1];
		pOp3 = (char*)basicTwoTbl[nIndex]->szOperand[2];
		wOpCodeType = basicTwoTbl[nIndex]->wType;
	}						  

	// GRP7일 경우 pOp0, 1, 2를 별도로세팅해 주어야 한다. (테이블에는 나와있지 않다.)
	if( wOpCodeType == ot_GRP7 )
	{
		if( pOp->wType == ot_SMSW || pOp->wType == ot_LMSW )
			pOp1 = "Ew"; 
		else if( pOp->wType != ot_INVLPG )
			pOp1 = "Ms" ;
	}
	else if( wOpCodeType == ot_GRP5 && pOp->Oprnd[0].wType == LT_FARMEMORY )
	{	// GRP5의 CALL, JMP에 FAR가 붙어서 Ev가 아니고 Ep가 된다.
		if( pOp->wType == ot_JMP || pOp->wType == ot_CALL )
			pOp1 = "Ep";
	}

	// ModRegRm, Sibling Byte를 조합한다.
	nR = nMakeModRegRm( pOp, pOp1, pOp2, pOp3, ModRegRm );
	if( nR < 0 ) // 에러발생
		return( -1 );
	// 확장 그룹일 경우 Reg Field에 Code 값을 세팅해 주어야 한다.
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
	//				FPU 확장 그룹은 아직 처리가 안된다.				//
	//////////////////////////////////////////////////////////////////

	// ModRegRm, Sibling을 Code로 복사한다.
	if( nR == 1 )
		pCode[nSize++] = ModRegRm[0];
	else if( nR == 2 )
	{
		pCode[nSize++] = ModRegRm[0];
		pCode[nSize++] = ModRegRm[1];
	}					
	
	// Displacement  (변위는 1, 2, 4바이트)
	if( pOp->Oprnd[0].wType == LT_MEMORY )
		nSize += nDisplacement( &pCode[nSize], pOp1[1], &pOp->Oprnd[0] );
	else if( pOp->Oprnd[1].wType == LT_MEMORY )
		nSize += nDisplacement( &pCode[nSize], pOp2[1], &pOp->Oprnd[1] );
	else if( pOp->Oprnd[2].wType == LT_MEMORY )
		nSize += nDisplacement( &pCode[nSize], pOp3[1], &pOp->Oprnd[2] );

	// Iv일 경우 짝을 이루는 오퍼랜드의 크기에 동조시킨다.
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
	} // JUMP 명령의 경우 Jb, jv
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

	// Max Weight를 출력해 본다.
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

// ^에 의한 Warp Up Instruction (return 양수)
// 아니면 0
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

// JUMP addr에서 addr 값을 EIP를 토대로 조정한다.
static int nRenderJumpAddr( OpStt *pOp, DWORD dwEIP )
{
	int nSize;

	nSize = 0;

	// 앞쪽으로 점프
	if( pOp->Oprnd[0].dwValue > dwEIP )
	{	// 한 바이트로는 처리가 안된다.  (4바이트로 가야 한다.)
		if( pOp->Oprnd[0].dwValue - dwEIP > 0x7F )  // 엄밀히 말해서 7F는 아니다.
		{	// 명령어 길이가 5바이트가 된다.
			pOp->Oprnd[0].wValueSize = 4;
			pOp->Oprnd[0].wSize = 4;
			pOp->Oprnd[0].dwValue = pOp->Oprnd[0].dwValue - dwEIP - 5;
		}
		else
		{	// 명령어 길이가 2 바이트가 된다.
			pOp->Oprnd[0].wValueSize = 1;
			pOp->Oprnd[0].wSize = 1;
			pOp->Oprnd[0].dwValue = pOp->Oprnd[0].dwValue - dwEIP - 2;
		}		
	}
	else  // 뒤쪽으로 점프
	{	// 한 바이트로는 처리가 안된다.  (4바이트로 가야 한다.)
		if( dwEIP - pOp->Oprnd[0].dwValue > 0x7F )  // 엄밀히 말해서 7F는 아니다.
		{	// 명령어 길이가 5바이트가 된다.
			pOp->Oprnd[0].wValueSize = 4;
			pOp->Oprnd[0].wSize = 4;
			pOp->Oprnd[0].dwValue = pOp->Oprnd[0].dwValue - dwEIP - 5;
		}
		else
		{	// 명령어 길이가 2 바이트가 된다.
			pOp->Oprnd[0].wValueSize = 1;
			pOp->Oprnd[0].wSize = 1;
			pOp->Oprnd[0].dwValue = pOp->Oprnd[0].dwValue - dwEIP - 2;
		}		
	}

	return( 0 );
}

static int nRenderCallAddr( OpStt *pOp, DWORD dwEIP )
{
	// 앞뒤 무조건 4바이트로 설정한다.
	pOp->Oprnd[0].wValueSize = 4;
	pOp->Oprnd[0].wSize = 4;
	pOp->Oprnd[0].dwValue = pOp->Oprnd[0].dwValue - dwEIP - 5;

	return( 0 );
}

// 주어진 pStr을 어셈블하여 pBinCode에 바이너리 코드를 저장하고 
// 바이너리 코드의 길이를 리턴한다. -1을 리턴하면 에러.	
// ( 0은 에러를 의미하지는 않는다. )
// JMP xxxx의 경우 Operand가 Jb, Jv인데 이 것은 EIP가 있어야 한다.
int nMyAsm( char *pStr, unsigned char *pBinCode, DWORD dwEIP )
{
	int		nR;
	OpStt	op;
	ODStt	odata;

	nR = 0;

	// Lexical Analysis
	memset( lex, 0, sizeof( lex ) );
	nTotalLex = nLexer( lex, pStr );
	if( nTotalLex < 0 )		// 에러가 나면 바로 돌아간다.
		return( -1 );

	// Parsing
	nR = nParsing( &op, lex );
	if( nR != 0 )
		return( -1 );		// 에러가 나면 바로 돌아간다.

	{// 오퍼랜드 분석한 것을 출력해 본다.
		MYASM_PRINTF( "OpCode( 0x%2X, %d)   ", op.wType, op.wType );
		if( op.Oprnd[0].wType != 0 )
			vDispOpnd( &op.Oprnd[0] );
		if( op.Oprnd[1].wType != 0 )
			vDispOpnd( &op.Oprnd[1] );
		if( op.Oprnd[2].wType != 0 )
			vDispOpnd( &op.Oprnd[2] );
		MYASM_PRINTF( "\n" );
	}

	// ^에 해당되는 명령
	nR = nWarpUpCode( &op, pBinCode );
	if( nR > 0 )
		goto MYASM_OK;
	
	// 오퍼랜드에 에러가 있는지 확인해 보아야 한다.
	nR = nChkOperandError( &op );
	if( nR == -1 )
		return( -1 );

	// 지정되지 않은 오퍼랜드의 크기 wSize를 명시적으로 조정해 준다.
	nR = nInheritSize( &op );

	// JUMP 명령일 경우 Operand EIP를 토대로 값을 조작한다.
	if( op.wType == ot_JMP || op.wType == ot_LOOP )
		nRenderJumpAddr( &op, dwEIP );
	else if( op.wType == ot_CALL )
	{  // CALL 명령이면 일단 상대 점프로 가정한다.
		nRenderCallAddr( &op, dwEIP );
	}
	
	// Searching CodeTable
	nR = nSearchCodeTable( &op, &odata );
	if( nR == -1 )
	{
		MYASM_PRINTF( "Code table search failed!\n" );
		return( -1 );		// 에러가 나면 바로 돌아간다.
	}
	else 
	{
	}
	
	{////////////////////////////////////////////////////////////
		int nTbl, nIndex;

		// 최적의 조합 엔트리를 구한다. 
		nR = nGetMaxWeight( &odata, &nTbl, &nIndex );
		if( nR > 0 )
		{	// 코드를 조합한다. (Assemble)
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

MYASM_OK:	// nR에 명령 길이가 들어가 있다.
	vDispAssembledCode( pBinCode, nR );

	return( nR );
}



