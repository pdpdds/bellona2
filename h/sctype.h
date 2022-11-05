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

	SCTYPE_DEL_LINE,							// 라인 끝까지 지운다.
	SCTYPE_FORK,								// conventional fork()
	SCTYPE_WAIT,								// child process가 종료될 때까지 대기한다.
	SCTYPE_WAITPID,								// 특정 Child Process가 종료될 때까지 대기한다.
	SCTYPE_EXECVE,								// execve

	SCTYPE_IS_GUI_MODE,							// GUI Mode 인가?
	SCTYPE_COPY_KEXECVE_PARAM,
	SCTYPE_GET_TIME,							// 현재 시간을 얻는다.
	SCTYPE_SET_FG_TID,							// TID를 fg thread로 설정한다.
	SCTYPE_SET_THREAD_ALIAS,					// Thread Alias를 설정한다.
	SCTYPE_SET_PROCESS_ALIAS,					// Process Alias를 설정한다.
	SCTYPE_GET_CUR_PID, 						// get current process id
	SCTYPE_GET_R3EXP_TBL,
	SCTYPE_WAITTID, 							// 특정 Thread가 종료될 때까지 대기한다.

    TOTAL_SYSTEM_CALL                           // number of system calls
	
} SYS_CALL_TYPE;

//////////////////////////////////////////////////////////////////////////////////////
typedef enum {
	GRXTYPE_NULL = 0,
	GRXTYPE_FIND_WALL_WINDOW,					// 바탕화면 윈도우를 찾는다.
	GRXTYPE_COPY_IMG_TO_WIN,					// 이미지를 윈도우에 복사한다.
	GRXTYPE_REFRESH_WIN,						// 윈도우를 다시 그린다.
	GRXTYPE_CREATE_WINDOW,						// 윈도우를 생성한다.
	GRXTYPE_MESSAGE_PUMPING,					// 윈도우에 전달된 메시지를 처리한다.
	GRXTYPE_CREATE_WIN_THREAD, 					// Win Thread를 생성한다.
	GRXTYPE_CLOSE_WIN_THREAD, 					// Win Thread를 닫는다.
	GRXTYPE_PRE_WMESG_HANDLING,					// Pre Window Message Handling
	GRXTYPE_POST_WMESG_HANDLING,				// Post Window Message Handling
	GRXTYPE_FIND_WMESG_FUNC,					// r3에서 지정한 window message handler를 찾는다.
	GRXTYPE_GET_SYS_COLOR,						// get system color
	GRXTYPE_GET_CLIENT_RECT,					// window의 client rect를 구한다.
	GRXTYPE_GET_WINDOW_RECT,					// window의 rect를 구한다.
	GRXTYPE_FILL_RECT,							// fill rect
	GRXTYPE_FILL_RECT_EX,						// fill rect (or)
	GRXTYPE_LINE,								// draw line
	GRXTYPE_3D_LOOK,							// 3d look으로 보이도록 테두리를 그린다.
	GRXTYPE_GET_WIN_ID,							// Window ID를 얻는다.
	GRXTYPE_REGISTER_GUI_TIMER,					// gui timer를 등록한다.
	GRXTYPE_UNREGISTER_GUI_TIMER,				// gui timer를 등록 해제한다.
	GRXTYPE_DRAWTEXT_XY,						// text를 출력한다.
	GRXTYPE_SET_WIN_TEXT,						// Window Title을 설정한다.
	GRXTYPE_INIT_MODULE_RES, 					// Gui Application의 window resource를 접근 가능한 형태로 초기화 한다.
	GRXTYPE_CREATE_BUTTON,						// 버튼을 생성한다.
	GRXTYPE_TB_ADD_ICON,						// 프로그램이 실행되면 Task Bar에 아이콘을 등록한다.
	GRXTYPE_TB_DEL_ICON,						// 프로그램이 종료될 시점에서 Task Bar에 아이콘을 등록해제한다.
	GRXTYPE_CLOSE_BUTTON,						// 버튼을 제거한다.
	GRXTYPE_POST_MESSAGE,						// 메시지를 전달한다.
	GRXTYPE_GET_SCR_INFO,						// 응용 프로그램에서 화면 정보를 얻는다.
	GRXTYPE_DRAW_LINE,							// 라인을 그린다.
	GRXTYPE_LOAD_BITMAP16,						// 사용자 메모리에 비트맵 이미지를 로드하여 이미지 구조체를 리턴한다.
	GRXTYPE_COPY_IMAGE16,						// 이미지를 윈도우 버퍼에 복사한다.
	
    TOTAL_GRX_SYSTEM_CALL                       // number of system calls
} GRX_CALL_TYPE;

#endif
