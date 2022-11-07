#ifndef BELLONA2_SIGNAL_HEADER_jj
#define BELLONA2_SIGNAL_HEADER_jj

#define	SIG_USER_0				0x01
#define	SIG_USER_1				0x02
#define	SIG_USER_2				0x04
#define	SIG_STOP				0x08
#define	SIG_CONT				0x10
#define	SIG_KILL				0x20

typedef int (*SIG_FUNC)( DWORD dwParam );

typedef struct SignalTag{
	DWORD		dwMask;
	DWORD		dwSignal;
	SIG_FUNC	func[32];		// 32 signal handler
	DWORD		param[32];		// 32 signal parameter
};
typedef struct SignalTag SignalStt;

extern BELL_EXPORT DWORD get_thread_signal_bits( struct ThreadTag *pT );
extern BELL_EXPORT int send_signal_to_thread( DWORD dwTID, DWORD dwSignal );
extern BELL_EXPORT int clear_thread_signal_bits( struct ThreadTag *pT, DWORD dwSig );

extern int 		copy_thread_sig_bits	();
extern void 	disp_signal_str			();
extern DWORD 	get_signal_bit			( char *pS );

#endif