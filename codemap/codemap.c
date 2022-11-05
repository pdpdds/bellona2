/*////////////////////////////////////////////////////////////////////////////////////////

1.  ���� ��������, MAP ����, COD ������ �� ���丮�� ��� �д�.
2.  CODEMAP <��Ƶ� ����� ��������> [������ DBG ����]
    -  DBG ���ϸ��� �Է��ϸ� ������ ���Ͽ� ����� ������ ����ϰ� 
	   �׷��� ������ ���������� �� �κп� �����δ�.
3.  �������� �̸����κ��� MAP ���� �̸��� ���Ͽ� MAP ������ ���� ó���� �� 
    MAP ���Ͽ� �����ϴ� COD ������ ó���Ѵ�.
4.  ���̺귯���� ��� ��Ƶ� ��� �Ʒ��� ���̺귯�� �̸��� ���� �� �װ��� COD
    ���ϸ� ��Ƶд�.
5.  _TEXT SEGMENT�� �����Լ��� �����Լ��� �����߾��µ� �̴� �߸��� ���̾���. (2003-06-20) 

  <�ܺ� ����� �Լ� Import>
  �Լ��� ũ�Ⱑ 0���� �������� ���� COD ������ ���� ���.
  extern �Լ��� library stub�� ��ũ�ϰ� �Ǹ� MAP ���Ͽ��� �ɺ��� ������ COD ������
  �����Ƿ� �Լ��� ũ�Ⱑ 0���� ���´�.  �̷��� �Լ����� �ش� ��⿡ �������� �ʴ�
  ���� ����.
  Stub Library ������ ��������� ���� JMP ��� �ϳ��� ���� ���̴�.

*/////////////////////////////////////////////////////////////////////////////////////////

#include "codemap.h"
#include "..\pefile.h"

static int init_codemap( CodeMapStt *pCM )
{
	memset( pCM, 0, sizeof( CodeMapStt ) );

	// ����Ʈ ��Ʈ���� �Ҵ��Ѵ�. (�����൵ Ȯ��� ���̹Ƿ� �������.)
	pCM->d.pFileTbl = (MyCoffDbg2FileStt*)CM_MALLOC( sizeof( MyCoffDbg2FileStt ) * CM_DEFAULT_ENT );
	pCM->d.pFuncTbl = (MyCoffDbg2FuncStt*)CM_MALLOC( sizeof( MyCoffDbg2FuncStt ) * CM_DEFAULT_ENT );
	pCM->d.pLineTbl = (struct _MY_IMAGE_LINENUMBER*)CM_MALLOC( sizeof( struct _MY_IMAGE_LINENUMBER ) * CM_DEFAULT_ENT *10 );
	pCM->d.pStrTbl  = (char*)CM_MALLOC( 64 * CM_DEFAULT_ENT );
	pCM->pSymIndex		  = (int*)CM_MALLOC( sizeof( int ) * CM_DEFAULT_ENT );
	
	// ������ �� ������ ó���� ������ ��Ʈ�ϱ� ������ �̸��Ҵ��� �ʿ䰡 ����.
	//pCM->d.pFuncNameIndex;
	//pCM->d.pFuncAddrIndex;

	// ��Ʈ���� ����(ũ��)�� �����Ѵ�.
	pCM->nMaxFileEnt	= CM_DEFAULT_ENT;;
	pCM->nMaxFuncEnt	= CM_DEFAULT_ENT;;
	pCM->nMaxLineEnt	= CM_DEFAULT_ENT;;
	pCM->nMaxStrTblSize	= 64 * CM_DEFAULT_ENT;
	pCM->nMaxSymIndex   = CM_DEFAULT_ENT;;

	// �ϳ��� ����� �Ҵ���� �ʾ����� �׳� ����!
	if( !pCM->d.pFileTbl | !pCM->d.pFuncTbl | !pCM->d.pLineTbl | !pCM->d.pStrTbl  )
		return( -1 );	
	
	return( 0 );
}

static int close_codemap( CodeMapStt *pCM )
{
	int				nI;
	HashEntStt		*pE, *pNext;

	if( pCM->d.pFileTbl       ) CM_FREE( pCM->d.pFileTbl        );
	if( pCM->d.pFuncTbl	      )	CM_FREE( pCM->d.pFuncTbl        );
	if( pCM->d.pLineTbl	      )	CM_FREE( pCM->d.pLineTbl        );
	if( pCM->d.pLocalTbl      )	CM_FREE( pCM->d.pLocalTbl       );
	if( pCM->d.pStrTbl		  )	CM_FREE( pCM->d.pStrTbl         );
	if( pCM->d.pFuncNameIndex ) CM_FREE( pCM->d.pFuncNameIndex  );
	if( pCM->d.pFuncAddrIndex ) CM_FREE( pCM->d.pFuncAddrIndex  );
	if( pCM->pSymIndex		  ) CM_FREE( pCM->pSymIndex		   	);

	for( nI = 0; nI < MAX_HASH_ENT; nI++ )
	{
		if( pCM->hash_index[nI] != NULL )
		{
			for( pE = pCM->hash_index[nI]; pE != NULL; nI++ )
			{
				pNext = pE->pNext;
				CM_FREE( pE );
				pE = pNext;
			}	
		}
	}
	return( 0 );	
}

// ����� ���� �ǳʶڴ�.
static char *skip_space( char *pS )
{
	for( ;; pS++)
	{
		if( *pS == ' ' || *pS == 9 )
			continue;
		break;
	}
	return( pS );
}

typedef struct {
	int		nType;
	char	*pS;
} KWStt;

typedef enum {
	KWT_UNKNOWN	= 0,
	KWT_NUMBER,				// ����
	KWT_STR,				// ���ڿ�

	KWT_COLON,				// �ݷ�
	KWT_SEMI_COLON,			// �����ݷ�
	KWT_QUESTION,			// ����ǥ
	KWT_F,					// F
	KWT_TEXT,
	KWT_SEGMENT,			
	KWT_PROC,
	KWT_ASSIGN,
	KWT_ENDP,
	KWT_PARA,
	KWT_NEAR,
	KWT_PREFERRED,

	END_OF_KWT
} KWT_TAG;

static KWStt	kw[] = {
	{ KWT_COLON			, ":"			},
	{ KWT_SEMI_COLON	, ";"			},
	{ KWT_QUESTION		, "?"			},
	{ KWT_F				, "f"			},
	{ KWT_ASSIGN		, "="			},
	{ KWT_TEXT			, "_TEXT"		},
	{ KWT_SEGMENT		, "SEGMENT"		},
	{ KWT_PROC			, "PROC"		},
	{ KWT_ENDP			, "ENDP"		},
	{ KWT_PARA			, "PARA"		},
	{ KWT_NEAR			, "NEAR"		},
	{ KWT_PREFERRED		, "Preferred"	},
	{ 0					, NULL			}
}; 

// �־��� �ܾ��� Ÿ���� ���Ѵ�.
static int get_keyword_type( char *pS )
{
	int nI;
	
	// ����
	if( '0' <= pS[0] && pS[0] <= '9' )
	{
		return( KWT_NUMBER );
	}

	// ������ΰ�?
	for( nI = 0; kw[nI].pS != NULL; nI++ )
	{
		if( strcmp( kw[nI].pS, pS ) == 0 )
			return( kw[nI].nType );
	}

	// �˼� ���� Ÿ��
	return( KWT_STR );
}

// �־��� ���ڿ� pS���� �� �ܾ �����Ѵ�.
static char *get_next_word( char *pWord, char *pS, int *pType )
{
	int nI;

	pWord[0] = 0;

	// ������ �ǳʶڴ�.
	pS = skip_space( pS );

	for( nI = 0; pS[nI] != 0;  nI++ )
	{	// CR, LF�� 0���� �����Ѵ�.
		if( pS[nI] == 10 || pS[nI] == 13 )
		{
			pS[nI] = 0;
			break;
		}
		// ���� �������� �ٲٰ� �ܾ��� ������ ó���Ѵ�.
		if( pS[nI] == ' ' || pS[nI] == 9 )
		{
			pS[nI] = ' ';
			break;
		}

		// ���й��ڸ� ���ư���.
		if( pS[nI] == ':' || pS[nI] == ';' )
		{
			if( nI == 0 )
			{
				pWord[0] = pS[0];
				nI++;
			}
			break;
		}

		pWord[nI] = pS[nI];
	}

	pWord[nI] = 0;

	// �ܾ��� Ÿ���� ����Ѵ�. 
	pType[0] = get_keyword_type( pWord );

	return( &pS[nI] );	
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

static DWORD dwHexValue( char *pS )
{
	DWORD dwR = 0;
	int   nI;

	uppercase( pS );

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		if( pS[nI] >= 'A' && pS[nI] <= 'Z' )
		{
			dwR = (DWORD)(dwR << 4);
			dwR += (DWORD)( ( pS[nI] - 'A' ) + 10 );
		}
		else if( pS[nI] >= '0' && pS[nI] <= '9' )
		{
			dwR = (DWORD)(dwR << 4);
			dwR += (DWORD)( pS[nI] - '0' );
		}
	}
	return( dwR );
}

// ���� ���̺��� Ȯ���Ѵ�.
static int expand_file_tbl( CodeMapStt *pCM )
{
	int					nSize;
	MyCoffDbg2FileStt	*pFTbl;

	nSize = sizeof( MyCoffDbg2FileStt ) * (pCM->nMaxFileEnt + CM_DEFAULT_ENT);
	pFTbl = (MyCoffDbg2FileStt*)CM_MALLOC( nSize );
	if( pFTbl == NULL )
		return( -1 );			// �޸𸮸� �Ҵ��� �� ������ ��������!
	// 0���� �ʱ�ȭ�Ѵ�.
	memset( pFTbl, 0, nSize );

	// ���� �����͸� �����Ѵ�.
	memcpy( pFTbl, pCM->d.pFileTbl, pCM->d.nTotalFileEnt * sizeof( MyCoffDbg2FileStt ) );
	// ���� ���۸� �����Ѵ�.
	CM_FREE( pCM->d.pFileTbl );
	pCM->d.pFileTbl = pFTbl;

	// MAX ���� ������Ų��.
	pCM->nMaxFileEnt += CM_DEFAULT_ENT;
	
	return( 0 );
}	

// �Լ� ���̺��� Ȯ���Ѵ�.
static int expand_func_tbl( CodeMapStt *pCM )
{
	int					nSize;
	MyCoffDbg2FuncStt	*pFTbl;

	nSize = sizeof( MyCoffDbg2FuncStt ) * (pCM->nMaxFuncEnt + CM_DEFAULT_ENT);
	pFTbl = (MyCoffDbg2FuncStt*)CM_MALLOC( nSize );
	if( pFTbl == NULL )
		return( -1 );			// �޸𸮸� �Ҵ��� �� ������ ��������!
	// 0���� �ʱ�ȭ�Ѵ�.
	memset( pFTbl, 0, nSize );

	// ���� �����͸� �����Ѵ�.
	memcpy( pFTbl, pCM->d.pFuncTbl, pCM->d.nTotalFuncEnt * sizeof( MyCoffDbg2FuncStt ) );
	// ���� ���۸� �����Ѵ�.
	CM_FREE( pCM->d.pFuncTbl );
	pCM->d.pFuncTbl = pFTbl;

	// MAX ���� ������Ų��.
	pCM->nMaxFuncEnt += CM_DEFAULT_ENT;
	
	return( 0 );
}	

// ������ nNameIndex�� ������ �ε����� ����ϴ� ���� ��Ʈ���� �ִ��� ã�´�.
static int find_file_ent( int nNameIndex, CodeMapStt *pCM )
{
	int					nI;
	MyCoffDbg2FileStt	*pFTbl;

	for( nI = 0; nI < pCM->d.nTotalFileEnt; nI++ )
	{
		pFTbl = &pCM->d.pFileTbl[nI];
		if( pFTbl->nNameIndex == nNameIndex )
			return( nI );
	}  

	// ã�� �� ����.
	return( -1 );
}	

// �ɺ� ���̺��� Ȯ���Ѵ�.
static int expand_str_tbl( CodeMapStt *pCM )
{
	char	*pTbl;
	int		nSize;

	nSize = pCM->nMaxStrTblSize + (64 * CM_DEFAULT_ENT);
	pTbl = (char*)CM_MALLOC( nSize );
	if( pTbl == NULL )
		return( -1 );

	// 0���� �ʱ�ȭ�Ѵ�.
	memset( pTbl, 0, nSize );
	memcpy( pTbl, pCM->d.pStrTbl, pCM->d.nStrTblSize );
	free( pCM->d.pStrTbl );
	pCM->d.pStrTbl = pTbl;

	pCM->nMaxStrTblSize = nSize;

	return( 0 );
}


// �ɺ� �ε��� ���̺��� Ȯ���Ѵ�.
static int expand_symindex_tbl( CodeMapStt *pCM )
{
	int *pTbl;
	int	nSize;

	nSize =  sizeof( int ) * (pCM->nMaxSymIndex + CM_DEFAULT_ENT);
	pTbl = (int*)CM_MALLOC( nSize );
	if( pTbl == NULL )
		return( -1 );

	// 0���� �ʱ�ȭ�Ѵ�.
	memset( pTbl, 0, nSize );
	memcpy( pTbl, pCM->pSymIndex, pCM->nTotalSymIndex * sizeof( int ) );
	free( pCM->pSymIndex );
	pCM->pSymIndex = pTbl;

	pCM->nMaxSymIndex += CM_DEFAULT_ENT;

	return( 0 );
}

// ���� Ž�� ��ƾ
/*
static int find_symbol( char *pS, CodeMapStt *pCM, int *pLastIndex )
{
	int nMid, nStart, nEnd, nR;

	nStart = 0;
	pLastIndex[0] = -1;
	nEnd   = pCM->nTotalSymIndex;
	
	for( ; nStart < nEnd; )
	{
		pLastIndex[0] = nMid = ( nStart + nEnd ) / 2;
		nR = strcmp( pS, &pCM->d.pStrTbl[ pCM->pSymIndex[ nMid ] ] );
		if( nR == 0 )
			return( nMid );
		if( nR > 0 )
		{
			if( nStart == nMid )
			{
				pLastIndex[0]++;
				break;
			}
			nStart = nMid;
		}
		else
		{
			if( nEnd == nMid )
				break;
			nEnd = nMid;
		}
	}

	return( -1 );
}

static int insert_sym_index( CodeMapStt *pCM, int nLastIndex, int nIndex )
{
	int nI;

	// nLastIndex ��ġ�� �ֱ� ���� ���� ������ ���� nLastIndex���� �ϳ��� �ε����� �ڷ� �δ�.
	if( nLastIndex >= 0 )
	{
		for( nI = pCM->nTotalSymIndex; nI > nLastIndex; nI-- )
			pCM->pSymIndex[nI] = pCM->pSymIndex[nI-1];

		pCM->pSymIndex[nI] = nIndex;
	}
	else
		pCM->pSymIndex[0] = nIndex;	// �Էµ� ���� �ϳ��� ���ٰ� ���� 0���� ���� �ִ´�.

	pCM->nTotalSymIndex++;

	return( 0 );
}
*/
/*
// (����Ž��) �ɺ��� ã�´�.
static int find_symbol( char *pS, CodeMapStt *pCM, int *pLastIndex )
{
	int nI;

	pLastIndex[0] = -1;

	for( nI = 0; nI < pCM->nTotalSymIndex; nI++ )
	{
		if( strcmp( pS, &pCM->d.pStrTbl[ pCM->pSymIndex[ nI ] ] ) == 0 )
			return( pCM->pSymIndex[ nI ] );
	}

	return( -1 );
}
static int insert_sym_index( CodeMapStt *pCM, int nLastIndex, int nIndex )
{
	pCM->pSymIndex[pCM->nTotalSymIndex] = nIndex;	// �Էµ� ���� �ϳ��� ���ٰ� ���� 0���� ���� �ִ´�.

	pCM->nTotalSymIndex++;

	return( 0 );
}
*/
/////////////////////////////////////////////////////////////////////////
// �ؽ� Ű ���� ���Ѵ�.
static DWORD get_hash_key( char *pS )
{
	int					nI;
	unsigned short int	wX, wM;

	wX = 0;

	for( nI = 0; pS[nI] != 0; nI++ )
	{
		// ROL
		if( wX & (unsigned short int)0x8000 )
			wM = 1;
		else
			wM = 0;
		wX = (unsigned short int)(wX << 1) + wM;

		// XOR
		wX = (unsigned short int)( wX ^ (unsigned short int)pS[nI] );
	}	

	wM = wX / MAX_HASH_ENT;
	wX = wX % MAX_HASH_ENT;
	wX = (wX ^ wM);
	return( (DWORD)wX );
}

// �ؽ� ���̺��� �̿��� Ž��
static int find_symbol( char *pS, CodeMapStt *pCM )
{
	int			nI;
	HashEntStt	*pE;

	nI = (int)get_hash_key( pS );

	pE = pCM->hash_index[nI];
	if( pE == NULL )
		return( -1 );	// ���� ��ϵ��� ���� �ɺ��̴�.

	for( ; pE != NULL; pE = pE->pNext )
	{	// Hash Entry���� ������� ������ �񱳸� �ؾ� �Ѵ�.
		if( strcmpi( &pCM->d.pStrTbl[ pCM->pSymIndex[ pE->nSymIndex ] ], pS ) == 0 )
			return( pCM->pSymIndex[ pE->nSymIndex ] );
	}
	return( -1 );
}
static int insert_sym_index( CodeMapStt *pCM, int nIndex )
{
	int			nI;
	char		*pS;
	HashEntStt	*pE, *pNew;

	pS = &pCM->d.pStrTbl[nIndex];
	nI = (int)get_hash_key( pS );
	pCM->pSymIndex[pCM->nTotalSymIndex] = nIndex;

	// �ؽ� ���̺� ����Ѵ�.
	pNew = (HashEntStt*)CM_MALLOC( sizeof( HashEntStt ) );
	if( pNew == NULL )
	{
		CM_PRINTF( "insert_sym_index() - memory allocation failed!\n" );
		return( -1 );
	}
	memset( pNew, 0, sizeof( HashEntStt ) );
	pNew->nSymIndex = pCM->nTotalSymIndex;
	nI = (int)get_hash_key( pS );
	pE = pCM->hash_index[nI];
	if( pE == NULL )
	{	// ���� ó���� �����Ѵ�.
		pCM->hash_index[ nI ] = pNew;
	}
	else
	{	
		pCM->nTotalCollision++;
		// ���� �ڿ� �����Ѵ�.
		for( nI = 0; pE->pNext != NULL; pE = pE->pNext, nI++ )
			;
		if( pCM->nMaxCollision < nI )
			pCM->nMaxCollision = nI;

		pE->pNext = pNew;
	}	

	pCM->nTotalSymIndex++;

	return( 0 );
}
/////////////////////////////////////////////////////////////////////////
// �ɺ� ���̺� ���ο� �ɺ��� ����Ѵ�.
// ������ ���� ������ ���� �ε����� �����Ѵ�.
static int register_symbol( char *pS, CodeMapStt *pCM )
{
	int nSize, nIndex, nR;

	// ������ �ɺ��� �̹� ��ϵǾ� �ִ��� Ȯ���Ѵ�.
	nIndex = find_symbol( pS, pCM );
	if( nIndex >= 0 )
		return( nIndex );		// ã�� �ε����� �����Ѵ�.

	// ������ Ȯ���ؾ� �ϴ��� Ȯ���Ѵ�.
	nSize = strlen( pS );
	if( pCM->nMaxStrTblSize <= pCM->d.nStrTblSize + nSize )
	{	// �ɺ� ���̺��� Ȯ���Ѵ�.
		nR = expand_str_tbl( pCM );
		if( nR < 0 )
			return(-1 );
	}

	// �ε��� ������ Ȯ���ؾ� �ϴ��� Ȯ���Ѵ�.
	if( pCM->nTotalSymIndex >= pCM->nMaxSymIndex )
	{
		nR = expand_symindex_tbl( pCM );
		if( nR < 0 )
			return( -1 );
	}

	// �ɺ��� �����Ѵ�.
	nIndex = pCM->d.nStrTblSize;
	strcpy( &pCM->d.pStrTbl[ nIndex ], pS );
	pCM->d.nStrTblSize += nSize + 1;	// ���� ���� 0���� ����Ѵ�.

	// �ɺ��� �ε����� �߰��Ѵ�.
	insert_sym_index( pCM, nIndex );
	
	return( nIndex );
}

// ������ �����ϴ��� Ȯ���Ѵ�.
static int file_exists( char *pPath )
{
	struct _finddata_t	fi;
	long				lR;

	lR = _findfirst( pPath, &fi );
	if( lR < 0 )
		return( -1 );	// ������ �������� �ʴ´�.

	_findclose( lR );
	
	// ������ ã�Ҵ�.
	return( 0 );
}

// Ǯ�н����� ������ ���ϸ��� �����Ѵ�.  ���� \�� ���´�.
static void make_pure_path( char *pS )
{
	int nI;

	for( nI = strlen( pS ); nI > 0; nI-- )
	{
		if( pS[nI] == '\\' )
		{
			pS[nI+1] = 0;
			return;
		}
	}
	strcpy( pS, ".\\" );
}

// ������ ����Ѵ�.
static int register_file( char *pMapFile, char *pS, DWORD dwAddr, CodeMapStt *pCM )
{
	char				szT[260];
	MyCoffDbg2FileStt	*pFile;
	int					nNameIndex, nFileIndex, nR;

	// ������ �ɺ��� �̹� ��ϵǾ� �ִ��� Ȯ���Ѵ�.
	nNameIndex = find_symbol( pS, pCM );
	if( nNameIndex >= 0 )
	{	// ���⼭ �׳� nNameIndex�� ������ ������ ���װ� �־���.  (2003-08-28)
		// pFunc->nFileIndex�� �͹��� ���� ū ���� �� �ᱹ 
		// filename����ϴ� �κп��� ������ �߻�(GP Fault)!!!.
		nFileIndex = find_file_ent( nNameIndex, pCM );
		if( nFileIndex >= 0 )
			return( nFileIndex );		// ã�� �ε����� �����Ѵ�.
		else
			return( -1 );
	}

	// ������ �����ϴ��� Ȯ���Ѵ�.
	strcpy( szT, pMapFile );
	make_pure_path( szT );
	nR = strlen( szT );		  // �������� �������ð� �ƴϸ� �������ø� ���δ�.
	if( nR > 0 && szT[nR-1] != '\\' )
		strcat( szT, "\\" );
	strcat( szT, pS );		  // path + cod_file_name
	nR = strlen( szT );
	strcpy( &szT[nR-3], "cod" );
	// ������ �����ϴ��� ã�� ����.
	nR = file_exists( szT );
	if( nR < 0 )
		return( -1 );		  // OCD ������ ã�� �� ������ ���� ����.

	// ���ϸ��� �ɺ��� ����Ѵ�.
	nNameIndex = register_symbol( pS, pCM );
	if( nNameIndex < 0 )
		return( -1 );	// ����
	
	// �̹� ��ϵǾ����� ã�ƺ���.
	nFileIndex = find_file_ent( nNameIndex, pCM );
	if( nFileIndex >= 0 )
	{						   
		pFile = &pCM->d.pFileTbl[nFileIndex];
		if( pFile->dwAddr > dwAddr )
			pFile->dwAddr = dwAddr;
		return( nFileIndex );		// �̹� ��ϵǾ� �־���.
	}

	// ���� ���̺� ������ �ִ��� Ȯ���Ѵ�.
	if( pCM->nMaxFileEnt <= pCM->d.nTotalFileEnt )
	{	// ������ ������ Ȯ���Ͽ��� �Ѵ�.
		nR = expand_file_tbl( pCM );
		if( nR < 0 )
			return( -1 );		// Ȯ���� �� ������ ���� ����!
	}		

	// �ʵ带 ä��� ī���͸� ������Ų��.
	nFileIndex = pCM->d.nTotalFileEnt;
	pFile = &pCM->d.pFileTbl[nFileIndex];
	memset( pFile, 0, sizeof( MyCoffDbg2FileStt ) );
	pFile->dwAddr     = dwAddr;
	pFile->nNameIndex = nNameIndex;
	pCM->d.nTotalFileEnt++;

	return( nFileIndex );
}

// �Լ��� ����Ѵ�.
static int register_func( char *pS, int nFileNameIndex, DWORD dwAddr, CodeMapStt *pCM )
{
	MyCoffDbg2FuncStt	*pFunc;
	int					nNameIndex, nFuncIndex, nR;

	// �Լ����� �ɺ��� ����Ѵ�.
	nNameIndex = register_symbol( pS, pCM );
	if( nNameIndex < 0 )
		return( -1 );
	// �̹� ��ϵǾ� �ִ����� Ȯ���� �� �ʿ䰡 ����.

	// ���� ���̺� ������ �ִ��� Ȯ���Ѵ�.
	if( pCM->nMaxFuncEnt <= pCM->d.nTotalFuncEnt )
	{	// ������ ������ Ȯ���Ͽ��� �Ѵ�.
		nR = expand_func_tbl( pCM );
		if( nR < 0 )
			return( -1 );		// Ȯ���� �� ������ ��������!
	}		

	// �ʵ带 ä��� ī���͸� ������Ų��.
	nFuncIndex = pCM->d.nTotalFuncEnt;
	pFunc = &pCM->d.pFuncTbl[nFuncIndex];
	memset( pFunc, 0, sizeof( MyCoffDbg2FuncStt ) );
	pFunc->dwAddr     = dwAddr;
	pFunc->nNameIndex = nNameIndex;
	pFunc->nFileIndex = nFileNameIndex;
	pCM->d.nTotalFuncEnt++;

	return( 1 );
}

// MAP ������ �� ������ ó���Ѵ�.
// <���ϰ�> 0 - �׳� ��ŵ, 1 - ó����.
static int process_map_line( char *pMapFile, char *pS, CodeMapStt *pCM )
{
	DWORD				dwRVA;
	int					nI, nType, nFileIndex;
	char				*pNext, szT[512], szFuncName[128], szFileName[128];

	// ���ڷ� �������� ������ �׳� ���ư���.		0001
	pNext = get_next_word( szT, pS, &nType );
	if( nType != KWT_NUMBER )
	{	// ��Ŀ�� ���̽� ��巹���� ���Ѵ�.
		if( nType != KWT_PREFERRED )
			return( 0 );
		for( nI = 0; nI < 4; nI++ )
			pNext = get_next_word( szT, pNext, &nType );
		pCM->dwLinkerBaseAddr = dwHexValue( szT );				
		return( 0 );
	}

	// ������ �ݷ��� ������ ������ �׳� ���ư���.	:	
	pNext = get_next_word( szT, pNext, &nType );
	if( nType != KWT_COLON )
		return( 0 );

	// ������ ���� ������ ������ �׳� ���ư���.		00001b94
	pNext = get_next_word( szT, pNext, &nType );
	if( nType != KWT_NUMBER )
		return( 0 );

	// ������ ���ڿ��� �ƴϸ� �׳� ���ư���.		_kdbg_printf
	pNext = get_next_word( szT, pNext, &nType );
	if( nType != KWT_STR )
		return( 0 );
	// �Լ� �̸��� ���ߴ�.
	strcpy( szFuncName, szT );

	// ������ ���� ������ ������ �׳� ���ư���.		00401000
	pNext = get_next_word( szT, pNext, &nType );
	if( nType != KWT_NUMBER )
		return( 0 );
	// RVA�� ���Ѵ�.
	dwRVA = dwHexValue( szT ) - pCM->dwLinkerBaseAddr;

	// ������ F�� ���;� �Ѵ�.							f
	pNext = get_next_word( szT, pNext, &nType );
	if( nType != KWT_F )
		return( 0 );

	// ���ڷ� �����ϴ� ���ϵ� �ִ�.					dbg.obj
	pNext = get_next_word( szT, pNext, &nType );
	if( nType != KWT_STR && nType != KWT_NUMBER )
		return( 0 );

	strcpy( szFileName, szT );
	if( pNext[0] == ':' )
	{
		pNext = get_next_word( szT, &pNext[1], &nType );
		strcat( szFileName, "\\" );
		strcat( szFileName, szT );
	}

	// RVA, �Լ� �̸�, ���� �̸��� ���ߴ�.
 	//CM_PRINTF( "0x%08X %-12s %s\n", dwRVA, szFileName, szFuncName );			

	// ������ ����Ѵ�.
	nFileIndex = register_file( pMapFile, szFileName, dwRVA, pCM );
	if( nFileIndex < 0 )
		return( 0 );	// ������ ������ �׳� ��ŵ�Ѵ�.
	else if( nFileIndex >= pCM->d.nTotalFileEnt )
	{	// �ɰ��� ����.
		return( -1 );
	}

	// �Լ��� ����Ѵ�.
	register_func( szFuncName, nFileIndex, dwRVA, pCM );

	return( 0 );
}

// MAP ���� ������ �м��Ͽ� pCM�� �����Ѵ�.
static int map_info( char *pFileName, CodeMapStt *pCM )
{
	int		nR;
	FILE	*pF;
	char	szT[512], *pS;

	// MAP ������ �����Ѵ�.
	pF = fopen( pFileName, "rt" );
	if( pF == NULL )
	{	// ������ ������ �� ����.
		CM_PRINTF( "Map file <%s> open error!\n", pFileName );
		return( -1 );
	}

	// �� ������ ó���Ѵ�.
	for( ;; )
	{
		pS = fgets( szT, sizeof(szT) -1, pF );
		if( pS == NULL )
		{
			nR = 0;
			break;		// ���̻� �о���� �� ����.
		}

		// �о���� ������ ó���Ѵ�.
		nR = process_map_line( pFileName, szT, pCM );
		if( nR < 0 )
			break;
	}

	// ������ �ݰ� ���ư���.
	fclose( pF );

	return( nR );
}

// Local Table�� Ȯ���Ѵ�.
static int expand_local_tbl( CodeMapStt *pCM )
{
	int					nSize;
	MyCoffDbg2LocalStt	*pLTbl;

	nSize = sizeof( MyCoffDbg2LocalStt ) * ( pCM->nMaxLocalEnt + CM_DEFAULT_ENT );
	pLTbl = (MyCoffDbg2LocalStt*)CM_MALLOC( nSize );
	if( pLTbl == NULL )
		return( -1 );			// �޸𸮸� �Ҵ��� �� ������ ��������!

	// 0���� �ʱ�ȭ�Ѵ�.
	memset( pLTbl, 0, nSize );

	// ���� �����͸� �����Ѵ�.
	memcpy( pLTbl, pCM->d.pLocalTbl, pCM->d.nTotalLocalEnt * sizeof( MyCoffDbg2LocalStt ) );

	// ���� ���۸� �����Ѵ�.
	CM_FREE( pCM->d.pLocalTbl );
	pCM->d.pLocalTbl = pLTbl;

	// MAX ���� ������Ų��.
	pCM->nMaxLocalEnt += CM_DEFAULT_ENT;
	
	return( 0 );
}	

// �ķ����Ϳ� ���������� �����Ѵ�.
static int register_param_and_local_var( CodeMapStt *pCM, char *pSymName, char *pValue )
{
	MyCoffDbg2LocalStt		*pL;
	int nValue,				nR, nNameIndex;

	nValue = atoi( pValue );
	
	// ���� ���̺� ������ �ִ��� Ȯ���Ѵ�.
	if( pCM->nMaxLocalEnt <= pCM->d.nTotalLocalEnt )
	{	// ������ ������ Ȯ���Ͽ��� �Ѵ�.
		nR = expand_local_tbl( pCM );
		if( nR < 0 )
			return( -1 );		// Ȯ���� �� ������ ��������!
	}		

	// �ɺ��� ����Ѵ�.
	if( pSymName[ strlen( pSymName ) -1 ] == '$' )
		pSymName[ strlen( pSymName ) -1 ] = 0;	  // �������� '$'�� �����Ѵ�.

	nNameIndex = register_symbol( pSymName, pCM );
	if( nNameIndex < 0 )
		return( -1 );		// �ɺ��� ����� �� ����.

	pL = &pCM->d.pLocalTbl[ pCM->d.nTotalLocalEnt ];

	// �ʵ带 ä��� ī���͸� ������Ų��.
	pL->nNameIndex = nNameIndex;
	pL->nEBPAdder  = nValue;
	pCM->d.nTotalLocalEnt++;

	return( 0 ); 
}

static MyCoffDbg2FuncStt *find_func_by_name( CodeMapStt *pCM, char *pFuncName )
{
	int nStart, nMid, nEnd, nR;

	nStart = 0;
	nEnd   = pCM->d.nTotalFuncEnt-1;
	

	for( ; nStart <= nEnd; )
	{
		nMid   = (nStart + nEnd) / 2;

		nR = strcmpi( pFuncName, &pCM->d.pStrTbl[ pCM->d.pFuncTbl[ pCM->d.pFuncNameIndex[ nMid] ].nNameIndex ] );
		if( nR == 0 )
			return( &pCM->d.pFuncTbl[ pCM->d.pFuncNameIndex[nMid] ] );
		else if( nR > 0 )
			nStart = nMid+1;
		else
			nEnd = nMid-1;
	}

	return( NULL );
}

// ���� ���̺��� Ȯ���Ѵ�.
static int expand_line_tbl( CodeMapStt *pCM )
{
	struct _MY_IMAGE_LINENUMBER *pLT;
	int							nSize;

	nSize = sizeof(struct _MY_IMAGE_LINENUMBER) * (pCM->nMaxLineEnt + CM_DEFAULT_ENT*4);
	pLT = (struct _MY_IMAGE_LINENUMBER*)CM_MALLOC( nSize );
	if( pLT == NULL )
		return( -1 );		// �޸𸮸� �Ҵ��� �� ����.
	memset( pLT, 0, nSize );

	// ���� �����͸� �����Ѵ�.
	memcpy( pLT, pCM->d.pLineTbl, sizeof(struct _MY_IMAGE_LINENUMBER) * pCM->nMaxLineEnt );
	CM_FREE( pCM->d.pLineTbl );
	pCM->d.pLineTbl = pLT;

	pCM->nMaxLineEnt += CM_DEFAULT_ENT*4;

	return( 0 );
}

// ���ι�ȣ�� �߰��Ѵ�.
static int register_linenum( CodeMapStt *pCM, int nLine, DWORD dwAddr )
{
	int							nR;
	struct _MY_IMAGE_LINENUMBER	*pL;

	if( pCM->nMaxLineEnt <= pCM->d.nTotalLineEnt )
	{
		nR = expand_line_tbl( pCM );
		if( nR < 0 )
			return( -1 );	// ���� ���̺��� Ȯ���� �� ����.
	}

	pL = &pCM->d.pLineTbl[ pCM->d.nTotalLineEnt ];
	pL->Linenumber = (unsigned short int)nLine;
	pL->Type.VirtualAddress = dwAddr;		  

	pCM->d.nTotalLineEnt++;

	return( pCM->d.nTotalLineEnt-1 );
}


// COD ���� ���� �Լ��� ó���Ѵ�.
static char szWord[512], szWord2[512], szWord3[512];
static int function_in_cod_file( CodeMapStt *pCM, FILE *pF, MyCoffDbg2FileStt *pFile )
{
	MyCoffDbg2FuncStt	*pFunc;
	char				szT[512], szFuncName[64], *pS, *pNext;
	DWORD				dwOffset, dwFuncStartOffset, dwFuncLastOffset;
	int					nR, nType, nType2, nType3, nLine, nLineIndex, nTotalFuncLocal, nFuncLocalIndex;

	nFuncLocalIndex = -1;
	nTotalFuncLocal = 0;

	// �Լ� ó���ϴ� �κ�
	// MyCoffDbg2FuncStt�� nLineIndex�� dwSize�� �����ϸ� �ȴ�.
	// MyCoffDbg2FileStt�� nTotalLine�� �����ؾ� �Ѵ�.
	for( ;; )
	{	// �� ������ �о���δ�.
		pNext = pS = fgets( szT, sizeof( szT )-1, pF );
		if( pS == NULL )
			goto QUIT;

		// 3���� Ű���带 �д´�.
		pNext = get_next_word( szWord,  pNext, &nType  );
		pNext = get_next_word( szWord2, pNext, &nType2 );
		pNext = get_next_word( szWord3, pNext, &nType3 );
		
		// �Լ��� �����ΰ�?
		if( nType2 == KWT_PROC && nType3 == KWT_NEAR )
			goto GET_LINE;

		// ���������� �ķ����͸� ��Ÿ���� �����ΰ�?
		if( nType2 != KWT_ASSIGN )
			continue;	

		// ù ��° Ű���尡 $�� �����°�?
		nR = strlen( szWord );
		if( nR <= 1 || szWord[nR-1] != '$' )
			continue;

		// parameter�� local variable�� ����Ѵ�.
		nR = register_param_and_local_var( pCM, szWord, szWord3 );
		if( nR == 0 )
		{
			if( nFuncLocalIndex < 0 )
			{	// �ش� �Լ��� ���� Local Index
				nFuncLocalIndex = pCM->d.nTotalLocalEnt-1;
			}
			// �ش� �Լ��� Local Entry(Local Var, Param) ����
			nTotalFuncLocal++;
		}
		continue;
			
GET_LINE:		
		dwFuncStartOffset = dwFuncLastOffset = 0xFFFFFFFF;
		
		// szWord�� �Լ� �̸��� ����Ǿ� �ִ�.
		strcpy( szFuncName, szWord );
		//CM_PRINTF( "FUNCTION : %s\n", szFuncName );

		// MAP ���Ͽ��� �ش� �Լ��� ã�´�.
		pFunc = find_func_by_name( pCM, szFuncName );
		if( pFunc == NULL )
		{	// COD ���Ͽ��� ������ MAP ���Ͽ��� �������� ����(LINK���� ����) �Լ�
			//CM_PRINTF( "Function <%s> is not exist in MAP file.\n", szFuncName );
			pCM->nTotalUselessFunction++;
			// ENDP���� ��ŵ�Ѵ�.
			for( ;; )
			{	// �� ������ �д´�.
				pNext = pS = fgets( szT, sizeof( szT )-1, pF );
				if( pS == NULL )
					goto QUIT;
				// �ܾ� 2���� �о���δ�.
				pNext = get_next_word( szWord, pNext, &nType );
				pNext = get_next_word( szWord2, pNext, &nType2 );
				if( nType2 == KWT_ENDP )
					break;
			}
			continue;
		}

		// ó���� �ķ����ͳ� ���������� ������ �����Ѵ�.
		pFunc->nLocalIndex = nFuncLocalIndex;
		pFunc->nTotalLocal = nTotalFuncLocal;
		nTotalFuncLocal = 0;
		nFuncLocalIndex = -1;

		// ���ι�ȣ�� ó���Ѵ�.
		for( nLine = 0;; )
		{	// �� ������ �д´�.
			pNext = pS = fgets( szT, sizeof( szT )-1, pF );
			if( pS == NULL )
				goto QUIT;

			// �ܾ� 2���� �о���δ�.
			pNext = get_next_word( szWord, pNext, &nType );
			pNext = get_next_word( szWord2, pNext, &nType2 );
			if( nType == KWT_SEMI_COLON )
			{	// �����ݷ����� �����ϸ� ���� ��ȣ.
				nLine = atoi( szWord2 );
				continue;
			}
			else if( nType == KWT_NUMBER )
			{	// �����ݷ��� �ƴϸ� ���� �������� ������
				if( nLine != 0 )  
				{	// ���ο� �ش��ϴ� �������� �����Ѵ�.
					dwOffset = dwHexValue( szWord );
					if( dwFuncStartOffset == 0xFFFFFFFF )
						dwFuncStartOffset = dwOffset;

					// .. ���ι�ȣ�� �������� �߰��Ѵ�.
					//CM_PRINTF( "LINENUM : %d, Offset : %05X\n", nLine, dwOffset );
								 
					// ���� ��ȣ�� �߰��Ѵ�.
					nLineIndex = register_linenum( pCM, nLine, dwOffset );
					
					// �Լ��� ���� ���ι�ȣ �ε����� �����Ѵ�.
					if( pFunc != NULL && pFunc->nLineIndex == 0 )
						pFunc->nLineIndex = nLineIndex;
					
					// ������ ���ι�ȣ ������ ������Ų��.
					pFile->nTotalLine++;
					nLine = 0;
					dwFuncLastOffset = dwOffset;
					continue;	// ���� ���ι�ȣ�� ã�´�.
				}
				else
				{	// �Լ��� ������ �������� ���ؾ� �Ѵ�.
					dwFuncLastOffset = dwHexValue( szWord );
					continue;
				}	
			}
			else if( nType2 == KWT_ENDP )
			{	// �Լ��� ũ��� ������ ũ�⸦ �����Ѵ�. 				
				pFunc->dwSize = dwFuncLastOffset - dwFuncStartOffset;
				pFile->dwSize = dwFuncLastOffset;
				//CM_PRINTF( "END : %s\n\n", szFuncName );
				
				// �ϳ��� �Լ��� ���� ó���� ������.
				break;		
			}
		}// ���ι�ȣ ���ϴ� for ��
	}// �Լ� ó���ϴ� for ��

QUIT:
	// ���̻� ó���� ���� ����.
	return( -1 );
}


// �ϳ��� COD ������ ó���Ѵ�.
static int cod_file( char *pFileName, CodeMapStt *pCM, MyCoffDbg2FileStt *pFile )
{
	int		nR;
	FILE	*pF;

	// MAP ������ �����Ѵ�.
	pF = fopen( pFileName, "rt" );
	if( pF == NULL )
	{	// ������ ������ �� ����.
		CM_PRINTF( "SKIP : %s (Open failed.)\n", pFileName );
		return( -1 );
	}

	//CM_PRINTF( "COD_FILE : %s\n", pFileName );

	// COD ���� ���� ���ԵǾ� �ִ� �Լ����� ó���Ѵ�.
	for( ;; )
	{
		nR = function_in_cod_file( pCM, pF, pFile ); 
		if( nR < 0 )
			break;
	}

	// ������ �ݰ� ���ư���.
	fclose( pF );
	// ó���� ������ ������ ������Ų��.
	pCM->nTotalProcessedFiles++;
	
	//CM_PRINTF( "Total %d linenumbers.\n", pCM->d.nTotalLineEnt );

	return( nR );
}

// Working �н��κ��� pCM�ӿ� ���Ե� COD ������ ã�� ó���Ѵ�.
static int cod_info( char *pWorkPath, CodeMapStt *pCM )
{
	MyCoffDbg2FileStt	*pFile;
	char				szT[260];
	int					nI, nX, nR;
	
	for( nI = 0; nI < pCM->d.nTotalFileEnt; nI++ )
	{
		pFile = &pCM->d.pFileTbl[nI];
		// COD ���� �̸��� �����Ѵ�.
		sprintf( szT, "%s%s", pWorkPath, &pCM->d.pStrTbl[pFile->nNameIndex] );
		nX = strlen( szT );
		strcpy( &szT[nX-3], "COD" );

		// COD ������ ó���Ѵ�.
		nR = cod_file( szT, pCM, pFile );
	}

	return( 0 );
}

// Ǯ �н��� ��ϵ� ������ ���ϸ��� �߶󳽴�.
static int cut_last_filename( char *pS )
{
	int nI;

	for( nI = strlen( pS )-1; nI > 0; nI-- )
	{
		if( pS[nI] == '\\' )
		{
			pS[nI+1] = 0;
			return( 0 );
		}
	}					

	return( -1 );
}

// �Լ� ���̺�κ��� (�ּ�, �Լ���)�� ���� �ε����� �����Ѵ�.
static int make_func_index( CodeMapStt *pCM )
{
	int					nI, nJ, nMinAddr, nMinName;

	// �� ���� �ε����� �Ҵ��Ѵ�.
	pCM->d.pFuncNameIndex = (int*)CM_MALLOC( sizeof(int) * pCM->d.nTotalFuncEnt );
	pCM->d.pFuncAddrIndex = (int*)CM_MALLOC( sizeof(int) * pCM->d.nTotalFuncEnt );
	if( !pCM->d.pFuncAddrIndex || !pCM->d.pFuncNameIndex )
	{
		CM_PRINTF( "make_func_index() - memory allocation error!\n" );
		return( -1 );
	}

	for( nI = 0; nI < pCM->d.nTotalFuncEnt; nI++ )
	{
		pCM->d.pFuncAddrIndex[nI] = nI;
		pCM->d.pFuncNameIndex[nI] = nI;
	}

	for( nI = 0; nI < pCM->d.nTotalFuncEnt; nI++ )
	{
		for( nMinAddr = nMinName = nJ = nI; nJ < pCM->d.nTotalFuncEnt; nJ++ )
		{
			if( pCM->d.pFuncTbl[pCM->d.pFuncAddrIndex[nJ]].dwAddr < pCM->d.pFuncTbl[pCM->d.pFuncAddrIndex[nMinAddr]].dwAddr )
				nMinAddr = nJ;
			if( strcmpi( &pCM->d.pStrTbl[pCM->d.pFuncTbl[pCM->d.pFuncNameIndex[nJ]].nNameIndex], 
				&pCM->d.pStrTbl[pCM->d.pFuncTbl[pCM->d.pFuncNameIndex[nMinName]].nNameIndex] ) < 0 )
				nMinName = nJ;
		}	

		nJ = pCM->d.pFuncNameIndex[nMinName];
		pCM->d.pFuncNameIndex[nMinName] = pCM->d.pFuncNameIndex[nI]; 
		pCM->d.pFuncNameIndex[nI] = nJ;

		nJ = pCM->d.pFuncAddrIndex[nMinAddr];
		pCM->d.pFuncAddrIndex[nMinAddr] = pCM->d.pFuncAddrIndex[nI];
		pCM->d.pFuncAddrIndex[nI] = nJ;
	}	

	// �̸��� ���� ���ĵ� �Լ����� ����Ѵ�.
//	{
//		MyCoffDbg2FuncStt	*pFunc;
//		for( nI = 0; nI < pCM->d.nTotalFuncEnt; nI++ )
//		{
//			// �̸��� ���� ���
//			//pFunc = &pCM->d.pFuncTbl[ pCM->d.pFuncNameIndex[nI] ];
//			// �ּҿ� ���� ���
//			pFunc = &pCM->d.pFuncTbl[ pCM->d.pFuncAddrIndex[nI] ];
//			CM_PRINTF( "[%3d] 0x%08X  %s\n", nI, pFunc->dwAddr, &pCM->d.pStrTbl[ pFunc->nNameIndex ] );
//		}
//	}

	return( 0 );
}

// ����� ������ ���Ͽ� ����Ѵ�.
static int write_debug_info( CodeMapStt *pCM, char *pDbgFile, int nCreateNew )
{
	MY_IMAGE_OPTIONAL_HEADER	iohd;
	MY_IMAGE_DOS_HEADER			doshd;
	int							nHandle;
	long						lDbgLoc, lOffs;

	lDbgLoc = 0;
	if( nCreateNew == 0 )
	{	// ���� ������ ����.
		nHandle = open( pDbgFile, _O_BINARY | _O_RDWR );
		if( nHandle < 0 )
			goto CN;		// ���� ������ ������ ���� �����Ѵ�.
		// ���� �����͸� �ڷ� �ű��.
		lDbgLoc = lseek( nHandle, 0, SEEK_END );
	}
	else
	{	// ������ ����� ���� �����Ѵ�.
		remove( pDbgFile );
CN:
		nCreateNew = 1;
		nHandle = open( pDbgFile, _O_BINARY | _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE );
		if( nHandle == -1 )
			return( -1 );
	}

	write( nHandle, &pCM->d, sizeof( MyCoffDbg2Stt ) );
	write( nHandle, pCM->d.pFileTbl, sizeof( MyCoffDbg2FileStt ) * pCM->d.nTotalFileEnt );
	write( nHandle, pCM->d.pFuncTbl, sizeof( MyCoffDbg2FuncStt ) * pCM->d.nTotalFuncEnt );
	write( nHandle, pCM->d.pFuncNameIndex, sizeof( int ) * pCM->d.nTotalFuncEnt );
	write( nHandle, pCM->d.pFuncAddrIndex, sizeof( int ) * pCM->d.nTotalFuncEnt );
	write( nHandle, pCM->d.pLineTbl,  sizeof( struct _MY_IMAGE_LINENUMBER ) * pCM->d.nTotalLineEnt );
	write( nHandle, pCM->d.pLocalTbl, sizeof( MyCoffDbg2LocalStt ) * pCM->d.nTotalLocalEnt );
	write( nHandle, pCM->d.pStrTbl, pCM->d.nStrTblSize );

	// ������ ���� ������ ��쿡�� ������ �ݰ� ���ư���.
	if( nCreateNew != 0 )
	{	
		close( nHandle );
		return( 0 );
	}

	// 2002-05-12
	// Image Optional Header�� dd_Debug_dwVAddr, dd_Debug_dwSize�� �����ؾ� �Ѵ�.
	lseek( nHandle, 0, SEEK_SET );
	// ���� ����� �д´�.
	read( nHandle, &doshd, sizeof( doshd ) );
	
	// Image Optional Header�� �д´�.
	lOffs = doshd.e_lfanew + sizeof( MY_IMAGE_FILE_HEADER );
	lseek( nHandle, lOffs, SEEK_SET );
	read( nHandle, &iohd, sizeof( iohd ) );

	// Virtual Address�� �ƴϰ� �ܼ��� ���� ���� ������ �������̴�.
	iohd.dd_Debug_dwVAddr = lDbgLoc;
	iohd.dd_Debug_dwSize  = pCM->d.dwSize;

	// Image Optional Header�� ����Ѵ�.
	lseek( nHandle, lOffs, SEEK_SET );
	write( nHandle, &iohd, sizeof( iohd ) );

	// ������ �ݰ� ���ư���.
	close( nHandle );

	printf( "dd_Debug_dwVAddr/dwSize = (%d/%d)\n", lDbgLoc, pCM->d.dwSize );

	return( 0 );
}
 
// MAIN �Լ����� codemap �Լ��� ȣ���Ѵ�.
static CodeMapStt	cm;
int codemap( char *pMapFile, char *pDbgFile )
{
	char		szT[260];
	int			nR, nCreateNew;

	// CodeStt�� �ʱ�ȭ�Ѵ�.
	nR = init_codemap( &cm );
	if( nR < 0 )
	{
		CM_PRINTF( "memory allocation error!\n" );
		return( -1 );
	}
	
	// MAP ������ ó���Ѵ�.
	nR = map_info( pMapFile, &cm );
	if( nR < 0 )
		return( -1 );

	// �Լ��� �̸��� �ּҿ� ���� ������ �Ѵ�.
	make_func_index( &cm );

	// COD ������ ó���Ѵ�.
	strcpy( szT, pMapFile );
	make_pure_path( szT );
	nR = cod_info( szT, &cm );
	if( nR , 0 )
		return( -1 );

	// ����� ������ ũ�⸦ ����Ѵ�.
	cm.d.dwSize  = sizeof( MyCoffDbg2Stt );
	cm.d.dwSize += sizeof( MyCoffDbg2FileStt  ) * cm.d.nTotalFileEnt;
	cm.d.dwSize += sizeof( MyCoffDbg2FuncStt  ) * cm.d.nTotalFuncEnt;
	cm.d.dwSize += sizeof( MyCoffDbg2LocalStt ) * cm.d.nTotalLocalEnt;
	cm.d.dwSize += sizeof( int ) * cm.d.nTotalFuncEnt * 2;
	cm.d.dwSize += sizeof( struct _MY_IMAGE_LINENUMBER ) * cm.d.nTotalLineEnt;
	cm.d.dwSize += cm.d.nStrTblSize;
	strcpy( cm.d.szMagicStr, MY_COFF_DBG2_MAGIC_STR );

	// ����� ������ �����Ѵ�.
	if( pDbgFile == NULL )
	{
		strcpy( szT, pMapFile );
		nR = strlen( szT );
		strcpy( &szT[nR-3], "DBG" );
		pDbgFile = szT;
		nCreateNew = 1;
	}	
	else
		nCreateNew = 0;
	write_debug_info( &cm, pDbgFile, nCreateNew );
	   
	// ó�� ����� ����Ѵ�.
	CM_PRINTF( "HASH  TABLE  : %d Collisions, %d Max Collisions\n", cm.nTotalCollision, cm.nMaxCollision );
	CM_PRINTF( "FILE  TABLE  : %d Processed, Total %d COD files.\n", cm.nTotalProcessedFiles, cm.d.nTotalFileEnt );
	CM_PRINTF( "FUNC  TABLE  : %d functions, %d unlinked functtions.\n", cm.d.nTotalFuncEnt, cm.nTotalUselessFunction );
	CM_PRINTF( "LOCAL TABLE  : %d local entries.\n", cm.d.nTotalLocalEnt );
	CM_PRINTF( "LINE  TABLE  : %d line numbers.\n", cm.d.nTotalLineEnt );
	CM_PRINTF( "SYMB  TABLE  : %d symbols, ( Size = %d )\n", cm.nTotalSymIndex, cm.d.nStrTblSize );
	CM_PRINTF( "DEBUG INFO  : %d bytes.\n", cm.d.dwSize );
	CM_PRINTF( "LINKER BASE : 0x%08X\n",  cm.dwLinkerBaseAddr );

	// ���α׷� ����� ��
	/*
	{
		int nI;
		MyCoffDbg2FileStt	*pFile;
		MyCoffDbg2FuncStt	*pFunc;
		CodeMapStt			*pCM;
		pCM = &cm;		
		for( nI = 0; nI < pCM->d.nTotalFileEnt; nI++ )
		{
			pFile = &pCM->d.pFileTbl[nI];
			CM_PRINTF( "[%2d] 0x%08X (%d bytes, %d lines ) %s\n", nI, pFile->dwAddr, pFile->dwSize, pFile->nTotalLine, &pCM->d.pStrTbl[ pFile->nNameIndex ] );
		}
		CM_PRINTF( "\n" );
		for( nI = 0; nI < pCM->d.nTotalFuncEnt; nI++ )
		{
			pFunc = &pCM->d.pFuncTbl[ pCM->d.pFuncAddrIndex[nI]];
			pFile = &pCM->d.pFileTbl[ pFunc->nFileIndex ];
			CM_PRINTF( "[%2d] 0x%08X (%d bytes) %s/%s\n", nI, pFunc->dwAddr, pFunc->dwSize, &pCM->d.pStrTbl[ pFile->nNameIndex ], &pCM->d.pStrTbl[ pFunc->nNameIndex ] );
		}
	}
	*/

  return( 0 );
}

