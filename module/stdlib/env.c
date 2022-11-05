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

// 환경변수를 초기화 한다.
int init_env_buff( UEnvStt *pEnv )
{
    // 환경 변수 버퍼를 할당한다.
    pEnv->nBuffSize = 1024;
    pEnv->pBuff = (char*)malloc( pEnv->nBuffSize );
    if( pEnv->pBuff == NULL )
        pEnv->nBuffSize = 0;

    pEnv->nDataSize = 0;	// 사용중인 데이터.
	
	// 환경 변수의 끝은 '0'이 2개 존재한다.
    pEnv->pBuff[0] = pEnv->pBuff[1] = 0;

    return( pEnv->nBuffSize );
}

// 환경변수 버퍼를 해제한다.
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

// 환경 변수 스트링을 얻어온다.
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
            return( -1 );       // 찾을 수 없다.

        nT = strlen( pName );
        if( memcmp( pS, pName, nT ) == 0 && pS[nT] == '=' )
        {   // value string을 복사한다.
            if( pStr != NULL )
                strcpy( pStr, &pS[nT+1] );
            return( nI );        // 찾았다.
        }

        nI += strlen( pS )+1;
    }   

    return( -1 );
}

// 환경 변수 스트링을 설정한다.
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
    {   // 기존 스트링을 지운다.
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
    
    // 가장 뒤에 스트링을 추가한다.
    sprintf( szT, "%s=%s", pName, pStr );
    strcpy( pD, szT );
    // 새로운 스트링의 길이를 더한다.
    pEnv->nDataSize += strlen( szT )+1; 
    // 기존 스트링의 길이를 뺀다.
    pEnv->nDataSize -= nOldSize+1; 
    
    // 가장 끝에 0을 추가한다.
    pEnv->pBuff[ pEnv->nDataSize ] = 0;

    return( 0 );
}

// 환경변수 스트링 전체를 출력한다.
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