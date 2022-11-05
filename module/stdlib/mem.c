#include "lib.h"

int create_sh_mem ( char *pName, DWORD dwSize )
{
	int				nR;
	ShMemParamStt	sh_param;

	// copy parameters
	memset( &sh_param, 0, sizeof( sh_param ) );
	if( pName == NULL )
		sh_param.szName[0] = 0;
	else
		strcpy( sh_param.szName, pName );
	sh_param.dwSize = dwSize;
		
	nR = system_call( SCTYPE_CREATE_SHMEM, (DWORD)&sh_param );

	// nR is the id of the newly created shared memory.
	return( nR );
}

int close_sh_mem( int nID )
{
	int nR;

	nR = system_call( SCTYPE_CLOSE_SHMEM, (DWORD)nID );

	return( nR );
}

int attach_sh_mem( int nID, DWORD dwAddr )
{
	int				nR;
	ShMemParamStt	sh_param;

	memset( &sh_param, 0, sizeof( sh_param ) );
	sh_param.nID	= nID;
	sh_param.dwAddr = dwAddr;

	nR = system_call( SCTYPE_ATTACH_SHMEM, (DWORD)&sh_param );

	return( nR);
}

int detach_sh_mem( int nID, DWORD dwAddr )
{
	int				nR;
	ShMemParamStt	sh_param;

	memset( &sh_param, 0, sizeof( sh_param ) );
	sh_param.nID	= nID;
	sh_param.dwAddr = dwAddr;

	nR = system_call( SCTYPE_DETACH_SHMEM, (DWORD)&sh_param );

	return( nR );
}

int find_sh_mem( char *pName )
{
	int				nR;
	ShMemParamStt	sh_param;

	memset( &sh_param, 0, sizeof( sh_param ) );
	strcpy( sh_param.szName, pName );

	nR = system_call( SCTYPE_FIND_SHMEM, (DWORD)&sh_param );

	return( nR );
}

int lock_sh_mem( int nID )
{
	int nR;

	nR = system_call( SCTYPE_LOCK_SHMEM, (DWORD)nID );

	return( nR );
}

int unlock_sh_mem( int nID )
{
	int nR;

	nR = system_call( SCTYPE_UNLOCK_SHMEM, (DWORD)nID );

	return( nR );
}

