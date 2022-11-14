#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>

#include "calcstr.h"
#include <funckey.h>

// get string
char* gets(char* pS)
{
	int		nI, nX, nY;
	char	szX[2];

	szX[1] = pS[0] = 0;

	for (;; )
	{
		szX[0] = (char)getch();
		if (szX[0] < 0)				// 2004-03-25
			continue;					// 
		if (szX[0] == (char)BK_ENTER)
			break;
		else if (szX[0] == (char)BK_BACKSPACE)
		{
			nI = strlen(pS);
			if (nI > 0)
			{
				pS[nI - 1] = 0;
				get_xy(&nX, &nY);
				set_xy(nX - 1, nY);
				printf(" ");
				get_xy(&nX, &nY);
				set_xy(nX - 1, nY);
			}
		}
		else if (szX[0] == (char)BK_RIGHT || szX[0] == (char)BK_LEFT ||
			szX[0] == (char)BK_HOME || szX[0] == (char)BK_END)
			continue;

		printf(szX);

		strcat(pS, szX);
	}

	return(pS);
}

void main( int argc, char *argv[] )
{
	char			szT[26], *pS;
	unsigned long	dwX;

	printf( "Calc String\n" );

	for( ;; )
	{
		printf( ">" );
		
		pS = gets( szT );
		if( pS == NULL )
			break;

		if( strcmpi( szT, "q" ) == 0 )
			break;

		if( nCalcStr( szT, &dwX ) < 0  )
			printf( "error.\n" );
		else
			printf( " -> %d, 0x%08X\n", dwX, dwX );
	}

	printf( "\nok.\n" );
}