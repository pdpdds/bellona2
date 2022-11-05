#include "lib.h"

UAreaStt *get_uarea()
{
	int				nR;
	R3ExportTblStt  r3exp;

	nR = syscall_stub( SCTYPE_GET_R3EXP_TBL, &r3exp );

	return( (UAreaStt*)r3exp.func[ R3EXI_UAREA ] );
}

// 모듈의 UArea 초기화.
int init_uarea( UAreaStt *pU )
{
	memset( pU, 0, sizeof( UAreaStt ) );
	
	// 파일 디스크립터 테이블 초기화.
	init_fd_tbl( &pU->fdtbl );

    // env buffer 초기화.
    init_env_buff( &pU->env );

	// 초기화 플래그 설정.
	pU->byInitFlag = 1;

	// CWD를 /로 설정한다.
	strcpy( pU->szCWD, "/" );
                           
	return( 0 );
}

// 모듈의 UArea 해제.
int close_uarea()
{
	UAreaStt *pU;

	pU = get_uarea();
	if( pU == NULL )
		return( -1 );
	
	if( pU->byInitFlag == 0 )
		return( 0 );// 초기화 되지 않았거나 이미 해제된 것.
	
	//printf( "close_fd_tbl: ok\n" );

    // 파일 디스크립터 테이블 해제.
    close_fd_tbl( &pU->fdtbl );

	//printf( "close_env_buff: ok\n" );

    // env buffer를 해제한다.
    close_env_buff( &pU->env );

	// 초기화 플래그를 해제한다.
	pU->byInitFlag = 0;

   return( 0 );
}

