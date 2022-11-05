#ifndef BELLONA2_DEBUG_REGISTER_HEADER
#define BELLONA2_DEBUG_REGISTER_HEADER

///////////////////////////////
#define DR_L0		0x00000001	   
#define DR_G0		0x00000002
#define DR_L1		0x00000004
#define DR_G1		0x00000008
#define DR_L2		0x00000010
#define DR_G2		0x00000020
#define DR_L3		0x00000040
#define DR_G3		0x00000080
#define DR_LE		0x00000100
#define DR_GE		0x00000200
#define	DR_SET		0x00000400
//					0x00000800
//					0x00001000
#define DR_GD		0x00002000
//					0x00004000
//					0x00008000
#define	DR_RW0		0x00010000
#define DR_LEN0		0x00040000
#define	DR_RW1		0x00100000
#define DR_LEN1		0x00400000
#define	DR_RW2		0x01000000
#define DR_LEN2		0x04000000
#define	DR_RW3		0x10000000
#define DR_LEN3		0x40000000
///////////////////////////////
#define DR_B0		0x00000001
#define DR_B1		0x00000002
#define DR_B2		0x00000004
#define DR_B3		0x00000008
#define DR_BD		0x00002000
#define DR_BS		0x00004000
#define DR_BT		0x00008000
///////////////////////////////
#define EFLAG_TS	8
///////////////////////////////
#define DR_RW0_INDEX 	16
#define DR_LEN0_INDEX	18
#define DR_L0_INDEX  	0
#define DR_G0_INDEX  	1
///////////////////////////////


extern void vInitDebugRegister();
extern void vDispDebugRegister();
extern int nSetBreakpoint( int nNo, DWORD dwAddr, int nLen, int nRW, int nLocal, int nGlobal );
extern int nResetBreakpoint( int nNo );
extern int set_iopl( int nIOPL );

#endif
