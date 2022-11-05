#include "lib.h"
#include <bellona2.h>

static SysModuleStt *pSM = 0;
/*
_declspec(naked) int module_call( void *pM,... )
{
	static DWORD dwHandler;

	if( pSM == NULL )
		pSM = (SysModuleStt*)system_call( SCTYPE_GET_SYS_MODULE_HANDLE, 0 );

	if( pSM == NULL || pSM->pHandler == NULL )
		_asm RETN;

	dwHandler = (DWORD)pSM->pHandler;
	
	_asm {
		MOV  EAX,dwHandler;
		JMP  EAX;
	}
}
*/
void set_module_alias( ModuleStt *pM, char *pAlias )
{
	strcpy( pM->szAlias, pAlias );
}

