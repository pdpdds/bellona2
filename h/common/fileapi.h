#ifndef BELLONA2_LIB_FILE_HEADER
#define BELLONA2_LIB_FILE_HEADER

#define MAX_PROCFD_CHUNK_ENT		32
#define MAX_PROCFD_CHUNK			32

#define PROC_FD_TYPE_FREE			0
#define PROC_FD_TYPE_USED			1

// process file descriptor structure
typedef struct {
	int	nType;
	int	nKernelHandle;			// kernel file descriptor index
} ProcFDStt;		

// processor file descriptor chunk structure
struct ProcFDChunkTag {
	int						nTotalFD;
	ProcFDStt				ent[MAX_PROCFD_CHUNK_ENT];
};
typedef struct ProcFDChunkTag ProcFDChunkStt;

// process file descriptor table structure
struct ProcFDTblTag {
	int				nTotalChunk;
	ProcFDChunkStt	*chunk[MAX_PROCFD_CHUNK];
};
typedef struct ProcFDTblTag ProcFDTblStt;

extern BELL_EXPORT BYTE *load_file		( char *pS, int *pSize );

extern int init_fd_tbl					( ProcFDTblStt *pFDTbl );
extern int close_fd_tbl					( ProcFDTblStt *pFDTbl );

#endif