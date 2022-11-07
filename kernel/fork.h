#ifndef BELLONA2_FORK_HEADER_jj
#define BELLONA2_FORK_HEADER_jj

struct ProcessTag;
struct ThreadTag;
struct ForkParamTag;

#define MAX_KERNEL_EVN_STR 1024             // 커널 영역에서 설정할 수 있는 환병변수 스트링의 크기.
/*
typedef struct ExecParamTag {
	char	szPath[260];		            // 실행 파일 패스
	char	szArg[260];			            // Argument string;
	char	szEnv[ MAX_KERNEL_EVN_STR ];    // 환경 변수 스트링.
};
typedef struct ExecParamTag;
*/

// kexecve 내부적으로 사용하는 구조체
typedef struct KExecveParamTag{
	int		nSize;							// 전체 크기.
	int		nArgc;
	char	**ppArgv;
	char 	**ppEnv;
	// 실제 사용되는 버퍼는 KExecveParamStt 바로 뒤쪽에 위치한다.  메모리 할당시 (구조체 + 버퍼) 할당.
} KExecveParamStt;

//typedef int (*FORK_FUNC)( ExecParamStt *pExecParam );
/*
typedef struct ForkParamTag {
	struct ThreadTag	*pParentThread;		// parent thread
	struct ProcessTag	*pNewProcess;		// newly created thread
	struct ThreadTag	*pNewThread;		// newly created thread
	ExecParamStt		exec_param;			// create parameter
	DWORD				dwStackSize;		// initial stack size
	int					nInitialState;		// initial state
	FORK_FUNC			pFunc;				// initial thread's entry point
};
typedef struct ForkParamTag ForkParamStt;
*/

// create new process and it's one thread
//extern int set_global_fork_param( ForkParamStt *pFP );
//extern int get_global_fork_param( ForkParamStt *pFP );
extern int launch_r3_program	( KExecveParamStt *pExecParam );
extern int call_r3_function		( void *pPram );
extern int call_r3_thread		( void *pPram );

#endif

