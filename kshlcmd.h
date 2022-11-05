#ifndef BELLONA2_DEBUG_COMMAND_oh
#define BELLONA2_DEBUG_COMMAND_oh

typedef struct KShlFuncTag{
	int 	nType;
	int 	(*pFunc)( int argc, char *argv[] );
	char	*pS;
	char	*pHelpStr;
} KShlFuncStt;

extern int		nIsCallThenImplantCC		();
extern int		kdbg_set_hw_breakpoint		( int argc, char *argv[] );
extern int		nINT3Breakpoint				( int argc, char *argv[] );
extern int		nChkInt3AddrAndRecover		();
extern int		kshell_function				( char *pCmdStr );

extern void		vDump						( int argc, char* argv[] );
extern void		vUasm						( int argc, char* argv[] );
extern void		vDispSymbol					( int argc, char* argv[] );
extern void		vDispSrcFile				( int argc, char *argv[] );
extern void		vDisplayMemBlock			( int argc, char *argv[] );
extern void		kdbg_change_register_value	( int argc, char *argv[] );
extern void		disp_version				();

extern DWORD	dump_memory					( DWORD dwAddr, DWORD dwSize );

extern BELL_EXPORT void read_cmos_time		( struct TTimeTag *pTime );

#endif
