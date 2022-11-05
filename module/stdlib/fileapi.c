#include "lib.h"

int close( int nHandle );

static int alloc_proc_fd( ProcFDTblStt *pFDTbl );
static ProcFDStt *get_proc_fd_ptr( ProcFDTblStt *pFDTbl, int nHandle );

// ���� ��ũ���� ���̺��� �ʱ�ȭ �Ѵ�.
int init_fd_tbl( ProcFDTblStt *pFDTbl )
{
	ProcFDStt	*local_fd[3];

	memset( pFDTbl, 0, sizeof( ProcFDTblStt ) );

	// allocate local 0, 1, 2 fd
	alloc_proc_fd( pFDTbl );
	alloc_proc_fd( pFDTbl );
	alloc_proc_fd( pFDTbl );

	local_fd[0] = get_proc_fd_ptr( pFDTbl, 0 );
	local_fd[1] = get_proc_fd_ptr( pFDTbl, 1 );
	local_fd[2] = get_proc_fd_ptr( pFDTbl, 2 );

	local_fd[0]->nKernelHandle = 0;
	local_fd[1]->nKernelHandle = 1;
	local_fd[2]->nKernelHandle = 2;

	return( 0 );
}

// ���� ��ũ���� ���̺��� �����Ѵ�.  (���� ������ ��� �ݴ´�.)
int close_fd_tbl( ProcFDTblStt *pFDTbl )
{
    ProcFDChunkStt *pChunk;
    int             nI, nIndex, nHandle;

    for( nIndex = 0; nIndex < pFDTbl->nTotalChunk; nIndex++ )
    {
        pChunk = pFDTbl->chunk[nIndex];
        if( pChunk == NULL || pChunk->nTotalFD <= 0 )
            continue;

        // ���� ���� ��ũ���Ͱ� �ϳ��� ������ �݈f��.
        for( nI = 0; nI < MAX_PROCFD_CHUNK_ENT; nI++ )
        {   // ������ �ݴ´�.
            if( pChunk->ent[nI].nType != PROC_FD_TYPE_FREE )
            {          
                nHandle = ( nIndex * MAX_PROCFD_CHUNK_ENT ) + nI;
                close( nHandle );            
            }
        }

        // chunk�� �����Ѵ�.
        free( pChunk );
        pFDTbl->chunk[nIndex] = NULL;
    }

    return( 0 );
}

// ���ο� chunk�� �Ҵ��Ѵ�.
// ���μ����� �����ϰ� �ִ� ���߿� chunk�� �������� �ʴ´�.
// ���μ����� ����ǰ� uarea�� �ı��� �� �����ȴ�.
static ProcFDChunkStt *alloc_new_fd_chunk( ProcFDTblStt *pFDTbl )
{
	ProcFDChunkStt	*pChunk;

	pChunk = (ProcFDChunkStt*)malloc( sizeof( ProcFDChunkStt ) );
	if( pChunk == NULL )
		return( NULL );

	memset( pChunk, 0, sizeof( ProcFDChunkStt ) );

	pFDTbl->chunk[ pFDTbl->nTotalChunk++ ] = pChunk;
	
	return( pChunk );
}

// FD ������ �Ҵ��ϰ� �� ID (Index)�� �����Ѵ�.
static int alloc_proc_fd( ProcFDTblStt *pFDTbl )
{
	int					nK, nI;
	ProcFDChunkStt		*pChunk;

	// �� ��Ʈ���� ������ ���� Chunk�� �ϳ� �Ҵ��Ѵ�.
	pChunk = NULL;
	for( nK = 0; nK < pFDTbl->nTotalChunk; nK++ )
	{
		pChunk = pFDTbl->chunk[nK];
		if( pChunk == NULL || pChunk->nTotalFD < MAX_PROCFD_CHUNK_ENT )
			break;
	}
	// ���ο� FD Chunk�� �Ҵ��Ѵ�.
	if( pChunk == NULL )
	{
		pChunk = alloc_new_fd_chunk( pFDTbl );
		if( pChunk == NULL )
			return( -1 );		// chunk allocation failed!
	}
	
	// free fd�� ã�´�.
	for( nI = 0; ; nI++ )
	{
		if( nI >= MAX_PROCFD_CHUNK_ENT )
			return( -1 );		// ã�� �� ����.

		if( pChunk->ent[nI].nType == PROC_FD_TYPE_FREE )
			break;
	}

	pChunk->ent[nI].nType = PROC_FD_TYPE_USED;
	pChunk->nTotalFD++;
		
	return( (nK * MAX_PROCFD_CHUNK_ENT ) + nI );
}

static int free_proc_fd( ProcFDTblStt *pFDTbl, int nHandle )
{
	int				nK, nI;
	ProcFDChunkStt	*pChunk;

	nK = nHandle / MAX_PROCFD_CHUNK_ENT;
	nI = nHandle % MAX_PROCFD_CHUNK_ENT;

	if( nK >= pFDTbl->nTotalChunk )
		return( -1 );

	pChunk = pFDTbl->chunk[nK];
    if( pChunk == NULL )
        return( -1 );

	pChunk->ent[nI].nType = PROC_FD_TYPE_FREE;
	pChunk->nTotalFD--;
	
	return( 0 );
}

// FD �ڵ�(Index)�κ��� FD ��Ʈ���� �ּҸ� ���Ѵ�.
static ProcFDStt *get_proc_fd_ptr( ProcFDTblStt *pFDTbl, int nHandle )
{
	int				nK, nI;
	ProcFDChunkStt	*pChunk;

	nK = nHandle / MAX_PROCFD_CHUNK_ENT;
	nI = nHandle % MAX_PROCFD_CHUNK_ENT;

	if( nK >= pFDTbl->nTotalChunk )
		return( NULL );

	pChunk = pFDTbl->chunk[nK];
    if( pChunk == NULL )
        return( NULL );

	return( &pChunk->ent[nI] );
}

static int set_proc_fd( ProcFDTblStt *pFDTbl, int nHandle, int nKernelHandle )
{
	ProcFDStt	*pFD;

	pFD = get_proc_fd_ptr( pFDTbl, nHandle );
	if( pFD == NULL )
		return( -1 );

	pFD->nKernelHandle = nKernelHandle;

	return( 0 );
}

static int get_proc_fd( ProcFDTblStt *pFDTbl, int nHandle )
{
	ProcFDStt	*pFD;

	pFD = get_proc_fd_ptr( pFDTbl, nHandle );
	if( pFD == NULL )
		return( -1 );

	return( pFD->nKernelHandle );
}

static ProcFDTblStt *get_fd_tbl()
{
	ProcFDTblStt	*pFDTbl;
	UAreaStt		*pUArea;

	pUArea = get_uarea();
	if( pUArea == NULL )
		return( NULL );

	pFDTbl = &pUArea->fdtbl;

	return( pFDTbl );
}

// syscall for read, write, lseek...
static int file_io( int nType, DWORD nHandle,... )
{
	ProcFDTblStt	*pFDTbl;
	int				nR, nKernHandle;

	pFDTbl = get_fd_tbl();
	if( pFDTbl == NULL )
		return( -1 );

	nKernHandle = get_proc_fd( pFDTbl, nHandle );
	if( nKernHandle < 0 )		// invalid handle
		return( -1 );

	nHandle = nKernHandle;

	nR = (int)system_call( nType, (DWORD)&nHandle );
 
	return( nR );
}

int open( char *pFileName, int nOFlag, ... )
{
	ProcFDTblStt	*pFDTbl;
	int				nHandle, nKernHandle, nR;

	pFDTbl = get_fd_tbl();
	if( pFDTbl == NULL )
		return( -1 );

	// allocate process fd
	nHandle = alloc_proc_fd( pFDTbl );
	if( nHandle < 0 )
		return( -1 );

	nKernHandle = (int)system_call( SCTYPE_OPEN, (DWORD)&pFileName );
	if( nKernHandle < 0 )
		goto OPEN_ERROR;

	nR = set_proc_fd( pFDTbl, nHandle, nKernHandle );

	return( nHandle );

OPEN_ERROR:
	// free process fd
	free_proc_fd( pFDTbl, nHandle );

	return( -1 );
}

int close( int nHandle )
{
	int nR;

	if( nHandle > 2 )
		nR = file_io( SCTYPE_CLOSE, nHandle );
	else 
		nR = 0;
    
	if( nR == 0 )
    {   // ���� ��ũ���͸� �����Ѵ�.
       	ProcFDTblStt  *pFDTbl = get_fd_tbl();
    	if( pFDTbl != NULL )
			free_proc_fd( pFDTbl, nHandle );

            //set_proc_fd( pFDTbl, nHandle, 0 );
    }
	
	return( nR );
}

int read( int nHandle, char *pBuff, int nSize )
{
	int nR;
	
	if( pBuff ==  NULL || nSize == 0 )
		return( 0 );
	
	nR = file_io( SCTYPE_READ, nHandle, pBuff, nSize );
	return( nR );
}

int write( int nHandle, char *pBuff, int nSize )
{
	int nR;

	if( pBuff ==  NULL || nSize == 0 )
		return( 0 );
	
	nR = file_io( SCTYPE_WRITE, nHandle, pBuff, nSize );
	return( nR );
}

long lseek( int nHandle, long lOffset, int nOrigin )
{
	int lR;

	lR = (long)file_io( SCTYPE_LSEEK, nHandle, lOffset, nOrigin );
	
	return( lR );
}

int rename( char *pOldName, char *pNewName )
{
	int		nR;
	DWORD	x[2];

	if( pOldName == NULL || pNewName == NULL || pOldName[0] == 0 || pNewName[0] == 0 )
		return( -1 );

	x[0] = (DWORD)pOldName;
	x[1] = (DWORD)pNewName;

	nR = (int)system_call( SCTYPE_RENAME, (DWORD)x );

	return( nR );
}

DIR *opendir( char *pFileName )
{
	DIR				*pDIR;
	ProcFDTblStt	*pFDTbl;
	int				nHandle, nKernHandle, nR;

	pFDTbl = get_fd_tbl();
	if( pFDTbl == NULL )
		return( NULL );

	// allocate process fd
	nHandle = alloc_proc_fd( pFDTbl );
	if( nHandle < 0 )
		return( NULL );

	nKernHandle = (int)syscall_stub( SCTYPE_OPENDIR, pFileName );
	if( nKernHandle < 0 )
	{
ERR:	free_proc_fd( pFDTbl, nHandle );
		return( NULL );
	}

	nR = set_proc_fd( pFDTbl, nHandle, nKernHandle );

	// DIR ����ü�� �Ҵ��Ͽ� �ʱ�ȭ �Ѵ�.
	pDIR = (DIR*)malloc( sizeof( DIR ) );
	if( pDIR == NULL )
		goto ERR;
	memset( pDIR, 0, sizeof( DIR ) );
	pDIR->lHandle = nHandle;

	return( pDIR );
}

struct dirent *readdir( DIR *pDir )
{
	int				nR;
	DIRENTStt		dent;
	ProcFDStt		*pFD;
	ProcFDTblStt	*pFDTbl;
	
	pFDTbl = get_fd_tbl();
	if( pFDTbl == NULL )
		return( NULL );
	pFD = get_proc_fd_ptr( pFDTbl, pDir->lHandle );
	if( pFD == NULL )
		return( NULL );

	if( pFD->nKernelHandle < 0 )
		return( NULL );

	nR =  (int)syscall_stub( SCTYPE_READDIR, &dent, pFD->nKernelHandle );
	if( nR < 0 )
		return( NULL );

	// ���丮 ������ �����Ѵ�.
	if( dent.szFileName[0] != 0 )
		strcpy( pDir->ent.d_name, dent.szFileName );
	else
		strcpy( pDir->ent.d_name, dent.szAlias );
	
	pDir->ent.d_size = dent.lFileSize;

	return( &pDir->ent );
}

int closedir( DIR *pDir ) 
{
	int				nR;
	ProcFDStt		*pFD;
	ProcFDTblStt	*pFDTbl;
	
	pFDTbl = get_fd_tbl();
	if( pFDTbl == NULL )
		return( -1 );

	pFD = get_proc_fd_ptr( pFDTbl, pDir->lHandle );
	if( pFD == NULL )
		return( -1 );

	nR = (int)syscall_stub( SCTYPE_CLOSEDIR, pFD->nKernelHandle );

	free_proc_fd( pFDTbl, pDir->lHandle );
	free( pDir );

	return( 0 );
}				

// �޸𸮸� �Ҵ��Ͽ� ������ �о���δ�.
BYTE *load_file( char *pS, int *pSize )
{
	BYTE	*pB;
	int		nHandle;

	// ������ �����Ѵ�.
	nHandle = open( pS, _O_RDONLY );
	if( nHandle < 0 )
		return( NULL );	// ������ �� �� ����.

	// ���� ũ�⸦ �˾Ƴ���.
	pSize[0] = lseek( nHandle, 0, SEEK_END );
	if( pSize[0] <= 0 )
	{
ERR:	close( nHandle );
		return( NULL );
	}
	
	// �޸𸮸� �Ҵ��Ѵ�.
	pB = (BYTE*)malloc( pSize[0] );
	if( pB == NULL )
		goto ERR;

	// ���� ��ü�� �д´�.
	lseek( nHandle, 0, SEEK_SET );
	read( nHandle, pB, pSize[0] );
	close( nHandle );

	// �Ҵ�� ���۸� �����Ѵ�.
	return( pB );
}


