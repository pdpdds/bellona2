#include "start.h"

extern "C" int kmain(unsigned long magic, unsigned long addr);

extern "C" void start()
{
	__asm
	{
		MOV     ESP, 0x40000; //���� ����

		PUSH    0; //�÷��� �������� �ʱ�ȭ
		POPF

			//GRUB�� ���� ��� �ִ� �������� ���ÿ� Ǫ���Ѵ�.
		PUSH    EBX; //��Ƽ��Ʈ ����ü ������
		PUSH    EAX; //��Ʈ�δ� ������

		//���� �� �Ķ���Ϳ� �Բ� kmain �Լ��� ȣ���Ѵ�.
		CALL    kmain; //C++ ���� �Լ� ȣ��

	//������ ����. 
	halt:
		jmp halt;
	}
}

