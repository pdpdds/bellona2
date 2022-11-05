///////////////////////////////////////////////////////////////////////////////////
#ifndef WIN32

#include "lib.h"

static UEnvStt *get_proc_env()
{
	UAreaStt *pU;

    pU = get_uarea();
	if( pU == NULL || pU->env.pBuff == NULL )
		return( NULL );

	return( &pU->env );
}

#endif
///////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32 

#include <wtypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <env.h>

static UEnvStt uenv;    
UEnvStt *get_proc_env() 
{                       
    return( &uenv );    
}                       

#endif 
///////////////////////////////////////////////////////////////////////////////////

// ȯ�溯���� �ʱ�ȭ �Ѵ�.
int init_env_buff( UEnvStt *pEnv )
{
    // ȯ�� ���� ���۸� �Ҵ��Ѵ�.
    pEnv->nBuffSize = 1024;
    pEnv->pBuff = (char*)malloc( pEnv->nBuffSize );
    if( pEnv->pBuff == NULL )
        pEnv->nBuffSize = 0;

    pEnv->nDataSize = 0;	// ������� ������.
	
	// ȯ�� ������ ���� '0'�� 2�� �����Ѵ�.
    pEnv->pBuff[0] = pEnv->pBuff[1] = 0;

    return( pEnv->nBuffSize );
}

// ȯ�溯�� ���۸� �����Ѵ�.
int close_env_buff( UEnvStt *pEnv )
{
    if( pEnv->pBuff != NULL )
    {
        free( pEnv->pBuff );
        pEnv->pBuff = NULL;
    }
    pEnv->nBuffSize = 0;
    pEnv->nDataSize = 0;

    return( 0 );
}

// ȯ�� ���� ��Ʈ���� ���´�.
int get_env_str( char *pName, char *pStr )
{
    char        *pS;
    UEnvStt     *pEnv;
    int         nI, nT;

    if( pStr != NULL )
        pStr[0] = 0;
    
    pEnv = get_proc_env();
    if( pEnv == NULL )
        return( -1 );

    for( nI = 0; nI < pEnv->nDataSize; )
    {
        pS = &pEnv->pBuff[nI];
        if( pS[0] == 0 )
            return( -1 );       // ã�� �� ����.

        nT = strlen( pName );
        if( memcmp( pS, pName, nT ) == 0 && pS[nT] == '=' )
        {   // value string�� �����Ѵ�.
            if( pStr != NULL )
                strcpy( pStr, &pS[nT+1] );
            return( nI );        // ã�Ҵ�.
        }

        nI += strlen( pS )+1;
    }   

    return( -1 );
}

// ȯ�� ���� ��Ʈ���� �����Ѵ�.
int set_env_str( char *pName, char *pStr )
{
    UEnvStt     *pEnv;
    int         nOffset, nD, nS, nOldSize;
    char         *pBuff, szT[260], *pD, *pS, *pNext;

    pEnv = get_proc_env();
    if( pEnv == NULL )
        return( -1 );

    pBuff = pEnv->pBuff;
    nOldSize = 0;
    nOffset = get_env_str( pName, NULL );
    if( nOffset >= 0 )
    {   // ���� ��Ʈ���� �����.
        pD = &pBuff[nOffset];
        nOldSize = nD = strlen( pD );
        pS = &pD[ nD + 1 ];
        for( ; pS[0] != 0; )
        {
            nS = strlen( pS );
            pNext = &pS[ nS+1 ];
            strcpy( pD, pS );
            
            nD = strlen( pD );
            pD = &pD[ nD + 1];
            pS = pNext;
        }
        pD[0] = 0;
    }
    else
    {
        nOldSize =  -1;     
        pD = &pBuff[pEnv->nDataSize];
    }
    
    // ���� �ڿ� ��Ʈ���� �߰��Ѵ�.
    sprintf( szT, "%s=%s", pName, pStr );
    strcpy( pD, szT );
    // ���ο� ��Ʈ���� ���̸� ���Ѵ�.
    pEnv->nDataSize += strlen( szT )+1; 
    // ���� ��Ʈ���� ���̸� ����.
    pEnv->nDataSize -= nOldSize+1; 
    
    // ���� ���� 0�� �߰��Ѵ�.
    pEnv->pBuff[ pEnv->nDataSize ] = 0;

    return( 0 );
}

// ȯ�溯�� ��Ʈ�� ��ü�� ����Ѵ�.
void disp_env_str()
{
    int     nI;
    char    *pS;
    UEnvStt *pEnv;

    pEnv = get_proc_env();
    if( pEnv == NULL )
        return;

    for( nI = 0; nI < pEnv->nDataSize; )
    {
        pS = &pEnv->pBuff[nI];
        if( pS[0] == 0 )
            break;

        printf( "%s\n", pS );
        
        nI += strlen( pS ) +1;
    }
}