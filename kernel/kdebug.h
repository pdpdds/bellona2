#ifndef BELLONA2_KERNEL_DEBUG
#define BELLONA2_KERNEL_DEBUG

#define DEFAULT_TAB_SIZE   4

#define BREAK_EXEC			0
#define BREAK_WRITE 		1
#define BREAK_IO			2
#define BREAK_READWRITE 	3

//typedef struct KDBGTag{
//	int nX, nY;
//};
//typedef struct KDBGTag KDBGStt;

typedef struct BreakEntTag{
	DWORD			dwAddr;
	UCHAR			byEnable;
	UCHAR			byOption;
	UCHAR			byLen;
	UCHAR			byGlobal;
	UCHAR			byLocal;
	unsigned short	wTask;
};
typedef struct BreakEntTag BreakEntStt;

#define MAX_INT3_ENT		32

typedef struct {
	DWORD	dwAddr;
	UCHAR	byCode;
} Int3EntStt;


//extern KDBGStt kdbg;
extern Int3EntStt int3_ent[MAX_INT3_ENT];;

extern BELL_EXPORT int  kdbg_printf( char *pFmt, ... );
extern BELL_EXPORT int	kdbg_printf_ex( void *pVCon, char *pFmt, ... );

extern BELL_EXPORT void set_gui_console_ftbl( void *pGCTbl );

//extern int  kdbg_dump_stack();
extern void		kdbg_clearscreen			();
extern void		kdbg_save_context			();
extern void		kdbg_set_register			();
extern void		kdbg_disp_register			();
extern void		kdbg_disp_next_code			();
extern void		kdbg_reload_context			();
extern void		kdbg_breakpoint				( DWORD dwT );
extern void		*pGetBackLinkTSS			( void *pTSS );
extern void		kdbg_set_debugee_rf			( DWORD dwTF );
extern void		kdbg_set_debugee_tf			( DWORD dwTF );
extern void		disp_src_and_regs			( struct TSSTag *pTss, DWORD dwAddr );
extern void		disp_call_stack				( struct TSSTag *pTSS, int nLocalFlag );
extern void 	kdbg_set_debugee_tid		( DWORD dwTID );

extern int		debugger_getchar			();
extern int		swap_cc_by_org_code			();
extern int		is_debugger_active			();
extern int		get_vertical_line_size		();
extern int		kassemble					( DWORD dwAddr );
extern int		recover_cc					( UCHAR *pAddr );
extern int		overwrite_cc				( UCHAR *pAddr );
extern int		kedit_memory				( DWORD dwAddr );
extern int		kdbg_key_input				( BKeyStt *pKey );
extern int		set_vertical_line_size		( int nLine );
extern int		set_debugger_active			( int nActive );
extern int		kdbg_set_trace_repeat		( int nRepeat );
extern int		kxy_printf					( int nX, int nY, char *pFmt, ... );
extern int		unassembling				( DWORD dwEIP, int nMaxDispLine );
extern int	 	disp_hw_breaks				();
extern int 		kdbg_clear_dr				( char ch );
extern int 		kdbg_set_dr					( DWORD dwAddr, int nOption, int nLength );


extern UINT16	wGetBackLink				( void *pV );

extern DWORD	dwTTYOut					( char *pB, int nTabSize, int x, int y, char *pS );
extern DWORD	kdbg_change_CR3 			();
extern DWORD	kdbg_get_debugee_tid		();

#endif
