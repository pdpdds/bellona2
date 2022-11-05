#include <bellona2.h>
#include "gui.h"

typedef enum {
	WC_UNKNOWN	= 0,	WC_WIN		= 1,		WC_MESGMON,
	END_OF_WC
} KCTag;

static int wf_win( int argc, char *argv[] )
{
	GuiStt	*pGui;
	WinStt	*pWin;
	char	*pStyleName;
	DWORD	dwParentID, dwWTID;;
	
	pGui = get_gui_stt();


	_asm {
		PUSHFD
		CLI
	}

	kdbg_printf( " WinID   Style  WTID  Title\n" ); 

	for( pWin = pGui->pStartLevelWin; pWin != NULL; pWin = pWin->pNextLevel )
	{
		if( pWin->pParentWin != NULL )
			dwParentID = pWin->pParentWin->dwID;
		else
			dwParentID = 0;

		if( pWin->pWStyle != NULL )
			pStyleName = pWin->pWStyle->szName;
		else
			pStyleName = "UNKNOWN";

		if( pWin->pWThread != NULL && pWin->pWThread->pThread != NULL )
			dwWTID = pWin->pWThread->pThread->dwID;
		else
			dwWTID = 0;
		
		kdbg_printf( " %4d %8s  %4d  %s\n", pWin->dwID, pStyleName, dwWTID, pWin->szTitle ); 
	}

	_asm POPFD

	return( 0 );
}

static MesgMonStt	mesg_mon;

static int disp_mesg_mon_list()
{
	int 			nI;
	MesgMonEntStt	*pEnt;

	if( mesg_mon.nTotal == 0 )
	{
		kdbg_printf( "No mesgmon entry.\n" );
		return( 0 );
	}

	kdbg_printf( "Index  WinID     start_mesg      end_mesg\n" );

	for( nI = 0; nI < MAX_MESGMON_ENT; nI++ )
	{
		pEnt = &mesg_mon.ent[nI];

		if( pEnt->dwWinID == 0 )
			continue;

		kdbg_printf( "[%d]  %4d    %12s   %12s\n", 
					nI, pEnt->dwWinID, 
					get_wmesg_str( pEnt->dwStartMesg ), 
					get_wmesg_str( pEnt->dwEndMesg ) );
	}
	
	return( 0 );	
}

// �ش� �޽����� ��ϵ� ������ Ȯ���Ѵ�.
int chk_mesgmon( DWORD dwWinID, DWORD dwMesg )
{
	int				nI, nR;
	MesgMonEntStt	*pEnt;
	
	if( mesg_mon.nTotal <= 0 )
		return( 0 );	// ��ϵ��� ����.

	nR = 0;

	_asm {
		PUSHFD
		CLI
	}
	for( nI = 0; nI < MAX_MESGMON_ENT; nI++ )
	{
		pEnt = &mesg_mon.ent[nI];
		if( pEnt->dwWinID == dwWinID && pEnt->dwStartMesg <= dwMesg && dwMesg <= pEnt->dwEndMesg )
		{
			pEnt->dwCounter++;
			nR = pEnt->dwCounter;
			break;
		}
	}

	_asm POPFD
	
	return( nR );	// ��ϵ��� ����.
}

// �޽��� ����� ��Ʈ���� ����Ѵ�.
static int reg_mesgmon( DWORD dwWinID, DWORD dwStartMesg, DWORD dwEndMesg )
{
	int 			nI;
	MesgMonEntStt	*pEnt;

	// ������ ��ϵ� ���� �����ϴ��� Ȯ���Ѵ�.
	// ������ ������ ��Ʈ���� �����ϸ� ������� �ʴ´�.
	for( nI = 0; nI < MAX_MESGMON_ENT; nI++ )
	{
		pEnt = &mesg_mon.ent[nI];
		
		if( pEnt->dwWinID == dwWinID && pEnt->dwStartMesg == dwStartMesg && pEnt->dwEndMesg == dwEndMesg )
			return( 0 );		// �̹� ������ ��Ʈ���� �����Ѵ�.
	}	

	for( nI = 0; nI < MAX_MESGMON_ENT; nI++ )
	{
		pEnt = &mesg_mon.ent[nI];
		
		if( pEnt->dwWinID != 0 )
			continue;

		mesg_mon.nTotal++;

		pEnt->dwWinID     = dwWinID;
		pEnt->dwStartMesg = dwStartMesg;
		pEnt->dwEndMesg   = dwEndMesg;
		pEnt->dwCounter   = 0;
		return( 1 );
	}	

	return( 0 );
}

// �޽��� ����� ��Ʈ���� ����Ѵ�.
static int unreg_mesgmon( DWORD dwIndex )
{
	MesgMonEntStt	*pEnt;

	if( mesg_mon.nTotal <= 0 )
		return( -1);

	pEnt = &mesg_mon.ent[dwIndex];
	if( pEnt->dwWinID == 0 )
		return( -1 );

	mesg_mon.nTotal--;

	pEnt->dwWinID	  = 0;
	pEnt->dwStartMesg = 0;
	pEnt->dwEndMesg   = 0;
	return( 1 );
}

static char *mesgmon_usage[] = {
	"[Usage]: mesg can be string or ordinal number", 
	"  mesgmon : display usage, win mesg list, registered mesgmon entries.",
	"  mesgmon <win_id> : monitor all the messages of the window.", 
	"  mesgmon x <index> : remove the index",
	"  mesgmon <win_id> <mesg> : monitor mesg of the window",
	"  mesgmon <win_id> <start_mesg> <end_mesg> : monitor from start to end",
	NULL
};
/*
==================== mesgmon ���� =====================================
mesgmon : ���ھ��� �Է��ϸ� ����, �޽�����/���ڿ�, ��ϵ� mesgmon ����Ʈ�� ����Ѵ�.
mesgmon <winid> : �ش� ������� ���޵Ǵ� ���޽����� ��µȴ�.
mesgmon x <index> : �ش� ��Ʈ���� �����Ѵ�.
mesgmon <winid> <start_mesg> : �ش� ������� ���޵Ǵ� �޽������� ����͸��Ѵ�.
mesgmon <winid> <start_mesg> <end_mesg> : �ش� ������� ���޵Ǵ� 
                   ���� �޽������� ������ �޽��� ���̸� ����͸��Ѵ�.
==========================================================================
*/
static int wf_mesgmon( int argc, char *argv[] )
{
	int		nR, nI;
	WinStt	*pWin;
	DWORD	dwWinID, dwStartMesg, dwEndMesg;
	
	if( argc < 2 )
	{	// ������ ����Ѵ�.
		for( nI = 0; mesgmon_usage[nI] != NULL; nI++ )
			kdbg_printf( "%s\n", mesgmon_usage[nI] );

		// �ķ����Ͱ� ������ ������ �޽��� ����� ����Ѵ�.
		disp_wmesg_list();
		// ������ ���� ������ ����Ѵ�.
		disp_mesg_mon_list();	
		return( 0 );
	}

	// unregister �ΰ� ?
	if( argv[1][0] == 'x' || argv[1][0] == 'X' )
	{
		if( argc < 3 )
		{
			kdbg_printf( "mesgmon x <index>\n" );
			return( -1 );
		}
		
		nR = dwDecValue( argv[2] );
		nR = unreg_mesgmon( nR );
		if( nR > 0 )
		{
			kdbg_printf( "unregistered.\n" );
			disp_mesg_mon_list();
		}
		else
			kdbg_printf( "unregister failed!\n" );
		return( nR );
	}
	
	// �ش� �����찡 �����ϴ��� ã�ƺ���.
	dwWinID = dwDecValue( argv[1] );
	pWin = find_window_by_id( dwWinID );
	if( pWin == NULL )
	{	// �����츦 ã�� �� ����.
		kdbg_printf( "WinID(%d) not found!\n", dwWinID );
		return( -1 );
	}

	if( argc == 2 )
	{	// window ID�� ������ ���.
		nR = reg_mesgmon( dwWinID, 0, 0xFFFFFFFF );
		return( nR );
	}

	// ���� �޽����� �Էµ�.
	if( is_digit( argv[2] ) != 0 )
	{	// �޽��� ���� ���� �Է��� ���.
		dwStartMesg = dwDecValue( argv[2] );
	}		
	else
	{	// �޽��� ��Ʈ���� �Է��� ���.
		dwStartMesg = get_wmesg_value( argv[2] );	// ���� �޽��� ��Ʈ��.
		if( dwStartMesg == 0 )
		{	// �޽��� ��Ʈ���� �߸� �Է���.
			kdbg_printf( "%s unknown win mesg!\n", argv[2] );
			return( -1 );
		}
	}

	dwEndMesg = dwStartMesg;
	if( argc == 3 )
	{	// ������ ID, ���� �޽����� �Է��� ���.
		nR = reg_mesgmon( dwWinID, dwStartMesg, dwEndMesg );
		if( nR >= 0 )
		{
			kdbg_printf( "registered.\n" );
			disp_mesg_mon_list();
		}
		else
			kdbg_printf( "register failed!\n" );
		return( nR );
	}

	// ���� �޽����� �Էµ�.
	if( is_digit( argv[3] ) != 0 )
	{	// �޽��� ���� ���� �Է��� ���.
		dwEndMesg = dwDecValue( argv[3] );
	}		
	else
	{	// �޽��� ��Ʈ���� �Է��� ���.
		dwEndMesg = get_wmesg_value( argv[3] );	// ���� �޽��� ��Ʈ��.
		if( dwStartMesg == 0 )
		{	// �޽��� ��Ʈ���� �߸� �Է���.
			kdbg_printf( "%s unknown win mesg!\n", argv[3] );
			return( -1 );
		}
	}

	nR = reg_mesgmon( dwWinID, dwStartMesg, dwEndMesg );
	if( nR >= 0 )
	{
		kdbg_printf( "registered.\n" );
		disp_mesg_mon_list();
	}
	else
		kdbg_printf( "register failed!\n" );

	return( nR );
}

#pragma data_seg( "data2" )
static KShlFuncStt wfunc[] = {
	{ WC_WIN,		wf_win,			"win",			"win [win_id]; list windows" 			},
	{ WC_MESGMON,	wf_mesgmon, 	"mesgmon", 		"mesgmon [win_id] [start_mesg] [end_mesg]; monitor message" 	},
	{ 0, NULL, NULL, NULL }
};
#pragma data_seg()

GuiExportStt gui_exp = {
	wfunc
};




