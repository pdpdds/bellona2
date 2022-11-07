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

// CMOS에서 현재 시간(BCD)을 읽어서 Binary로 변경한 후 TTime구조체에 리턴한다.
void read_cmos_time( TTimeStt *pTime )
{
	DWORD dwR;
    int   nSec, nMin, nHour, nWeek, nDay, nMon, nYear;

	for( dwR = 0x80; dwR & 0x80; )  // 상태 레지스터 A의 비트 7이 1이 될때까지 기다린다.
		dwR = dwReadCMOS( (DWORD)0x0A );

    nSec  =  (int)dwReadCMOS( (DWORD)0 );  // 초
    nMin  =  (int)dwReadCMOS( (DWORD)2 );  // 분
    nHour =  (int)dwReadCMOS( (DWORD)4 );  // 시
    nWeek =  (int)dwReadCMOS( (DWORD)6 );  // 요일
    nDay  =  (int)dwReadCMOS( (DWORD)7 );  // 일
    nMon  =  (int)dwReadCMOS( (DWORD)8 );  // 월
    nYear =  (int)dwReadCMOS( (DWORD)9 );  // 년

    // BCD의 형태이므로 Binary로 바꾸어준다.
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
			if( pB[dwI] == 0 )		// nWriteToVideoMem_Len은 중간에 0이 나오면 출력을 중단하므로 ' '으로 변경해 준다.
				szT[nI++] = ' ';
			else
				szT[nI++] = pB[dwI];
			
		}	  
	}

	// display the last line
	get_cursor_xy( &nX, &nY );
	nWriteToVideoMem_Len( 80 - 17 , nY, szT, 16 );

	kdbg_printf( "\n" );

	// 다음에 그냥 'd' 만 입력하면 dwLastAddress 다음부터 출력된다.
	return( dwLastAddress + 1);
};			

// dump memory
static G_dwNextDumpAddr = 0;
static int kf_KC_DUMP( int argc, char* argv[] )
{
	DWORD dwAddr, dwSize, dwR;

	// 주소가 지정되지 않았으면 이전에 덤프했던 바로 다음 주소부터 출력한다.
	if( argc < 2 )
		dwAddr = G_dwNextDumpAddr;

	if( argc >=2 )
		dwAddr = dwHexValue( argv[1] );
	
	if( argc >= 3 )
		dwSize = dwHexValue( argv[2] );
	else
		dwSize = 16*10;		// 기본적으로 10줄을 출력한다.

	dwR = dump_memory( dwAddr, dwSize );
	if( dwR != 0xFFFFFFFF )
	    G_dwNextDumpAddr = dwR;

	return( 0 );
}

// 모든 모듈을 뒤져 함수 이름으로 함수 구조체를 찾는다.
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
		{	// 찾은 모듈의 주소와 함께 리턴한다.
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
			// 함수 이름으로 주소를 찾아 본다.
			pFunc = find_func_ent( argv[1], &pM );
			if( pFunc != NULL )
				dwEIP = pFunc->dwAddr + pM->dwLoadAddr;
			else
			{	// 함수를 찾을 수 없다.
				kdbg_printf( "function %s not found!\n", argv[1] );
				return( -1 );	// function not found
			}
		}
		nMaxDispLine = get_vertical_line_size();
	}

	// 지정된 메모리에 접근할 수 있는지 검사
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

// 디버깅 정보에 포함된 심볼들을 출력한다.
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
	{	// User Level 모듈.
		if( check_memory_validity( 0, (DWORD)pDbg ) < 0 )																
		{
			kdbg_printf( "%s: deactivated user level module!\n", pModuleName );
			return( 0 );
		}
	}

	// 올바른 디버그 정보?
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
		
		// 함수명 앞의 '_'는 빼고 출력한다. 
		if( pFuncName[0] == '_' )
			pFuncName++;
			
		if( pPrefix != NULL && pPrefix[0] != 0 )
		{
			if( memcmp( pPrefix, pFuncName, nPrefixSize ) != 0 )
			    continue;
		}	

		kdbg_printf( "[%4d] 0x%08X [%s] %s (size=0x%X)\n", 
			nIndex, dwBaseAddr + pFunc->dwAddr, pModuleName, pFuncName, pFunc->dwSize );

		pDisplayed[0]++;	// 출력된 개수를 증가 시킨다.
		
		if( pDisplayed[0] > 0 && ( pDisplayed[0] % nLinesPerPage ) == 0 )
		{
			if( getchar() == 27 )
			    return( -1 );
		}
	}
	
	return( 0 );
}

// 내장하고 있는 심볼을 출력한다.
static int kf_KC_SYMBOL( int argc, char* argv[] )
{
	int 			nI;
	ModuleStt		*pM;
	SysModuleStt 	*pSM;
	char 			*pPrefix, *pModPrefix;
	int 			nR, nDisplayed, nModPrefixLength;

	// prefix가 지정되어 있으면 prefix로 시작하는 심볼만 출력한다.
	if( argc >= 2 )
	{
	    pPrefix = argv[1];
		if( strcmpi( pPrefix, "*" ) == 0 )
			pPrefix = "";
	}
	else
	    pPrefix = NULL;
	    
	// 모듈이름이 지정되어 있으면 해당 모듈에서만 심볼을 찾는다.
	if( argc >= 3)
	{
	    pModPrefix = argv[2];
		nModPrefixLength = strlen( pModPrefix );
	}
	else
		pModPrefix = NULL;    

	// 모듈 등록정보가 저장된 구조체를 얻는다.	    
	pSM = get_sys_module();
	
	nDisplayed = 0;
	for( nI = 0; nI < MAX_MODULE; nI++ )
	{
		pM = pSM->mod[nI];
		if( pM == NULL )
			continue;		
			
		if( pModPrefix != NULL )
		{	// 지정된 Prefix와 다른 모듈은 건너뛴다.
			if( memcmp( pModPrefix, pM->szAlias, nModPrefixLength ) != 0 )
				continue;
		}	
		
		nR = disp_module_symbol( pM->szAlias, pM->pMyDbg, pM->dwLoadAddr, pPrefix, &nDisplayed );
	}
	kdbg_printf( "Total %d symbols.\n", nDisplayed );

	return( 0 );
}

// INT3 명령에 의해 주소에 0xCC를 심는다.
static int kf_KC_INT3( int argc, char *argv[] )
{
	DWORD	dwAddr;
	UCHAR	*pX;
	int		nClear, nR, nI, nTotal;
	
	if( argc < 2 )		// INT3의 리스트를 보여준다.
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

	nClear = 0; // INT3 addr x   (끝에 x가 붙으면 해제한다.)
	if( argc >= 3 && strcmpi( argv[2], "x" ) == 0 )
		nClear = 1;

	// 주소를 16진수 값으로 변경한다.
	dwAddr	= dwHexValue( argv[1] );
	pX		= (UCHAR*)dwAddr;

	// 설정 옵션인데 이미 0xCC이다.
	if( nClear == 0 && pX[0] == (UCHAR)0xCC )
	{
		kdbg_printf( "error : [0x%08X] is already 0xCC.\n", dwAddr );
		return( -1 );
	}
	// 해제 옵션인데 0xCC가 아니다.
	if( nClear != 0 && pX[0] != (UCHAR)0xCC )
	{
		kdbg_printf( "error : [0x%08X] is not 0xCC (%02X).\n", dwAddr, pX[0] );
		return( -1 );
	}


	// 주소와 원래 바이트를 보관하고 0xCC를 심는다..
	if( nClear == 0 )
		nR = overwrite_cc( pX );	// 심는다.
	else
		nR = recover_cc( pX );		// 복구한다.
				   	
	return( nR );	// 1 = 성공, 0 = 실패
}

// 다음에 실행할 명령이 CALL이면 다음 바이트를 CC로 바꾸고 주소와 원래 바이트를 보관, 1리턴
static char _strOffset[32], _strIP[12], _strOpCode[32], _strOperand[100], _strSize[12], _strDump[64];
static char *strArray[] = {_strOffset, _strIP, _strOpCode, _strOperand, _strSize, _strDump };
int nIsCallThenImplantCC()
{
	OpStt	Op;
	UCHAR	*pBuff;			// 역어셈블할 코드의 시작 위치
	int		nSize;          // 방금 역어셈블한 코드의 크기
	TSSStt	*pTSS;
				  	
	pTSS = (TSSStt*)pGetBackLinkTSS( &dbg_tss );
	if( pTSS == NULL )
		return( 0 );

	if( pTSS->dwEFLAG & MASK_VM )
		pBuff = (UCHAR*)pTSS->dwEIP + (DWORD)( (DWORD)pTSS->wCS << 4 );
	else
		pBuff = (UCHAR*)pTSS->dwEIP;

	// 한 라인을 역어셈블 한다.
	nSize = nDisAssembleOneCode( (DWORD)pBuff, &Op, pBuff, strArray );

	// CALL 명령이 아니면 0을 리턴하고 돌아간다.
	if( Op.wType != ot_CALL )
		return( 0 );

	// 다음 바이트가 이미 INT3이면 그냥 돌아간다.
	if( pBuff[nSize] == (UCHAR)0xCC )
	{
		kdbg_printf( "next byte is already CC\n" );
		return( 0 );
	}

	// 주소와 원래 바이트를 보관하고 0xCC를 심는다..
	if( overwrite_cc( &pBuff[nSize] ) == 1 )
	{
		kdbg_printf( "INT 3 planted\n" );
		return( 1 );	// 성공적으로 0xCC를 심었다.
	}
	else
	{
		kdbg_printf( "INT 3 is not planted\n" );
		return( 0 );	// 0xCC를 심는데 실패했다.
	}
}

//  리지스터 값을 변경한다.
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
												   
	// 새로운 값을 얻는다.
	dwValue = dwHexValue( argv[2] );

	// 레지스터 타입을 얻는다.
	uppercase( argv[1] );	// 대문자로 변경한다.
	nI = nSearchTbl( rsvTbl, 0, MAX_RSVSYM, argv[1] );
	if( nI == -1 )		// 심볼을 못 찾았다.  
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

// 브레이크 포인트를 설정한다.
static int kf_KC_BREAK( int argc, char *argv[] )
{
	DWORD	dwAddr;
	int		nR, nOption, nLength;
	char 	*pOptionStr, *pSizeStr;

	if( argc < 2 )
	{	// 사용법을 출력한다.
		kdbg_printf( "usage: break <address> [ w | rw | io | x ] [size]\n" );
		
		// 하드웨어 브레이크 포인트의 내용을 출력한다.
		disp_hw_breaks();
		return(0);
	}
	else if( argc == 2)
		nOption = BREAK_EXEC;
		
	//  옵션이 지정되어 있다.
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
		{	// 잘못된 옵션 스트링.
			kdbg_printf( "invalid option : %s\n", pOptionStr );
			return( -1 );
		}	
	}

	// 크기가 지정되었다.
	if( argc >= 4 )		
	{
		pSizeStr = argv[3];
		nLength = dwDecValue( pSizeStr ) -1;
		nLength &= 3;
	}
	else
		nLength = 4;

	// 주소를 변경한다.
	dwAddr = dwHexValue( argv[1] );

	// breakpoint를 설정한다.
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
	if( argc >= 3 )	// 레지스터 값을 변경한다.	
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
	
	return( 1000 ); // shell을 끝내라.
}

// trace
static int kf_KC_TRACE( int argc, char *argv[] )
{
	int		nRepeat;

	if( is_debugger_active() == 0 )
		return( 0 );

	if( argc >= 2 )
	{	 // trace 회수가 지정되었다.
		nRepeat = dwDecValue( argv[1] );
		kdbg_set_trace_repeat( nRepeat );
	}	

	kdbg_set_debugee_tf( 1 );
	
	return( 1000 ); // shell을 끝내라.
}

// proceed
static int kf_KC_PROCEED( int argc, char *argv[] )
{
	if( nIsCallThenImplantCC() == 1 )		// CALL 명령이 아니면 t와 동일하게 실행한다.
	{
		kdbg_printf( "ImplantCC() = 1\n" );
		kdbg_set_debugee_tf( 0 );			// TF를 끈다.
	}
	else
		kdbg_set_debugee_tf( 1 );
	return( 1000 ); // shell을 끝내라.
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

// 커널 쉘에서 새로운 프로그램을 로드하여 실행한다.
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

	// 파일이 존재하는지 먼저 확인한 후 프로세스를 fork 한다.  (2004-03-26)
	nR = kget_file_info( argv[1], NULL );
	if( nR <= 0 )
	{
		kdbg_printf( "%s not found or error!\n", argv[1] );
		return( -1 );
	}

	// fork new process  ( 내부적으로 kmalloc()을 사용한다. 최종적으로 프로세스가 종료되면서 해제된다.)
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
	// 원래 스택 덤프만 뜨던 것을 CallStack을 보여주도록 변경.
	//kdbg_dump_stack();

	if( is_debugger_active() == 0 )
	{
		kdbg_printf( "Kernel debugger is not active.\n" );
		return( 0 );
	}

	if( argc >= 2 && strcmpi( argv[1], "local" ) == 0 )
		nLocalFlag = 1;

	// 콜스택을 출력한다.
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

		// 메모리 체크.
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

	// 속성이 지정되어 있는가?
	szAttr[0] = 0;
	if( argc >= 3 )
		strcpy( szAttr, argv[2] );

	// 페이지 디렉토리, 페이지 테이블 엔트리를 출력한다.
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
	{	// 리스트를 출력한다.
		disp_module();
		return( 0 );
	}

	// 모듈을 로드하고 내부적으로 엔트리 포인트가 호출된다.
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
	{	// 로드된 모듈들의 디버깅 정보를 출력한다.
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

	// 모듈을 찾는다.
	pM = find_module_by_alias( argv[1], &nI );
	if( pM == NULL )
	{	// 모듈을 찾을 수 없다.
		kdbg_printf( "module %s not found!\n", argv[1] );
		return( -1 );
	}

	// 디버그 정보 파일을 새로 로드한다.
	pMyDbg = load_mydbg2_info( argv[2] );
	if( pMyDbg == NULL )
	{	// 디버그 정보 파일을 로드할 수 없다.
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
		nR = get_vesa_info();			// 리스트를 출력한다.
		if( nR == 0 )
			disp_vesa_info();
	}
	else
	{
		dwMode = dwHexValue( argv[1] );
		nR =vesa_mode_test( dwMode );	// 비디오 모드를 설정한다.
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

// EXPORT TABLE을 출력한다.
int kf_KC_EXP( int argc, char *argv[] )
{
	ModuleStt *pM;

	// 인자가 없으면 Bellona2의 export table을 출력한다. 
	if( argc < 2 )
		disp_export_table( bell.pExp );
	else
	{	// 모듈을 찾는다. 
		pM = find_module_by_alias( argv[1], NULL );
		if( pM == NULL )
		{	// 모듈을 찾을 수 없다.
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
	{	// 모드를 선택하지 않았다. 
		kdbg_printf( "startg <Mode>\n" );
		return( 0 );
	}

	// 모드 값을 구한다. 
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

    // FDD 디바이스를 찾는다.
    pFddDevObj = find_first_blkdev_obj( &nTotalDev );
    if( pFddDevObj == NULL )
    {   // 찾을 수 없다.
        kdbg_printf( "FDD Device not found!\n" );
        return( -1 );
    }

    // 모든 FDD Device를 찾아 reset 시킨다.
    for( nI = 0; pFddDevObj != NULL; )
    {
        // FDD를 찾았다.
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

// 파일의 이름과 테스트할 회수를 지정한다.
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
    
    // 파일을 테스트 한다.
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
	{	// Thread를 찾을 수 없다.
		kdbg_printf( "thread(%d) not found!\n", dwTID );
		return( 0 );
	}

	kdbg_set_debugee_tid( dwTID );
	
	return( 0 );
}

// 쓰레드의 상태를 변경한다.
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

	// 상태 값을 얻는다.
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
	{	// 쓰레드를 찾을 수 없다.
		kdbg_printf( "thread %d not found!\n", dwTID );
		return( 0 );
	}

	// 쓰레드의 상태를 변경한다.
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

	// 마지막에 NULL을 추가한다.
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

	// 입력된 문자열을 파싱한다.
	argc = kdbg_arg_parsing( argv, pCmdStr );
	if( argc == 0 )
		return( 0 );

	/*==================================*/
	// 커널 모듈에 존재하는 명령을 찾는다.
	pF = get_kc_func( kfunc, argv[0] );
	if( pF != NULL )
	{
		// 실제 함수를 호출한다.
		nR = pF->pFunc( argc, argv );
		return( nR );
	}
	/*==================================*/

	if( is_gui_mode() == 0 )
		return( -1 );

	pGuiExp = get_gui_exp();
	if( pGuiExp != NULL && pGuiExp->pFTbl != NULL )
	{// GUI 모듈에 존재하는 명령을 찾는다.
		pF = get_kc_func( pGuiExp->pFTbl, argv[0] );
		if( pF != NULL )
		{	// 실제 함수를 호출한다.
			nR = pF->pFunc( argc, argv );
			return( nR );
		}
	}
	
	return( -1 );
}

