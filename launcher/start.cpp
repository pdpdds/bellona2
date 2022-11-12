#include "start.h"

extern "C" int kmain(unsigned long magic, unsigned long addr);

extern "C" void start()
{
	__asm
	{
		MOV     ESP, 0x40000; //스택 설정

		PUSH    0; //플래그 레지스터 초기화
		POPF

			//GRUB에 의해 담겨 있는 정보값을 스택에 푸쉬한다.
		PUSH    EBX; //멀티부트 구조체 포인터
		PUSH    EAX; //부트로더 매직값

		//위의 두 파라메터와 함께 kmain 함수를 호출한다.
		CALL    kmain; //C++ 메인 함수 호출

	//루프를 돈다. 
	halt:
		jmp halt;
	}
}

