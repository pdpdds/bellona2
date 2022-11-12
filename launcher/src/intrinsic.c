#include "intrinsic.h"

int _outp(unsigned short, int);
unsigned long _outpd(unsigned int, int);
unsigned short _outpw(unsigned short, unsigned short);
int _inp(unsigned short);
unsigned short _inpw(unsigned short);
unsigned long _inpd(unsigned int shor);
 unsigned char inports(unsigned short port, unsigned short* buffer, int count);

unsigned char inports(unsigned short port, unsigned short* buffer, int count)
{
	__asm
	{
		mov dx, port
		mov edi, buffer; Address of buffer
		mov ecx, count; Repeat count times
		rep insw
	}
}


#define DMA_PICU1       0x0020
#define DMA_PICU2       0x00A0

__declspec(naked) void SendEOI()
{
	_asm
	{
		PUSH EBP
		MOV  EBP, ESP
		PUSH EAX

		; [EBP] < -EBP
		; [EBP + 4] < -RET Addr
		; [EBP + 8] < -IRQ 번호

		MOV AL, 20H; EOI 신호를 보낸다.
		OUT DMA_PICU1, AL

		CMP BYTE PTR[EBP + 8], 7
		JBE END_OF_EOI
		OUT DMA_PICU2, AL; Send to 2 also

		END_OF_EOI :
		POP EAX
			POP EBP
			RET
	}
}

#ifdef __cplusplus
extern "C" {
#endif
	void OutPortByte(uint16_t port, uint8_t value)
	{
		_outp(port, value);
	}

	void OutPortWord(uint16_t port, uint16_t value)
	{
		_outpw(port, value);
	}

	void OutPortDWord(uint16_t port, unsigned int value)
	{
		_outpd(port, value);
	}

	long InPortDWord(unsigned int port)
	{
		return _inpd(port);
	}

	uint8_t InPortByte(uint16_t port)
	{

		return (uint8_t)_inp(port);
	}

	uint16_t InPortWord(uint16_t port)
	{
		return _inpw(port);
	}

	uint8_t InPorts(uint16_t port, uint16_t* buffer, int count)
	{
		return inports(port, buffer, count);
	}

	int DisableInterrupts()
	{
		int fl = 0;

		__asm	PUSHFD
		__asm	POP fl
		__asm	CLI

		return fl;
	}

	void RestoreInterrupts(const int flags)
	{
		__asm	PUSH	flags
		__asm	POPFD

	}

	void EnableInterrupts()
	{
		__asm sti
	}

	const int kMasterIcw1 = 0x20;
	const int kMasterIcw2 = 0x21;
	const int kSlaveIcw1 = 0xa0;
	const int kSlaveIcw2 = 0xa1;
	const int kUserCs = 0x1b;

	void EnableIrq(int irq)
	{
		if (irq < 8)
			OutPortByte(kMasterIcw2, InPortByte(kMasterIcw2) & (unsigned char)(~(1 << irq)));
		else
			OutPortByte(kSlaveIcw2, InPortByte(kSlaveIcw2) & (unsigned char)(~(1 << (irq - 8))));
	}

	void DisableIrq(int irq)
	{
		if (irq < 8)
			OutPortByte(kMasterIcw2, InPortByte(kMasterIcw2) | (1 << irq));
		else
			OutPortByte(kSlaveIcw2, InPortByte(kSlaveIcw2) | (1 << (irq - 8)));
	}
#ifdef __cplusplus
}
#endif


