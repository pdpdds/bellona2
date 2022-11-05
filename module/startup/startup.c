/*************************************************
 ** VC++�� �������� ���� CS:EIP�� ��Ʈ�� ����Ʈ **
 ************************************************/

#include <types.h>
#include <signal.h> 
#include <uarea.h>

extern void main_startup( unsigned long dwMain, DWORD dwSigStruct );
extern int main( int argc, char *argv[] );

static UAreaStt uarea;

void mainCRTStartup()
{
	// ���׸�Ʈ �������� ���� �ʱ�ȭ �Ѵ�.
	_asm {	
		MOV DX,SS
		MOV DS,DX
		MOV ES,DX
		MOV FS,DX
		MOV GS,DX
	}

	// ���ϵ��� �ʴ´�.
	main_startup( (unsigned long)main, (DWORD)&uarea );

	for( ;; );
}

