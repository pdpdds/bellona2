/*************************************************
 ** VC++로 컴파일할 때의 CS:EIP의 엔트리 포인트 **
 ************************************************/

#include <types.h>
#include <signal.h> 
#include <uarea.h>

extern void main_startup( unsigned long dwMain, DWORD dwSigStruct );
extern int main( int argc, char *argv[] );

static UAreaStt uarea;

void mainCRTStartup()
{
	// 세그먼트 레지스터 값을 초기화 한다.
	_asm {	
		MOV DX,SS
		MOV DS,DX
		MOV ES,DX
		MOV FS,DX
		MOV GS,DX
	}

	// 리턴되지 않는다.
	main_startup( (unsigned long)main, (DWORD)&uarea );

	for( ;; );
}

