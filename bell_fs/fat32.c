#include "vfs.h"

static int		fat32_vnode_remove_entry ( VNodeStt		*pVNode			);
static int		recover_fat12_cheat		 ( VFSStt		*pVFS			);
static int		create_fat32_fs			 ( DWORD		dwParam			);
static int		fat32_delete_vnode		 ( VNodeStt		*pVNode			);
static int		recover_fat16_cheat		 ( VFSStt		*pVFS			);
static int		fat32_vnode_remove		 ( VNodeStt 	*pParentNode,	char			*pS 		);
static int		fat32_calc_filesize 	 ( VNodeStt 	*pV,			FAT32FileStt	*pFSSD		);
static int		fat32_read_dirent		 ( VNodeStt 	*pDirNode,		DirEntInfoStt	*pEntInfo	);
static int		cheat_fat16 			 ( VFSStt		*pVFS,			FAT32PrivateStt *pPrivate,	int 	  nRootBlocks 	);
static int		cheat_fat12 			 ( VFSStt		*pVFS,			FAT32PrivateStt *pPrivate,	int 	  nRootSectors	);
static int		fat32_vnode_write		 ( VNodeStt 	*pVNode,		DWORD			dwOffs, 	char	  *pBuff, 		int 	nSize	);
static int		fat32_vnode_get_info     ( VNodeStt	    *pVNode,        char            *pName,     DIRENTStt *pDirEnt );

static long 	fat32_vnode_lseek		 ( VNodeStt 	*pVNode,		long			lOffset,	int 	  nOrigin 		);
static DWORD	get_next_cluster_number  ( VNodeStt 	*pVNode,		DWORD			dwCluster	);
static VNodeStt *fat32_new_vnode		 ( VNodeManStt	*pVNodeMan		);
static VNodeStt *fat32_vnode_open		 ( VNodeStt		*pParentNode,	char			*pName,		DWORD	  dwMode		);
static VNodeStt *fat32_vnode_create		 ( VNodeStt		*pParentNode,	char			*pName,		DWORD	  dwType		);
static VNodeStt *fat32_vnode_create_entry( VNodeStt		*pParentNode,	char			*pName,		DWORD	  dwType		);

// dbs for ram disk
static unsigned char fat32_dbs[] = {
0xEB, 0x58, 0x90, 0x4D, 0x53, 0x57, 0x49, 0x4E, 0x34, 0x2E, 0x31, 0x00, 0x02, 0x08, 0x20, 0x00,
0x02, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x3F, 0x00, 0xFF, 0x00, 0x3F, 0x00, 0x00, 0x00,
0x4B, 0xF5, 0x7F, 0x00, 0xF6, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
0x01, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x80, 0x00, 0x29, 0xE0, 0x11, 0x56, 0x26, 0x57, 0x49, 0x4E, 0x39, 0x38, 0x20, 0x20, 0x20, 0x20,
0x20, 0x20, 0x46, 0x41, 0x54, 0x33, 0x32, 0x20, 0x20, 0x20, 0xFA, 0x33, 0xC9, 0x8E, 0xD1, 0xBC,
0xF8, 0x7B, 0x8E, 0xC1, 0xBD, 0x78, 0x00, 0xC5, 0x76, 0x00, 0x1E, 0x56, 0x16, 0x55, 0xBF, 0x22,
0x05, 0x89, 0x7E, 0x00, 0x89, 0x4E, 0x02, 0xB1, 0x0B, 0xFC, 0xF3, 0xA4, 0x8E, 0xD9, 0xBD, 0x00,
0x7C, 0xC6, 0x45, 0xFE, 0x0F, 0x8B, 0x46, 0x18, 0x88, 0x45, 0xF9, 0x38, 0x4E, 0x40, 0x7D, 0x25,
0x8B, 0xC1, 0x99, 0xBB, 0x00, 0x07, 0xE8, 0x97, 0x00, 0x72, 0x1A, 0x83, 0xEB, 0x3A, 0x66, 0xA1,
0x1C, 0x7C, 0x66, 0x3B, 0x07, 0x8A, 0x57, 0xFC, 0x75, 0x06, 0x80, 0xCA, 0x02, 0x88, 0x56, 0x02,
0x80, 0xC3, 0x10, 0x73, 0xED, 0xBF, 0x02, 0x00, 0x83, 0x7E, 0x16, 0x00, 0x75, 0x45, 0x8B, 0x46,
0x1C, 0x8B, 0x56, 0x1E, 0xB9, 0x03, 0x00, 0x49, 0x40, 0x75, 0x01, 0x42, 0xBB, 0x00, 0x7E, 0xE8,
0x5F, 0x00, 0x73, 0x26, 0xB0, 0xF8, 0x4F, 0x74, 0x1D, 0x8B, 0x46, 0x32, 0x33, 0xD2, 0xB9, 0x03,
0x00, 0x3B, 0xC8, 0x77, 0x1E, 0x8B, 0x76, 0x0E, 0x3B, 0xCE, 0x73, 0x17, 0x2B, 0xF1, 0x03, 0x46,
0x1C, 0x13, 0x56, 0x1E, 0xEB, 0xD1, 0x73, 0x0B, 0xEB, 0x27, 0x83, 0x7E, 0x2A, 0x00, 0x77, 0x03,
0xE9, 0xFD, 0x02, 0xBE, 0x7E, 0x7D, 0xAC, 0x98, 0x03, 0xF0, 0xAC, 0x84, 0xC0, 0x74, 0x17, 0x3C,
0xFF, 0x74, 0x09, 0xB4, 0x0E, 0xBB, 0x07, 0x00, 0xCD, 0x10, 0xEB, 0xEE, 0xBE, 0x81, 0x7D, 0xEB,
0xE5, 0xBE, 0x7F, 0x7D, 0xEB, 0xE0, 0x98, 0xCD, 0x16, 0x5E, 0x1F, 0x66, 0x8F, 0x04, 0xCD, 0x19,
0x41, 0x56, 0x66, 0x6A, 0x00, 0x52, 0x50, 0x06, 0x53, 0x6A, 0x01, 0x6A, 0x10, 0x8B, 0xF4, 0x60,
0x80, 0x7E, 0x02, 0x0E, 0x75, 0x04, 0xB4, 0x42, 0xEB, 0x1D, 0x91, 0x92, 0x33, 0xD2, 0xF7, 0x76,
0x18, 0x91, 0xF7, 0x76, 0x18, 0x42, 0x87, 0xCA, 0xF7, 0x76, 0x1A, 0x8A, 0xF2, 0x8A, 0xE8, 0xC0,
0xCC, 0x02, 0x0A, 0xCC, 0xB8, 0x01, 0x02, 0x8A, 0x56, 0x40, 0xCD, 0x13, 0x61, 0x8D, 0x64, 0x10,
0x5E, 0x72, 0x0A, 0x40, 0x75, 0x01, 0x42, 0x03, 0x5E, 0x0B, 0x49, 0x75, 0xB4, 0xC3, 0x03, 0x18,
0x01, 0x27, 0x0D, 0x0A, 0x49, 0x6E, 0x76, 0x61, 0x6C, 0x69, 0x64, 0x20, 0x73, 0x79, 0x73, 0x74,
0x65, 0x6D, 0x20, 0x64, 0x69, 0x73, 0x6B, 0xFF, 0x0D, 0x0A, 0x44, 0x69, 0x73, 0x6B, 0x20, 0x49,
0x2F, 0x4F, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0xFF, 0x0D, 0x0A, 0x52, 0x65, 0x70, 0x6C, 0x61,
0x63, 0x65, 0x20, 0x74, 0x68, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6B, 0x2C, 0x20, 0x61, 0x6E, 0x64,
0x20, 0x74, 0x68, 0x65, 0x6E, 0x20, 0x70, 0x72, 0x65, 0x73, 0x73, 0x20, 0x61, 0x6E, 0x79, 0x20,
0x6B, 0x65, 0x79, 0x0D, 0x0A, 0x00, 0x00, 0x00, 0x49, 0x4F, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
0x53, 0x59, 0x53, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x20, 0x20, 0x20, 0x53, 0x59, 0x53, 0x7E, 0x01,
0x00, 0x57, 0x49, 0x4E, 0x42, 0x4F, 0x4F, 0x54, 0x20, 0x53, 0x59, 0x53, 0x00, 0x00, 0x55, 0xAA
};

// 한 문자를 받아들여 대문자로 리턴한다.
static char cUpperChar( char ch )
{
	if( ch >= 'a' && ch <= 'z' )
	{
		return( ch - 'a' + 'A' );
	}
	return( ch );
}

// FAT32 long filename entry의 checksum을 만든다.
static BYTE byFAT32_chksum( BYTE *pShortName )
{
	int  nI;
	BYTE x, y;

	x = 0;
	for( nI = 0; nI < 8+3; nI++ )
	{
		y = pShortName[nI];

		_asm {
			PUSH EAX
				 MOV  AH, x
				 MOV  AL, y
				 ROR  AH, 1
				 ADD  AH, AL
				 MOV  x,  AH
			POP  EAX
		}		 
	}

	//JPRINTF( "checksum = 0x%02X\n", x );

	return( x );
}

// serial은 ~뒤에 붙는 번호  (성공, 실패여부 리턴)
static int nMakeShortName( char *pShortName, char *pRealShortName, BYTE *pChksum, char *pLongName, int nSerial )
{
	int		nI, nX;
	char	szT[16];
	int		nLongNameLen;		// long name에서 확장자 부분을 제외한 이름의 길이
	char	ch, szExt[4];

	// 공백 8 + 3을 채운다.	(11바이트) 
	strcpy( pShortName, "           " );		
	
	// 마지막에 등장하는 '.'을 찾아 확장자를 설정한다.
	for( nI = strlen( pLongName )-1; nI >0; nI-- )
	{
		if( pLongName[nI] == '.' )
		{
			nI++;
			break;
		}
	}
	
	if( nI > 0 )	
	{	// 확장자를 제외한 이름의 길이
		nLongNameLen = nI-1;	// 위에서 ++을 해주었기 때문에 -1을 해 준다.		

		// 대문자로 만들어서 확장자 3자를 복사한다.
		for( nX = 0; nX < 3; nX++ )
		{
			ch = cUpperChar( pLongName[nI+nX] );
			szExt[nX]			= ch;
			szExt[nX+1]			= 0;
			pShortName[8+nX]	= ch;
		}
	}
	else	//  no extension
	{
		szExt[0] = 0;
		nLongNameLen = strlen( pLongName );
	}


	// 8에서 ~와 그 뒤에 따라온 숫자를 뺀만큼의 이름을 복사
	if( nLongNameLen > 8 )		// pLongName이 8자 이상이면 번호를 붙여준다.
	{
		sprintf( szT, "~%d", nSerial );
		nI = strlen( szT );
		nX = 8 - nI;	
		
		for( nI = 0; nI < nX; nI++ )
		{
			ch = cUpperChar( pLongName[ nI ] );
			pShortName[nI]			= ch;
			pRealShortName[nI]		= ch;
			pRealShortName[nI+1]	= 0;
		}
		
		for( nX = 0; nI < 8 && szT[nX] != 0; nI++, nX++ )
		{
			ch = cUpperChar( szT[ nX ] );
			pShortName[nI]			= ch;
			pRealShortName[nI]		= ch;
			pRealShortName[nI+1]	= 0;
		}
	}
	else	// 8자 이하면 대문자로 바꾸어 그냥 이름을 복사한다.
	{
		for( nI = 0; nI < 8 && pLongName[nI] != '.' && pLongName[nI] != 0; nI++ )
		{
			ch = cUpperChar( pLongName[nI] );
			pShortName[nI]       = ch;
			pRealShortName[nI]   = ch;
			pRealShortName[nI+1] = 0;
		}
	}

	// 체크섬을 만든다.
	pChksum[0] = byFAT32_chksum( pShortName );

	return( 0 );  // 성공 
}		

// 디렉토리 엔트리가 short이면 1을 리턴하고 아니면 0을 리턴한다.
static int nIsShortEnt( FAT32DirEntStt *pEnt )
{
	if( pEnt->u.l.wZero != 0 || pEnt->u.s.dwFileSize == 0 )
		return( 1 );

	return(0);
}

// 데이터 클러스터 번호를 블록번호(섹터)로 변경한다.
DWORD fat32_cluster_to_block( FAT32PrivateStt *pPrivate, DWORD dwCluster )
{
	DWORD dwBlock;

	if( dwCluster < pPrivate->dwRootDirStrtCluster )
	{	// 사이즈가 0인 파일은 클러스터 값이 0일 수도 있으므로 에러는 아니다.
		return( 0 );		
	}

	if( pPrivate->bySectorsPerCluster == 0 || pPrivate->dwRootLoc == 0 )
		return( 0 );

	dwBlock = (DWORD)( dwCluster - pPrivate->dwRootDirStrtCluster ) * (DWORD)pPrivate->bySectorsPerCluster + pPrivate->dwRootLoc;

	return( dwBlock );
}	

typedef enum {
	_MDER_OK	    = 0,	// 잘 조합되었다.
	_MDER_OK_LONG_ABORT,	// long name을 조합하다가 갑자기 short name이 나왔다.  지금까지 조합된 long name을 버리고 short name만 처리한다. 
	_MDER_ERROR,			// 에러가 발생했으므로 중단한다.
	_MDER_SKIP,				// 건너뛴다.
	_MDER_CONTINUE,			// 다음 클러스터를 읽어서 계속 조합해 나가야 한다. 
	_MDER_EMPTY,			// 지워진 엔트리
	_MDER_END,				// 0으로 시작하는 마지막 엔트리 (종료한다.)

	ENDOF_MDER
} MAME_DIR_ENT_RESULT_TAG;

// short dir entry에서 파일명을 구한다.  (이름의 길이를 리턴한다)
static int nGetShortEntryName( char *pFileName, FAT32DirEntStt *pDirEnt )
{
	int		nK, nI;
	char	*pF, *pE;

	pFileName[0] = 0;
	pF = pDirEnt->u.s.name;
	pE = pDirEnt->u.s.ext;

	// filename (8)
	for( nK = 0, nI = 0; nI < 8; nI++ )
	{
		if( pF[nI] == 0x20 || pF[nI] == 0 )
			break;

		pFileName[nK++] = pF[nI];
		pFileName[nK] = 0;
	}

	
	// 확장자가 있으면 점을 추가한다.
	if( pE[0] != 0 && pE[0] != 0x20 )
	{
		pFileName[nK++] = '.';
		pFileName[nK] = 0;
	}
	else
		return( nK ); // 확장자가 없으면 .이 붙지 않고 그냥 리턴된다.

	// 확장자를 붙인다.
	for( nI = 0; nI < 3; nI++ )
	{
		if( pE[nI] == 0x20 || pE[nI] == 0 )
			break;

		pFileName[nK++] = pE[nI];
		pFileName[nK] = 0;
	}						

	return( nK );
}

typedef struct LongNameSubStrEntTag {
	char szSubStr[32];
};
typedef struct LongNameSubStrEntTag LongNameSubStrEntStt;

typedef struct LongNameTag {
	int						nTotal;
	LongNameSubStrEntStt	ent[20];
};
typedef struct LongNameTag LongNameStt;

// lomng directory entry에서 name 13 또는 26바이트를 꺼낸다. (스트링의 길이를 리턴한다.)
static int nGetLongEntrySubName( char *pSubStr, FAT32DirEntStt *pDirEnt )
{
	int nI, nK;

	pSubStr[0] = 0;

	// 첫 번째 이름 부분
	for( nK = nI = 0; nI < 5*2; )
	{
		if( pDirEnt->u.l.name[nI] == 0 || pDirEnt->u.l.name[nI] == -1 )
			return( nK );

		pSubStr[nK++] = pDirEnt->u.l.name[nI++];
		pSubStr[nK]   = pDirEnt->u.l.name[nI];
		
		// 2 byte 문자인 경우
		if( pDirEnt->u.l.name[nI] != 0 )
		{
			nK++;
			pSubStr[nK] = 0;
		}	 		
		nI++;
	}

	// 두 번째 이름 부분
	for( nI = 0; nI < 6*2; )
	{
		if( pDirEnt->u.l.name2[nI] == 0 || pDirEnt->u.l.name2[nI] == -1 )
			return( nK );

		pSubStr[nK++] = pDirEnt->u.l.name2[nI++];
		pSubStr[nK]   = pDirEnt->u.l.name2[nI];
		
		// 2 byte 문자인 경우
		if( pDirEnt->u.l.name2[nI] != 0 )
		{
			nK++;
			pSubStr[nK] = 0;
		}	 		
		nI++;
	}

	// 세 번째 이름 부분
	for( nI = 0; nI < 2*2; )
	{
		if( pDirEnt->u.l.name3[nI] == 0 || pDirEnt->u.l.name3[nI] == -1 )
			return( nK );

		pSubStr[nK++] = pDirEnt->u.l.name3[nI++];
		pSubStr[nK]   = pDirEnt->u.l.name3[nI];
		
		// 2 byte 문자인 경우
		if( pDirEnt->u.l.name3[nI] != 0 )
		{
			nK++;
			pSubStr[nK] = 0;
		}	 		
		nI++;
	}	 

	return( nK );
}

// 디렉토리 엔트리의 이름을 조합한다.
static int _nMakeDirEntName( LongNameStt *pLName, FAT32DirEntStt *pDirEnt, int nMax, int nPrevResult, int *pUsedEntries )
{
	int nIndex, nX, nI, nLength, nNeededEntries;

	// 전혀 사용된 적이 없는 엔트리인가?
	if( pDirEnt->u.s.name[0] == 0 )
		return( _MDER_END );

	// 지워진 엔트리인가?
	if( (UCHAR)pDirEnt->u.s.name[0] == (UCHAR)0xE5 )
	{
		pUsedEntries[0] = 1;
		return( _MDER_EMPTY );
	}

	// short entry인가? 
	if( nIsShortEnt( pDirEnt ) )
	{
		if( nPrevResult == _MDER_CONTINUE )
			return( _MDER_ERROR );	// long entry name을 조합하다 말았는데 갑자기 short entry가 등장했다.

		// short entry name을 얻는다.
		nLength = nGetShortEntryName( pLName->ent[0].szSubStr, pDirEnt );
		if( nLength <= 0 )
			return( _MDER_ERROR );

		pUsedEntries[0] = 1;
		pLName->nTotal = 1;
		return( _MDER_OK );
	}

	// 필요한 long entry의 개수를 구한다.
	nNeededEntries = (int)( (UCHAR)pDirEnt->u.l.sequence & (UCHAR)0x3F );	

	// long entry name을 조합한다.
	for( nI = 0; nI < nMax && nI < nNeededEntries; nI++ )
	{	
		// 인덱스를 구한다.
		nIndex = (int)( (UCHAR)pDirEnt[nI].u.l.sequence	& (UCHAR)0x3F );
		
		//  Entry의 sub name str을 구한다.
		nX = nGetLongEntrySubName( pLName->ent[nIndex-1].szSubStr, &pDirEnt[nI] );
		if( nX < 0 )
			return( _MDER_ERROR );

		pUsedEntries[0]++;
		pLName->nTotal++;
	}

	// 마지막 sequence까지 처리되었다. 
	if( nI == nNeededEntries )
		return( _MDER_OK );
	else  // 처리할 것이 더 남아있다.
		return( _MDER_CONTINUE );
}

// 디렉토리 엔트리의 이름을 조합한다.  pLName에는 중간결과가 들어간다.
static int nMakeDirEntName( LongNameStt *pLName, char *pFileName, FAT32DirEntStt *pDirEnt, int nMax, int nPrevResult, int *pUsedEntries )
{
	int nR, nI;

	nR = _nMakeDirEntName( pLName, pDirEnt, nMax, nPrevResult, pUsedEntries );
	if( nR != _MDER_OK )
		return( nR );

	// SubString을 모두 연결한다.
	pFileName[0] = 0;
	for( nI = 0; nI	< pLName->nTotal; nI++ )
		strcat( pFileName, pLName->ent[nI].szSubStr );

	return( nR );
}	

// long name entry를 만든다.
static char *pMake_fat32_LongEntry( FAT32DirEntStt *pEnt, BYTE byChksum, int nSerial, char *pS )
{
	int nI, nJ, nX;						

	pEnt->u.l.sequence	= (BYTE)nSerial;
	pEnt->u.l.byAttr	= 0x0F;
	pEnt->u.l.byType	= 0x00;
	pEnt->u.l.byChkSum	= byChksum;
	pEnt->u.l.wZero		= 0;			

	// 이름의 첫 부분
	nX = 0;
	for( nI = 0; nI < 5; nI++)
	{
		if( pS[nX] == 0 )
		{
			pEnt->u.l.name[nI*2]	= (char)0xFF;
			pEnt->u.l.name[nI*2+1]	= (char)0xFF;
		
		}
		else
		{
			pEnt->u.l.name[nI*2]	= pS[nX++];
			pEnt->u.l.name[nI*2+1]	= 0;
		}
	}

	// 이름의 둘째 부분
	for( nJ = 0; nJ < 6; nJ++ )
	{
		if( pS[nX] == 0 )
		{
			pEnt->u.l.name2[nJ*2]	= (char)0xFF;
			pEnt->u.l.name2[nJ*2+1]	= (char)0xFF;
		}
		else
		{
			pEnt->u.l.name2[nJ*2]	= pS[nX++];
			pEnt->u.l.name2[nJ*2+1]	= 0;
		}
	}

	// 이름의 세째 부분
	for( nJ = 0; nJ < 2; nJ++ )
	{
		if( pS[nX] == 0 )
		{
			pEnt->u.l.name3[nJ*2]	= (char)0xFF;
			pEnt->u.l.name3[nJ*2+1]	= (char)0xFF;
		}
		else
		{
			pEnt->u.l.name3[nJ*2]	= pS[nX++];
			pEnt->u.l.name3[nJ*2+1]	= 0;
		}
	}			  
	
	return( &pS[nX] );
}

// 디렉토리 엔트리를 만들고 사용한 슬롯의 개수를 리턴한다.
static int fat32_make_dirent( VNodeStt *pVNode, FAT32DirEntStt *pEnt, char *pFileName, DWORD dwType )
{
	BYTE			chksum;
	SHORTDIRENT		*pShortEnt;
	int				nSlot, nI, nR, nSerial;
	char			szShortName83[12], szRealShortName[13], *pS;

	nSlot = 0;

	if( pFileName == NULL || pFileName[0] == 0 )
		return( -1 );

	nI = strlen( pFileName );

	// 사용할 슬롯의 개수를 구한다.
	nSlot = (( nI + 12 ) / 13) + 1;
	// 슬롯 배열을 0으로 초기화한다.
	memset( pEnt, 0, sizeof( FAT32DirEntStt ) * nSlot );

	// Short File Name을 만든다.
	for( nI = 1; nI < 9999; nI++ ) // ~1부터 시작한다.
	{	// szShortName83 = "TESTDI_1   ", szReadShortName = "TESTDI_1" <- no space is appended
		nR = nMakeShortName( szShortName83, szRealShortName, &chksum, pFileName, nI );
		if( nR == -1 )
			return( -1 );
		
		// find short filename 
		nR = fat32_find_dirent( pVNode, szRealShortName );
		if( nR == 0 )
			break;
	}
	if( nI == 9999 )
		return( -1 );

	// Short Name Entry를 만든다.
	pShortEnt = &pEnt[nSlot-1].u.s;
	memcpy( pShortEnt->name, szShortName83, 8 + 3 );
	pShortEnt->byAttr				= (UCHAR)dwType;
	pShortEnt->wEAHandle			= 0;           
	pShortEnt->dwFileSize			= 0;
	pShortEnt->wCreateTime			= 0;         
	pShortEnt->wCreateDate			= 0;         
	pShortEnt->wStartCluster		= 0;       
	pShortEnt->wLastAccessDate		= 0;     
	
	// Long Name Entry를 만든다.
	pS = pFileName;
	for( nSerial = 1, nI = nSlot-2; nI >= 0; nI--, nSerial++ )
	{
		if( nI == 0 )
			nSerial |= (int)0x40;		// mark this is the last slot

		pS = pMake_fat32_LongEntry( &pEnt[nI], chksum, nSerial, pS );	// 마지막 엔트리
	}

	return( nSlot );
}
/*
// 2003-08-08 수정하기 이전 버전.
// get next cluster chain in the fat16 filesystem.
static DWORD get_next_cluster_number( VNodeStt *pVNode, DWORD dwCluster )
{
	BlkDevObjStt		*pDevObj;
	unsigned short		*pFAT16;
	FAT32PrivateStt		*pPrivate;
	CacheEntStt			*pCacheEnt;
	int					nFatEntriesPerBlock;
	DWORD				dwNextCluster;
	DWORD				dwBlock;	
	DWORD				*pFAT32;
	DWORD				dwAlignedBlock, dwCIndex, dwIndex;
	
	pPrivate = pVNode->pVFS->pPrivate;
	pDevObj  = pVNode->pVFS->pDevObj;

	if( pVNode->pVFS->dwType == VFS_TYPE_FAT16 )
		nFatEntriesPerBlock = 256;
	else
		nFatEntriesPerBlock = 128;

	dwNextCluster = 0;
	
	// Block 번호를 클러스터 당 섹터 수로 정렬한다.
	dwBlock        = dwCluster / nFatEntriesPerBlock;			
	dwAlignedBlock = (dwBlock / (DWORD)pPrivate->bySectorsPerCluster);
	dwAlignedBlock *= (DWORD)pPrivate->bySectorsPerCluster; 

	dwCIndex = dwAlignedBlock * (DWORD)nFatEntriesPerBlock;		// start index of the buffer
	dwIndex  = dwCluster - dwCIndex;							// index in the buffer

	// get FAT buffer address
	pCacheEnt = (CacheEntStt*)get_cache_entry( pVNode->pVFS->pCache, pDevObj, 
		pPrivate->dwFATLoc + dwAlignedBlock, (int)pPrivate->bySectorsPerCluster );
	if( pCacheEnt == NULL )
		return( 0 );		// reading failed!
		
	if( pVNode->pVFS->dwType == VFS_TYPE_FAT16 )
	{
		BlkDevObjStt	*pObj;
		F16PartStt		*pF16Part;

		pObj		= pVNode->pVFS->pDevObj;
		pF16Part	= pObj->pPtr;

		// get next root cluster ?
		if( 2 <= dwIndex && dwIndex < 2 + (DWORD)pF16Part->nRootClusters )
		{	// is dwIndex is the last root cluster
			if( dwIndex == 2 + (DWORD)pF16Part->nRootClusters -1 )
				return( (DWORD)0x0FFFFFFF );
			else
				return( dwIndex + 1 );
		}
		else // normal data cluster
			dwIndex -= (DWORD)pF16Part->nRootClusters;

		pFAT16 = (unsigned short*)pCacheEnt->pBuff;
		dwNextCluster = pFAT16[dwIndex];
		if( dwNextCluster >= 0xFFF7 )		// 마지막 클러스터 이면 '0x0FFF0000'을 더한다.
			dwNextCluster += 0x0FFF0000;
		else if( 0 < dwNextCluster )
			dwNextCluster += (DWORD)pF16Part->nRootClusters;
	}
	else
	{
		pFAT32 = (DWORD*)pCacheEnt->pBuff;
		dwNextCluster = pFAT32[dwIndex];
	}

	return( dwNextCluster );
}
*/

// 2003-08-08 수정.  파일 시스템 클러스터 기준의 캐시를 다시 떼어내기 위한 것.
// get next cluster chain in the fat16 filesystem.
static DWORD get_next_cluster_number( VNodeStt *pVNode, DWORD dwCluster )
{
	unsigned short		*pFAT16;
	DWORD				dwBlock;	 // dwCluster가 위치한 상대 블록 번호.  (FAT시작 위치부터)
	DWORD				*pFAT32;
	BlkDevObjStt		*pDevObj;
	FAT32PrivateStt		*pPrivate;
	DWORD				dwNextCluster;
	DWORD				dwCIndex, dwIndex;
	int					nFatEntriesPerBlock;
	
	pPrivate = pVNode->pVFS->pPrivate;
	pDevObj  = pVNode->pVFS->pDevObj;

	if( pVNode->pVFS->dwType == VFS_TYPE_FAT16 )
		nFatEntriesPerBlock = 256;
	else
		nFatEntriesPerBlock = 128;

	dwNextCluster = 0;
	
	// Block 번호를 클러스터 당 섹터 수로 정렬한다.
	dwBlock  = dwCluster / nFatEntriesPerBlock;			
	dwCIndex = dwBlock * (DWORD)nFatEntriesPerBlock;
	dwIndex  = dwCluster - dwCIndex;				

	// 캐시되고 있는 블록 버퍼의 주소를 알아낸다.
	pFAT16 = (UINT16*)get_blk_cache_buff( pDevObj, pPrivate->dwFATLoc + dwBlock );
	if( pFAT16 == NULL )
		return( 0 );	// 버퍼링에 에러가 발생했다.

	// get FAT buffer address
	if( pVNode->pVFS->dwType == VFS_TYPE_FAT16 )
	{
		BlkDevObjStt	*pObj;
		F16PartStt		*pF16Part;

		pObj	 = pVNode->pVFS->pDevObj;
		pF16Part = pObj->pPtr;

		// get next root cluster ?
		if( 2 <= dwIndex && dwIndex < 2 + (DWORD)pF16Part->nRootClusters )
		{	
			if( dwIndex == 2 + (DWORD)pF16Part->nRootClusters -1 )
				return( (DWORD)0x0FFFFFFF );
			else
				return( dwIndex + 1 );
		}
		else // normal data cluster
			dwIndex -= (DWORD)pF16Part->nRootClusters;

		dwNextCluster = pFAT16[ dwIndex ];
		if( dwNextCluster >= 0xFFF7 )		// 마지막 클러스터 이면 '0x0FFF0000'을 더한다.
			dwNextCluster += 0x0FFF0000;
		else if( 0 < dwNextCluster )
			dwNextCluster += (DWORD)pF16Part->nRootClusters;
	}
	else
	{
		pFAT32 = (DWORD*)pFAT16;
		dwNextCluster = pFAT32[ dwIndex ];
	}

	return( dwNextCluster );
}

// dwCluster에 대한 다음 FAT32 Chain을 설정한다.
static int set_next_cluster_number( VNodeStt *pVNode, DWORD dwCluster, DWORD dwNextCluster )
{
	unsigned short	*pFAT16;
	FAT32PrivateStt *pPrivate;
	BlkDevObjStt	*pDevObj;
	//CacheEntStt		*pCacheEnt;
	DWORD			dwBlock, *pFAT32;
	int				nFatEntriesPerBlock;
	DWORD			dwCIndex, dwIndex;
	
	pPrivate = pVNode->pVFS->pPrivate;
	pDevObj  = pVNode->pVFS->pDevObj;

	if( pVNode->pVFS->dwType == VFS_TYPE_FAT16 )
		nFatEntriesPerBlock = 256;
	else
		nFatEntriesPerBlock = 128;

	// Block 번호를 클러스터 당 섹터 수로 정렬한다.
	dwBlock  = dwCluster / nFatEntriesPerBlock;			
	dwCIndex = dwBlock * (DWORD)nFatEntriesPerBlock;
	dwIndex  = dwCluster - dwCIndex;				

	// 캐시되고 있는 블록 버퍼의 주소를 알아낸다.
	pFAT16 = (UINT16*)get_blk_cache_buff( pDevObj, pPrivate->dwFATLoc + dwBlock );
	if( pFAT16 == NULL )
		return( 0 );	// 버퍼링에 에러가 발생했다.

	else if( pVNode->pVFS->dwType == VFS_TYPE_FAT16 )
	{
		BlkDevObjStt	*pObj;
		F16PartStt		*pF16Part;

		pObj		= pVNode->pVFS->pDevObj;
		pF16Part	= pObj->pPtr;

		dwIndex -= (DWORD)pF16Part->nRootClusters;
		pFAT16[dwIndex] = (unsigned short)( dwNextCluster - (DWORD)pF16Part->nRootClusters );
	}
	else
	{
		pFAT32 = (DWORD*)pFAT16;
		pFAT32[dwIndex] = dwNextCluster;
	}

	return( 0 );
}

// allocate free cluster
static DWORD get_free_cluster( VFSStt *pVFS  )
{
	unsigned short	*pFAT16;
	BlkDevObjStt	*pDevObj;
	FAT32PrivateStt *pPrivate;
	//CacheEntStt		*pCacheEnt;
	DWORD			dwFreeCluster;
	DWORD			dwBlock, *pFAT32;
	int				nI, nK, nBlocks, nEntries;
		
	pPrivate = pVFS->pPrivate;
	pDevObj  = pVFS->pDevObj;

	dwBlock  = pPrivate->dwFATLoc;
	nBlocks  = (int)pPrivate->dwBigSectorsPerFat;

	for( nI = 0; nI < nBlocks; nI ++ )
	{
		// 캐시되고 있는 블록 버퍼의 주소를 알아낸다.
		pFAT16 = (UINT16*)get_blk_cache_buff( pDevObj, dwBlock + nI );
		if( pFAT16 == NULL )
			return( 0 );	// 버퍼링에 에러가 발생했다.

		if( pVFS->dwType == VFS_TYPE_FAT16 )
		{
			nEntries =  512 / 2;
			for( nK = 0; nK < nEntries; nK++ )
			{
				if( pFAT16[nK] == 0 )
				{
					F16PartStt	*pF16Part;

					pF16Part = pVFS->pDevObj->pPtr;
					dwFreeCluster = (DWORD)nK + ( nI * 256 ); // not nI * nEntries !!!
					dwFreeCluster += pF16Part->nRootClusters;
					return( dwFreeCluster );
				}
			}	
		}
		else
		{
			pFAT32 = (DWORD*)pFAT16;
			nEntries = 512 / 4;
			for( nK = 0; nK < nEntries; nK++ )
			{
				if( pFAT32[nK] == 0 )
				{
					dwFreeCluster = (DWORD)nK + ( nI * 128 ); // not nI * nEntries !!!
					return( dwFreeCluster );
				}
			}	
		}
	}

	return( 0 );
}

// rewrite vnode's short entry to the parent directory
static int fat32_write_short_entry( VNodeStt *pVNode )
{
	int				nR;
	VNodeStt		*pDir;
	FAT32FileStt	*pFSSD;
	long			lR, lOffset;
	DWORD			dwStartCluster;

	nR    = 0;
	pFSSD = pVNode->pFSSD;
	
	// when normal file is closed its entry size field must be updated if it was opened by RW mode.
	pDir = pVNode->pParentVNode;
	if( pDir != NULL && !(pVNode->dwType & FT_DIRECTORY ) )
	{
		lOffset = (long)pFSSD->ei.nShortEntOffset;
		lR = fat32_vnode_lseek( pDir, lOffset, FSEEK_SET );
		if( lR == lOffset )
		{
			pFSSD->ei.short_ent.dwFileSize = (DWORD)pFSSD->lFileSize;

			dwStartCluster			= (DWORD)pFSSD->ei.short_ent.wEAHandle;
			dwStartCluster			= (DWORD)( dwStartCluster << 16 );
			dwStartCluster			= dwStartCluster + (DWORD)pFSSD->ei.short_ent.wStartCluster;
		
			// FAT12, FAT16일 때 dirent의 start cluster에 nRootBlock을 빼주어야 한다.
			if( pVNode->pVFS->dwType == VFS_TYPE_FAT12 )
			{
				BlkDevObjStt	*pObj;
				FDDFat12Stt		*pFDDFat12;

				pObj		= pVNode->pVFS->pDevObj;
				pFDDFat12	= pObj->pPtr;
				if( 0x0FFFFFF7 <= dwStartCluster && dwStartCluster <= 0x0FFFFFFF )
					dwStartCluster &= 0xFFF;
				else if( 0 < dwStartCluster )
					dwStartCluster -= (DWORD)pFDDFat12->nRootBlocks;
			}
			else if( pVNode->pVFS->dwType == VFS_TYPE_FAT16 )
			{
				BlkDevObjStt	*pObj;
				F16PartStt		*pF16Part;
		
				pObj		= pVNode->pVFS->pDevObj;
				pF16Part	= pObj->pPtr;
				if( 0x0FFFFFF7 <= dwStartCluster && dwStartCluster <= 0x0FFFFFFF )
					dwStartCluster &= 0xFFFF;
				else if( 0 < dwStartCluster )
					dwStartCluster -= (DWORD)pF16Part->nRootClusters;
			}

			pFSSD->ei.short_ent.wEAHandle       = (unsigned short)( (DWORD)dwStartCluster >> 16 );
			pFSSD->ei.short_ent.wStartCluster	= (unsigned short)( dwStartCluster );
			
			// update directory entry
			nR = fat32_vnode_write( pDir, lOffset, (char*)&pFSSD->ei.short_ent, sizeof( SHORTDIRENT ) );
		}	
	}	

	return( nR );
}

// vnode close function
static int fat32_vnode_close( VNodeStt *pVNode )
{
	int				nR;
	
	if( pVNode == NULL )
		return( -1 );
	
	nR = 0;
	// rewrite short entry
	if( pVNode->pParentVNode != NULL )
		nR = fat32_write_short_entry( pVNode );

	// subfilesystem root vnode is not need to be unregistered. it is not registered.
	if( pVNode->pParentVNode != NULL )
		nR = unregister_vnode_hash( pVNode->pVFS->pVNodeMan, pVNode );

	// release structure
	nR = fat32_delete_vnode( pVNode );

	return( nR );
}

// pVNode에 연결된 nTh번째 클러스터 번호를 구한다.
static DWORD get_fat32_nth_cluster_number( VNodeStt *pVNode, int nTh )
{
	int				nI;
	DWORD			dwCluster;
	FAT32FileStt	*pFSSD;

	pFSSD = pVNode->pFSSD;
	// 시작 클러스터 번호
	dwCluster = pFSSD->dwStartCluster;

	for( nI = 0; nI < nTh; nI++ )
	{	// nTh만큼 다음 클러스터 번호를 구한다. 
		dwCluster = get_next_cluster_number( pVNode, dwCluster );
		if( dwCluster == 0 || dwCluster >= (DWORD)0x0FFFFFF7 )
			return( 0 );
	}

	return( dwCluster );
}

/*
extern DWORD dump_memory( BYTE *pBuff, DWORD dwSize );

// pVNode의 dwCluster의 dwClusterOffset에 pBuff에 데이터를 읽는다.
static int old_fat32_vnode_cluster_read( VNodeStt *pVNode, DWORD dwCluster, DWORD dwClusterOffset, char *pBuff, int nToRead )
{
	DWORD			dwBlock;
	char			*pBlockBuff;
	BlkDevObjStt	*pDevObj;
	FAT32PrivateStt *pPrivate;
	CacheEntStt		*pCacheEnt;
	
	pPrivate = pVNode->pVFS->pPrivate;
	pDevObj  = pVNode->pVFS->pDevObj;

	// 데이터 클러스터 번호를 블록 번호로 변경한다.
	dwBlock = fat32_cluster_to_block( pPrivate, dwCluster );

	// 지정된 클러스터의 캐쉬된 엔트리를 구한다.
	pCacheEnt = (CacheEntStt*)get_cache_entry( pVNode->pVFS->pCache, pDevObj, dwBlock, (int)pPrivate->bySectorsPerCluster );
	if( pCacheEnt == NULL )
		return( -1 );			
	
	pBlockBuff = (char*)pCacheEnt->pBuff;
	
	// 버퍼에 복사하고 Access를 1로 한다.
	memcpy( pBuff, &pBlockBuff[dwClusterOffset], nToRead );

	return( nToRead );
}
*/

// pVNode의 dwCluster의 dwClusterOffset에 pBuff에 데이터를 읽는다.
// fat32_vnode_read에서 호출된다.  
// dwClusterOffset + nToRead가 클러스터 크기를 넘는 일은 없다.
static int new_fat32_vnode_cluster_read( VNodeStt *pVNode, DWORD dwCluster, DWORD dwClusterOffset, char *pBuff, int nToRead )
{
	BCacheEntStt	*pEnt;
	BlkDevObjStt	*pDevObj;
	FAT32PrivateStt *pPrivate;
	char			*pBlockBuff;
	int				nCopied, nThisRead;
	DWORD			dwBlock, dwBlockOffset;
	
	pPrivate = pVNode->pVFS->pPrivate;
	pDevObj  = pVNode->pVFS->pDevObj;

	// 데이터 클러스터 번호를 블록 번호로 변경한다.
	dwBlock = fat32_cluster_to_block( pPrivate, dwCluster );
	// 클러스터의 시작 블록이 캐시에 없으면 일단 클러스터 전체를 캐시로 읽어 들인다.
	pEnt = find_hash_ent( pDevObj->pCache, dwBlock );
	if( pEnt == NULL )
	{
		bcache_scatter_load( pDevObj, dwBlock, (int)pPrivate->bySectorsPerCluster );
	}
	
	dwBlock += dwClusterOffset / pDevObj->nBlkSize;
	dwBlockOffset = dwClusterOffset % pDevObj->nBlkSize;
	nCopied = 0;

	for( ;; )
	{
		pBlockBuff = get_blk_cache_buff( pDevObj, dwBlock );
		if( pBlockBuff == NULL )
		{
			kdbg_printf( "new_fat32_vnode_cluster_read: error!\n" );
			return( -1 );
		}

		nThisRead = pDevObj->nBlkSize - dwBlockOffset;
		if( nThisRead > nToRead - nCopied )
			nThisRead = nToRead - nCopied;

		memcpy( &pBuff[ nCopied ], &pBlockBuff[dwBlockOffset], nThisRead );

		nCopied += nThisRead;
		if( nCopied >= nToRead )
			break;

		dwBlockOffset = 0;
		dwBlock++;
	}

	return( nCopied );
}

/*
// pVNode의 dwCluster의 dwClusterOffset에 pBuff의 데이터를 기록한다.
static int fat32_vnode_cluster_write( VNodeStt *pVNode, DWORD dwCluster, DWORD dwClusterOffset, char *pBuff, int nToWrite )
{
	DWORD			dwBlock;
	char			*pBlockBuff;
	BlkDevObjStt	*pDevObj;
	FAT32PrivateStt *pPrivate;
	CacheEntStt		*pCacheEnt;
	
	pPrivate = pVNode->pVFS->pPrivate;
	pDevObj  = pVNode->pVFS->pDevObj;

	// 데이터 클러스터 번호를 블록 번호로 변경한다.
	dwBlock = fat32_cluster_to_block( pPrivate, dwCluster );

	// 지정된 클러스터의 캐쉬된 엔트리를 구한다.
	pCacheEnt = (CacheEntStt*)get_cache_entry( pVNode->pVFS->pCache, pDevObj, dwBlock, (int)pPrivate->bySectorsPerCluster );
	if( pCacheEnt == NULL )
		return( -1 );			
	
	pBlockBuff = (char*)pCacheEnt->pBuff;

	// 버퍼에 복사하고 Dirty bit를 1로 한다.
	memcpy( &pBlockBuff[dwClusterOffset], pBuff, nToWrite );
	set_cahce_block_flag_dirty( pVNode->pVFS->pCache, pCacheEnt );

	return( nToWrite );
}
*/

// pVNode의 dwCluster의 dwClusterOffset에 pBuff의 데이터를 기록한다.
static int fat32_vnode_cluster_write( VNodeStt *pVNode, DWORD dwCluster, DWORD dwClusterOffset, char *pBuff, int nToWrite )
{
	BlkDevObjStt	*pDevObj;
	FAT32PrivateStt *pPrivate;
	char			*pBlockBuff;
	int				nCopied, nThisWrite;
	DWORD			dwBlock, dwBlockOffset;
	
	pPrivate = pVNode->pVFS->pPrivate;
	pDevObj  = pVNode->pVFS->pDevObj;

	// 데이터 클러스터 번호를 블록 번호로 변경한다.
	dwBlock = fat32_cluster_to_block( pPrivate, dwCluster );

	dwBlock += dwClusterOffset / pDevObj->nBlkSize;
	dwBlockOffset = dwClusterOffset % pDevObj->nBlkSize;
	nCopied = 0;

	for( ;; )
	{
		pBlockBuff = get_blk_cache_buff( pDevObj, dwBlock );
		if( pBlockBuff == NULL )
		{
			kdbg_printf( "fat32_vnode_cluster_write: error!\n" );
			return( -1 );
		}

		nThisWrite = pDevObj->nBlkSize - dwBlockOffset;
		if( nThisWrite > nToWrite - nCopied )
			nThisWrite = nToWrite - nCopied;

		memcpy( &pBlockBuff[dwBlockOffset], &pBuff[ nCopied ], nThisWrite );

		nCopied += nThisWrite;
		if( nCopied >= nToWrite )
			break;

		dwBlockOffset = 0;
		dwBlock++;
	}

	return( nToWrite );
}

// increase file size
static int fat32_vnode_increase( VNodeStt *pV, int nSize )
{
	int				nR;
	DWORD			dwX;
	FAT32FileStt	*pFSSD;
	FAT32PrivateStt	*pPrivate;
	int				nIncreaseSize;
	
	pFSSD	 = pV->pFSSD;
	pPrivate = pV->pVFS->pPrivate;

	// there is no need to allocate new cluster to increase file size
	if( nSize <= (int)pFSSD->dwLastAvailable )
	{
		pFSSD->lFileSize		+= (long)nSize;
		pFSSD->dwLastAvailable	-= (DWORD)nSize;
		pV->dwFileSize			= (DWORD)pFSSD->lFileSize;
		return( 0 );
	}

	nIncreaseSize = nSize - (int)pFSSD->dwLastAvailable;
	pFSSD->lFileSize		+= (long)pFSSD->dwLastAvailable;
	pFSSD->dwLastAvailable	=  0;

	// allocate new cluster and increase file size
	for( ; nIncreaseSize > 0; )
	{
		// allocate new cluster
		dwX = get_free_cluster( pV->pVFS );
		if( dwX == 0 )
			goto INC_ERROR;

		// link the cluster to the end of the file
		nR = set_next_cluster_number( pV, pFSSD->dwLastCluster, dwX );
		if( nR == -1 )
			goto INC_ERROR;

		// set last chain
		nR = set_next_cluster_number( pV, dwX, (DWORD)0x0FFFFFFF );
		if( nR == -1 )
		{
			JPRINTF( "fat32_vnode_increase() - critical error!\n" );
			goto INC_ERROR;	// shit! what the hell is this.
		}

		// set the current cluster field. (maybe the file size was 512 alignmented)
		if( pFSSD->dwCurCluster > 0x0FFFFFF7 )
			pFSSD->dwCurCluster = dwX;

		pFSSD->dwLastCluster = dwX;
		pFSSD->nTotalCluster++;

		// set lFileSize, dwLastAvailable
		if( nIncreaseSize <= pPrivate->nClusterSize )
		{
			pFSSD->lFileSize		+= nIncreaseSize;
			pFSSD->dwLastAvailable	= pPrivate->nClusterSize - nIncreaseSize;
			break;
		}

		pFSSD->lFileSize += pPrivate->nClusterSize;
		nIncreaseSize   -= pPrivate->nClusterSize;
	}	

	pV->dwFileSize = (DWORD)pFSSD->lFileSize;
	return( 0 );

INC_ERROR:
	pV->dwFileSize = (DWORD)pFSSD->lFileSize;	// synchronize file size

	return( -1 );
}

// 클러스터의 오프셋을 일반화한다.
static int generalize_cluster_offset( VNodeStt *pV )
{
	DWORD			dwX;
	FAT32FileStt	*pFSSD;
	FAT32PrivateStt	*pPrivate;

	pFSSD	 = pV->pFSSD;
	pPrivate = pV->pVFS->pPrivate;

	// cluster offset이 클러스터의 끝에 가 있으면 current cluster를 조정해 주어야 한다.
	for( dwX = pFSSD->dwCurCluster; pFSSD->nClusterOffset >= pPrivate->nClusterSize; )
	{
		// 다음 클러스터를 구한다.
		dwX = get_next_cluster_number( pV, dwX );
		if( dwX == 0 || dwX >= (DWORD)0x0FFFFFF7 )
			return( -1 );

		pFSSD->dwCurCluster   =  dwX;
		pFSSD->nClusterOffset -= pPrivate->nClusterSize;
	}	

	return( 0 );
}	

// pFSSD의 lFileOffset부터 nSize만큼을 기록한다.
static int fat32_vnode_raw_write( VNodeStt *pV, char *pBuff, int nSize )
{
	FAT32FileStt	*pFSSD;
	FAT32PrivateStt	*pPrivate;
	int				nTotalWrite, nR, nWriteSize, nIncreaseSize;
	DWORD			dwCluster, dwClusterOffset, dwClusterSize, dwX;
	
	pFSSD	 = pV->pFSSD;
	pPrivate = pV->pVFS->pPrivate;

	// calculate the size to increase
	nIncreaseSize = nSize - ( pFSSD->lFileSize - pFSSD->lFileOffset );

	// increase file size
	if( nIncreaseSize > 0 )
	{
		nR = fat32_vnode_increase( pV, nIncreaseSize );
		if( nR < 0 )
			return( -1 );
		
		// the file size is changed
		pFSSD->nEntryInfoDirty = 1;
	}		

	// generalize cluster offset
	if( generalize_cluster_offset( pV ) < 0 )
		return( -1 );

	dwClusterSize	= (DWORD)pPrivate->nClusterSize;
	dwCluster		= pFSSD->dwCurCluster;
	dwClusterOffset = (DWORD)pFSSD->nClusterOffset;
	nTotalWrite		= 0;

	// write by the nSize
	for( ;; )
	{
		// 현재 클러스터에서 쓸 수있는 크기를 결정한다.
		nWriteSize = (int)( dwClusterSize - dwClusterOffset );
		if( nWriteSize > nSize )
			nWriteSize = nSize;

		// 클러스터에 기록한다.
		nR = fat32_vnode_cluster_write( pV, dwCluster, dwClusterOffset, pBuff, nWriteSize );
		if( nR < 0 )
			return( -1 );

		nTotalWrite	+= nR;	// 읽은 크기를 더한다.
		pBuff		+= nR;	// 버퍼의 포인터를 증가한다.
		nSize		-= nR;

		// 다 썼는가?
		if( nTotalWrite >= nSize )
			break;

		dwClusterOffset =  0;
		
		// 다음 클러스터를 구한다.
		dwX = get_next_cluster_number( pV, dwCluster );
		if( dwX == 0 || dwX >= (DWORD)0x0FFFFFF7 )
			break;

		dwCluster = dwX;
	}	

	// dwCurCluster, dwClusterOffset을 설정한다.
	pFSSD->dwCurCluster    = dwCluster;
	pFSSD->nClusterOffset  = (int)dwClusterOffset + nR;
						   
	// 파일 포인터를 옮긴다.
	pFSSD->lFileOffset += nTotalWrite;
	
	return( nTotalWrite );
}

// vnode write
static int fat32_vnode_write( VNodeStt *pVNode, DWORD dwOffs, char *pBuff, int nSize )
{
	int		nR;
	long	lTemp;

	// set file offset
	lTemp = fat32_vnode_lseek( pVNode, (long)dwOffs, FSEEK_SET );
	if( lTemp == -1 )
		return( -1 );

	nR = fat32_vnode_raw_write( pVNode, pBuff, nSize );

	return( nR );
}	

// vnode read  ( this function is 512 alignment safe )
static int fat32_vnode_read( VNodeStt *pVNode, DWORD dwOffs, char *pBuff, int nSize )
{
	DWORD				dwClusterOffset, dwCluster;
	FAT32PrivateStt		*pPrivate;
	FAT32FileStt		*pFSSD;
	int					nReadableSize, nReadSize, nTh, nClusterSize, nToRead, nR;

	dwCluster = 0;
	nReadSize = 0;		// total read size
	pPrivate  = (FAT32PrivateStt*)pVNode->pVFS->pPrivate;
	pFSSD     = pVNode->pFSSD;

	// calculate cluset index
	nClusterSize	= (int)(512 * (int)pPrivate->bySectorsPerCluster);
	nTh				= (int)( dwOffs / (DWORD)nClusterSize );
	dwClusterOffset = (DWORD)(dwOffs % (DWORD)nClusterSize );

NEXT_CLUSTER:
	// pVNode에 연결된 nTh번째 클러스터 번호를 구한다.
	if( dwCluster == 0 )
		dwCluster = get_fat32_nth_cluster_number( pVNode, nTh );
	else	// 다음 클러스터를 구한다.
		dwCluster = get_next_cluster_number( pVNode, dwCluster );

	// 더이상 읽을 클러스터가 없다.
	if( dwCluster == 0 || dwCluster >= (DWORD)0x0FFFFFF7 )
		return( nReadSize );		// 그동안 읽은 크기를 리턴한다.

	// 마지막 클러스터 인가?
	if( dwCluster == pFSSD->dwLastCluster )
		nReadableSize = (int)( nClusterSize - (int)pFSSD->dwLastAvailable );
	else
		nReadableSize = nClusterSize;

	// calculate the next read size
	nToRead = nReadableSize - (int)dwClusterOffset;
	if( nSize < nToRead )
		nToRead = nSize;
	
	// read from the cluster
	nR = new_fat32_vnode_cluster_read( pVNode, dwCluster, dwClusterOffset, pBuff, nToRead );
	if( nR < 0 )
		return( -1 );

	nReadSize		+= nR;	// increase total read size
	pBuff			+= nR;	// move buffer pointer
	dwClusterOffset = 0;	// next time we read from cluster's offset 0

	// more to read?
	nSize -= nR;
	if( nSize > 0 )
		goto NEXT_CLUSTER;

	return( nReadSize );
}

// 현재 오프셋에서 디렉토리 엔트리(short, long)를 하나 조합하여 리턴한다.
static int fat32_vnode_readdir_entry( VNodeStt *pVNode, FAT32FileStt *pFSSD )
{
	DirEntInfoStt	ei; 
	DWORD			dwStartCluster;
	int				nOffset, nR, nTotalReadSlot;

	// get current internal offser
	nOffset = (int)fat32_vnode_lseek( pVNode, 0, FSEEK_CUR );

	// read directory entry
	nR = fat32_read_dirent( pVNode, &ei );
	if( nR < 0 )
		return( -1 );		// directory entry not found!

	nTotalReadSlot = nR;

	// copy the found entry info to pFSSD
	memcpy( &pFSSD->ei, &ei, sizeof( DirEntInfoStt ) );

	// if the entry is a long one then read the short entry.
	if( ei.szShortName[0] == 0 )
	{	// set long entry offset
		pFSSD->ei.nLongEntOffset = nOffset;		
		
		// get offset of the short entry
		nOffset = (int)fat32_vnode_lseek( pVNode, 0, FSEEK_CUR );

		// read next short entry
		nR = fat32_read_dirent( pVNode, &ei );
		if( nR < 0 )
			return( -1 );		// short entry doesn't follow the lond entry

		nTotalReadSlot += nR;
		// copy short directory entry
		memcpy( &pFSSD->ei.short_ent, &ei.short_ent, sizeof( SHORTDIRENT ) );
		pFSSD->ei.nShortEntOffset = nOffset;		
	}
	else // set short entry offset
		pFSSD->ei.nShortEntOffset = nOffset;		

	// assemble 8.3 filename 
	nR = nGetShortEntryName( pFSSD->ei.szShortName, (FAT32DirEntStt*)&ei.short_ent );
	pFSSD->lFileOffset		= 0;
	pFSSD->nClusterOffset	= 0;
	pFSSD->lFileSize		= (DWORD)ei.short_ent.dwFileSize;

	dwStartCluster			= (DWORD)ei.short_ent.wEAHandle;
	dwStartCluster			= (DWORD)( dwStartCluster << 16 );
	dwStartCluster			= dwStartCluster + (DWORD)ei.short_ent.wStartCluster;
	
	// FAT12, FAT16일 때 dirent의 start cluster에 nRootBlock을 더해주어야 한다.
	if( pVNode->pVFS->dwType == VFS_TYPE_FAT12 )
	{
		BlkDevObjStt	*pObj;
		FDDFat12Stt		*pFDDFat12;

		pObj		= pVNode->pVFS->pDevObj;
		pFDDFat12	= pObj->pPtr;
		if( 0x0FF7 <= dwStartCluster && dwStartCluster <= 0x0FFF )
			dwStartCluster += 0x0FFFF000;
		else if( 0 < dwStartCluster )
			dwStartCluster += (DWORD)pFDDFat12->nRootBlocks;
	}
	else if( pVNode->pVFS->dwType == VFS_TYPE_FAT16 )
	{
		BlkDevObjStt	*pObj;
		F16PartStt		*pF16Part;
		
		pObj		= pVNode->pVFS->pDevObj;
		pF16Part	= pObj->pPtr;
		if( 0xFFF7 <= dwStartCluster && dwStartCluster <= 0xFFFF )
			dwStartCluster += 0x0FFF0000;
		else if( 0 < dwStartCluster )
			dwStartCluster += (DWORD)pF16Part->nRootClusters;
	}

	pFSSD->dwCurCluster		= dwStartCluster;
	pFSSD->dwStartCluster	= dwStartCluster;
	
	pFSSD->ei.short_ent.wEAHandle       = (unsigned short)( (DWORD)dwStartCluster >> 16 );
	pFSSD->ei.short_ent.wStartCluster	= (unsigned short)( dwStartCluster );
	
	// return the total number of read slots
	return( nTotalReadSlot );
}

// long + short 디렉토리 엔트리를 읽어서 리턴한다.
static int fat32_vnode_readdir( VNodeStt *pVNode, DWORD dwOffs, DIRENTStt *pDIRENT )
{
	int				nR;
	long			lR;
	FAT32FileStt	fssd;
	DWORD			dwStartCluster;

	// 파일 포인터를 옮긴다.
	lR = fat32_vnode_lseek( pVNode, (long)dwOffs, FSEEK_SET );
	if( lR != (long)dwOffs )
		return( -1 );

	// 디렉토리 엔트리를 읽는다.
	nR = fat32_vnode_readdir_entry( pVNode, &fssd );
	if( nR < 0 )
		return( -1 );			

	strcpy( pDIRENT->szFileName, fssd.ei.szLongName );
	strcpy( pDIRENT->szAlias,    fssd.ei.szShortName );
	pDIRENT->lFileSize		= (long)fssd.ei.short_ent.dwFileSize;
	pDIRENT->dwFileType		= (DWORD)fssd.ei.short_ent.byAttr;
											 	
	dwStartCluster			= (DWORD)fssd.ei.short_ent.wEAHandle;
	dwStartCluster			= dwStartCluster + (DWORD)fssd.ei.short_ent.wStartCluster;
	pDIRENT->dwStartCluster = dwStartCluster;
	pDIRENT->dwStartBlock	= (DWORD)fat32_cluster_to_block( pVNode->pVFS->pPrivate, pDIRENT->dwStartCluster );

	return( nR );
}	

// rename directory entry  (new file does not exist.)
static int fat32_vnode_rename( VNodeStt *pOldParent, char *pOldName, VNodeStt *pNewParent, char *pNewName )
{
	int				nR, nX;
	VNodeStt		*pOldVNode, *pNewVNode;
	FAT32FileStt	*pNewFSSD, *pOldFSSD;
		
	// open old file
	pOldVNode = fat32_vnode_open( pOldParent, pOldName, FM_READ | FM_WRITE );
	if( pOldVNode == NULL )
	{
		ERROR_PRINTF( "fat32_vnode_rename() - opening the old file failed!\n" );
		return( -1 );
	}

	// create directory only (no data cluster allocation )
	pNewVNode = fat32_vnode_create_entry( pNewParent, pNewName, FM_READ | FM_WRITE );
	if( pNewVNode == NULL )
	{
		ERROR_PRINTF( "fat32_vnode_rename() - creating the new directory entry failed!\n" );
		fat32_vnode_close( pOldVNode );
		return( -1 );
	}

	pNewFSSD = pNewVNode->pFSSD;
	pOldFSSD = pOldVNode->pFSSD;
	
	// copy short entry information
	pNewFSSD->ei.short_ent.byAttr		  =	pOldFSSD->ei.short_ent.byAttr			;
	pNewFSSD->ei.short_ent.wEAHandle	  =	pOldFSSD->ei.short_ent.wEAHandle		;
	pNewFSSD->ei.short_ent.wCreateTime	  =	pOldFSSD->ei.short_ent.wCreateTime		;
	pNewFSSD->ei.short_ent.wCreateDate    =	pOldFSSD->ei.short_ent.wCreateDate		;
	pNewFSSD->ei.short_ent.wStartCluster  =	pOldFSSD->ei.short_ent.wStartCluster	;
	pNewFSSD->ei.short_ent.dwFileSize     =	pOldFSSD->ei.short_ent.dwFileSize		;

	// write new short entry
   	nR = fat32_vnode_close( pNewVNode );
	if( nR < 0 )
	{
		ERROR_PRINTF( "fat32_vnode_rename() - closing new vnode failed!\n" );
		nR = fat32_vnode_close( pOldVNode );
		return( -1 );
	}

	// delete old vnodes directory entry, ( no data cluster release )
	nR = fat32_vnode_remove_entry( pOldVNode );
	if( nR < 0 )
		ERROR_PRINTF( "fat32_vnode_rename() - deleting old vnode failed!\n" );

	// subfilesystem root vnode is not need to be unregistered. it is not registered.
	if( pOldVNode->pParentVNode != NULL )
		nX = unregister_vnode_hash( pOldVNode->pVFS->pVNodeMan, pOldVNode );

	// release old vnode
	nX = fat32_delete_vnode( pOldVNode );

	return( nR );
}		

// 연결된 블록 디바이스를 읽어들여서 마운트 한다.
static int fat32_fs_mount( void *pV )
{
	int					nR;
	BOOTSECTOR32		dbs;
	VFSStt				*pVFS;
	//CacheManStt			*pCache;
	VNodeOPStt			vnode_op;
	FAT32PrivateStt		*pPrivate;
	VNodeManStt			*pVNodeMan;
	VNodeStt			*pRootNode;

	pVFS = (VFSStt*)pV;

	// Private 구조체를 할당한다.
	pPrivate = (FAT32PrivateStt*)MALLOC( sizeof( FAT32PrivateStt ) );
	if( pPrivate == NULL )
		return( -1 );

	// set default vnode operations
	memset( &vnode_op, 0, sizeof( vnode_op ) );
	vnode_op.open	  = fat32_vnode_open;
	vnode_op.create	  = fat32_vnode_create;
	vnode_op.close	  = fat32_vnode_close;
	vnode_op.read	  = fat32_vnode_read;
	vnode_op.lseek	  = fat32_vnode_lseek;
	vnode_op.write	  = fat32_vnode_write;
	vnode_op.readdir  = fat32_vnode_readdir;
	vnode_op.remove   = fat32_vnode_remove;
	vnode_op.rename   = fat32_vnode_rename;
	vnode_op.get_info = fat32_vnode_get_info;
	
	// make vnode manager
	pVNodeMan = make_vnode_manager( &vnode_op, 512 );  // 512 = total hash index
	if( pVNodeMan == NULL )
	{
		FREE( pPrivate );
		return( -1 );
	}		   

	// ROOT NODE는 파일을 생성시키는 함수에 의해 만들어지는 것이 아니고 Mount될 때 vnode가 생성된다.
	// ROOT NODE를 만들기 위해 VNODE를 하나 할당한다.
	pRootNode = fat32_new_vnode( pVNodeMan );
	if( pRootNode == NULL )
	{
		FREE( pPrivate );
		delete_vnode_manager( pVNodeMan );
		return( -1 );
	}

	/*
	// Cache Manager 구조체를 할당한다.
	pCache = alloc_cache_man( pVFS, 64 );	// 64 = max cache entry
	if( pCache == NULL )
	{
ABORT_1:
		FREE( pPrivate );
		fat32_delete_vnode( pRootNode );
		delete_vnode_manager( pVNodeMan );
        
        // Block Device에서 읽은 내용을 버린다.
        discard_block_device ( pVFS->pDevObj );

        return( -1 );	  
	}	
*/
	// DBS를 읽어들인다.
	nR = read_block( pVFS->pDevObj, 0, (char*)&dbs, 1 );
	if( nR < 0 )
	{
		JPRINTF( "Read DBS failed!\n" );
ABORT_1:
		FREE( pPrivate );
		fat32_delete_vnode( pRootNode );
		delete_vnode_manager( pVNodeMan );
        discard_block_device ( pVFS->pDevObj );
        return( -1 );	  
	}

	// fat12 dbs error checking
	if( pVFS->dwType == VFS_TYPE_FAT12 && dbs.wRootEntries != 224 )
	{
		JPRINTF( "DBS.wRootEntries != 224\n" );
		goto ABORT_1;
	}

	// Pravate Data의 필드를 세팅한다.
	memset( pPrivate, 0, sizeof( FAT32PrivateStt ) );
	// DBS의 공통부분
	pPrivate->bySectorsPerCluster	= dbs.bySectorsPerCluster ; 
    pPrivate->wReservedSectors		= dbs.wReservedSectors	  ;	 
    pPrivate->wSectorsPerTrack		= dbs.wSectorsPerTrack	  ;
	pPrivate->wHeads				= dbs.wHeads			  ;
	if( pVFS->dwType == VFS_TYPE_FAT32 )
	{	// FAT32
	    pPrivate->dwBigTotalSectors		= dbs.dwBigTotalSectors	;
		pPrivate->dwBigSectorsPerFat	= dbs.dwBigSectorsPerFat;
	    pPrivate->dwRootDirStrtCluster	= dbs.dwRootDirStrtClus	;
		pPrivate->dwFATLoc				= (DWORD)pPrivate->wReservedSectors;
		pPrivate->dwRootLoc				= (DWORD)pPrivate->dwFATLoc + pPrivate->dwBigSectorsPerFat * 2;
		pPrivate->nClusterSize			= (int)dbs.bySectorsPerCluster * (int)512;

	}	
	else if( pVFS->dwType == VFS_TYPE_FAT12 || pVFS->dwType == VFS_TYPE_FAT16 )
	{	// FAT12, FAT16
		if( pVFS->dwType == VFS_TYPE_FAT12 )
			pPrivate->dwBigTotalSectors		= (DWORD)dbs.wTotalSectors ;
		else
			pPrivate->dwBigTotalSectors		= (DWORD)dbs.dwBigTotalSectors;
		pPrivate->dwBigSectorsPerFat	= (DWORD)dbs.wSectorsPerFAT;
	    pPrivate->dwRootDirStrtCluster	= 2;
		pPrivate->dwFATLoc				= 1;
		pPrivate->dwRootLoc				= (DWORD)pPrivate->dwFATLoc + pPrivate->dwBigSectorsPerFat * 2;
		pPrivate->nClusterSize			= (int)dbs.bySectorsPerCluster * (int)512;

		// convert fat12 to fat32
		if( pVFS->dwType == VFS_TYPE_FAT12 )
			nR = cheat_fat12( pVFS, pPrivate, ( (int)dbs.wRootEntries / (int)(512/32) ) );	// laster parameter = root sectors
		else  // convert fat16 to fat32
			nR = cheat_fat16( pVFS, pPrivate, ( (int)dbs.wRootEntries / (int)(512/32) ) );
		
		if( nR < 0 )
			goto MOUNT_ERROR;
	}	
	else
	{	// it supports only FAT12, FAT32
MOUNT_ERROR:
		FREE( pPrivate );
		fat32_delete_vnode( pRootNode );
		delete_vnode_manager( pVNodeMan );
		//free_cache_man( pVFS, pCache );
		return( -1 );	  
	}	

	// set pointers
	//pVFS->pCache	= pCache;
	pVFS->pPrivate  = pPrivate;
	pVFS->pRootNode = pRootNode;
	pVFS->pVNodeMan = pVNodeMan;

	{// make root node ( root vnode will not be registered or hashed )
		FAT32FileStt	*pFSSD;
		
		pRootNode->nRefCount = 0;

		pRootNode->pVFS			= pVFS;
		pRootNode->dwType		= VNTYPE_ROOT | FT_DIRECTORY;

		pFSSD					= pRootNode->pFSSD;
		pFSSD->nTotalCluster	= 0;								// 일단 0으로 되어야 한다.
		pFSSD->dwLastCluster	= 0;								
		pFSSD->dwLastAvailable	= 0;								// 마지막 클러스터중 파일 크기에 포함되지 않은 잔여분
		pFSSD->nClusterOffset	= 0;	
		pFSSD->lFileSize		= 0;		
		pFSSD->lFileOffset		= 0;	
		pFSSD->dwStartCluster	= pPrivate->dwRootDirStrtCluster;	// 루트 디렉토리의 시작 클러스터 번호는 2번
		pFSSD->dwCurCluster		= pPrivate->dwRootDirStrtCluster;

		// nLastCluster, nTotalCluster, dwLastAvailable를 설정한다.
		fat32_calc_filesize( pRootNode, pFSSD );				   
	}

	return(0);
}

// unmount fileystem
static int fat32_fs_unmount( void *pV )
{
	VFSStt			*pVFS;
	BlkDevObjStt	*pObj;
	BlkDevIoctlStt	ioctl;

	pVFS = (VFSStt*)pV;

	// if vfs type is fat12 mount time cheating must be recoverd.
	if( pVFS->dwType == VFS_TYPE_FAT12 )
		recover_fat12_cheat( pVFS );
	else if( pVFS->dwType == VFS_TYPE_FAT16 )
		recover_fat16_cheat( pVFS );

	// if block device media is a removable one, it must be synchronized.
	pObj = pVFS->pDevObj;
	if( pObj->nAttr & BLKDEV_ATTR_REMOVABLE )
	{	// flush cache first
		//flush_cache( pVFS );
		
		// flushing (write dirty data)
		ioctl.nFunc = BLKDEV_IOCTL_FLUSH;
		pObj->pDev->op.ioctl( pObj, &ioctl );		  

		// invalidating	(release buffer)
		ioctl.nFunc = BLKDEV_IOCTL_INVALIDATE;
		pObj->pDev->op.ioctl( pObj, &ioctl );		  
	}	

	// release private and root vnode
	if( pVFS->pPrivate )
	{
		FREE( pVFS->pPrivate );
		pVFS->pPrivate = NULL;
	}
	if( pVFS->pRootNode )
	{
		fat32_delete_vnode( pVFS->pRootNode );
		pVFS->pRootNode = NULL;
	}
	if( pVFS->pVNodeMan )
	{
		delete_vnode_manager( pVFS->pVNodeMan );
		pVFS->pVNodeMan = NULL;
	}
	//if( pVFS->pCache )
	//{
	//	free_cache_man( pVFS, pVFS->pCache );
	//	pVFS->pCache = NULL;
	//}
	
	return(0);
}

// dirty mark된 블록을 디스크에 기록한다.
static int fat32_fs_sync( void *pV )
{
	// ^^;




	return(0);
}

// FAT32에 관계된 IOCTL.
static int fat32_fs_ioctl( int nCmd, DWORD dwParam )
{
	int nR;

	switch( nCmd )
	{
	case VFS_IOCTL_INIT :					// 파일 시스템 초기화  (포맷)
		nR = create_fat32_fs( dwParam );	// pV == VOID Device Object
		break;

	default :
		return( -1 );						// 알 수 없는 IOCTL COMMAND
		break;
	}		
	
	return(0);
}

// HDD DBS의 내용을 출력한다.
static void disp_fat32_dbs( UCHAR *pB )
{
	BOOTSECTOR32	*pHD;

    pHD = (BOOTSECTOR32*)pB;                        

    JPRINTF( "wBytesPerSector      :  %d\n", pHD->wBytesPerSector );
    JPRINTF( "bySectorsPerCluster  :  %d\n", pHD->bySectorsPerCluster );
    JPRINTF( "wReservedSectors     :  %d\n", pHD->wReservedSectors );
    JPRINTF( "byNumberOfFATs       :  %d\n", pHD->byNumberOfFATs );
    JPRINTF( "wRootEntries         :  %d\n", pHD->wRootEntries );
    JPRINTF( "wTotalSectors        :  %d\n", pHD->wTotalSectors );
    JPRINTF( "byMediaDescriptor    :  %x\n", pHD->byMediaDescriptor );
    JPRINTF( "wSectorsPerFAT       :  %d\n", pHD->wSectorsPerFAT );
    JPRINTF( "wSectorsPerTrack     :  %d\n", pHD->wSectorsPerTrack );
    JPRINTF( "wHeads               :  %d\n", pHD->wHeads );
    JPRINTF( "dwHiddenSectors      :  %x\n", pHD->dwHiddenSectors );
    JPRINTF( "dwBigTotalSectors    :  %x\n", pHD->dwBigTotalSectors );
    JPRINTF( "dwBigSectorsPerFat   :  %x\n", pHD->dwBigSectorsPerFat );
    JPRINTF( "wExtFlags            :  %d\n", pHD->wExtFlags );
    JPRINTF( "wFS_Version          :  %d\n", pHD->wFS_Version );
    JPRINTF( "dwRootDirStrtClus    :  %x\n", pHD->dwRootDirStrtClus );
    JPRINTF( "wFSInfoSec           :  %d\n", pHD->wFSInfoSec );
    JPRINTF( "wBkUpBootSec         :  %d\n", pHD->wBkUpBootSec );
    JPRINTF( "byDriveNumber        :  %d\n", pHD->byDriveNumber );
    JPRINTF( "byBootSignature      :  %x\n", pHD->byBootSignature );
    JPRINTF( "dwVolumeID           :  %x\n", pHD->dwVolumeID );

	JPRINTF( "sizeof( BOOTSECTOR32 ) = %d\n", sizeof( BOOTSECTOR32 ) );
}						

static int make_fat32_dbs( BOOTSECTOR32 *pRD, int nTotalSector )
{
	int nFATEntry, nFATSectors;

	nFATEntry	= ( nTotalSector - 1 + 1) / 2;
	nFATSectors	= ( nFATEntry + 2 + 127 ) / ( 128 );

    pRD->wHeads					= 0;
    pRD->dwHiddenSectors		= 0;
	pRD->wReservedSectors		= 1;
    pRD->wSectorsPerTrack		= 0;
	pRD->bySectorsPerCluster	= 1;//2; 
    pRD->dwRootDirStrtClus		= 2;
    pRD->dwBigTotalSectors		= (DWORD)nTotalSector;
    pRD->dwBigSectorsPerFat		= (DWORD)nFATSectors;

	return( 0 );
}	

static int make_fat32_fat_header_entry( char *pBuff )
{
	DWORD *pDW;

	pDW = (DWORD*)pBuff;

	// 0으로 클리어한다.
	memset( pBuff, 0, 512 );

	// FAT ID를 기록한다.
	pDW[0] = (DWORD)0xFFFFFFF8;
	pDW[1] = (DWORD)0x7FFFFFFF;
	pDW[2] = (DWORD)0xFFFFFFF8;		// 루트 디렉토리는 처음에 한 클러스터를 갖는다.

	return(0);
}

// 오픈된 블록 디바이스 객체에 대해 FAT32 파일 시스템을 생성한다.
static int create_fat32_fs( DWORD dwVoidObj )		
{
	BlkDevObjStt	*pObj;
	BOOTSECTOR32	*pRD;
	char			buff[512], fat[512];
	int				nDBS_loc, nFAT1_loc, nFAT2_loc, nRootDir_loc;
	int				nR, nI;

	pObj = (BlkDevObjStt*)dwVoidObj;

	pRD  = (BOOTSECTOR32*)buff;
	memcpy( buff, fat32_dbs, sizeof( buff ) );
	
	// FAT32의 DBS를 만든다.
	make_fat32_dbs( pRD, pObj->dwTotalBlk );

	// FAT을 만든다.
	make_fat32_fat_header_entry( fat );
	
	// 만든 DBS를 출력해 본다.
	//disp_fat32_dbs( buff );

	// 각 부분의 위치를 확인한다.
	nDBS_loc     = 0;
	nFAT1_loc    = (int)pRD->wReservedSectors + nDBS_loc;
	nFAT2_loc    = nFAT1_loc + (int)pRD->dwBigSectorsPerFat;
	nRootDir_loc = nFAT2_loc + (int)pRD->dwBigSectorsPerFat;

	// DBS를 기록한다.
	write_block( pObj, (DWORD)nDBS_loc, (char*)pRD, 1 );

	// FAT의 첫번째 섹터를 기록
	nR = write_block( pObj, (DWORD)nFAT1_loc, fat, 1 );
	nR = write_block( pObj, (DWORD)nFAT2_loc, fat, 1 );

	memset( fat, 0, sizeof( fat ) );
	// FAT의 나머지 부분을 0으로 클리어한다.
	for( nI = 1; nI < (int)pRD->dwBigSectorsPerFat; nI++ )
		nR = write_block( pObj, (DWORD)nFAT1_loc+nI, fat, 1 );
	for( nI = 1; nI < (int)pRD->dwBigSectorsPerFat; nI++ )
		nR = write_block( pObj, (DWORD)nFAT2_loc+nI, fat, 1 );

	// ROOT 디렉토리의 첫 번째 클러스터를 를 모두 0으로 클리어한다.
	for( nI = 0; nI < (int)pRD->bySectorsPerCluster; nI++ )
		nR = write_block( pObj, (DWORD)nRootDir_loc+nI, fat, 1 );

	return( 0 );
}

// free fat file struct
int free_fat_fs_struct( void *pVoidVFS )
{
	int		nR;
	VFSStt	*pVFS;

	if( pVoidVFS == NULL )
		return( -1 );

	pVFS = (VFSStt*)pVoidVFS;

	// unregister filesystem
	nR = unregister_filesystem( pVFS );

	// unlink device object from vfs
	nR = unlink_filesystem_device( pVFS );

	FREE( pVFS );

	return( 0 );	
}

// fat32를 위한 구조체를 할당하고 operation을 세팅한 후 리턴한다.
void *alloc_fat_fs_struct( BlkDevObjStt *pDevObj, DWORD dwType )
{
	int		nR;
	VFSStt	*pVFS;
	
	// 필요한 메모리를 할당한다. (파일 스스템의 오픈)
	pVFS = (VFSStt*)MALLOC( sizeof( VFSStt ) );
	if( pVFS == NULL )
		return( NULL );
	
	// set filesystem name and type
	memset( pVFS, 0, sizeof( VFSStt ) );

	if( dwType == VFS_TYPE_FAT32 )
	{
		strcpy( pVFS->szName, "FAT32" );
		pVFS->dwType     = VFS_TYPE_FAT32;
	}
	else if( dwType == VFS_TYPE_FAT12 )
	{
		strcpy( pVFS->szName, "FAT12" );
		pVFS->dwType     = VFS_TYPE_FAT12;
	}
	else if( dwType == VFS_TYPE_FAT16 )
	{
		strcpy( pVFS->szName, "FAT16" );
		pVFS->dwType     = VFS_TYPE_FAT16;
	}
	else
	{	// if dwType is not the one of FAT12, FAT16 or FAT32, it is an error.
		FREE( pVFS );
		return( NULL );
	} 
	
	pVFS->op.mount	 = fat32_fs_mount;
	pVFS->op.unmount = fat32_fs_unmount;
	pVFS->op.sync	 = fat32_fs_sync;
	pVFS->op.ioctl   = fat32_fs_ioctl;
	pVFS->op.free	 = free_fat_fs_struct;

	// 할당된 fat32 파일 시스템을 등록한다.
	nR = register_filesystem( pVFS );
	if( nR == -1 )
	{
		free_fat_fs_struct( pVFS );
		return( NULL );
	}

	// 파일 시스템에 실제 디바이스를 연결한다.
	link_filesystem_device( pVFS, pDevObj );	
	
	return( pVFS );
}

// pFSSD까지 할당한다.
static VNodeStt *fat32_new_vnode( VNodeManStt *pVNodeMan )
{
	VNodeStt		*pV;
	FAT32FileStt	*pFSSD;

	// FAT32FILEStt를 할당한다.
	pFSSD = (FAT32FileStt*)MALLOC( sizeof( FAT32FileStt ) );
	if( pFSSD == NULL )
		return( NULL );

	// vnode를 할당한다.
	pV = new_vnode( pVNodeMan );
	if( pV == NULL )
	{
		FREE( pFSSD );
		return( NULL );
	}

	pV->pFSSD = pFSSD;
	// 0으로 clear한다.
	memset( pFSSD, 0, sizeof( FAT32FileStt ) );

	return( pV );
}

// pFSSD가 할당되어 있으므로 그것까지 해제해 주어야 한다.
static int fat32_delete_vnode( VNodeStt *pVNode )
{
	int			nR;
	void		*pFSSD;

	if( pVNode == NULL )
		return( -1 );

	pFSSD = pVNode->pFSSD;
	nR = delete_vnode( pVNode );

	// ref count가 0이면 pFSSD를 지운다.
	if( nR == 0 && pFSSD != NULL )
		FREE( pFSSD );
	
	return( nR );
}				

// set fssd's current cluseter number and cluster offset by reference to lOffset.
static int fat32_cluster_tracking( VNodeStt *pVNode, long lOffset )
{
	FAT32FileStt	*pFSSD;
	FAT32PrivateStt	*pPrivate;
	DWORD			dwCluster, dwX;
	int				nI, nCluster, nBytePerCluster, nClusterOffset;
	
	pFSSD		= pVNode->pFSSD;
	dwCluster	= pFSSD->dwStartCluster;
	pPrivate	= pVNode->pVFS->pPrivate;

	// bytes per cluster
	nBytePerCluster = pPrivate->bySectorsPerCluster * 512;

	nCluster		= (int)lOffset / nBytePerCluster;	// the total number of clusters to track
	nClusterOffset	= (int)lOffset % nBytePerCluster;	// the offset of the last tracked cluseter
	
	for( nI = 0; nI < nCluster; nI++ )
	{
		dwX = get_next_cluster_number( pVNode, dwCluster );

		if( dwX == 0 || dwX >= (DWORD)0x0FFFFFF7 )
		{	// the file size is 512 alignmented and file pointer is on the end of the file.
			if( nI+1 == nCluster && nClusterOffset == 0 )
			{
				dwCluster = (DWORD)0x0FFFFFFF;
				break;
			}
			
			return( -1 );
		}

		dwCluster = dwX;
	}

	pFSSD->dwCurCluster		= dwCluster;			
	pFSSD->nClusterOffset	= nClusterOffset;

	return( 0 );
}	

// vnode의 파일 포인터와 클러스터 번호 및 기타 인덱스를 조정한다.
static long fat32_vnode_lseek( VNodeStt *pVNode, long lOffset, int nOrigin )
{
	int				nR;
	long			lNew, lCur;
	FAT32FileStt	*pFSSD;

	pFSSD = pVNode->pFSSD;
	lCur  = pFSSD->lFileOffset;
	switch( nOrigin )
	{
	case FSEEK_SET :
		lNew = lOffset;
		break;
	case FSEEK_CUR :
		lNew = lOffset + lCur;
		break;
	case FSEEK_END :
		lNew = lOffset + (long)pFSSD->lFileSize;
		break;
	}

	// 파일 포인터가 파일크기보다 크면 포인터를 파일크기로 설정한다.
	if( lNew >= (long)pFSSD->lFileSize )
		lNew = (long)pFSSD->lFileSize;
	else if( lNew < 0 )
		lNew = 0;

	// 파일 오프셋이 이미 같은지 확인한다.
	if( lNew == pFSSD->lFileOffset )
		return( lNew );

	// file offset에 따라 cluster번호와 cluster offset을 설정한다.
	nR = fat32_cluster_tracking( pVNode, lNew );
	if( nR < 0 )
		return( -1 );

	// 파일 포인터를 새로 설정한다.
	pFSSD->lFileOffset = lNew;
	
	return( lNew );		 
}

// pFSSD의 lFileOffset부터 nSize만큼을 읽어 리턴한다.
static int fat32_vnode_raw_read( VNodeStt *pV, char *pBuff, int nSize )
{
	FAT32FileStt	*pFSSD;
	FAT32PrivateStt	*pPrivate;
	int				nReadableSize, nReadSize, nTotalRead, nR;
	DWORD			dwCluster, dwClusterOffset, dwClusterSize, dwX;
	
	pFSSD	 = pV->pFSSD;
	pPrivate = pV->pVFS->pPrivate;

	// nReadSize를 결정한다.
	if( pFSSD->lFileOffset + (long)nSize < pFSSD->lFileSize )
		nReadSize = nSize;
	else
		nReadSize = pFSSD->lFileSize - pFSSD->lFileOffset;

	// 클러스터의 오프셋을 일반화한다.
	if( generalize_cluster_offset( pV ) < 0 )
		return( -1 );

	dwClusterSize	= (DWORD)pPrivate->bySectorsPerCluster * (DWORD)512;
	dwCluster		= pFSSD->dwCurCluster;
	dwClusterOffset = (DWORD)pFSSD->nClusterOffset;
	nTotalRead		= 0;

	// nReadSize만큼의 데이터를 읽는다.
	for( ; nReadSize > 0; )
	{
		// 현재 클러스터에서 읽을 수 있는 크기를 결정한다.
		if( dwCluster == pFSSD->dwLastCluster )		// 마지막 클러스터인가?
			nReadableSize = dwClusterSize - pFSSD->dwLastAvailable;
		else
			nReadableSize = dwClusterSize;

		if( nTotalRead + nReadableSize > nSize )
			nReadableSize = nSize - nTotalRead;
	


		//kdbg_printf( "dwCluster=%d, dwClusterOffset=%d, nReadableSize=%d\n", 
		//	dwCluster, dwClusterOffset, nReadableSize );


		// 클러스터에서 읽는다.
		nR = new_fat32_vnode_cluster_read( pV, dwCluster, dwClusterOffset, pBuff, nReadableSize );
		if( nR < 0 )
			return( -1 );

		nTotalRead	+= nR;				// 읽은 크기를 더한다.
		pBuff		+= nR;				// 버퍼의 포인터를 증가한다.
		if( nR == (int)dwClusterSize )
			dwClusterOffset =  0;		// 클러스터의 처음부터 다시 읽게 된다.
		else
			dwClusterOffset += (DWORD)nR;

		// 다 읽었는가?
		if( nTotalRead >= nSize )
			break;

		// 다음 클러스터를 구한다.
		dwX = get_next_cluster_number( pV, dwCluster );
		// 더이상 클러스터를 구할 수 없다.
		if( dwX == 0 || dwX >= (DWORD)0x0FFFFFF7 )
			break;

		dwCluster = dwX;
	}	

	// dwCurCluster, dwClusterOffset을 설정한다.
	pFSSD->dwCurCluster    = dwCluster;
	pFSSD->nClusterOffset  = (int)dwClusterOffset;
						   
	// 파일 포인터를 옮긴다.
	pFSSD->lFileOffset += nTotalRead;
	
	return( nTotalRead );
}

// 디렉토리 엔트리 조각을 조합하여 완전한 디렉토리 엔트리를 리턴한다.
static int fat32_assemble_dirent( DirEntInfoStt *pEntInfo, FAT32DirEntStt *pDirEnt, LongNameStt *pNameTemp, int nPrevResult )
{
	int nIndex, nX, nLength, nR;

	// volume label 또는 short entry인가? 
	if( nIsShortEnt( pDirEnt ) )
	{
		if( nPrevResult == _MDER_CONTINUE )
		{	//return( _MDER_ERROR );	// long entry name을 조합하다 말았는데 갑자기 short entry가 등장했다.
			nR = _MDER_OK_LONG_ABORT;
		}
		else
			nR = _MDER_OK;

		// short entry name을 얻는다.
		nLength = nGetShortEntryName( pEntInfo->szShortName, pDirEnt );
		if( nLength <= 0 )
			return( _MDER_ERROR );

		return( nR );		// short name을 조합했을 때에만 ok를 리턴한다.
	}

	// 인덱스를 구한다.
	nIndex = (int)( (UCHAR)pDirEnt->u.l.sequence & (UCHAR)0x3F );
		
	//  Entry의 sub name str을 구한다.
	nX = nGetLongEntrySubName( pNameTemp->ent[nIndex-1].szSubStr, pDirEnt );
	if( nX < 0 )
		return( _MDER_ERROR );

	pNameTemp->nTotal++;

	// 마지막 sequence까지 처리되었으면 SubString을 모두 연결하여 long name을 조합한다.
	if( nIndex == 1 )
	{
		pEntInfo->szLongName[0] = 0;
		for( nX = 0; nX	< pNameTemp->nTotal; nX++ )
			strcat( pEntInfo->szLongName, pNameTemp->ent[nX].szSubStr );	
		return( _MDER_OK );
	}
	else	
		return( _MDER_CONTINUE );
}	

// 디렉토리 엔트리의 조각들을 읽어들여 완전한 디렉토리 엔트리를 리턴한다.
static int fat32_read_dirent( VNodeStt *pDirNode, DirEntInfoStt *pEntInfo )
{
	int				nR, nTotalRead, nPrevResult;
	LongNameStt		name_temp;
	FAT32DirEntStt	ent;
			 
	nTotalRead  = 0;
	nPrevResult = 0;	// _MDER_OK;
	memset( pEntInfo,   0, sizeof( DirEntInfoStt ) );
	memset( &name_temp, 0, sizeof( name_temp     ) );
	for( ;; )
	{	// 하나의 엔트리 조각을 읽어들인다.
		nR = fat32_vnode_raw_read( pDirNode, (char*)&ent, sizeof( FAT32DirEntStt ) );
		if( nR != sizeof( FAT32DirEntStt ) )
			return( -1 );		// 엔트리를 읽을 수 없다.

		nTotalRead += nR;

		if( ent.u.s.name[0] == 0 )
			return( -1 );		// 엔트리의 끝이다.

		if( (UCHAR)ent.u.s.name[0] == (UCHAR)0xE5 )
		{
			memset( &name_temp, 0, sizeof( name_temp ) );
			continue;			// 지워진 엔트리는 건너뛴다.
		}

		// 엔트리의 조각들을 조합한다. 
		nPrevResult = fat32_assemble_dirent( pEntInfo, &ent, &name_temp, nPrevResult );

		if( nPrevResult == _MDER_ERROR )
		{
			ERROR_PRINTF( "fat32_read_dirent() = _MDERROR\n" );
			return(-1 );
		}

		if( nPrevResult == _MDER_OK_LONG_ABORT )
		{
			memset( &name_temp, 0, sizeof( name_temp     ) );		// 조합된 long name을 포기하고 short name만 조합해서 ok를 리턴한다. 
			nPrevResult = _MDER_OK;									// 다시 _MDER_OK로 한다.
		}

		if( nPrevResult == _MDER_OK )
		{	// short entry일 경우 pEntInfo에 복사한다.
			if( nIsShortEnt( &ent ) )
				memcpy( &pEntInfo->short_ent, &ent, sizeof( ent ) );		
			
			return( nTotalRead );
		}
	}
}

// 파일의 크기와 마지막 클러스터 번호를 구한다.
static int fat32_calc_filesize( VNodeStt *pV, FAT32FileStt *pFSSD )
{											  
	int				nTotalCluster;
	DWORD			dwX, dwCluster;
	FAT32PrivateStt *pPrivate;

	pPrivate = pV->pVFS->pPrivate;

	// find last cluster number and the total number of clusters
	nTotalCluster  = 1;
	dwCluster = pFSSD->dwStartCluster;
	
	for( ;; )
	{	// get next cluster
		dwX = get_next_cluster_number( pV, dwCluster );	

		if( dwX == 0 || dwX >= (DWORD)0x0FFFFFF7 )
			break;		
		dwCluster = dwX;
		nTotalCluster++;
	}
	
	// set last cluster number
	pFSSD->dwLastCluster = dwCluster;
	pFSSD->nTotalCluster = nTotalCluster;

	dwX = (DWORD)nTotalCluster * (DWORD)pPrivate->nClusterSize;
	
	// is it a directory
	if( pV->dwType & FT_DIRECTORY )
	{	// directory size is 0 byte
		pFSSD->lFileSize		= (long)dwX;
		pFSSD->dwLastAvailable	= 0;
	}
	else
	{
		pFSSD->dwLastAvailable = dwX - (DWORD)pFSSD->lFileSize;
	}	

	return( 0 );
}

// find_fat32_dir_ent의 리턴값으로 사용된다.
#define FAT32_LONG_ENT		1
#define FAT32_SHORT_ENT		2
// parent node에서 pName의 엔트리를 찾는다. (찾은 정보는 pFSSD에 리턴된다.)
static int fat32_fssd_find_dirent( VNodeStt *pV, FAT32FileStt *pFSSD )
{
	int				nR;
	VNodeStt		*pParentNode; 

	pParentNode = pV->pParentVNode;

	// rewind file pointer
	fat32_vnode_lseek( pParentNode, 0, FSEEK_SET );

	for( ;; )
	{	// read one directory entry
		nR = fat32_vnode_readdir_entry( pParentNode, pFSSD );
		if( nR < 0 )
			return( -1 );		// entry not found

		// ignorecase
		if( strcmpi( pV->szName, pFSSD->ei.szLongName  ) == 0 || 
			strcmpi( pV->szName, pFSSD->ei.szShortName ) == 0 )
			break;		// i found it.
	}			  

	return( 1 );		
}

//  파일을 오픈한다.  pName은 순수한 파일명
static VNodeStt *fat32_vnode_open( VNodeStt *pParentNode, char *pName, DWORD dwMode )
{
	int					nR;
	VNodeStt			*pV;
	FAT32FileStt		*pFSSD;
	FAT32PrivateStt		*pPrivate;

	pPrivate = pParentNode->pVFS->pPrivate;

	// pName으로 만들어진 vnode가 이미 존재하는지 찾아 본다.
	pV = find_vnode( pParentNode->pVFS->pVNodeMan, pParentNode, pName );
	if( pV == NULL )
	{	
		// vnode를 할당한다.  (vnode operation이 복사되어 리턴된다.)
		pV = fat32_new_vnode( pParentNode->pVFS->pVNodeMan );
		if( pV == NULL )
			return( NULL );		// vnode allocation error

		// 아래 정보는 fat32_find_dirent를 call하기 전에 세팅되어야 한다.
		pFSSD				= pV->pFSSD;
		pV->pVFS			= pParentNode->pVFS;
		pV->pParentVNode	= pParentNode;								
		strcpy( pV->szName, pName );	

		// pParentNode에 pName엔트리가 있는지 찾는다.  (pParentNode는 pV->pParentVNode로 구해 사용한다.)
		nR = fat32_fssd_find_dirent( pV, pFSSD );
		if( nR < 0 )	
		{	// file not found error
			fat32_delete_vnode( pV );
			return( NULL );			
		}	  

		// type을 설정한다.
		pV->dwType = pFSSD->ei.short_ent.byAttr;

		//  VNode의 파일명으로 Hash한다.
		register_vnode_hash( pV->pVFS->pVNodeMan, pV );
	}
	else
	{	// 오픈하고자 하는 vnode가 이미 존재한다.
		if( pV->pBranchVNode != NULL )	// 마운트된 vnode인가?
			pV = pV->pBranchVNode;

		pFSSD = pV->pFSSD;
	}

	// 마지막 클러스터와 파일 크기를 구한다.
	nR = fat32_calc_filesize( pV, pFSSD );
	pV->dwFileSize = (DWORD)pFSSD->lFileSize;

	return( pV );
}

static int fat32_vnode_get_info( VNodeStt *pParentNode, char	*pName,	DIRENTStt *pDirEnt )
{
	int 				nR;
	FAT32FileStt		fssd;
	VNodeStt			vnode;

	// 아래 정보는 fat32_find_dirent를 call하기 전에 세팅되어야 한다.
	memset( &vnode, 0, sizeof( vnode ) );
	memset( &fssd, 0, sizeof( fssd ) );
	vnode.pFSSD 		= &fssd;
	vnode.pVFS			= pParentNode->pVFS;
	vnode.pParentVNode	= pParentNode;								
	strcpy( vnode.szName, pName );	
	
	// pParentNode에 pName엔트리가 있는지 찾는다.  (pParentNode는 pV->pParentVNode로 구해 사용한다.)
	nR = fat32_fssd_find_dirent( &vnode, &fssd );
	if( nR < 0 )	
		return( 0 );	// 찾을 수 없다.

	strcpy( pDirEnt->szFileName, fssd.ei.szLongName );		// long file name
	strcpy( pDirEnt->szAlias, fssd.ei.szShortName );		// short file name
	pDirEnt->lFileSize 		= fssd.ei.short_ent.dwFileSize;
	pDirEnt->dwFileType 	= fssd.ei.short_ent.byAttr; 	// file type
	pDirEnt->dwStartCluster = fssd.dwStartCluster; 			// start cluster
	pDirEnt->dwStartBlock 	= 0;							// start block
	
	return( 1 );
}

// 파일의 오프셋 값을 구한다.
static int fat32_vnode_file_pointer( VNodeStt *pVNode )
{
	FAT32FileStt	*pFSSD;

	pFSSD = pVNode->pFSSD;

	return( (int)pFSSD->lFileOffset );
}									  

// pParentNode에서 slot개수 만큼의 빈 엔트리를 찾아서 오프셋을 리턴한다.
static int nFindFreeDirEnt( VNodeStt *pDirNode, int nSlot )
{
	FAT32DirEntStt	ent;
	int				nOffset, nTotalFree;
	int				nR, nI;
			 
	nOffset		= 0;
	nTotalFree	= 0;
	
	// 파일 포인터를 처음으로 옮긴다.
	fat32_vnode_lseek( pDirNode, 0, FSEEK_SET );
	
	for( ;; )
	{	
		if( nTotalFree == 0 )
			nOffset = fat32_vnode_file_pointer( pDirNode );
		
		// 하나의 엔트리 조각을 읽어들인다.		   	
		nR = fat32_vnode_raw_read( pDirNode, (char*)&ent, sizeof( FAT32DirEntStt ) );
		if( nR != sizeof( FAT32DirEntStt ) )
			break;		// 새로 엔트리를 추가해야 한다.

		if( ent.u.s.name[0] == 0 )	//  엔트리의 마지막
			break;

		if( (UCHAR)ent.u.s.name[0] == (UCHAR)0xE5 )
		{
			nTotalFree++;
			if( nTotalFree == nSlot )	// 찾은 free entry의 시작 오프셋을 리턴한다.
				return( nOffset );
		}
		else
			nTotalFree = 0;
	}
		
	memset( &ent, 0, sizeof( ent ) );

	// 빈 엔트리를 기록하고 오프셋을 리턴한다.
	for( nI = 0; nI < nSlot; nI++ )
	{
		nR = fat32_vnode_raw_write( pDirNode, (char*)&ent, sizeof( FAT32DirEntStt ) );
		if( nR != sizeof( FAT32DirEntStt ) )
			return(-1 );		// 엔트리를 기록할 수 없다.
	}	

	return( nOffset );
}

// copy directory entries to empty slot
static int save_directory_entry( VNodeStt *pV, int nOffset, FAT32DirEntStt *pDirEnt, int nSlot )
{
	int		nR;
	int		nWriteSize;
	
	nWriteSize = nSlot * sizeof( FAT32DirEntStt );

	// vnode의 파일 포인터를 옮긴다.
	nR = (int)fat32_vnode_lseek( pV, nOffset, FSEEK_SET );
	if( nR < -1 )
		return( -1 );
		
	// 엔트리들을 기록한다.
	nR = fat32_vnode_raw_write( pV, (char*)pDirEnt, nWriteSize );
	if( nR != nWriteSize )
		return( -1 );

	return( nSlot );
}	

//  alter start cluster and save directory entries 
static int nSaveDirEnt( VNodeStt *pV, int nOffset, FAT32DirEntStt *pDirEnt, int nSlot )
{
	int					nR;
	BlkDevObjStt		*pObj;
	DWORD				dwStartCluster;
	FAT32DirEntStt		dir_ent[20+1];			// plus 1 for short alias lost

	memcpy( dir_ent, pDirEnt, nSlot * sizeof( FAT32DirEntStt ) );
	pDirEnt = dir_ent;

	dwStartCluster =  ( (DWORD)pDirEnt[nSlot-1].u.s.wEAHandle << 16 );
	dwStartCluster += pDirEnt[nSlot-1].u.s.wStartCluster;

	// alter start cluster
	if( pV->pVFS->dwType == VFS_TYPE_FAT12 )
	{
		FDDFat12Stt		*pFDDFat12;

		pObj		= pV->pVFS->pDevObj;
		pFDDFat12	= pObj->pPtr;

		if( 0 < dwStartCluster < 0x0FF7 )
			dwStartCluster -= (DWORD)pFDDFat12->nRootBlocks;
	}
	else if( pV->pVFS->dwType == VFS_TYPE_FAT16 )
	{
		F16PartStt		*pF16Part;
		
		pObj		= pV->pVFS->pDevObj;
		pF16Part	= pObj->pPtr;

		if( 0 < dwStartCluster < 0xFFF7 )
			dwStartCluster -= (DWORD)pF16Part->nRootClusters;
	}

	pDirEnt[nSlot-1].u.s.wEAHandle       = (unsigned short)( (DWORD)dwStartCluster >> 16 );
	pDirEnt[nSlot-1].u.s.wStartCluster	= (unsigned short)( dwStartCluster );

	// save
	nR = save_directory_entry( pV, nOffset, pDirEnt, nSlot );
	
	return( nR );
}	

// create a file int the pParentNode
// existance checking is done in the file file.c
static VNodeStt *fat32_vnode_create( VNodeStt *pParentNode, char *pName, DWORD dwType )
{
	DWORD				dwX;
	int					nSlot;
	VFSStt				*pVFS;
	FAT32FileStt		*pFSSD;
	SHORTDIRENT			*pShortEnt;
	FAT32PrivateStt		*pPrivate;
	VNodeStt			*pNewVNode;
	DWORD				dwFreeCluster;
	FAT32DirEntStt		dir_ent[20+1];			// plus 1 for short alias lost
	int					nI, nR, nOffset;
	int					nLongEntOffset, nShortEntOffset;
	
	pVFS     = pParentNode->pVFS;
	pPrivate = pVFS->pPrivate;
				 
	// 새로운 VNode를 하나 할당받는다. (vnode operation이 복사되어 리턴된다.)
	pNewVNode = fat32_new_vnode( pVFS->pVNodeMan );
	if( pNewVNode == NULL )
		return( NULL );

	// pName으로 디렉토리 엔트리를 만든다. 
	nSlot  = fat32_make_dirent( pParentNode, dir_ent, pName, dwType );
	if( nSlot < 0 )
		goto VC_ERROR;

	// 새로운 클러스터를 할당받는다.
	dwFreeCluster = get_free_cluster( pVFS );
	if( dwFreeCluster == 0 )
		goto VC_ERROR;
	
	pShortEnt = &dir_ent[nSlot-1].u.s;
	// short alias의 시작 클러스터 번호를 설정한다.
	dwX = dwFreeCluster;
	dwX = (DWORD)( dwX >> 16 );
	pShortEnt->wEAHandle     = (unsigned short)dwX;
	pShortEnt->wStartCluster = (unsigned short)dwFreeCluster;

	// pParentNode에서 slot개수 만큼의 빈 엔트리를 찾는다.
	nOffset = nFindFreeDirEnt( pParentNode, nSlot );
	if( nOffset < 0 )
		goto VC_ERROR;

	// 생성된 엔트리를 빈 슬롯에 복사한다.
	nR = nSaveDirEnt( pParentNode, nOffset, dir_ent, nSlot );
	if( nR < 0 )
		goto VC_ERROR;
	
	// VNode를 만들어 준다.
	pNewVNode->dwType			= dwType;
	pNewVNode->dwFileSize		= 0;				
	pNewVNode->pVFS				= pVFS;
	pNewVNode->pParentVNode		= pParentNode;		
	strcpy( pNewVNode->szName, pName );
						  	
	nLongEntOffset  = nOffset;
	nShortEntOffset = nOffset + (nSlot-1) * sizeof( FAT32DirEntStt );	
	pFSSD			= pNewVNode->pFSSD;

	// FSSD를 설정한다.
	pFSSD->nTotalCluster			= 1;
	pFSSD->dwCurCluster				= dwFreeCluster;		// 현재 클러스터번호
	pFSSD->dwLastCluster			= dwFreeCluster;
	pFSSD->dwStartCluster			= dwFreeCluster;
	pFSSD->dwLastAvailable			= (DWORD)pPrivate->nClusterSize;
	pFSSD->ei.nLongEntOffset		= nLongEntOffset;		// long entry의 오프셋
	pFSSD->ei.nShortEntOffset		= nShortEntOffset;		// short entry의 오프셋
	memcpy( &pFSSD->ei.short_ent, pShortEnt, sizeof( SHORTDIRENT ) );	// short entry 자체를 복사해 둔다.

	//  VNode의 파일명으로 Hash한다.
	register_vnode_hash( pVFS->pVNodeMan, pNewVNode );

	// 마지막 Chain임을 설정한다.
	set_next_cluster_number( pNewVNode, dwFreeCluster, (DWORD)0x0FFFFFFF );

	// 디렉토리일 경우 (.) (..)을 만들어 주어야 한다.
	if( dwType & FT_DIRECTORY )
	{
		FAT32FileStt *pParentFSSD;

		// 일단 0으로 clear한다.
		memset( dir_ent, 0, sizeof( FAT32DirEntStt ) *3 );

		pParentFSSD = pParentNode->pFSSD;
		// name, attr, start cluster만 지정해 주면 된다.
		strcpy( dir_ent[0].u.s.name, ".          " );	// name.ext
		dir_ent[0].u.s.byAttr = (UCHAR)FT_DIRECTORY;
		dwX = dwFreeCluster;
		dwX = (DWORD)( dwX >> 16 );
		dir_ent[0].u.s.wEAHandle	 = (unsigned short)dwX;
		dir_ent[0].u.s.wStartCluster = (unsigned short)dwFreeCluster;
		
		// parent directory
		strcpy( dir_ent[1].u.s.name, "..         " );	// name.ext
		dir_ent[1].u.s.byAttr = (UCHAR)FT_DIRECTORY;

		dwX = pParentFSSD->dwStartCluster;
		dwX = (DWORD)( dwX >> 16 );
		dir_ent[1].u.s.wEAHandle	 = (unsigned short)dwX;
		dir_ent[1].u.s.wStartCluster = (unsigned short)pParentFSSD->dwStartCluster;

		// 파일에 기록한다.
		nI = fat32_vnode_write( pNewVNode, 0, (char*)dir_ent, sizeof(FAT32DirEntStt)*3 );
		if( nI < 0 )
		{
			ERROR_PRINTF( "fat32_vnode_create() - [.], [..] write error.\n" );
		}
	}	

	return( pNewVNode );

VC_ERROR:
	fat32_delete_vnode( pNewVNode );
	return( NULL );
}

// create a file entry int the pParentNode, no data cluster is allocated.
// existance checking is done in the file file.c
// this function is used in the fat32_vnode_rename function.
static VNodeStt *fat32_vnode_create_entry( VNodeStt *pParentNode, char *pName, DWORD dwType )
{
	int					nSlot;
	VFSStt				*pVFS;
	FAT32FileStt		*pFSSD;
	SHORTDIRENT			*pShortEnt;
	FAT32PrivateStt		*pPrivate;
	VNodeStt			*pNewVNode;
	DWORD				dwFreeCluster;
	FAT32DirEntStt		dir_ent[20+1];			// plus 1 for short alias lost
	int					nR, nOffset, nLongEntOffset, nShortEntOffset;
	
	pVFS     = pParentNode->pVFS;
	pPrivate = pVFS->pPrivate;
				 
	// allocate new vnode, (vnode operation will be duplicated.)
	pNewVNode = fat32_new_vnode( pVFS->pVNodeMan );
	if( pNewVNode == NULL )
		return( NULL );

	// make directory entry by pName. 
	nSlot  = fat32_make_dirent( pParentNode, dir_ent, pName, dwType );
	if( nSlot < 0 )
		goto VC_ERROR;

	pShortEnt = &dir_ent[nSlot-1].u.s;
	// set start cluster to 0
	pShortEnt->wEAHandle     = 0;
	pShortEnt->wStartCluster = 0;

	// find empty slot in pParentNode
	nOffset = nFindFreeDirEnt( pParentNode, nSlot );
	if( nOffset < 0 )
		goto VC_ERROR;

	// copy created entries to empty slots
	nR = nSaveDirEnt( pParentNode, nOffset, dir_ent, nSlot );
	if( nR < 0 )
		goto VC_ERROR;
	
	// make vnode
	pNewVNode->dwType			= dwType;
	pNewVNode->dwFileSize		= 0;				
	pNewVNode->pVFS				= pVFS;
	pNewVNode->pParentVNode		= pParentNode;		
	strcpy( pNewVNode->szName, pName );
						  	
	nLongEntOffset  = nOffset;
	nShortEntOffset = nOffset + (nSlot-1) * sizeof( FAT32DirEntStt );	
	pFSSD			= pNewVNode->pFSSD;
	
	// set FSSD
	dwFreeCluster					= 0;
	pFSSD->nTotalCluster			= 1;
	pFSSD->dwCurCluster				= dwFreeCluster;
	pFSSD->dwLastCluster			= dwFreeCluster;
	pFSSD->dwStartCluster			= dwFreeCluster;
	pFSSD->dwLastAvailable			= 0;
	pFSSD->ei.nLongEntOffset		= nLongEntOffset;		// long entry offset
	pFSSD->ei.nShortEntOffset		= nShortEntOffset;		// short entry offset
	memcpy( &pFSSD->ei.short_ent, pShortEnt, sizeof( SHORTDIRENT ) );	// short entry 자체를 복사해 둔다.

	//  hash by vnode filename
	register_vnode_hash( pVFS->pVNodeMan, pNewVNode );

	return( pNewVNode );

VC_ERROR:
	fat32_delete_vnode( pNewVNode );
	return( NULL );
}

// delete file entry pVNode int pParentNode
static int fat32_vnode_remove_entry( VNodeStt *pVNode )
{
	SHORTDIRENT			ent;
	FAT32FileStt		*pFSSD;
	int					nI, nR, nSize;
	VNodeStt			*pParentNode;

	pFSSD = pVNode->pFSSD;
	pParentNode = pVNode->pParentVNode;

	// delete long entry
	if( pFSSD->ei.szLongName[0] != 0 )
	{	// nSize만큼 엔트리를 지운다.
		nSize = pFSSD->ei.nShortEntOffset - pFSSD->ei.nLongEntOffset;
		for( nI = 0; nSize > 0; )
		{	// long entry를 읽는다.
			nR = fat32_vnode_read( pParentNode, pFSSD->ei.nLongEntOffset + nI, (char*)&ent, sizeof( ent ) );
			if( nR != sizeof( ent ) )
				return( -1 );			

			// set 0xE5
			ent.name[0] = (BYTE)0xE5;

			// rewrite entries
			nR = fat32_vnode_write( pParentNode, pFSSD->ei.nLongEntOffset + nI, (char*)&ent, sizeof( ent ) );
			if( nR != sizeof( ent ) )
				return( -1 );			// unrecoverable critical error  ( delete a part of long entried !!! )

			nI		+= sizeof( ent );
			nSize	-= sizeof( ent );
		}
	}

	// delete short short entry
	if( pFSSD->ei.szShortName[0] != 0 )
	{
		nR = fat32_vnode_read( pParentNode, pFSSD->ei.nShortEntOffset, (char*)&ent, sizeof( ent ) );
		if( nR != sizeof( ent ) )		// unrecoverable critical error.   ( delete long entries only !!! )
			return( -1 );			

		// set 0xE5
		ent.name[0] = (BYTE)0xE5;

		// rewrite entries
		nR = fat32_vnode_write( pParentNode, pFSSD->ei.nShortEntOffset, (char*)&ent, sizeof( ent ) );
	}	

	return( 0 );
}	

// delete file entry pS int pParentNode directory
static int fat32_vnode_remove( VNodeStt *pParentNode, char *pS )
{
	int					nR;
	VNodeStt			*pV, vnode;
	FAT32FileStt		*pFSSD, fssd;

	pV					= &vnode;
	pV->pFSSD			= pFSSD = &fssd;
	pV->pVFS			= pParentNode->pVFS;
	pV->pParentVNode	= pParentNode;								
	strcpy( pV->szName, pS );	

	// find file entry
	nR = fat32_fssd_find_dirent( pV, pFSSD );
	if( nR < 0 )	// file not found
		return( -1 );

	// delete the found entry
	nR = fat32_vnode_remove_entry( pV );

	return( nR );
}

// convert fat12 to fat32
static int cheat_fat12( VFSStt *pVFS, FAT32PrivateStt *pPrivate, int nRootBlocks )
{
	int				nR;
	DWORD			*pFat;
	BlkDevObjStt	*pObj;
	char			*pRoot;
	BYTE			*pOrgFat;
	int				nFatBlocks;
	FDDFat12Stt		*pFDDFat12;
	int				nFatEntries, nFatIndex, nI;
							
	// allocate fdd fat12 device object
	pObj = (BlkDevObjStt*)MALLOC( sizeof( BlkDevObjStt ) );
	if( pObj == NULL )
		return( -1 );
	memset( pObj, 0, sizeof( BlkDevObjStt ) );

	// allocate root directory buffer
	pRoot = (char*)MALLOC( nRootBlocks * 512 );
	if( pRoot == NULL )
	{
		FREE( pObj );
		return( -1 );
	}	
	memset( pRoot, 0, nRootBlocks * 512 );

	// allocate fat buffer
	nFatEntries = (int)pPrivate->dwBigTotalSectors + nRootBlocks + 2;
	nFatBlocks  = (int)( nFatEntries + 127 ) / 128;
	nFatEntries = (int)( nFatBlocks * 128 );
	pFat = (DWORD*)MALLOC( nFatEntries * 4 ); 
	if( pFat == NULL )
	{
		FREE( pObj );
		FREE( pRoot );
		return( -1 );
	}
	memset( pFat, 0, nFatEntries * 4 );
	
	// allocate temporary buffer for fat12 entries
	pOrgFat = (BYTE*)MALLOC( 512 * (int)pPrivate->dwBigSectorsPerFat );
	if( pOrgFat == NULL )
	{
		FREE( pObj );
		FREE( pRoot );
		FREE( pFat );
		return( -1 );
	}
	memset( pOrgFat, 0, 512 * (int)pPrivate->dwBigSectorsPerFat );

	// read fat12 entries
	nR = read_block( pVFS->pDevObj, pPrivate->dwFATLoc, pOrgFat, (int)pPrivate->dwBigSectorsPerFat );
	if( nR < 0 )
		goto CRITICAL_ERROR;

	// read root sectors
	nR = read_block( pVFS->pDevObj, pPrivate->dwRootLoc, pRoot, nRootBlocks );
	if( nR < 0 )
		goto CRITICAL_ERROR;
	
	// open FDD FAT12 Block device 
	nR = open_block_device( pObj, FDD_FAT12_MAJOR, 0 );	// minor = 0
	if( nR < 0 )
	{
CRITICAL_ERROR:
		FREE( pObj );
		FREE( pRoot );
		FREE( pFat );
		FREE( pOrgFat );
		return( -1 );
	}
	// pObj->pPtr pointer is valid after openning block device
	pFDDFat12 = pObj->pPtr;

	// set pointers
	pFDDFat12->pFat		   = (char*)pFat;
	pFDDFat12->pRootDir	   = pRoot;
	pFDDFat12->nFatBlocks  = nFatBlocks;
	pFDDFat12->nRootBlocks = nRootBlocks;
	pFDDFat12->nFatEntries = nFatEntries;

	// split fat entry, add the number of root sector and fill pFDDFat12->pFat entries
	pFat[0] = pFat[1] = 0x0FFFFFFF;
	for( nFatIndex = nRootBlocks + 2, nI = 2; nI < (int)pPrivate->dwBigTotalSectors+2;  )
	{
		WORD	*pX, *pY, wX, wY;
		int		nT;

		nT = ( nI * 3 ) / 2;

		pX = (WORD*)&pOrgFat[nT];
		pY = (WORD*)&pOrgFat[nT+1];
		wX = pX[0];
		wY = pY[0];
		wX = (WORD)( wX & (WORD)0x0FFF );
		wY = (WORD)( wY >> 4 );
		
		if( wX == 0 )
			pFat[nFatIndex++] = 0;
		else if( 0 < wX && wX < (unsigned short)0x0FF7)
			pFat[nFatIndex++] = (DWORD)wX + (DWORD)nRootBlocks;
		else 
			pFat[nFatIndex++] = (DWORD)wX + (DWORD)0x0FFFF000;

		if( wY == 0 )
			pFat[nFatIndex++] = 0;
		else if( 0 < wY && wY < (unsigned short)0x0FF7)
			pFat[nFatIndex++] = (DWORD)wY + (DWORD)nRootBlocks;
		else 
			pFat[nFatIndex++] = (DWORD)wY + (DWORD)0x0FFFF000;

		nI += 2;
	}			

	// fill root dir fat entries
	for( nI = 2; nI < nRootBlocks - 1 + 2; nI++ )
		pFat[ nI ] = (DWORD)( nI + 1 );
	pFat[nI] = (DWORD)0x0FFFFFFF;		// end of chain
	   	
	// alter pPrivate->nRootLoc
	pPrivate->dwBigTotalSectors	 += 2 * ( (DWORD)nFatBlocks - pPrivate->dwBigSectorsPerFat );
	pPrivate->dwBigSectorsPerFat = (DWORD)nFatBlocks;		// assign increased number of fat block
	pFDDFat12->nRelocation = (int)pPrivate->dwRootLoc;		// do not delete
	pPrivate->dwRootLoc    = pPrivate->dwFATLoc + ( pPrivate->dwBigSectorsPerFat * 2 );
	pFDDFat12->nRootLoc    = (int)pPrivate->dwRootLoc;
	pFDDFat12->nFatLoc     = (int)pPrivate->dwFATLoc;
	pFDDFat12->nRelocation = (int)pPrivate->dwRootLoc - pFDDFat12->nRelocation;

	// set base device object
	pFDDFat12->pBaseDevObj = pVFS->pDevObj;
	// replace block device object of vfs
	pVFS->pDevObj = pObj;

	// release original fat buffer
	FREE( pOrgFat );

	return( 0 );
}

// recover fat12 cheat
static int recover_fat12_cheat( VFSStt *pVFS )
{
	FDDFat12Stt		*pFDDFat12;
	BlkDevObjStt	*pObj;

	pObj = pVFS->pDevObj;
	pFDDFat12 = pObj->pPtr;

	// recover original blkdev object ( FDDFAT12 -> FDD35 )
	pVFS->pDevObj = pFDDFat12->pBaseDevObj;

	// close block device
	close_block_device( pObj );		// pFDDFAT12 is released in this close_block_device function.

	FREE( pObj );

	return( 0 );
}

// convert fat16 to fat32 ///////////////////////////////////////////////////////////////////////
static int cheat_fat16( VFSStt *pVFS, FAT32PrivateStt *pPrivate, int nRootBlocks )
{
	int				nR;
	BlkDevObjStt	*pObj;
	F16PartStt		*pF16Part;
							
	// allocate fdd fat16 device object
	pObj = (BlkDevObjStt*)MALLOC( sizeof( BlkDevObjStt ) );
	if( pObj == NULL )
		return( -1 );
	else
		memset( pObj, 0, sizeof( BlkDevObjStt ) );

	// open FDD FAT16 block device 
	nR = open_block_device( pObj, FAT16_PART_MAJOR, 0 );	// minor = 0
	if( nR < 0 )
	{
		FREE( pObj );
		return( -1 );
	}

	// set F16 data fields
	pF16Part = pObj->pPtr;
	pF16Part->nRootBlocks   = nRootBlocks;
	pF16Part->nRootLoc      = pPrivate->dwRootLoc;
	pF16Part->nRootClusters = (nRootBlocks + (int)(pPrivate->bySectorsPerCluster-1)) / (int)pPrivate->bySectorsPerCluster;
	pF16Part->nRelocation   = ( pF16Part->nRootClusters * (int)pPrivate->bySectorsPerCluster ) - nRootBlocks;
	pF16Part->nDataLoc      = pF16Part->nRootLoc + ( pF16Part->nRootClusters * (int)pPrivate->bySectorsPerCluster );

	// set base device object
	pF16Part->pBaseDevObj = pVFS->pDevObj;
	// replace block device object of vfs
	pVFS->pDevObj = pObj;

	return( 0 );
}

// recover fat12 cheat
static int recover_fat16_cheat( VFSStt *pVFS )
{
	F16PartStt		*pF16Part;
	BlkDevObjStt	*pObj;

	pObj = pVFS->pDevObj;
	pF16Part = pObj->pPtr;

	// recover original blkdev object ( FDDFAT12 -> FDD35 )
	pVFS->pDevObj = pF16Part->pBaseDevObj;

	// close block device
	close_block_device( pObj );		// pF16Part is released in this close_block_device function.

	FREE( pObj );

	return( 0 );
}

// find the file in pDir
int fat32_find_dirent( void *pVoidDir, char *pFileName )
{
	int			nR;
	DIRENTStt	de;
	VNodeStt	*pDir;
	DWORD		dwOffs;

	pDir = pVoidDir;

	// rewind pointer
	fat32_vnode_lseek( pDir, (long)0, FSEEK_SET );

	for( dwOffs = 0; ; )
	{	
		// get current file pointer
		dwOffs = (DWORD)fat32_vnode_lseek( pDir, (long)0, FSEEK_CUR );

		// read next directory entry
		nR = fat32_vnode_readdir( pDir, dwOffs, &de );
		if( nR < 0 )
			return( 0 );	// not found

		if( strcmpi( de.szFileName, pFileName ) == 0 || strcmpi( de.szAlias, pFileName ) == 0 )
			return( 1 );	// found
	}
}
