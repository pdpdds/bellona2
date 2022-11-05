#include "bellona2.h"

#define MAX_KCMD_HISTORY	32
#define MAX_STRINGBUFF_SIZE 260

typedef struct {
	int		nTotal;
	int		nDispX;
	int		nX;
	char	szCmdStr[MAX_STRINGBUFF_SIZE];
} KShellCmdStt;

static int  nIns  = 1;
static int	nKCmd = 0;		 
static KShellCmdStt	kcmd[ MAX_KCMD_HISTORY ];

// 프롬프트를 출력한다.
static int kdbg_disp_prompt( char *pPrompt )
{
	kdbg_printf( "\r" );
	
	if( pPrompt != NULL && pPrompt[0] != 0 )
		kdbg_printf( "%s", pPrompt );

	//jshell_disp_prompt();

	return( 0 );
}

// 지금까지 입력된 글자를 출력하고 커서의 위치를 잡는다.
static int kdbg_disp_cmd_str( int nX, int nY, KShellCmdStt *pKC )
{			  
	int nH, nI, nK;

	// 출력 가능한 길이
	nH = 79 - nX;	
	nI = strlen( &pKC->szCmdStr[pKC->nDispX] );
	
	//지정된 길이 이상은 출력하지 않는다.
	nWriteToVideoMem_Len( nX, nY, &pKC->szCmdStr[pKC->nDispX], nH );
	
	// GUI쪽 커서를 옮겨준다.  (GUI쪽은 Direct Write하기 전에 먼저 커서를 움직여준다.)
	gui_set_cursor_xy( nX + pKC->nX - pKC->nDispX, nY );

	// GUi Console에서 알아서 출력한다.
	gui_direct_write( &pKC->szCmdStr[pKC->nDispX], nX );
	
	nK = nI + nX;
	
	// 나머지 부분이 있으면 공백으로 채운다.
	for( ; nI < nH; nI++, nK++ )
		nWriteToVideoMem( nK, nY, " " );

	// 커서의 위치를 잡는다.
	set_cursor_xy( nX + pKC->nX - pKC->nDispX, nY );

	return( 0 );
}

// 명령 구조체를 초기화한다.
static int kdbg_init_cmdstt( KShellCmdStt *pKC )
{
	memset( pKC, 0, sizeof( KShellCmdStt ) );

	return( 0 );
}

static void vDelCell( char *pS )
{
	if( pS[0] == 0 )
		return;

	pS[0] = pS[1];
	vDelCell( &pS[1] );
}
static void vInsCell( char *pS, char ch )
{
	char nextCh;

	nextCh = pS[0];
	pS[0] = ch;

	if( ch == 0 )
		return;
	else
		vInsCell( &pS[1], nextCh );
}

// 명령의 히스토리 위쪽으로 움직인다.
static int nGetPrevCmd( int nKCmd )
{	
	if( nKCmd == 0 )
		nKCmd = MAX_KCMD_HISTORY-1;
	else
		nKCmd--;
	return( nKCmd );
}

// 명령의 히스토리 아래쪽으로 움직인다.
static int nGetNextCmd( int nKCmd )
{
	nKCmd++;
	if( nKCmd == MAX_KCMD_HISTORY )
		nKCmd = 0;
	return( nKCmd );
}
			
// 입력된 문자를 스트링에 추가한다.
static kdbg_cmd_add_char( KShellCmdStt *pCmd, int nKey, int nH )
{
	int		nI, nX;
	char	szT[16];

	switch( nKey )
	{
	case BK_INS :  
		if( nIns == 0 )
			nIns = 1;
		else
			nIns = 0;
		break;

	case BK_HOME :
		pCmd->nX = 0;
		break;

	case BK_END :
		pCmd->nX = pCmd->nTotal;
		break;
	
	case BK_UP :
		for( nI = 0; nI < MAX_KCMD_HISTORY; nI++ )
		{
			nKCmd = nGetPrevCmd( nKCmd );
			if( kcmd[nKCmd].nTotal != 0 )
				break;
			nX = nGetPrevCmd( nKCmd );
			if( kcmd[nX].nTotal != 0 )
				break;
		}
		break;
	
	case BK_DOWN :
		for( nI = 0; nI < MAX_KCMD_HISTORY; nI++ )
		{
			nKCmd = nGetNextCmd( nKCmd );
			if( kcmd[nKCmd].nTotal != 0 )
				break;
			nX = nGetNextCmd( nKCmd );
			if( kcmd[nX].nTotal != 0 )
				break;
		}
		break;	
	
	case BK_LEFT :
		if( pCmd->nX > 0 )
			pCmd->nX--;
		break;

	case BK_RIGHT :
		if( pCmd->nX < pCmd->nTotal )
			pCmd->nX++;
		break;

	case BK_BACKSPACE :
		if( pCmd->nTotal > 0 && pCmd->nX > 0 )
		{	// delete the last char in kshell buffer
			vDelCell( &pCmd->szCmdStr[ pCmd->nX-1 ] );
			pCmd->nTotal--;
			pCmd->nX--;

			// BACKSPACE ECHO
			sprintf( szT, "%c[1D %c[1D", 0x1B, 0x1B );
			send_string_to_remote_shell( szT );
		}
		break;

	case BK_DEL :
		if( pCmd->nTotal > 0 && pCmd->nX < pCmd->nTotal )
		{	// 셀에서 한 문자를 지운다.
			vDelCell( &pCmd->szCmdStr[ pCmd->nX ] );
			pCmd->nTotal--;
		}
		break;

	case BK_ENTER :  // 엔터는 그냥 무시한다.
		break;

	default:
		if( pCmd->nTotal >= MAX_STRINGBUFF_SIZE )
			break;

		if( nIns || pCmd->nX == pCmd->nTotal )  // 삽입모드
		{
			vInsCell( &pCmd->szCmdStr[ pCmd->nX ], (char)nKey );
			pCmd->nTotal++;

			// insert echo
			sprintf( szT, "%c[1@%c", 0x1B, (char)nKey );
			send_string_to_remote_shell( szT );
		}
		else
		{		
			pCmd->szCmdStr[ pCmd->nX ] = (char)nKey;
			if( pCmd->nX >= pCmd->nTotal )
				pCmd->nTotal++;

			// normal echo
			sprintf( szT, "%c", (char)nKey );
			send_string_to_remote_shell( szT );
		}

		pCmd->nX++;

		break;
	}

	if( pCmd->nX < pCmd->nDispX )
		pCmd->nDispX = pCmd->nX;
	else if( pCmd->nX > pCmd->nDispX + nH )
		pCmd->nDispX = pCmd->nX - nH;	// 한 칸 옆으로 스크롤한다.	
	
	return( pCmd->nTotal );	
}

typedef struct DBGFKeyTag {
	int		nKey;
	char	*pStr;
};
typedef struct DBGFKeyTag DBGFKeyStt;

static DBGFKeyStt	dbg_fkey_tbl[] = {
	{ BK_F5,	"stack" },		// dump stack
	{ BK_F6,	"u"		},		// unassemble
	{ BK_F8,	"t"		},		// track
	{ BK_F0,	"p"		},		// proceed
	{ 0, NULL },
};

// 디버거가 활성화되어 있을 떄에만 사용 가능한 명령.
static int get_debug_function_key_str( int nKey, char *pStr )
{
	int nI;

	pStr[0] = 0;
	for( nI = 0; dbg_fkey_tbl[nI].nKey != 0; nI++ )
	{
		if( dbg_fkey_tbl[nI].nKey == nKey )
		{
			strcpy( pStr, dbg_fkey_tbl[nI].pStr );
			break;
		}	
	}	

	if( pStr[0] == 0 )
		return( 0 );
	else
		return( 1 );
}		

// kernel shell
int kdbg_shell( char *pPrompt )
{
	char	szT[MAX_STRINGBUFF_SIZE];
	int		nKey, nReturn, nX, nY, nH, nR;

	nKCmd = 0;
	kdbg_init_cmdstt( &kcmd[nKCmd] );

	for( ;; )
	{
DISP_PROMPT:
		// 프롬프트를 출력한다.
		kdbg_disp_prompt( pPrompt );

		// 커서의 위치를 알아낸다.
		get_cursor_xy( &nX, &nY );

		nH = 79 - nX;
		
REDISP:	// 지금까지 입력된 글자를 출력하고 커서의 위치를 잡는다.
		kdbg_disp_cmd_str( nX, nY, &kcmd[nKCmd ] );

		// 한 문자를 입력 받는다.
		for( nKey = -1; nKey == -1; )
		{
			nKey = getchar();
		}

		if( is_debugger_active() )
		{	// 디버거가 활성화되어 있을 때에만 사용 가능한 명령.
			nR = get_debug_function_key_str( nKey, szT );
			if( nR != 0 )
			{
				kdbg_printf( "\n" );	// next line
				goto EXEC_COMMAND;
			}
		}

		if( nKey == BK_ENTER )
		{	// line feed
			kdbg_printf( "\n" );
			
			// no input char (empty string)
			if( kcmd[nKCmd].nTotal == 0 )
				goto DISP_PROMPT;											 
			
			// Execute Command !!====================//
			strcpy( szT, kcmd[nKCmd].szCmdStr );	 //
EXEC_COMMAND:										 //
			nReturn = kshell_function( szT );		 //
			//=======================================//
			if( nReturn == -1 )		// unknown command
			{
				strcpy( szT, kcmd[nKCmd].szCmdStr );
				nR = jshell_command( szT );			// call jshell function
			}
			else if( nReturn == 1000 )
				break;				// terminate shell

			nKCmd++;
			if( nKCmd == MAX_KCMD_HISTORY )
				nKCmd = 0;

			// initialize cmd structure, insert previous command to command history buffer
			kdbg_init_cmdstt( &kcmd[nKCmd] );				
			goto DISP_PROMPT;
		}
		else
		{	// append the input character to command string
			kdbg_cmd_add_char( &kcmd[nKCmd], nKey, nH );		
			goto REDISP;
		}
	}	

	return( 0 );
}

