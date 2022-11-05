// __chkesp를 없애려면 컴파일 옵션의 /GZ를 제거하면 된다.

#include "ush.h"

static char base_dir[260];

#ifdef WIN32
	#include <windows.h>
	extern int set_fg_proc( DWORD dwID );
	extern void set_xy( int nX, int nY );
	extern void get_xy( int *pX, int *pY );
#endif

extern int init_ush_md();
extern int close_ush_md();

// 다음 라인을 읽는다.
static char *get_next_line( char *pStr, long *pSize, char *pBuff )
{
    int nCopied;
        
    for( nCopied = 0, pStr[0] = 0; nCopied < pSize[0]; )
    {
        pStr[nCopied] = pBuff[nCopied];
        nCopied++;
        pStr[nCopied] = 0;
        if( pStr[nCopied-1] == 10 )
            break;
    }

    pSize[0] = nCopied;
    
    return( &pBuff[nCopied] );
}

// 파일을 출력한다.
void print_file( char *pFileName )
{
    char *pBuff, *pNext, szT[260];
    long    lSize, lStrSize, lPrintedSize;

    // 파일을 로드한다.
    pNext = pBuff = load_file( pFileName, &lSize );
    if( pBuff == NULL )
        return;

    // 한 라인씩 출력한다.
    for( lPrintedSize = 0;; )
    {
        lStrSize = sizeof( szT ) -1;
        if( lStrSize + lPrintedSize >= lSize )
            lStrSize = lSize - lPrintedSize;

        pNext = get_next_line( szT, &lStrSize, pNext );
        if( szT[0] == 0 )
            break;
        printf( szT );

        lPrintedSize += lStrSize;
        if( lPrintedSize >= lSize )
            break;      
    }

    // 메모리를 해제한다.
    free( pBuff );
}

// 배너를 출력한다.
static void disp_banner( char *pDir, char *pFileName )
{
    char    szT[260];

    sprintf( szT, "%s%s", pDir, pFileName );

    print_file( szT );
}

#define MAX_UARGV               128
#define STOP_INTERACTIVE_MODE (-100)

typedef int (*UFUNC)( int argc, char *argv[] );

typedef struct {
    UFUNC   pFunc;
    char    *pCmd;
    int     nParams;
    char    *pHelp;
    char    *pUsage;
} UCmdTblStt;

typedef struct {
    int     argc;
    char    *argv[MAX_UARGV];
    char    szCmdStr[260];
    char    buff[1024];
} UCmdStt;

static void disp_prompt()
{
    printf( "$>" );
}

static int is_white_space( char ch )
{

    if( ch == ' ' || ch == 9 || ch == 13 || ch == 10 )
        return( 1 );

    return( 0 );
}   

static char *skip_space( char *pS )
{
    for( ;; )
    {
        if( pS[0] == ' ' || pS[0] == 9 || pS[0] == 13 || pS[0] == 10 )
        {
            pS++;
            continue;
        }
        break;
    }
    return( pS );
}

// 한 문자로 이루어진 명령 또는 토큰
static int is_one_byte_token( char ch )
{
    if( ch == '=' )
        return( 1 );    
    
    return( 0 );
}

// 다음 단어를 구한다.
static char *get_next_word( char *pWord, char *pS )
{
    int nI, nQuoteFlag;

    pWord[0] = 0;
    pS = skip_space( pS );

    if( is_one_byte_token( pS[0] ) )
    {
        pWord[0] = pS[0];
        pWord[1] = 0;
        return( &pS[1] );
    }

    nQuoteFlag = 0;
    for( nI = 0;; )
    {
        if( pS[nI] == '"' )
        {   // 문자열 선두에 "이 등장.
            if( nI == 0 || nQuoteFlag != 0 )
            {
                nQuoteFlag = ~nQuoteFlag;
                pWord[nI] = 0;
                pS++;
                if( pS[nI] == '"' ) // "" <- 공백 문자열.
                {
                    strcat( pWord, "\"\"" );
                    pS++;
                    break;
                }
                continue;
            }
            else  // 문자열 중간에 "이 등장.
                break;
        }

        pWord[nI] = pS[nI];
        if( pS[nI] == 0 )
            break;
        nI++;
        pWord[nI] = 0;

        if( nQuoteFlag != 0 )
            continue;

        if( is_white_space( pS[nI] ) || is_one_byte_token( pS[nI] ) )
            break;
    }   
    return( &pS[nI] );
}

static int chk_cmd_syntax( char *pS )
{
    int nTotal, nI;

    for( nTotal = nI = 0; pS[nI] != 0; nI++ )
    {
        if( pS[nI] == '"' )
            nTotal++;
    }

    if( nTotal & 1 )
        return( -1 );
    return( 0 );
}

// 명령을 파싱한다.
static int parse_command( UCmdStt *pCmd, char *pStr )
{
    int     nR;
    char    *pNext, szWord[260];

    memset( pCmd, 0, sizeof( UCmdStt ) );

    // 명령 스트링 전체를 복사한다.
    strcpy( pCmd->szCmdStr, pStr );

    //  "가 짝수개인지 확인한다.
    nR = chk_cmd_syntax( pStr );
    if( nR < 0 )
        return( -1 );

    pNext = pStr;
    pCmd->argv[0] = pCmd->buff;
    for( ;; )
    {   // 한 단어를 가져온다.
        pNext = get_next_word( szWord, pNext );
        if( szWord[0] == 0 )
            break;
        else if( strcmp( szWord, "\"\"" ) == 0 )
            szWord[0] = 0;

        strcpy( pCmd->argv[ pCmd->argc ], szWord );
        pCmd->argc++;
        pCmd->argv[ pCmd->argc ] = pCmd->argv[ pCmd->argc-1 ] + strlen( szWord ) +1;
    }   

    return( 0 );
}

static int uc_exit( int argc, char *argv[] )
{
    return( STOP_INTERACTIVE_MODE );
}

// set : 환경변수 전체를 출력.
// set name : 환경벼수 name을 출력.
// set name=value : 환경변수를 새로 설정.
static int uc_set( int argc, char *argv[] )
{
    int nR;

    // 전체 환경 변수 출력.
    if( argc <= 1 )
    {
        disp_env_str();
    }
    // 특정 환경 변수를 출력.
    else if( argc == 2 )
    {
    }
    else if( argc == 3 )    // set hello 24
    {
        if( strcmp( argv[2], "=" ) == 0 )
            return( -1 );   // 에러.
        nR = set_env_str( argv[1], argv[2] );
    }             
    else
    {
        if( strcmp( argv[2], "=" ) == 0 )
            nR = set_env_str( argv[1], argv[3] );
        else
            set_env_str( argv[1], argv[2] );
    }

    // 무조건 0을 리턴해야 한다.
    return( 0 );
}

// 현재 디렉토리를 출력한다.
static int uc_pwd( int argc, char *argv[] )
{
	char	szT[260];

	getcwd( szT, sizeof( szT ) -1 );

	printf( "%s\n", szT );

	return( 0 );
}

void print_version()
{
    printf( "B2OS User Shell v0.1\n" );
}

// 버전을 출력한다.
static int uc_ver( int argc, char *argv[] )
{
	print_version();

	return( 0 );
}

// 디렉토리를 변경한다.
static int uc_cd( int argc, char *argv[] )
{
	int nR;
	
	nR = chdir( argv[1] );
	if( nR < 0 )
		printf( "chdir: failed!\n" );

	return( 0 );
}

// 디렉토리 목록을 출력한다.
static int uc_ls( int argc, char *argv[] )
{
	int		nR;
	char	*pPath, *pOption;

	if( argc == 1 )
		pOption = pPath = "";
	else if( argc == 2 )
	{
		pPath   = argv[1];
		pOption = "";
	}
	else 
	{
		pPath   = argv[1];
		pOption = argv[2];
	}
	
	nR = list_directory( pPath, pOption );

	return( 0 );
}

static int uc_help( int argc, char *argv[] );

// function, cmd_string, number of param, help, usage
static UCmdTblStt uctbl[] = {
    { uc_help, "h",    0, "This help",  "" }, 
    { uc_help, "?",    0, "This help",  "" }, 
	{ uc_cd,   "cd",   1, "Change Dir", "<target directory>" }, 
    { uc_exit, "exit", 0, "Exit shell", "[return value]" }, 
	{ uc_ls,   "ls",   2, "List directory", "[path] [option]" },
    { uc_pwd,  "pwd",  0, "Print working directory." }, 
    { uc_set,  "set",  0, "Manage environment string.", "[name[=value]]" }, 
	{ uc_ver,  "ver",  0, "Print version", "" },

    { NULL, NULL, 0, NULL, NULL }
};

// 도움말 출력.
static int uc_help( int argc, char *argv[] )
{
    int nI;

    for( nI = 0; uctbl[nI].pFunc != NULL; nI++ )
    {
        printf( "%-12s : %s\n", uctbl[nI].pCmd, uctbl[nI].pHelp );
    }
    return( 0 );
}

static UCmdTblStt *find_cmd( char *pS )
{
    int nI;

    for( nI = 0; uctbl[nI].pFunc != NULL; nI++ )
    {
        if( strcmpi( pS, uctbl[nI].pCmd ) == 0 )
            return( &uctbl[nI] );
    }
    return( NULL );
}

static int disp_argv( UCmdStt *pCmd )
{
    int nI;
    
    printf( "total %d argv[]\n", pCmd->argc );
    for( nI = 0; nI < pCmd->argc; nI++ )
    {
        printf( "%s\n", pCmd->argv[nI] );
    }                                    
    return( 0 );
}

// 명령을 찾아 실행한다.
static int run_command( UCmdStt *pCmd )
{
    int         nR;
    UCmdTblStt  *pCmdTbl;

    // argv[]들을 출력한다. 
    //disp_argv( pCmd ); return( 0 );

    if( pCmd->argc <= 0 )
        return( 0 );

    pCmdTbl = find_cmd( pCmd->argv[0] );
    if( pCmdTbl == NULL )
        return( -1 );

    nR = pCmdTbl->pFunc( pCmd->argc, pCmd->argv );

    return( nR );
}
                     
// 인터액티브 모드로 명령을 처리한다.
int interactive_mode()
{
    int     nR;
    UCmdStt cmd;
    char    szCmdStr[260];

    for( ;; )
    {
        // 프롬프트를 출력한다.
        disp_prompt();

        // 명령을 입력 받는다.
        szCmdStr[0] = 0;
        nR = input_str( szCmdStr, sizeof( szCmdStr ) -1 );
        printf( "\n" );

        // 명령을 파싱한다.
        nR = parse_command( &cmd, szCmdStr );
        if( nR < 0 )
        {   // 파싱하는 도중 에러가 발생함.
            printf( "syntax error!\n" );
            continue;
        }

        // 명령을 찾아 실행한다.
        nR = run_command( &cmd );
        if( nR == STOP_INTERACTIVE_MODE )
            break;
        else if( nR < 0 )
        {
            printf( "unknown command.\n" );
        }
    }  
    printf( "bye~\n" );

    return( 0 );
}
