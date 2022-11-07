#ifndef BELLONA2_SERIAL_PORT_HEADER_jj
#define BELLONA2_SERIAL_PORT_HEADER_jj

#define COM1_BASE	0x3F8
#define COM2_BASE	0x2F8
#define COM3_BASE	0x3E8
#define COM4_BASE	0x2E8
#define _THR		    0	// Transmitter Hold Register
#define _RBR		    0   // Receiver Buffer Register
#define _DLL		    0	// Divisor Latch Register LSB
#define _DLM		    1	// Divisor Latch Register MSB
#define _IER		    1	// Interrupt Enable Register
#define _IIR		    2	// Interrupt Identification Register
#define _LCR		    3	// Line Control Register
#define _MCR		    4	// Modem Control Register
#define _LSR		    5   // Line Status Register
#define _MSR		    6	// Modem Status Register

struct CharDevTag;

#define SERIAL_PORT_BUFF_SIZE	512
typedef struct SerialPortTag
{
	int		nIrq;
	DWORD	dwBase;
	int		nBaudRate;
	int		nTotalO, nTotalI;
	int		nO1, nO2, nI1, nI2;
	UCHAR	o_buff[ SERIAL_PORT_BUFF_SIZE ];
	UCHAR	i_buff[ SERIAL_PORT_BUFF_SIZE ];
	struct CharDevTag	*pDev;
};
typedef struct SerialPortTag SerialPortStt;

extern int serial_irq( int nIRQ );
extern int init_serial_port_driver ( int nMajor );

#endif
