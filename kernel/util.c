/*
	bellona2.bin과 stdlib.mod 에서 사용.
*/

#include "bellona2.h"

// 입력된 16진수 스트링을 DWORD 값으로 변경한다.
DWORD dwHexValue( char *pS )
{
	DWORD dwR = 0;
	int   nI;

	uppercase( pS );

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( pS[nI] >= 'A' && pS[nI] <= 'F' )
		{
			dwR = (DWORD)(dwR << 4);
			dwR += (DWORD)( ( pS[nI] - 'A' ) + 10 );
		}
		else if( pS[nI] >= 'a' && pS[nI] <= 'f' )
		{
			dwR = (DWORD)(dwR << 4);
			dwR += (DWORD)( ( pS[nI] - 'a' ) + 10 );
		}
		else if( pS[nI] >= '0' && pS[nI] <= '9' )
		{
			dwR = (DWORD)(dwR << 4);
			dwR += (DWORD)( pS[nI] - '0' );
		}
	}
	return( dwR );
}
			
// 입력된 10진수 스트링을 DWORD 값으로 변경한다.
DWORD dwDecValue( char *pS )
{
	DWORD dwR = 0;
	int   nI;

	uppercase( pS );

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( pS[nI] >= '0' && pS[nI] <= '9' )
		{
			dwR = (DWORD)(dwR * 10);
			dwR += (DWORD)( pS[nI] - '0' );
		}
	}
	return( dwR );
}

void uppercase( char *pS )
{
	int nI;

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( pS[nI] >= 'a' && pS[nI] <= 'z' )
			pS[nI] = pS[nI] - ( 'a' - 'A' );
	}
}

void lowercase( char *pS )
{
	int nI;

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( pS[nI] >= 'A' && pS[nI] <= 'Z' )
			pS[nI] = pS[nI] + ( 'a' - 'A' );
	}
}

int  strcmp( char *pA, char *pB )
{
	int nI;

	for( nI = 0; ; nI++ )
	{
		if( pA[nI] == pB[nI] )
		{
			if( pA[nI] == 0 )
				return( 0 );
		}
		else if( pA[nI] > pB[nI] )
			return( 1 );
		else
			return( -1 );
	}
	return( 0 );
}

int  strcmpi( char *pA, char *pB )
{
	char szA[512], szB[512];

	strcpy( szA, pA );
	strcpy( szB, pB );
	lowercase( szA );
	lowercase( szB );

	return( strcmp( szA, szB ) );
}

void* memcpy( void* pD, void *pS, long lSize )
{
	long lX;
	UCHAR *pDest = (UCHAR*)pD;
	UCHAR *pSrc  = (UCHAR*)pS;

	for( lX = 0; lX < lSize; lX++ )
		pDest[lX] = pSrc[lX];

	return( pD );
}

int memcmp( void* pD, void *pS, int lSize )
{
	long lX;
	UCHAR *pDest = (UCHAR*)pD;
	UCHAR *pSrc  = (UCHAR*)pS;

	for( lX = 0; lX < lSize; lX++ )
	{
		if( pDest[lX] > pSrc[lX] )
			return( 1 );
		else if( pDest[lX] < pSrc[lX] )
			return( -1 );
	}

	return( 0 );
}

/*
void get_align_count( DWORD dwAddr, DWORD dwSize, int nAlignSize, 
							int *pBody, int *pTail )
{
	pBody[0] = dwSize / nAlignSize;
	pTail[0] = dwSize - ( pBody[0] * nAlignSize );
}
*/

void *memset( void *pD, UCHAR byTe, long lSize )
{
	long	lX;
	DWORD	dwT;
	UCHAR	*pDest;
	int		nHead, nBody, nTail;

	//	간단한 C 버전의 memset 
	if( lSize < 32 )
	{
		pDest = (UCHAR*)pD;
		for( lX = 0; lX < lSize; lX++ )
			pDest[lX] = byTe;
		return( pD );
	}

	// 복잡한 ASM 버전의 memset
	dwT = (DWORD)( (DWORD)((DWORD)pD + 3 ) >> 2 );
	dwT = (DWORD)( dwT << 2 );
	nHead = (int)( dwT - (DWORD)pD );
	if( nHead > lSize )
		nHead = lSize;
	nBody = (int)( (lSize - nHead) / 4 );
	nTail = (int)( lSize - nHead - (nBody*4) );
	
	dwT = (DWORD)byTe;
	_asm {
			PUSH EAX
			PUSH ECX
			PUSH EDI
			PUSHFD

			MOV EAX, dwT
			MOV AH,  AL
			MOV CX,  AX
			SHL EAX, 16
			MOV AX,  CX

			MOV EDI, pD
			CLD

			MOV ECX, nHead
			CMP ECX, 0 
			JE  SET_BODY
				REP STOSB

SET_BODY:	MOV ECX, nBody
			CMP ECX, 0 
			JE  SET_TAIL
			    REP STOSD

SET_TAIL:	MOV ECX, nTail
			CMP ECX, 0 
			JE  BACK
				REP STOSB

BACK:		POPFD
			POP EDI
			POP ECX
			POP EAX	
	}	

	return( pD );
}

// 문자열 길이 구하기
int  strlen( void* pS )
{
	int nI;
	UCHAR *pStr = (UCHAR*)pS;

	for( nI = 0; pStr[nI] != 0; nI++ )
		;

	return( nI );
}

// 문자열 더하기
char *strcat( char *pS, char *pT )
{
	int nI, nJ;

	nI = strlen( pS );
	for( nJ = 0; ; nJ++ )
	{
		pS[nI+nJ] = pT[nJ];
		if( pT[nJ] == 0 )
			break;
	}
	return( pS );
}

// 문자열 복사
char *strcpy( char *pS, char *pT )
{
	int nI;

	if( pS == NULL || pT == NULL )
		return( pS );

	for( nI = 0; ; nI++ )
	{
		pS[nI] = pT[nI];
		if( pS[nI] == 0 )
			break;
	}
	return( pS );
}

int is_digit( char *pS )
{
	if( '0' <= pS[0]  && pS[0] <= '9' )
		return( 1 );
	else 
		return( 0 );
}		   

int is_char( char ch )
{
	if( 'a' <= ch  && ch <= 'z' )
		return( 1 );
	if( 'A' <= ch  && ch <= 'Z' )
		return( 1 );
	return( 0 );
}		   

// decimal을 스트링으로 변경한다.
static int nDecStr( char *pS, long lX )
{
	long	lY;
	int		nI, nJ;
	char	szT[32];

	pS[0] = 0;

	if( lX < 0 )	// 일단 양수로 전환한다.
		lX *= -1;

	if( lX == 0 )
	{
		pS[0] = '0';
		pS[1] = 0;
		return( 1 );
	}	

	for( nI = 0; lX > 0; )
	{
		lY = ( lX % 10 );
		lX = ( lX / 10 );

		pS[nI] = (char)( lX + '0' );
		pS[nI++] = 0;
	}

	// 스트링을 뒤집어야 한다.
	strcpy( szT, pS );
	for( nJ = 0; nI > 0; nI--, nJ++ )
	{
		pS[nJ] = szT[nI-1];
		pS[nJ+1] = 0;
	}				 

	return( nJ );
}

int vsprintf( char *buffer, char *format, va_list argptr )
{
	int nR;
	nR = _ffmt( buffer, format, (long*)argptr );
	return( nR );
}

int sprintf( char *pS, char *pFmt, ... )
{
	va_list va;
	int		nI;

	va_start( va, pFmt );
	nI = vsprintf( pS, pFmt, va );
	va_end( va ); 

	return( nI );
}

// Path에서 마지막 파일명만 얻어낸다.
char *pGetPureFileName( char *pS )
{
	int nI;

	for( nI = strlen( pS )-1; nI > 0; nI-- )
	{
		if( pS[nI] == '/' || pS[nI] == '\\' )
			return( &pS[nI+1] );
	}

	return( pS );
}

DWORD segoffset_to_offset32( DWORD dwSegOffs )
{
	UINT16	*pW;
	DWORD	dwOffset32;

	pW = (UINT16*)&dwSegOffs;

	dwOffset32 = (DWORD)( ( (DWORD)pW[1] << 4) + (DWORD)pW[0] );
	
	return( dwOffset32 );
}

// 패스에서 파일명의 위치를 찾아 리턴한다.
char *get_pure_filename( char *pS )
{
	int nI;

	if( pS == NULL )
		return( NULL );

	for( nI = strlen( pS )-1; nI > 0; nI-- )
	{
		if( pS[nI] == '/' || pS[nI] == '\\' )
			return( &pS[nI+1] );
	}
	return( &pS[nI] );
}

void i64_shr( __int64 *pLDW, int nC )
{
	int		nI;
	DWORD 	x[2];

	memcpy( x, pLDW, 8 );


	for( nI = 0; nI < nC; nI++ )
	{
		x[0] = (DWORD)( x[0] >> 1 );
		if( x[1] & 1 )
			x[0] |= (DWORD)0x80000000;

		x[1] = (DWORD)(x[1] >> 1);

	}

	memcpy( pLDW, x, 8 );	
}

