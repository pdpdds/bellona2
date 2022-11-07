#ifndef LUCIFER_UNASSEMBLER_HEADER
#define LUCIFER_UNASSEMBLER_HEADER

#include <types.h>

struct OperandTag{
    UINT16 wType;         // Memory, Register
    UINT16 wSize;         // 8, 16, 32bit Object
    UINT16 wWidth;        // 8, 16, 32bit Address Line when Object is Memory
    UINT16 wRegBase;
    UINT16 wIndex;
    UINT16 wScale;
	
	// ���� �߰��� ��.
	UINT16 wValueSize;		// Value�� �ڸ���
	ULONG  dwValue;			// ��� ���ڰ� �� ���.
};
typedef struct OperandTag OperandStt;

struct OpTag{
    UINT16          wType;         // OpCode Type   ex) ot_ADD
    UINT16          wLength;       // Total Length
    UCHAR           byAddrPrx;     // Address Size Override Prefix
    UCHAR           byOprndPrx;    // Operand Size Override Prefix
    OperandStt      Oprnd[3];      // Operand 1,2,3
    UINT16          wDispSize;     // Displacement Size
    ULONG           dwDisp;        // Displacement
    UINT16          wImmSize;      // Immediate Data Size
    ULONG           dwImm;         // Immediate Data
    ULONG           dwIP;
	// ���� �߰��� ��.
	UINT16			wSegPrx;		// ���׸�Ʈ �����Ƚ�
};
typedef struct OpTag OpStt;

struct ModregRmTag{
    UINT16 wSize;
    UINT16 wMod;
    UINT16 wReg;
    UINT16 wRm ;
    UINT16 wScale;
    UINT16 wIndex;
    UINT16 wBase;
};
typedef struct ModregRmTag ModRegRmStt;

extern UINT16	wSetDefaultBit( UINT16 wDefaultBit );
extern char*	pDispOp( char *strArr[], UCHAR* pS, OpStt* pOp );
extern UINT16	wLucifer( UCHAR* pS, OpStt* pOp );
extern int		nDisAssembleOneCode( ULONG dwIP, OpStt *pOp, UCHAR *pBuff, char *strArr[] );
extern char		*pDosDispOp( CHAR* pBuff, UCHAR* pAddr, UCHAR* pS, OpStt* pOp );
#endif
