#ifndef BELLONA2_EXPORT_HEADER_jj
#define BELLONA2_EXPORT_HEADER_jj

extern DWORD find_function_address( ModuleStt *pM, char *pFuncName );
extern int disp_export_table( MY_IMAGE_EXPORT_DIRECTORY *pExp );
extern int realize_exp_table( MY_IMAGE_EXPORT_DIRECTORY *pExp, DWORD dwRelocBase );

#endif