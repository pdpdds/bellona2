#pragma once
#include <stdint.h>

#ifdef __cplusplus
void *operator new(size_t size);
void *operator new[](size_t size);
void *operator new(size_t, void *p);
void *operator new[](size_t, void *p);

void operator delete(void *p);
void operator delete(void *p, size_t size);
void operator delete[](void *p);
void operator delete[](void *p, size_t size);
#endif

int __cdecl _purecall();

#ifdef  __cplusplus
extern "C" {
#endif
	void OutPortByte(uint16_t port, uint8_t value);
	void OutPortWord(uint16_t port, uint16_t value);
	void OutPortDWord(uint16_t port, unsigned int value);
	uint8_t InPortByte(uint16_t port);
	uint16_t InPortWord(uint16_t port);
	long InPortDWord(unsigned int port);
	uint8_t InPorts(uint16_t port, uint16_t* buffer, int count);

	int DisableInterrupts();
	void RestoreInterrupts(const int flags);
	void EnableInterrupts();

	void EnableIrq(int irq);
	void DisableIrq(int);
	
	extern unsigned char _BitScanReverse(unsigned long * _Index, unsigned long _Mask);
	extern unsigned char _BitScanForward(unsigned long * _Index, unsigned long _Mask);

#ifdef  __cplusplus
}


#endif

