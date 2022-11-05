#include "lib.h"

UAreaStt *get_uarea()
{
	int				nR;
	R3ExportTblStt  r3exp;

	nR = syscall_stub( SCTYPE_GET_R3EXP_TBL, &r3exp );

	return( (UAreaStt*)r3exp.func[ R3EXI_UAREA ] );
}

// ����� UArea �ʱ�ȭ.
int init_uarea( UAreaStt *pU )
{
	memset( pU, 0, sizeof( UAreaStt ) );
	
	// ���� ��ũ���� ���̺� �ʱ�ȭ.
	init_fd_tbl( &pU->fdtbl );

    // env buffer �ʱ�ȭ.
    init_env_buff( &pU->env );

	// �ʱ�ȭ �÷��� ����.
	pU->byInitFlag = 1;

	// CWD�� /�� �����Ѵ�.
	strcpy( pU->szCWD, "/" );
                           
	return( 0 );
}

// ����� UArea ����.
int close_uarea()
{
	UAreaStt *pU;

	pU = get_uarea();
	if( pU == NULL )
		return( -1 );
	
	if( pU->byInitFlag == 0 )
		return( 0 );// �ʱ�ȭ ���� �ʾҰų� �̹� ������ ��.
	
	//printf( "close_fd_tbl: ok\n" );

    // ���� ��ũ���� ���̺� ����.
    close_fd_tbl( &pU->fdtbl );

	//printf( "close_env_buff: ok\n" );

    // env buffer�� �����Ѵ�.
    close_env_buff( &pU->env );

	// �ʱ�ȭ �÷��׸� �����Ѵ�.
	pU->byInitFlag = 0;

   return( 0 );
}

