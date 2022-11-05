#ifndef BELLONA2_SYSCALL_TYPE_h
#define BELLONA2_SYSCALL_TYPE_h

typedef enum {
	///////////   RETURN VALUE   ///////////////
	SYSCALL_INVALID_ID		= (-101),
	SYSCALL_FUNC_NOT_FOUND	= (-100),

	///////////   FUNCTION TYPE  ///////////////
    SCTYPE_NULL             = 0,
	SCTYPE_TTYOUT,               				// used for printf
	SCTYPE_EXIT,								// exit
	SCTYPE_REG_EXPORT,							// register export table
	SCTYPE_CALL_SCHEDULER,						// call kernel_scheduler
	SCTYPE_GETCH,								// get a char from stdin
	SCTYPE_MALLOC,								// allocate memory
	SCTYPE_FREE,								// release memory
	SCTYPE_CREATE_THREAD,						// create new thread
	SCTYPE_EXIT_USER_THREAD,					// exit user_thread
	SCTYPE_GET_CUR_TID,							// get current thread id
		
	SCTYPE_CREATE_SHMEM,						// create shared memory
	SCTYPE_CLOSE_SHMEM,							// close shared memory
	SCTYPE_ATTACH_SHMEM,						// attach shared memory
	SCTYPE_DETACH_SHMEM,						// detatch shared memory
	SCTYPE_FIND_SHMEM,							// find shared memory
	SCTYPE_LOCK_SHMEM,							// lock shared memory
	SCTYPE_UNLOCK_SHMEM,						// unlock shared memory

	SCTYPE_OPEN,								// open file
	SCTYPE_CLOSE,								// close file
	SCTYPE_READ,								// read file
	SCTYPE_WRITE,								// write file
	SCTYPE_LSEEK,								// lseek file
	SCTYPE_RENAME,								// rename file

	SCTYPE_DELAY,								// delay
	SCTYPE_PAUSE,								// pause

	SCTYPE_CREATE_SEMAPHORE,
	SCTYPE_OPEN_SEMAPHORE,
	SCTYPE_CLOSE_SEMAPHORE,
	SCTYPE_INC_SEMAPHORE,
	SCTYPE_DEC_SEMAPHORE,

	SCTYPE_GET_SYS_MODULE_HANDLE,				// get system module handle
	SCTYPE_GET_MODULE_HANDLE,					// get_module_handle

	SCTYPE_GET_CURSOR_XY,						// get current cursor position
	SCTYPE_SET_CURSOR_XY,						// get current cursor position
	SCTYPE_SET_FG_PROCESS,						// set foreground process

    SCTYPE_KBHIT,                               // kbhit
    SCTYPE_DIRECT_DISPSTR,                      // direct string display

	SCTYPE_OPENDIR,
	SCTYPE_READDIR,
	SCTYPE_CLOSEDIR,

	SCTYPE_DEL_LINE,							// ���� ������ �����.
	SCTYPE_FORK,								// conventional fork()
	SCTYPE_WAIT,								// child process�� ����� ������ ����Ѵ�.
	SCTYPE_WAITPID,								// Ư�� Child Process�� ����� ������ ����Ѵ�.
	SCTYPE_EXECVE,								// execve

	SCTYPE_IS_GUI_MODE,							// GUI Mode �ΰ�?
	SCTYPE_COPY_KEXECVE_PARAM,
	SCTYPE_GET_TIME,							// ���� �ð��� ��´�.
	SCTYPE_SET_FG_TID,							// TID�� fg thread�� �����Ѵ�.
	SCTYPE_SET_THREAD_ALIAS,					// Thread Alias�� �����Ѵ�.
	SCTYPE_SET_PROCESS_ALIAS,					// Process Alias�� �����Ѵ�.
	SCTYPE_GET_CUR_PID, 						// get current process id
	SCTYPE_GET_R3EXP_TBL,
	SCTYPE_WAITTID, 							// Ư�� Thread�� ����� ������ ����Ѵ�.

    TOTAL_SYSTEM_CALL                           // number of system calls
	
} SYS_CALL_TYPE;

//////////////////////////////////////////////////////////////////////////////////////
typedef enum {
	GRXTYPE_NULL = 0,
	GRXTYPE_FIND_WALL_WINDOW,					// ����ȭ�� �����츦 ã�´�.
	GRXTYPE_COPY_IMG_TO_WIN,					// �̹����� �����쿡 �����Ѵ�.
	GRXTYPE_REFRESH_WIN,						// �����츦 �ٽ� �׸���.
	GRXTYPE_CREATE_WINDOW,						// �����츦 �����Ѵ�.
	GRXTYPE_MESSAGE_PUMPING,					// �����쿡 ���޵� �޽����� ó���Ѵ�.
	GRXTYPE_CREATE_WIN_THREAD, 					// Win Thread�� �����Ѵ�.
	GRXTYPE_CLOSE_WIN_THREAD, 					// Win Thread�� �ݴ´�.
	GRXTYPE_PRE_WMESG_HANDLING,					// Pre Window Message Handling
	GRXTYPE_POST_WMESG_HANDLING,				// Post Window Message Handling
	GRXTYPE_FIND_WMESG_FUNC,					// r3���� ������ window message handler�� ã�´�.
	GRXTYPE_GET_SYS_COLOR,						// get system color
	GRXTYPE_GET_CLIENT_RECT,					// window�� client rect�� ���Ѵ�.
	GRXTYPE_GET_WINDOW_RECT,					// window�� rect�� ���Ѵ�.
	GRXTYPE_FILL_RECT,							// fill rect
	GRXTYPE_FILL_RECT_EX,						// fill rect (or)
	GRXTYPE_LINE,								// draw line
	GRXTYPE_3D_LOOK,							// 3d look���� ���̵��� �׵θ��� �׸���.
	GRXTYPE_GET_WIN_ID,							// Window ID�� ��´�.
	GRXTYPE_REGISTER_GUI_TIMER,					// gui timer�� ����Ѵ�.
	GRXTYPE_UNREGISTER_GUI_TIMER,				// gui timer�� ��� �����Ѵ�.
	GRXTYPE_DRAWTEXT_XY,						// text�� ����Ѵ�.
	GRXTYPE_SET_WIN_TEXT,						// Window Title�� �����Ѵ�.
	GRXTYPE_INIT_MODULE_RES, 					// Gui Application�� window resource�� ���� ������ ���·� �ʱ�ȭ �Ѵ�.
	GRXTYPE_CREATE_BUTTON,						// ��ư�� �����Ѵ�.
	GRXTYPE_TB_ADD_ICON,						// ���α׷��� ����Ǹ� Task Bar�� �������� ����Ѵ�.
	GRXTYPE_TB_DEL_ICON,						// ���α׷��� ����� �������� Task Bar�� �������� ��������Ѵ�.
	GRXTYPE_CLOSE_BUTTON,						// ��ư�� �����Ѵ�.
	GRXTYPE_POST_MESSAGE,						// �޽����� �����Ѵ�.
	GRXTYPE_GET_SCR_INFO,						// ���� ���α׷����� ȭ�� ������ ��´�.
	GRXTYPE_DRAW_LINE,							// ������ �׸���.
	GRXTYPE_LOAD_BITMAP16,						// ����� �޸𸮿� ��Ʈ�� �̹����� �ε��Ͽ� �̹��� ����ü�� �����Ѵ�.
	GRXTYPE_COPY_IMAGE16,						// �̹����� ������ ���ۿ� �����Ѵ�.
	
    TOTAL_GRX_SYSTEM_CALL                       // number of system calls
} GRX_CALL_TYPE;

#endif
