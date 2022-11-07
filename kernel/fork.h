#ifndef BELLONA2_FORK_HEADER_jj
#define BELLONA2_FORK_HEADER_jj

struct ProcessTag;
struct ThreadTag;
struct ForkParamTag;

#define MAX_KERNEL_EVN_STR 1024             // Ŀ�� �������� ������ �� �ִ� ȯ������ ��Ʈ���� ũ��.
/*
typedef struct ExecParamTag {
	char	szPath[260];		            // ���� ���� �н�
	char	szArg[260];			            // Argument string;
	char	szEnv[ MAX_KERNEL_EVN_STR ];    // ȯ�� ���� ��Ʈ��.
};
typedef struct ExecParamTag;
*/

// kexecve ���������� ����ϴ� ����ü
typedef struct KExecveParamTag{
	int		nSize;							// ��ü ũ��.
	int		nArgc;
	char	**ppArgv;
	char 	**ppEnv;
	// ���� ���Ǵ� ���۴� KExecveParamStt �ٷ� ���ʿ� ��ġ�Ѵ�.  �޸� �Ҵ�� (����ü + ����) �Ҵ�.
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

