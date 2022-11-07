#ifndef BELLONA_LIB_MEM_HEADER
#define BELLONA_LIB_MEM_HEADER

#include <types.h>

typedef struct {
	char	szName[128];
	int		nID;
	DWORD	dwAddr;
	DWORD	dwSize;
} ShMemParamStt;

extern int close_sh_mem( int nID );
extern int find_sh_mem( char *pName );
extern int attach_sh_mem( int nID, DWORD dwAddr );
extern int detach_sh_mem( int nID, DWORD dwAddr );
extern int create_sh_mem( char *pName, DWORD dwSize );
extern int lock_sh_mem( int nID );
extern int unlock_sh_mem( int nID );

#endif
