#include "vfs.h"

extern int getchar();

// read from kbd
int std_in_read( VNodeStt *pVNode, DWORD dwOffs, char *pBuff, int nSize )
{
	int nTotal;

	if( pBuff == NULL || nSize <= 0 )
		return( -1 );

	for( nTotal = 0; nTotal < nSize; )
	{
		pBuff[nTotal] = (char)getchar();
		nTotal++;
	}	  

	return( nTotal );
}

int std_out_write( VNodeStt *pVNode, DWORD dwOffs, char *pBuff, int nSize )
{
	int		nTotal, nI;
	char	szT[128];

	if( nSize <= 0 || pBuff == 0 )
		return( 0 );
	
	for( nTotal = nI = 0; nTotal < nSize; )
	{
		nI = nSize - nTotal;
		if( nI <= 0 )
			return( -1 );

		if( nI > sizeof( szT ) -1 )
			nI = sizeof( szT )-1;

		memcpy( szT, &pBuff[nTotal], nI );
		szT[nI] = 0;

		// write
		kdbg_printf( "%s", szT );

		nTotal += nI;
	}				 

	return( nTotal );
}				

int std_err_write( VNodeStt *pVNode, DWORD dwOffs, char *pBuff, int nSize )
{
	int nR;

	nR = std_out_write( pVNode, dwOffs, pBuff, nSize );

	return( nR );
}				

