#include "vfs.h"

/*	2002-05-27
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
*/
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

unsigned long atoul( char *pS )
{
	int		nType;
	DWORD	dwX;

	dwX = 0;

	nType = nIsNumber( pS );
	if( nType == 10 )
		dwX = dwDecValue( pS );
	else if( nType == 16 )
		dwX = dwHexValue( pS );

	return( dwX );
}

