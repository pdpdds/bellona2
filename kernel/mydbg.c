#ifndef COFFDBG
	#include "bellona2.h"
#endif
#ifdef COFFDBG
	#define RELOC_BASE	0x400000
	#include <windows.h>
	#include <stdlib.h>
	#include <stdio.h>
#endif
#include "MyDbg.h"

static MDbgStt	*pMD = NULL;
int nGetMyDbgInfo( UCHAR *pB )
{
	pMD = (MDbgStt*)pB;

	// 디버그 정보가 가공되어 있는지 확인해 본다.
	if( pMD->dwMagic != (DWORD)MDB_MAGIC )
		return( -1 );		// 디버그 정보가 가공되어 있지 않다.

	pMD->pMDFile			= (MDFileEntStt*)	&pB[ pMD->nMDFileOffs			];                          
	pMD->pMDFunc			= (MDFuncEntStt*)	&pB[ pMD->nMDFuncOffs			];                          
	pMD->pMDAFunc			= (MDAsmFuncEntStt*)&pB[ pMD->nMDAsmFuncOffs		];                      
	pMD->pMDLine			= (MDLineEntStt*)	&pB[ pMD->nMDLineOffs			];                          
	pMD->pFileNameITbl		= (DWORD*)			&pB[ pMD->nFileNameITblOffs		];              
	pMD->pFuncNameITbl		= (DWORD*)			&pB[ pMD->nFuncNameITblOffs		];              
	pMD->pFuncAddrITbl		= (DWORD*)			&pB[ pMD->nFuncAddrITblOffs		];              
	pMD->pAsmFuncNameITbl	= (DWORD*)			&pB[ pMD->nAsmFuncNameITblOffs	];              
	pMD->pAsmFuncAddrITbl	= (DWORD*)			&pB[ pMD->nAsmFuncAddrITblOffs	];              
	pMD->pStrTbl			= (UCHAR*)			&pB[ pMD->nStrTblOffs			];                          

	return( 0 );
}

MDbgStt *pGetMD()
{						  
	return( pMD );
}


static int nGetFuncIndexByRVA( DWORD dwRVA )
{
	int				nX, nMid, nY, nPre;
	MDFuncEntStt	*pMDFunc;
	DWORD			*pFuncAddrITbl;	

	if( pMD == NULL )
		return( -1 );

	nX				= 0; 
	nY				= pMD->nTotalFuncEnt-1;
	pMDFunc			= pMD->pMDFunc;
	pFuncAddrITbl	= pMD->pFuncAddrITbl;


	nPre = 0;
	for( ;; )
	{
		nMid = ( nX + nY ) / 2;

		if( nPre == nMid )
			break;

		if( pMDFunc[ pFuncAddrITbl[nMid] ].dwRVA == dwRVA )
			return( nMid );
		else if( pMDFunc[ pFuncAddrITbl[nMid] ].dwRVA < dwRVA )
			nX = nMid;
		else
			nY = nMid;
		nPre = nMid;
	}

	if( nMid == pMD->nTotalFuncEnt -2 )
	{
		if( pMDFunc[ pFuncAddrITbl[nMid+1] ].dwRVA == dwRVA )
			return( nMid+1 );
	}

	return( -1 );
}

static int nGetAsmFuncIndexByRVA( DWORD dwRVA )
{
	int				nX, nMid, nY, nPre;
	MDAsmFuncEntStt	*pMDAFunc;
	DWORD			*pAsmFuncAddrITbl;	

	if( pMD == NULL )
		return( -1 );

	nX					= 0; 
	nY					= pMD->nTotalAsmFuncEnt-1;
	pMDAFunc			= pMD->pMDAFunc;
	pAsmFuncAddrITbl	= pMD->pAsmFuncAddrITbl;

	nPre = 0;
	for( ;; )
	{
		nMid = ( nX + nY ) / 2;

		if( nPre == nMid )
			break;

		if( pMDAFunc[ pAsmFuncAddrITbl[nMid] ].dwRVA == dwRVA )
			return( nMid );
		else if( pMDAFunc[ pAsmFuncAddrITbl[nMid] ].dwRVA < dwRVA )
			nX = nMid;
		else
			nY = nMid;
		nPre = nMid;
	}

	if( nMid == pMD->nTotalAsmFuncEnt -2 )
	{
		if( pMDAFunc[ pAsmFuncAddrITbl[nMid+1] ].dwRVA == dwRVA )
			return( nMid+1 );
	}

	return( -1 );
}

// pIndex, pAsmFunc에 Assembli Function여부와 그 인덱스가 리턴된다.
char *pGetFuncNameByAddr( DWORD dwAddr, int *pIndex, int *pAsmFuncFlag )
{	
	MDFuncEntStt	*pMDFunc;
	MDAsmFuncEntStt	*pMDAFunc;
	char			*pStrTbl;
	int				nI;

	if( pMD == NULL )		 			  
		return( NULL );

	pMDFunc  = pMD->pMDFunc;
	pMDAFunc = pMD->pMDAFunc;
	pStrTbl  = pMD->pStrTbl;
		 	
	// Function Symbol
	*pAsmFuncFlag = 0;
	nI = nGetFuncIndexByRVA( dwAddr - (DWORD)RELOC_BASE );
	if( nI != -1 )
	{
		*pIndex = nI;
		return( &pStrTbl[pMDFunc[nI].dwNameOffs] );
	}
	
	// Assembly Function Symbol
	*pAsmFuncFlag = 1;
	nI = nGetAsmFuncIndexByRVA( dwAddr - (DWORD)RELOC_BASE );
	if( nI != -1 )
	{
		*pIndex = nI;
		return( &pStrTbl[ pMDAFunc[nI].dwNameOffs ] );
	}

	return( NULL );
}




