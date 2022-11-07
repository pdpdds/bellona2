#ifndef BELLONA2_MODULE_HEADER_jj
#define BELLONA2_MODULE_HEADER_jj

#define MTYPE_MODULE	1
#define MTYPE_APPS		2

#define LOAD_MODULE_TEMP_ADDR	(0x100000 * 132)	// 132M	 
#define LOAD_MODULE_START_ADDR	(0x100000 * 136)	// 136M	
#define LOAD_MODULE_SPACE_SIZE	(0x100000 * 120)	// 120M

typedef int (*MODULE_MAIN_FUNC)( DWORD dwModuleStt, int argc, char *argv[] );

struct ModuleTag {
	char						szAlias[32];		// module alias
	UINT16						wFileType;			// file type
	UINT16						wModuleType;		// module type						
	DWORD						dwFileSize;			// module size
	DWORD						dwLoadAddr;			// module address
	int							nTotalPage;			// memory page size
	DWORD						dwEntryPoint;		// entry point
	void						*pMyDbg;			// module debug information
	MY_IMAGE_EXPORT_DIRECTORY	*pExp;				// module export function table
	BYTE						byUserLevel;		// user level module ?
	int							nProcessCount;		// number of processes
};
typedef struct ModuleTag ModuleStt;

#define MAX_MODULE		32

typedef struct {
	int				nTotal;							// number of registered modules
	ModuleStt		*mod[MAX_MODULE];
} SysModuleStt;

/////////////////////////////////////////////////////

extern SysModuleStt	sys_module;

extern int disp_module();
extern int init_sys_module_struct();
extern SysModuleStt *get_sys_module();
extern int unload_module( char *pAlias );
extern int inc_process_count( ModuleStt *pM );
extern int dec_process_count( ModuleStt *pM );
extern int load_bellona2_dbginfo( char *pPath );
extern ModuleStt *load_module( char *pFileName );
extern ModuleStt *find_module_by_addr( DWORD dwAddr );
extern ModuleStt *load_user_module( char *pFileName );
extern ModuleStt *load_pe( char *pFileName, UINT16 wType );
extern ModuleStt *find_module_by_alias( char *pAlias, int *pNi );
extern int import_linking( MY_IMAGE_IMPORT_DESCRIPTOR *pImp, DWORD dwBaseAddr );

#endif