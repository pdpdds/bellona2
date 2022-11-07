#include "bellona2.h"

static int kf_KC_HELP( int argc, char *argv[] );

typedef enum {
	KC_UNKNOWN	= 0,	KC_CLS		= 1,	KC_REBOOT,			KC_DUMP,
	KC_UASM,			KC_REG,				KC_HELP,			KC_TSS,
	KC_DR,				KC_SYMBOL,			KC_VER,				KC_GO,
	KC_TRACE,			KC_PROCEED, 		KC_INT1,			KC_INT3,
	KC_BREAK, 			KC_PROC,			KC_BC,				KC_EXEC,
	KC_THREAD,			KC_GDT,				KC_SRC,				KC_DUMPHDD,
	KC_CHKMEM,			KC_FDDTEST,			KC_CUREIP,			KC_IDT,
	KC_STACK,			KC_KILL,			KC_MAP,				KC_FG,
	KC_CDEV,			KC_SHMEM,			KC_CD_INFO,			KC_SIGNAL,
	KC_MODULE,			KC_UNLOAD,			KC_DBGINFO,			
	KC_PICK,			KC_EDITMEM,			KC_ASM,				KC_V86LIB,
	KC_L50,				KC_VMODE,			KC_L25,				KC_EXP,
	KC_START_GUI,       KC_RESET_FDD,       KC_FILE_TEST,		KC_TIME, 
	KC_VCON,			KC_TSTATE,			KC_SCHQ,			KC_ADDRS,

	END_OF_KC
} KCTag;

extern DWORD dwReadCMOS( DWORD dwReg );  

// CMOS���� ���� �ð�(BCD)�� �о Binary�� ������ �� TTime����ü�� �����Ѵ�.
void read_cmos_time( TTimeStt *pTime )
{
	DWORD dwR;
    int   nSec, nMin, nHour, nWeek, nDay, nMon, nYear;

	for( dwR = 0x80; dwR & 0x80; )  // ���� �������� A�� ��Ʈ 7�� 1�� �ɶ����� ��ٸ���.
		dwR = dwReadCMOS( (DWORD)0x0A );

    nSec  =  (int)dwReadCMOS( (DWORD)0 );  // ��
    nMin  =  (int)dwReadCMOS( (DWORD)2 );  // ��
    nHour =  (int)dwReadCMOS( (DWORD)4 );  // ��
    nWeek =  (int)dwReadCMOS( (DWORD)6 );  // ����
    nDay  =  (int)dwReadCMOS( (DWORD)7 );  // ��
    nMon  =  (int)dwReadCMOS( (DWORD)8 );  // ��
    nYear =  (int)dwReadCMOS( (DWORD)9 );  // ��

    // BCD�� �����̹Ƿ� Binary�� �ٲپ��ش�.
    pTime->nSec  = (nSec  % 16) +(nSec  / 16)*10;
    pTime->nMin  = (nMin  % 16) +(nMin  / 16)*10;
    pTime->nHour = (nHour % 16) +(nHour / 16)*10;
    pTime->nWeek = (nWeek % 16) +(nWeek / 16)*10;
    pTime->nDay  = (nDay  % 16) +(nDay  / 16)*10;
    pTime->nMon  = (nMon  % 16) +(nMon  / 16)*10;
    pTime->nYear = (nYear % 16) +(nYear / 16)*10;
	pTime->nYear += 2000;
}

DWORD dump_memory( DWORD dwAddr, DWORD dwSize )
{
	UCHAR	*pB;
	int		nX, nY;
	char	szT[16], nI, nFlag;
	DWORD	dwT, dwI, dwLastAddress;

	pB = (UCHAR*)dwAddr;
	
	// set alignment to 16 for fine view
	for( dwT = 0;; )
	{
		if( ((DWORD)pB % 16 ) == 0 )
			break;
		else
		{
			pB--;
			dwSize++;
			dwT++;
		}
	}

    // clear string buffer by space
	memset( szT, ' ', sizeof( szT ) );
	nFlag = nI = 0;

	for( dwI = 0; dwI < dwSize; dwI++ )
	{
		if( (dwI % 16 ) == 0 )
		{	
			if( nFlag == 0 )
				nFlag++;
			else
			{	// display char itself
				get_cursor_xy( &nX, &nY );
				nWriteToVideoMem_Len( 80 - 17 , nY, szT, 16 );
				memset( szT, ' ', sizeof( szT ) );
				nI = 0;
			}	
			
			// display addres
			kdbg_printf( "\n %08X  ", (DWORD)&pB[dwI] );
		}
		else if( (dwI % 8 ) == 0 )  // 8th
			kdbg_printf( " -" );
		
		if( dwT > 0 )
		{
			kdbg_printf( "   " );  // just fill by space
			dwT--;
			szT[nI++] = ' ';
		}
		else
		{				
			if( check_memory_validity( 0, (DWORD)&pB[dwI] ) == -1 )
				return( 0xFFFFFFFF );
			
			dwLastAddress = (DWORD)&pB[ dwI ];
			kdbg_printf( " %02X", pB[ dwI ] );			// HEX
			if( pB[dwI] == 0 )		// nWriteToVideoMem_Len�� �߰��� 0�� ������ ����� �ߴ��ϹǷ� ' '���� ������ �ش�.
				szT[nI++] = ' ';
			else
				szT[nI++] = pB[dwI];
			
		}	  
	}

	// display the last line
	get_cursor_xy( &nX, &nY );
	nWriteToVideoMem_Len( 80 - 17 , nY, szT, 16 );

	kdbg_printf( "\n" );

	// ������ �׳� 'd' �� �Է��ϸ� dwLastAddress �������� ��µȴ�.
	return( dwLastAddress + 1);
};			

// dump memory
static G_dwNextDumpAddr = 0;
static int kf_KC_DUMP( int argc, char* argv[] )
{
	DWORD dwAddr, dwSize, dwR;

	// �ּҰ� �������� �ʾ����� ������ �����ߴ� �ٷ� ���� �ּҺ��� ����Ѵ�.
	if( argc < 2 )
		dwAddr = G_dwNextDumpAddr;

	if( argc >=2 )
		dwAddr = dwHexValue( argv[1] );
	
	if( argc >= 3 )
		dwSize = dwHexValue( argv[2] );
	else
		dwSize = 16*10;		// �⺻������ 10���� ����Ѵ�.

	dwR = dump_memory( dwAddr, dwSize );
	if( dwR != 0xFFFFFFFF )
	    G_dwNextDumpAddr = dwR;

	return( 0 );
}

// ��� ����� ���� �Լ� �̸����� �Լ� ����ü�� ã�´�.
static MyCoffDbg2FuncStt *find_func_ent( char *pFuncName, ModuleStt **ppM )
{
	int					nI;
	ModuleStt			*pM;
	SysModuleStt 		*pSM;
	MyCoffDbg2FuncStt	*pFunc;
	
	pSM = get_sys_module();
	for( nI = 0; nI < MAX_MODULE; nI++ )
	{
		pM = pSM->mod[nI];
		if( pM == NULL )
			continue;		
	
		pFunc = get_func_ent_by_name( pM->pMyDbg, pFuncName );
		if( pFunc != NULL )
		{	// ã�� ����� �ּҿ� �Բ� �����Ѵ�.
			ppM[0] = pM;
			return( pFunc );
		}
	}
	return( NULL );
}

// unassembling
static int kf_KC_UASM( int argc, char* argv[] )		
{
	TSSStt				*pTSS;
	DWORD				dwEIP;
	MyCoffDbg2FuncStt	*pFunc;
	int					nMaxDispLine;
	
	// set initial EIP
	if( argc < 2 )			// no parameter
	{	// start address is eip of the tss' backlink
		pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
		if( pTSS == NULL )
			return( 0 );
		else
		{
			if( pTSS->dwEFLAG & MASK_VM )
				dwEIP = pTSS->dwEIP + (DWORD)( (DWORD)pTSS->wCS << 4 );
			else			
				dwEIP = pTSS->dwEIP;
		}
		nMaxDispLine = 20;
	}
	else
	{	// unassemble absolute address
		if( is_digit( argv[1] ) )	
			dwEIP = dwHexValue( argv[1] );
		else	
		{	
			ModuleStt *pM;
			// �Լ� �̸����� �ּҸ� ã�� ����.
			pFunc = find_func_ent( argv[1], &pM );
			if( pFunc != NULL )
				dwEIP = pFunc->dwAddr + pM->dwLoadAddr;
			else
			{	// �Լ��� ã�� �� ����.
				kdbg_printf( "function %s not found!\n", argv[1] );
				return( -1 );	// function not found
			}
		}
		nMaxDispLine = get_vertical_line_size();
	}

	// ������ �޸𸮿� ������ �� �ִ��� �˻�
	if( check_memory_validity( 0, (DWORD)dwEIP ) == -1 )
	{
		kdbg_printf( "\ninvalid memory\n" );
		return(-1);
	}

	unassembling( dwEIP, nMaxDispLine );

	return( 0 );
}

// display source filenames
static int kf_KC_SRC( int argc, char* argv[] )
{
	int					nI, nLinesPerPage;
	MyCoffDbg2FileStt	*pFile;
	char				*pFileName;

	if( pMy == NULL )
	{
		kdbg_printf( "no debug information!\n" );
		return( 0 );
	}
	
	nLinesPerPage = get_vertical_line_size() - 2;
	for( nI = 0; nI < pMy->nTotalFileEnt; nI++ )
	{
		pFile = &pMy->pFileTbl[nI];

		pFileName = &pMy->pStrTbl[ pFile->nNameIndex ];

		kdbg_printf( "[%2d] %-13s 0x%08X  (0x%08X / %5d)bytes (%5d)lines\n",
			nI, pFileName, pFile->dwAddr + (DWORD)RELOC_BASE, pFile->dwSize, pFile->dwSize, pFile->nTotalLine );

		if( nI > 0 && (nI % nLinesPerPage) == 0 )
		{
			if( getchar() == 27 )
			    break;
		}
	}

	return( 0 );
}

// ����� ������ ���Ե� �ɺ����� ����Ѵ�.
static int disp_module_symbol( char *pModuleName, MyCoffDbg2Stt *pDbg, DWORD dwBaseAddr, char *pPrefix, int *pDisplayed )
{
	MyCoffDbg2FuncStt	*pFunc;
	char				*pFuncName;
	int					nPrefixSize, nIndex, nLinesPerPage;

	if( pDbg == NULL )
	{
		kdbg_printf( "No debug information!\n" );
		return( 0 );
	}
	else if( (DWORD)pDbg > 0x80000000 )
	{	// User Level ���.
		if( check_memory_validity( 0, (DWORD)pDbg ) < 0 )																
		{
			kdbg_printf( "%s: deactivated user level module!\n", pModuleName );
			return( 0 );
		}
	}

	// �ùٸ� ����� ����?
	if( memcmp( pDbg->szMagicStr, MY_COFF_DBG2_MAGIC_STR, 4 ) != 0 )
	{
		kdbg_printf( "disp_module_symbol: invalid debug info!\n" );
		return( -1 );
	}	
	
	if( pPrefix != NULL )
	    nPrefixSize = strlen( pPrefix );

	nLinesPerPage = get_vertical_line_size() - 2;
	for( nIndex = 0; nIndex < pDbg->nTotalFuncEnt; nIndex++ )
	{
		pFunc = &pDbg->pFuncTbl[ pDbg->pFuncNameIndex[ nIndex ] ];
		pFuncName = &pDbg->pStrTbl[ pFunc->nNameIndex ];
		
		// �Լ��� ���� '_'�� ���� ����Ѵ�. 
		if( pFuncName[0] == '_' )
			pFuncName++;
			
		if( pPrefix != NULL && pPrefix[0] != 0 )
		{
			if( memcmp( pPrefix, pFuncName, nPrefixSize ) != 0 )
			    continue;
		}	

		kdbg_printf( "[%4d] 0x%08X [%s] %s (size=0x%X)\n", 
			nIndex, dwBaseAddr + pFunc->dwAddr, pModuleName, pFuncName, pFunc->dwSize );

		pDisplayed[0]++;	// ��µ� ������ ���� ��Ų��.
		
		if( pDisplayed[0] > 0 && ( pDisplayed[0] % nLinesPerPage ) == 0 )
		{
			if( getchar() == 27 )
			    return( -1 );
		}
	}
	
	return( 0 );
}

// �����ϰ� �ִ� �ɺ��� ����Ѵ�.
static int kf_KC_SYMBOL( int argc, char* argv[] )
{
	int 			nI;
	ModuleStt		*pM;
	SysModuleStt 	*pSM;
	char 			*pPrefix, *pModPrefix;
	int 			nR, nDisplayed, nModPrefixLength;

	// prefix�� �����Ǿ� ������ prefix�� �����ϴ� �ɺ��� ����Ѵ�.
	if( argc >= 2 )
	{
	    pPrefix = argv[1];
		if( strcmpi( pPrefix, "*" ) == 0 )
			pPrefix = "";
	}
	else
	    pPrefix = NULL;
	    
	// ����̸��� �����Ǿ� ������ �ش� ��⿡���� �ɺ��� ã�´�.
	if( argc >= 3)
	{
	    pModPrefix = argv[2];
		nModPrefixLength = strlen( pModPrefix );
	}
	else
		pModPrefix = NULL;    

	// ��� ��������� ����� ����ü�� ��´�.	    
	pSM = get_sys_module();
	
	nDisplayed = 0;
	for( nI = 0; nI < MAX_MODULE; nI++ )
	{
		pM = pSM->mod[nI];
		if( pM == NULL )
			continue;		
			
		if( pModPrefix != NULL )
		{	// ������ Prefix�� �ٸ� ����� �ǳʶڴ�.
			if( memcmp( pModPrefix, pM->szAlias, nModPrefixLength ) != 0 )
				continue;
		}	
		
		nR = disp_module_symbol( pM->szAlias, pM->pMyDbg, pM->dwLoadAddr, pPrefix, &nDisplayed );
	}
	kdbg_printf( "Total %d symbols.\n", nDisplayed );

	return( 0 );
}

// INT3 ��ɿ� ���� �ּҿ� 0xCC�� �ɴ´�.
static int kf_KC_INT3( int argc, char *argv[] )
{
	DWORD	dwAddr;
	UCHAR	*pX;
	int		nClear, nR, nI, nTotal;
	
	if( argc < 2 )		// INT3�� ����Ʈ�� �����ش�.
	{
		kdbg_printf( "INT3 Breakpoint list.\n" );
		kdbg_printf( "usage: int3 <address> [x]\n" );
		for( nTotal = nR = nI = 0; nI < MAX_INT3_ENT; nI++ )
		{
			if( int3_ent[nI].dwAddr != 0 )
			{
				kdbg_printf( "[%2d] 0x%08X\n", nR++, int3_ent[nI].dwAddr );
				nTotal++;
			}
		}			   
		kdbg_printf( "Total %d INT3 entries.\n", nTotal );
		
		return( 0);
	}			

	nClear = 0; // INT3 addr x   (���� x�� ������ �����Ѵ�.)
	if( argc >= 3 && strcmpi( argv[2], "x" ) == 0 )
		nClear = 1;

	// �ּҸ� 16���� ������ �����Ѵ�.
	dwAddr	= dwHexValue( argv[1] );
	pX		= (UCHAR*)dwAddr;

	// ���� �ɼ��ε� �̹� 0xCC�̴�.
	if( nClear == 0 && pX[0] == (UCHAR)0xCC )
	{
		kdbg_printf( "error : [0x%08X] is already 0xCC.\n", dwAddr );
		return( -1 );
	}
	// ���� �ɼ��ε� 0xCC�� �ƴϴ�.
	if( nClear != 0 && pX[0] != (UCHAR)0xCC )
	{
		kdbg_printf( "error : [0x%08X] is not 0xCC (%02X).\n", dwAddr, pX[0] );
		return( -1 );
	}


	// �ּҿ� ���� ����Ʈ�� �����ϰ� 0xCC�� �ɴ´�..
	if( nClear == 0 )
		nR = overwrite_cc( pX );	// �ɴ´�.
	else
		nR = recover_cc( pX );		// �����Ѵ�.
				   	
	return( nR );	// 1 = ����, 0 = ����
}

// ������ ������ ����� CALL�̸� ���� ����Ʈ�� CC�� �ٲٰ� �ּҿ� ���� ����Ʈ�� ����, 1����
static char _strOffset[32], _strIP[12], _strOpCode[32], _strOperand[100], _strSize[12], _strDump[64];
static char *strArray[] = {_strOffset, _strIP, _strOpCode, _strOperand, _strSize, _strDump };
int nIsCallThenImplantCC()
{
	OpStt	Op;
	UCHAR	*pBuff;			// ��������� �ڵ��� ���� ��ġ
	int		nSize;          // ��� ��������� �ڵ��� ũ��
	TSSStt	*pTSS;
				  	
	pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
	if( pTSS == NULL )
		return( 0 );

	if( pTSS->dwEFLAG & MASK_VM )
		pBuff = (UCHAR*)pTSS->dwEIP + (DWORD)( (DWORD)pTSS->wCS << 4 );
	else
		pBuff = (UCHAR*)pTSS->dwEIP;

	// �� ������ ������� �Ѵ�.
	nSize = nDisAssembleOneCode( (DWORD)pBuff, &Op, pBuff, strArray );

	// CALL ����� �ƴϸ� 0�� �����ϰ� ���ư���.
	if( Op.wType != ot_CALL )
		return( 0 );

	// ���� ����Ʈ�� �̹� INT3�̸� �׳� ���ư���.
	if( pBuff[nSize] == (UCHAR)0xCC )
	{
		kdbg_printf( "next byte is already CC\n" );
		return( 0 );
	}

	// �ּҿ� ���� ����Ʈ�� �����ϰ� 0xCC�� �ɴ´�..
	if( overwrite_cc( &pBuff[nSize] ) == 1 )
	{
		kdbg_printf( "INT 3 planted\n" );
		return( 1 );	// ���������� 0xCC�� �ɾ���.
	}
	else
	{
		kdbg_printf( "INT 3 is not planted\n" );
		return( 0 );	// 0xCC�� �ɴµ� �����ߴ�.
	}
}

//  �������� ���� �����Ѵ�.
void kdbg_change_register_value( int argc, char *argv[] )
{
	TSSStt		*pTSS;
	int			nI;
    DWORD       dwValue;
	UCHAR		*pByte;
	UINT16		*pWord;
					  	
	pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
	if( pTSS == NULL )
		return;
												   
	// ���ο� ���� ��´�.
	dwValue = dwHexValue( argv[2] );

	// �������� Ÿ���� ��´�.
	uppercase( argv[1] );	// �빮�ڷ� �����Ѵ�.
	nI = nSearchTbl( rsvTbl, 0, MAX_RSVSYM, argv[1] );
	if( nI == -1 )		// �ɺ��� �� ã�Ҵ�.  
		return;				   
	
	switch( rsvTbl[nI].nSubType )
	{
        case rAH : pByte = (UCHAR*)&pTSS->dwEAX; pByte[1] = (UCHAR)dwValue; break;
        case rAL : pByte = (UCHAR*)&pTSS->dwEAX; pByte[0] = (UCHAR)dwValue; break;
        case rBH : pByte = (UCHAR*)&pTSS->dwEBX; pByte[1] = (UCHAR)dwValue; break;
        case rBL : pByte = (UCHAR*)&pTSS->dwEBX; pByte[0] = (UCHAR)dwValue; break;
        case rCH : pByte = (UCHAR*)&pTSS->dwECX; pByte[1] = (UCHAR)dwValue; break;
        case rCL : pByte = (UCHAR*)&pTSS->dwECX; pByte[0] = (UCHAR)dwValue; break;
        case rDH : pByte = (UCHAR*)&pTSS->dwEDX; pByte[1] = (UCHAR)dwValue; break;
        case rDL : pByte = (UCHAR*)&pTSS->dwEDX; pByte[0] = (UCHAR)dwValue; break;

        case rAX : pWord = (UINT16*)&pTSS->dwEAX; pWord[0] = (UINT16)dwValue; break;
        case rBX : pWord = (UINT16*)&pTSS->dwEBX; pWord[0] = (UINT16)dwValue; break;
        case rCX : pWord = (UINT16*)&pTSS->dwECX; pWord[0] = (UINT16)dwValue; break;
        case rDX : pWord = (UINT16*)&pTSS->dwEDX; pWord[0] = (UINT16)dwValue; break;
        case rSI : pWord = (UINT16*)&pTSS->dwESI; pWord[0] = (UINT16)dwValue; break;
        case rDI : pWord = (UINT16*)&pTSS->dwEDI; pWord[0] = (UINT16)dwValue; break;
        case rBP : pWord = (UINT16*)&pTSS->dwEBP; pWord[0] = (UINT16)dwValue; break;
        case rSP : pWord = (UINT16*)&pTSS->dwESP; pWord[0] = (UINT16)dwValue; break;

        case rCS : pTSS->wCS = (UINT16)dwValue; break;
        case rDS : pTSS->wDS = (UINT16)dwValue; break;
        case rES : pTSS->wES = (UINT16)dwValue; break;
        case rFS : pTSS->wFS = (UINT16)dwValue; break;
        case rGS : pTSS->wGS = (UINT16)dwValue; break;
        case rSS : pTSS->wSS = (UINT16)dwValue; break;
    
        case rEAX : pTSS->dwEAX = dwValue; break;
        case rEBX : pTSS->dwEBX = dwValue; break;
        case rECX : pTSS->dwECX = dwValue; break;
        case rEDX : pTSS->dwEDX = dwValue; break;
        case rESI : pTSS->dwESI = dwValue; break;
        case rEDI : pTSS->dwEDI = dwValue; break;
        case rEBP : pTSS->dwEBP = dwValue; break;
        case rESP : pTSS->dwESP = dwValue; break;
        case rEIP : pTSS->dwEIP = dwValue; break;
        case rC3  : pTSS->dwCR3 = dwValue; break;
	}
}

///////////////////////////////////////////////////
static int kf_KC_DUMPHDD( int argc, char *argv[] )
{
	int 	nDrv, dwIndex;
	
	if( argc < 3 )
	{
		kdbg_printf( "usage: dumphdd <drv;0=c> <sector_num>\n" ); 
		return( -1 );
	}
	
	nDrv    = (int)dwDecValue( argv[1] );
	dwIndex = dwDecValue( argv[2] );

	dump_hdd( nDrv, dwIndex );

	return( 0 );
}

// �극��ũ ����Ʈ�� �����Ѵ�.
static int kf_KC_BREAK( int argc, char *argv[] )
{
	DWORD	dwAddr;
	int		nR, nOption, nLength;
	char 	*pOptionStr, *pSizeStr;

	if( argc < 2 )
	{	// ������ ����Ѵ�.
		kdbg_printf( "usage: break <address> [ w | rw | io | x ] [size]\n" );
		
		// �ϵ���� �극��ũ ����Ʈ�� ������ ����Ѵ�.
		disp_hw_breaks();
		return(0);
	}
	else if( argc == 2)
		nOption = BREAK_EXEC;
		
	//  �ɼ��� �����Ǿ� �ִ�.
	if( argc >= 3 )		
	{
		pOptionStr = argv[2];
		if( strcmpi( pOptionStr, "x" ) == 0 )
			nOption = BREAK_EXEC;
		else if( strcmpi( pOptionStr, "w" ) == 0 )
			nOption = BREAK_WRITE;
		else if( strcmpi( pOptionStr, "rw" ) == 0 )
			nOption = BREAK_READWRITE;
		else if( strcmpi( pOptionStr, "io" ) == 0 )
			nOption = BREAK_IO;
		else
		{	// �߸��� �ɼ� ��Ʈ��.
			kdbg_printf( "invalid option : %s\n", pOptionStr );
			return( -1 );
		}	
	}

	// ũ�Ⱑ �����Ǿ���.
	if( argc >= 4 )		
	{
		pSizeStr = argv[3];
		nLength = dwDecValue( pSizeStr ) -1;
		nLength &= 3;
	}
	else
		nLength = 4;

	// �ּҸ� �����Ѵ�.
	dwAddr = dwHexValue( argv[1] );

	// breakpoint�� �����Ѵ�.
	nR = kdbg_set_dr( dwAddr, nOption, nLength );
	if( nR == 0 )
	    kdbg_printf( "ok.\n" );
				 	
	return( nR );
}

static int kf_KC_BC( int argc, char *argv[] )
{
	int		nR;

	if( argc < 2 )
	{	// display usage
		kdbg_printf( "bc <*,0,1,2,3>\n" );
		return(0);
	}
	
	// set debug register
	nR = kdbg_clear_dr( argv[1][0] );
				 	
	return( nR );
}

// clear screen
static int kf_KC_CLS( int argc, char *argv[] )
{
	kdbg_clearscreen();
	return( 0 );
}

// reboot system
static int kf_KC_REBOOT( int argc, char *argv[] )
{
	kdbg_printf( "Reboot system.\n" );
	vRebootSystem();
	
	return( 0 );
}

// display debugee's register
static int kf_KC_REG( int argc, char *argv[] )
{
	if( argc >= 3 )	// �������� ���� �����Ѵ�.	
		kdbg_change_register_value( argc, argv );

	disp_src_and_regs( NULL, 0 );

	return( 0 );
}				

// display tss
static int kf_KC_TSS( int argc, char *argv[] )
{
	int nR;

	nR = nDispTSS( argc, argv );

	return( nR );
}				

// display debug register
static int kf_KC_DR( int argc, char *argv[] )
{
	vDispDebugRegister();

	return(0);
}				

extern int find_3c905b_nic( PCIDeviceStt *pPCI );

//  display logo and version string
void disp_version()
{
	kdbg_printf( "Bellona2 Kernel (c) Copyright 1995-2003 OHJJ (%s)\n", __TIMESTAMP__ );
}

// display kernel logo and version
static int kf_KC_VER( int argc, char *argv[] )
{
	disp_version();

	return(0);
}						

// go
static int kf_KC_GO( int argc, char *argv[] )
{
	if( is_debugger_active() == 0 )
		return( 0 );

	if( argc >= 2  )
	{
		kf_KC_INT3( argc, argv );
	}	

	kdbg_set_debugee_tf( 0 );
	
	return( 1000 ); // shell�� ������.
}

// trace
static int kf_KC_TRACE( int argc, char *argv[] )
{
	int		nRepeat;

	if( is_debugger_active() == 0 )
		return( 0 );

	if( argc >= 2 )
	{	 // trace ȸ���� �����Ǿ���.
		nRepeat = dwDecValue( argv[1] );
		kdbg_set_trace_repeat( nRepeat );
	}	

	kdbg_set_debugee_tf( 1 );
	
	return( 1000 ); // shell�� ������.
}

// proceed
static int kf_KC_PROCEED( int argc, char *argv[] )
{
	if( nIsCallThenImplantCC() == 1 )		// CALL ����� �ƴϸ� t�� �����ϰ� �����Ѵ�.
	{
		kdbg_printf( "ImplantCC() = 1\n" );
		kdbg_set_debugee_tf( 0 );			// TF�� ����.
	}
	else
		kdbg_set_debugee_tf( 1 );
	return( 1000 ); // shell�� ������.
}

// int 1
static int kf_KC_INT1( int argc, char *argv[] )
{
	kdbg_breakpoint( 0 );
	return( 0 );
}

// display process
static int kf_KC_PROC( int argc, char *argv[] )
{
	DWORD dwPID;

	if( argc > 1 )
	{
		dwPID = dwDecValue( argv[1] );
		disp_process( dwPID );
	}
	else
		disp_process_list();

	return( 0 );
}

// display thread
static int kf_KC_THREAD( int argc, char *argv[] )
{
	DWORD	dwTID;

	if( argc > 1 )
	{
		dwTID = dwDecValue( argv[1] );
		disp_thread( dwTID );
	}	
	else
		disp_thread_list();

	return( 0 );
}

// display gdt
static int kf_KC_GDT( int argc, char *argv[] )
{
	vDispGDT( gdt, nTotalGDTEnt );
	return( 0 );
}

// check kernel memory block chain
static int kf_KC_CHKMEM( int argc, char *argv[] )
{
	validate_mem_pool( &kmp );

	return(0);
}

// display unassembled code of current EIP
static int kf_KC_CUREIP( int argc, char *argv[] )
{
	kdbg_disp_next_code();

	return(0);
}

// fdd track reading test
static int kf_KC_FDDTEST( int argc, char *argv[] )
{
	fdd_read_test();

	return(0);
}

// Ŀ�� ������ ���ο� ���α׷��� �ε��Ͽ� �����Ѵ�.
static int kf_KC_EXEC( int argc, char *argv[] )
{
	int					nR;
	int					nPID;
	KExecveParamStt		*pEP;
	
	if( argc < 2 )
	{
		kdbg_printf( "exec <file_path> [parameters...]\n" );
		return( 0 );
	}

	// ������ �����ϴ��� ���� Ȯ���� �� ���μ����� fork �Ѵ�.  (2004-03-26)
	nR = kget_file_info( argv[1], NULL );
	if( nR <= 0 )
	{
		kdbg_printf( "%s not found or error!\n", argv[1] );
		return( -1 );
	}

	// fork new process  ( ���������� kmalloc()�� ����Ѵ�. ���������� ���μ����� ����Ǹ鼭 �����ȴ�.)
	pEP = make_kexecve_param( argv[1], &argv[2], NULL );
	if( pEP == NULL )
	{
		kdbg_printf( "kf_KC_EXEC: pEP = NULL!\n" );
		return( -1 );
	}
	
	nPID = kernel_fork( r0_fork_thread, (DWORD)pEP, (128*1024), TS_READY_NORMAL, get_current_vconsole() );
		
	return( 0 );
}	

// display idt
static int kf_KC_IDT( int argc, char *argv[] )
{
	display_idt( idt );
	return( 0 );
}

// dump stack
static int kf_KC_STACK( int argc, char *argv[] )
{	
	int	nLocalFlag;

	nLocalFlag = 0;
	// ���� ���� ������ �ߴ� ���� CallStack�� �����ֵ��� ����.
	//kdbg_dump_stack();

	if( is_debugger_active() == 0 )
	{
		kdbg_printf( "Kernel debugger is not active.\n" );
		return( 0 );
	}

	if( argc >= 2 && strcmpi( argv[1], "local" ) == 0 )
		nLocalFlag = 1;

	// �ݽ����� ����Ѵ�.
	disp_call_stack( NULL, nLocalFlag );

	return( 0 );
}

// kill thread or process
static int kf_KC_KILL( int argc, char *argv[] )
{
	int		nR;
	DWORD	dwAddr;

	if( argc < 2 )
	{
		kdbg_printf( "kill < thred | process >\n" );
		return( 0 );
	}

	// get address of a process or thread .
	dwAddr = dwHexValue( argv[1] );

	// check if the addrss points a thread structure
	if( is_thread( (ThreadStt*)dwAddr ) )
		nR = kill_thread( (ThreadStt*)dwAddr );		
	else if( is_process( (ProcessStt*)dwAddr ) )
		nR = kill_process( (ProcessStt*)dwAddr );
	else
	{
		kdbg_printf( "0x%08X does not point a thread or process!\n", dwAddr );
		return( 0 );
	}

	return( nR );
}

// kill thread or process
static int kf_KC_MAP( int argc, char *argv[] )
{
	char	szAttr[32];
	DWORD	dwVAddr, *pPD;
	int		nFree, nSingle, nMulti, nR;

	if( argc < 2 )
	{	// display physical memory information
		nFree = get_available_phys_size( &nSingle, &nMulti );
		kdbg_printf( "total size(%d)kb  free(%d)kb  single(%d)kb  multi(%d)kb\n", 
			bell.nPhysSize / 1024, nFree*4, nSingle*4, nMulti*4 );
		nR = -1;

		kdbg_printf( "kmp: (addr=0x%X, size=0x%X)\n", kmp.dwStartAddr, kmp.dwSize );

		// �޸� üũ.
		validate_mem_pool( NULL );

		kdbg_printf( "usage: map <address> [attrib_str]\n" );
		goto BACK;
	}

	// get the virtual address
	dwVAddr = dwHexValue( argv[1] );

	// display mapping relations of the virtual address
	//nR = disp_map( dwVAddr );

	if( is_debugger_active() )
	{
		TSSStt	*pTSS;
		pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
		pPD = (DWORD*)pTSS->dwCR3;
	}
	else
		pPD = bell.pPD;

	// �Ӽ��� �����Ǿ� �ִ°�?
	szAttr[0] = 0;
	if( argc >= 3 )
		strcpy( szAttr, argv[2] );

	// ������ ���丮, ������ ���̺� ��Ʈ���� ����Ѵ�.
	disp_page_dir( pPD, dwVAddr, szAttr );

BACK:
	return( nR );
}

// display char devices
static int kf_KC_CDEV( int argc, char *argv[] )
{
	CharDevObjStt	*pObj;
	int				nTotal, nI;
	
	// find the first registered block device object
	pObj = find_first_chardev_obj( &nTotal );
	if( pObj == NULL )
		return( -1 );

	kdbg_printf( "name           major     minor\n" );
	for( nI = 0; pObj != NULL && nI < nTotal; nI++ )
	{
		kdbg_printf( "%-16s %-4d      %-4d\n", pObj->pDev->szName, pObj->pDev->nMajor, pObj->nMinor );
		pObj = pObj->pNext;
	}
	return( 0 );
}

// display shared memory structure
static int kf_KC_SHMEM( int argc, char *argv[] )
{
	disp_shmem();

	return( 0 );
}

// set foreground thread or process
static int kf_KC_FG( int argc, char *argv[] )
{
	int			nR;
	ThreadStt	*pT;
	ProcessStt	*pP;
	DWORD		dwID;

	if( argc < 2 )
	{
		kdbg_printf( "fg < thred id | process id >\n" );
		return( 0 );
	}

	dwID = dwDecValue( argv[1] );
	pT = find_thread_by_id( dwID );
	if( pT != NULL )
	{
		nR = set_global_foreground_thread( pT );
		return( nR );
	}

	pP = find_process_by_id( dwID );
	if( pP != NULL )
	{
		nR = set_fg_process( NULL, pP );
		return( nR );
	}

	kdbg_printf( "%d is invalid.\n", dwID );

	return( 0 );
}	

// get cd_rom media information
static int kf_CD_INFO( int argc, char *argv[] )
{
	int		nR, nI, nDrv;
	DWORD	dwTotalBlock, dwBlockSize;

	if( argc < 2 )
	{
		kdbg_printf( "cdinfo <drv_number>\n" );
		return(-1);
	}

	nDrv = (int)dwDecValue( argv[1] );

	for( nI = 0; nI < 10; nI++ )
	{
		nR = read_cd_capacity( nDrv, &dwTotalBlock, &dwBlockSize );
		if( nR < 0 )
		{
			kdbg_printf( "read_cd_capacity() function failed!\n" );
			return( -1 );
		}
		if( dwBlockSize != 0 )
			break;
	}
			
	kdbg_printf( "Total Block = %d, Block Size = %d\n", dwTotalBlock, dwBlockSize );
	return( 0 );
}

// send signal
static int kf_KC_SIGNAL( int argc, char *argv[] )
{
	int			nR;
	ThreadStt	*pT;
	ProcessStt	*pP;
	DWORD		dwID, dwSignal;

	if( argc < 3 )
	{
		kdbg_printf( "signal < thread_id | process_id > <signal>\n" );
		disp_signal_str();
		return(-1 );
	}

	dwID = dwDecValue( argv[1] );
	dwSignal = get_signal_bit( argv[2] );
	if( dwSignal == 0 )
	{
		kdbg_printf( "invalid signal value!\n" );
		return( -1 );
	}

	pT = find_thread_by_id( dwID );
	if( pT != NULL )
	{
		nR = send_signal_to_thread( dwID, dwSignal );
		return( nR );
	}				 	
	
	pP = find_process_by_id( dwID );
	if( pP != NULL )
	{	// make send signal to process !!
		nR = -1;
		return( nR );
	}					 		

	kdbg_printf( "id(%d) not found!\n", dwID );
	return( -1 );
}

// load kernel extension module
static int kf_KC_MODULE( int argc, char *argv[] )
{
	ModuleStt	*pM;

	if( argc < 2 )
	{	// ����Ʈ�� ����Ѵ�.
		disp_module();
		return( 0 );
	}

	// ����� �ε��ϰ� ���������� ��Ʈ�� ����Ʈ�� ȣ��ȴ�.
	pM = load_module( argv[1] );
	if( pM == NULL )
		return( -1 );

	return( 0 );
}

// load kernel extension module
static int kf_KC_UNLOAD( int argc, char *argv[] )
{
	int nR;

	if( argc < 2 )
	{
		disp_module();
		return( -1 );
	}

	nR = unload_module( argv[1] );

	return( nR );
}

// load kernel debug information
static int kf_KC_DBGINFO( int argc, char *argv[] )
{
	int					nI;
	ModuleStt			*pM;
	SysModuleStt 		*pSM;
	MyCoffDbg2Stt		*pMyDbg;
	
	pSM = get_sys_module();

	if( argc < 3 )
	{	// �ε�� ������ ����� ������ ����Ѵ�.
		for( nI = 0; nI < MAX_MODULE; nI++ )
		{
			pM = pSM->mod[nI];
			if( pM == NULL )
				continue;

			if( pM->pMyDbg == NULL )
			{
				kdbg_printf( "[%d] %-16s : no debug information\n", nI, pM->szAlias );
				continue;
			}
						
			pMyDbg = (MyCoffDbg2Stt*)pM->pMyDbg;
			kdbg_printf( "[%d] %-12s:size(%6d),files(%3d),func(%4d),line(%4d),strtbl(%6d)\n",
				nI, pM->szAlias,
				pMyDbg->dwSize,       
				pMyDbg->nTotalFileEnt,
				pMyDbg->nTotalFuncEnt,
				pMyDbg->nTotalLineEnt,
				pMyDbg->nStrTblSize );  
		}
		kdbg_printf( "Usage: dbginfo <module_name> <dbginfo_file>\n" );
		return( 0 );
	}

	// ����� ã�´�.
	pM = find_module_by_alias( argv[1], &nI );
	if( pM == NULL )
	{	// ����� ã�� �� ����.
		kdbg_printf( "module %s not found!\n", argv[1] );
		return( -1 );
	}

	// ����� ���� ������ ���� �ε��Ѵ�.
	pMyDbg = load_mydbg2_info( argv[2] );
	if( pMyDbg == NULL )
	{	// ����� ���� ������ �ε��� �� ����.
		kdbg_printf( "dbginfo %s loading failed!\n", argv[2] );
		return( -1 );
	}
	
	pM->pMyDbg = pMyDbg;

	kdbg_printf( "[%d] %-16s : size(%6d), files(%3d), func(%4d), line(%4d), str_tbl(%6d)\n",
		nI, pM->szAlias,
		pMyDbg->dwSize,       
		pMyDbg->nTotalFileEnt,
		pMyDbg->nTotalFuncEnt,
		pMyDbg->nTotalLineEnt,
		pMyDbg->nStrTblSize );  

	kdbg_printf( "ok.\n" );
	return( 0 );
}

static int kf_KC_PICK( int argc, char *argv[] )
{
	DWORD dwTID;

	if( argc < 2 )
	{
		kdbg_printf( "pick <thread_id>\n" );
		return( 0 );
	}

	dwTID = dwDecValue( argv[1] );

	set_pick_thread_id( dwTID );

	return(0);
}

static int kf_KC_EDITMEM( int argc, char *argv[] )
{
	DWORD dwAddr;
	TSSStt *pTSS;

	if( argc < 2 )
	{	// debugger is not active
		if( is_debugger_active() == 0 )
		{
			kdbg_printf( "editmem <addr>\n" );
			return( 0 );
		}
		else  // get debuggee's EIP
		{
			pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
			if( pTSS == NULL )
			{
				kdbg_printf( "editmem - no backlink!\n" );
				return(0);
			}
			dwAddr = pTSS->dwEIP;			
		}
	}
	else
		dwAddr = dwHexValue( argv[1] );

	kedit_memory( dwAddr );

	return(0);
}

static int kf_KC_VMODE( int argc, char *argv[] )
{	
	int		nR;
	DWORD	dwMode;

	if( argc < 2 )
	{
		nR = get_vesa_info();			// ����Ʈ�� ����Ѵ�.
		if( nR == 0 )
			disp_vesa_info();
	}
	else
	{
		dwMode = dwHexValue( argv[1] );
		nR =vesa_mode_test( dwMode );	// ���� ��带 �����Ѵ�.
	}

	return( 0 );
}

static int kf_KC_TIME( int argc, char *argv[] )
{		
	TTimeStt	t;
	
	read_cmos_time( &t );
	kdbg_printf( "%04d/%02d/%02d %d:%d(%d)\n", 
		t.nYear, t.nMon, t.nDay, t.nHour, t.nMin, t.nSec );

	return( 0 );
}

static int kf_KC_L50( int argc, char *argv[] )
{		
	lines_xx( 50 );
	return( 0 );
}

static int kf_KC_L25( int argc, char *argv[] )
{		
	lines_xx( 25 );
	return( 0 );
}

static int kf_KC_ASM( int argc, char *argv[] )
{
	DWORD dwAddr;
	TSSStt *pTSS;

	if( argc < 2 )
	{	// debugger is not active
		if( is_debugger_active() == 0 )
		{
			kdbg_printf( "assemble <addr>\n" );
			return( 0 );
		}
		else  // get debuggee's EIP
		{
			pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
			if( pTSS == NULL )
			{
				kdbg_printf( "assemble - no backlink!\n" );
				return(0);
			}
			dwAddr = pTSS->dwEIP;			
		}
	}
	else
		dwAddr = dwHexValue( argv[1] );

	kassemble( dwAddr );

	return(0);
}

// EXPORT TABLE�� ����Ѵ�.
int kf_KC_EXP( int argc, char *argv[] )
{
	ModuleStt *pM;

	// ���ڰ� ������ Bellona2�� export table�� ����Ѵ�. 
	if( argc < 2 )
		disp_export_table( bell.pExp );
	else
	{	// ����� ã�´�. 
		pM = find_module_by_alias( argv[1], NULL );
		if( pM == NULL )
		{	// ����� ã�� �� ����.
			kdbg_printf( "module %s not found!\n" );
			return(0 );
		}
		disp_export_table( pM->pExp );
	}
	
	return( 0 );
}

static int kf_KC_START_GUI( int argc, char *argv[] )
{
	DWORD	dwMode;

	if( argc < 2 )
	{	// ��带 �������� �ʾҴ�. 
		kdbg_printf( "startg <Mode>\n" );
		return( 0 );
	}

	// ��� ���� ���Ѵ�. 
	dwMode = dwHexValue( argv[1] );	
	start_gui( dwMode );
	
	return( 0 );
}
static int kf_KC_V86LIB( int argc, char *argv[] )
{
	int nR;

	if( argc < 2 )
	{
		kdbg_printf( "V86LIB <lib_path]\n" );
		return( -1 );
	}

	nR = load_v86_lib( argv[1] );

	return( 0 );
}

static int kf_KC_RESET_FDD( int argc, char *argv[] )
{
    BlkDevObjStt    *pFddDevObj;
    int             nR, nI, nTotalDev;

    // FDD ����̽��� ã�´�.
    pFddDevObj = find_first_blkdev_obj( &nTotalDev );
    if( pFddDevObj == NULL )
    {   // ã�� �� ����.
        kdbg_printf( "FDD Device not found!\n" );
        return( -1 );
    }

    // ��� FDD Device�� ã�� reset ��Ų��.
    for( nI = 0; pFddDevObj != NULL; )
    {
        // FDD�� ã�Ҵ�.
        if( pFddDevObj->pDev != NULL && pFddDevObj->pDev->nMajor == FDD35_MAJOR )
        {
            nR = discard_block_device( pFddDevObj );
            nR = reset_block_device( pFddDevObj );
            kdbg_printf( "Reset FDD device(%d)\n", nI );
        }

        pFddDevObj = pFddDevObj->pNext;
    }
    return( 0 );
}

// ������ �̸��� �׽�Ʈ�� ȸ���� �����Ѵ�.
static int kf_KC_FILE_TEST( int argc, char *argv[] )
{
	int     nNumTest;
    
	if( argc < 3 )
	{
		kdbg_printf( "filetest <filename> <num_of_test>\n" );
		return( -1 );
	}

    nNumTest = dwDecValue( argv[2] );
    if( nNumTest < 0 )
        nNumTest = 1;
    
    // ������ �׽�Ʈ �Ѵ�.
    file_test( argv[1], nNumTest );

	return( 0 );
}

static int kf_KC_VCON( int argc, char *argv[] )
{
	disp_vconsole();
	return( 0 );
}

static int kf_KC_SCHQ( int argc, char *argv[] )
{
	display_schedule_q();
	return( 0 );
}

static int kf_KC_ADDRS( int argc, char *argv[] )
{
	ThreadStt	*pT;
	DWORD		dwTID;
	
	if( argc < 2 )
	{
		kdbg_printf( "addrs <thread id>\n" );
		return( 0 );
	}

	dwTID = dwDecValue( argv[1] );
	pT = find_thread_by_id( dwTID );
	if( pT == NULL )
	{	// Thread�� ã�� �� ����.
		kdbg_printf( "thread(%d) not found!\n", dwTID );
		return( 0 );
	}

	kdbg_set_debugee_tid( dwTID );
	
	return( 0 );
}

// �������� ���¸� �����Ѵ�.
static int kf_KC_TSTATE( int argc, char *argv[] )
{
	ThreadStt	*pT;
	DWORD		dwTID;
	int			nState;
	char		*pState;

	if( argc <= 1 )
	{
		kdbg_printf( "usage: tstate <tid> <new_state>\n" );
		kdbg_printf( "  tc    : TIME_CRITICAL\n" );
		kdbg_printf( "  high  : HIGHEST\n" );
		kdbg_printf( "  +norm : ABOVE_NORMAL\n" );
		kdbg_printf( "  norm  : NORMAL\n" );
		kdbg_printf( "  -norm : BELOW_NORMAL\n" );	
		kdbg_printf( "  low   : LOWEST\n" );			
		kdbg_printf( "  idle  : IDLE\n" );			
		kdbg_printf( "  lazy  : LAZY\n" );		
		kdbg_printf( "  wait  : WAIT\n" );
		return( 0 );
	}

	dwTID = dwDecValue( argv[1] );

	// ���� ���� ��´�.
	pState = argv[2];
	if( strcmpi( pState, "tc" ) ==  0 )
		nState = TS_READY_TIME_CRITICAL;
	else if( strcmpi( pState, "high" ) ==  0 )
		nState = TS_READY_HIGHEST;
	else if( strcmpi( pState, "+norm" ) ==  0 )
		nState = TS_READY_ABOVE_NORMAL;
	else if( strcmpi( pState, "norm" ) ==  0 )
		nState = TS_READY_NORMAL;
	else if( strcmpi( pState, "-norm" ) ==  0 )
		nState = TS_READY_BELOW_NORMAL;
	else if( strcmpi( pState, "low" ) ==  0 )
		nState = TS_READY_LOWEST;
	else if( strcmpi( pState, "idle" ) ==  0 )
		nState = TS_READY_IDLE;
	else if( strcmpi( pState, "lazy" ) ==  0 )
		nState = TS_READY_LAZY;
	else if( strcmpi( pState, "wait" ) ==  0 )
		nState = TS_WAIT;
	else 
	{
		kdbg_printf( "unknown thread state: %s\n", pState );
		return( 0 );
	}

	pT = find_thread_by_id( dwTID );
	if( pT == NULL )
	{	// �����带 ã�� �� ����.
		kdbg_printf( "thread %d not found!\n", dwTID );
		return( 0 );
	}

	// �������� ���¸� �����Ѵ�.
	change_thread_state( NULL, pT, nState );
	
	return( 0 );
}

#pragma data_seg( "data2" )
static KShlFuncStt kfunc[] = {
    { KC_CUREIP,    kf_KC_CUREIP,    ".",           "display the current EIP code." },
    { KC_HELP,      kf_KC_HELP,      "?",           "help screen" },
	{ KC_ADDRS,		kf_KC_ADDRS,	 "addrs", 		"<thread id>; set debugger address space" },
    { KC_ASM,       kf_KC_ASM,       "asm",         "<addr>; assemble" },
    { KC_BC,        kf_KC_BC,        "bc",          "[*,0,1,2,3]; clear breakpoints" },
    { KC_BREAK,     kf_KC_BREAK,     "break",       "<address> [r,rw,io,x] [1,2,4]; set breakpoint" },
    { KC_CDEV,      kf_KC_CDEV,      "cdev",        "display char devices" },
    { KC_CD_INFO,   kf_CD_INFO,      "cdinfo",      "get CD-ROM media info" },
    { KC_CHKMEM,    kf_KC_CHKMEM,    "chkmem",      "check memory chain" },
    { KC_CLS,       kf_KC_CLS,       "cls",         "clear screen" },
    { KC_DBGINFO,   kf_KC_DBGINFO,   "dbginfo",     "[module_name] [dbg_filename]; load debug information" },
    { KC_DR,        kf_KC_DR,        "dr",          "display debug registers" },
    { KC_DUMP,      kf_KC_DUMP,      "d",           "[address] [size]; dump memory" },
    { KC_DUMPHDD,   kf_KC_DUMPHDD,   "dumphdd",     "dump hdd sector <drv;0=c> <sector_num>" },
    { KC_EDITMEM,   kf_KC_EDITMEM,   "editmem",     "[address]; edit memory" },
    { KC_EXEC,      kf_KC_EXEC,      "exec",        "execute new program" },
	{ KC_EXP,		kf_KC_EXP,		 "exp",		    "list export function table" },
	{ KC_FILE_TEST,	kf_KC_FILE_TEST, "filetest",    "filetest." },
    { KC_FDDTEST,   kf_KC_FDDTEST,   "fddtest",     "fdd test." },
    { KC_FG,        kf_KC_FG,        "fg",          "set foreground thread or process <ADDR>" },
    { KC_GDT,       kf_KC_GDT,       "gdt",         "display gdt" },
    { KC_GO,        kf_KC_GO,        "g",           "run to [addr]" },
    { KC_HELP,      kf_KC_HELP,      "h",           "help screen" },
    { KC_IDT,       kf_KC_IDT,       "idt",         "display idt" },
    { KC_INT1,      kf_KC_INT1,      "int1",        "invoke int 1" },
    { KC_INT3,      kf_KC_INT3,      "int3",        "insert int 3 to [<addr> [x]]" },
    { KC_KILL,      kf_KC_KILL,      "kill",        "kill thread or process <ADDR>" },
	{ KC_L50,		kf_KC_L50,		 "l50",		    "set vertical line to 50." },
	{ KC_L25,		kf_KC_L25,		 "l25",		    "set vertical line to 25." },
    { KC_MAP,       kf_KC_MAP,       "map",         "display mapping info [VADDR] [ATTR]" },
    { KC_MODULE,    kf_KC_MODULE,    "module",      "load kernel extension module [filename]" },
    { KC_PICK,      kf_KC_PICK,      "pick",		"pick <thread_id>." },
    { KC_PROC,      kf_KC_PROC,      "process",     "display process table" },
    { KC_PROCEED,   kf_KC_PROCEED,   "p",           "proceed" },
    { KC_REBOOT,    kf_KC_REBOOT,    "reboot",      "reboot system" },
    { KC_REG,       kf_KC_REG,       "r",           "display registers [<register> <value>]" },
	{ KC_RESET_FDD,	kf_KC_RESET_FDD, "resetfdd",    "reset fdd." },
	{ KC_SCHQ,      kf_KC_SCHQ,      "schq",        "display schedule Queue"    },
    { KC_SHMEM,     kf_KC_SHMEM,     "shmem",       "display shared memory info" },
    { KC_SIGNAL,    kf_KC_SIGNAL,    "signal",      "send signal <TID|PID> <signal>." },
    { KC_SRC,       kf_KC_SRC,       "src",         "display source file list" },
    { KC_STACK,     kf_KC_STACK,     "stack",       "dump stack" },
	{ KC_START_GUI,	kf_KC_START_GUI, "startg",	    "start graphic user interface" },
    { KC_SYMBOL,    kf_KC_SYMBOL,    "symbol",      "display symbols. symbol [symbol prefix, *] [module prefix]" },
    { KC_THREAD,    kf_KC_THREAD,    "thread",      "display thread <thread_addr>" },
    { KC_TRACE,     kf_KC_TRACE,     "t",           "trace one step" },
    { KC_TIME,      kf_KC_TIME,      "time",        "time" },
    { KC_TSS,       kf_KC_TSS,       "tss",         "display tss [tss_id]" },
	{ KC_TSTATE,	kf_KC_TSTATE,	 "tstate",      "change thread state <tid> <new_state>"       },
    { KC_UASM,      kf_KC_UASM,      "u",           "unassemble [addr]" },
    { KC_UNLOAD,    kf_KC_UNLOAD,    "unload",      "unload module <alias>" },
	{ KC_VCON,		kf_KC_VCON,		 "vcon",		"list virtual consoles"     },
    { KC_VER,       kf_KC_VER,       "ver",         "version info" },
	{ KC_V86LIB,	kf_KC_V86LIB,	 "v86lib",	    "load v86 library" },
	{ KC_VMODE,		kf_KC_VMODE,	 "vmode",	    "change video mode [mode_value]" },

	{ 0, NULL, NULL, NULL }
};
#pragma data_seg()

// display help screen
static int kf_KC_HELP( int argc, char *argv[] )
{
	int nI;

	for( nI = 0; kfunc[nI].pFunc != NULL; nI++ )
	{
        if( kfunc[nI].pHelpStr == NULL )
            kfunc[nI].pHelpStr = "";

		kdbg_printf( "%-10s : %s\n", kfunc[nI].pS, kfunc[nI].pHelpStr );
		if( nI > 0 && ( nI % (get_vertical_line_size() -2) ) == 0 )
			getchar();
	}
	
	disp_jhelp();

	return( 0 );
}

static KShlFuncStt *get_kc_func( KShlFuncStt *pFTbl, char *pS )
{
	int 		nI;
	
	for( nI = 0; pFTbl[nI].pS != NULL; nI++ )
	{
		if( strcmpi( pS, pFTbl[nI].pS ) == 0 )
			return( &pFTbl[nI] );
	}

	return( NULL );
}						 

// parse command string
static int kdbg_arg_parsing( char *argv[], char *pS )
{
	int  nTotal;

	for( nTotal = 0;; )
	{
		pS = skip_space( pS );
		if( pS[0] == 0 )
			break;
		
		argv[nTotal++] = pS;

		pS = search_space( pS );
		if( pS[0] == 0 )
			break;
		pS[0] = 0;
		pS++;
	}

	// �������� NULL�� �߰��Ѵ�.
	argv[nTotal] = NULL;
	return( nTotal );
}

// call kshell function
int kshell_function( char *pCmdStr )
{
	int				nR;
	KShlFuncStt		*pF;
	int				argc;
	GuiExportStt	*pGuiExp;
	char			*argv[ MAX_ARGV ];

	// �Էµ� ���ڿ��� �Ľ��Ѵ�.
	argc = kdbg_arg_parsing( argv, pCmdStr );
	if( argc == 0 )
		return( 0 );

	/*==================================*/
	// Ŀ�� ��⿡ �����ϴ� ����� ã�´�.
	pF = get_kc_func( kfunc, argv[0] );
	if( pF != NULL )
	{
		// ���� �Լ��� ȣ���Ѵ�.
		nR = pF->pFunc( argc, argv );
		return( nR );
	}
	/*==================================*/

	if( is_gui_mode() == 0 )
		return( -1 );

	pGuiExp = get_gui_exp();
	if( pGuiExp != NULL && pGuiExp->pFTbl != NULL )
	{// GUI ��⿡ �����ϴ� ����� ã�´�.
		pF = get_kc_func( pGuiExp->pFTbl, argv[0] );
		if( pF != NULL )
		{	// ���� �Լ��� ȣ���Ѵ�.
			nR = pF->pFunc( argc, argv );
			return( nR );
		}
	}
	
	return( -1 );
}

