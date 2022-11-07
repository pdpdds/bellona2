#include "bellona2.h"

UINT16 RegTbl8 [] = { rAL,  rCL,  rDL,  rBL,  rAH,  rCH,  rDH,  rBH };
UINT16 RegTbl16[] = { rAX,  rCX,  rDX,  rBX,  rSP,  rBP,  rSI,  rDI };
UINT16 RegTbl32[] = { rEAX, rECX, rEDX, rEBX, rESP, rEBP, rESI ,rEDI};
UINT16 SegRegTbl[]= { rES,  rCS,  rSS,  rDS,  rFS,  rGS,  0,    0   };

UINT16 swGrp01Tbl[] = {
    ot_ADD,  ot_OR ,  ot_ADC,  ot_SBB,  ot_AND,  ot_SUB,  ot_XOR,  ot_CMP
};

UINT16 swGrp02Tbl[] = {
  ot_ROL,  ot_ROR,  ot_RCL,  ot_RCR,  ot_SHL,  ot_SHR,  ot_DB ,  ot_SAR
};

UINT16 swGrp03Tbl[] = {
  ot_TEST,  ot_DB  ,  ot_NOT ,  ot_NEG ,  ot_MUL ,  ot_IMUL,  ot_DIV ,  ot_IDIV
};

UINT16 swGrp04Tbl[] = {
  ot_INC,  ot_DEC  ,  ot_DB,   ot_DB,  ot_DB,  ot_DB,  ot_DB,  ot_DB,
};

UINT16 swGrp05Tbl[] = {
  ot_INC,  ot_DEC  ,  ot_CALL, ot_CALL, ot_JMP,  ot_JMP,  ot_PUSH , ot_DB
};

UINT16 swGrp06Tbl[] = {
  ot_SLDT,  ot_STR  ,  ot_LLDT, ot_LTR, ot_VERR,  ot_VERW,  ot_DB , ot_DB
};

UINT16 swGrp07Tbl[] = {
  ot_SGDT,  ot_SIDT  ,  ot_LGDT, ot_LIDT, ot_SMSW,  ot_DB,  ot_LMSW , ot_INVLPG
};

UINT16 swGrp08Tbl[] = {
  ot_DB,  ot_DB  ,  ot_DB, ot_DB, ot_BT ,  ot_BTS,  ot_BTR , ot_BTC
};

///////////////////////// FPU Instruction /////////////////////////////////
UINT16 swEscD8Tbl[] = {
    ot_FADD, ot_FMUL, ot_FCOM, ot_FCOMP, ot_FSUB, ot_FSUBR, ot_FDIV, ot_FDIVR
};

UINT16 swEscD9Tbl1[] = {
    ot_FLD, ot_DB, ot_FST, ot_FSTP, ot_FLDENV, ot_FLDCW, ot_FSTENV, ot_FSTCW
};

UINT16 swEscD9Tbl2[] = {
    ot_FCHS, ot_FABS,  ot_DB,    ot_DB,     ot_FTST,   ot_FXAM,  ot_DB,  ot_DB,
    ot_FLD1, ot_FLDL2T,ot_FLDL2E,ot_FLDPI,  ot_FLDLG2, ot_FLDLN2,ot_FLDZ,ot_DB,
    ot_F2XM1,ot_FYL2X, ot_FPTAN, ot_FPATAN, ot_FXTRACT,ot_FPREM1,ot_FDECSTP,ot_FINCSTP,
    ot_FPREM,ot_FYL2XP1,ot_FSQRT,ot_FSINCOS,ot_FRNDINT,ot_FSCALE,ot_FSIN,ot_FCOS
};

UINT16 swEscDaTbl[] = {
    ot_FIADD,ot_FIMUL,ot_FICOM,ot_FICOMP,ot_FISUB,ot_FISUBR,ot_FIDIV,ot_FIDIVR
};

UINT16 swEscDbTbl[] = {
    ot_FILD,ot_DB,ot_FIST,ot_FISTP,ot_DB,ot_FLD,ot_DB,ot_FSTP
};

UINT16 swEscDcTbl[] = {
    ot_FADD, ot_FMUL, ot_DB, ot_DB, ot_FSUBR, ot_FSUB, ot_FDIVR, ot_FDIV
};

UINT16 swEscDdTbl1[] = {
    ot_FLD, ot_DB, ot_FST, ot_FSTP, ot_FRSTOR, ot_DB, ot_FSAVE, ot_FSTSW
};
UINT16 swEscDdTbl2[] = {
    ot_FFREE, ot_DB, ot_FST, ot_FSTP, ot_FUCOM, ot_FUCOMP, ot_DB, ot_DB
};
UINT16 swEscDeTbl[] = {
    ot_FADDP, ot_FMULP, ot_DB, ot_DB, ot_FSUBRP, ot_FSUBP, ot_FDIVRP, ot_FDIVP
};

UINT16 swEscDfTbl[] = {
    ot_FILD, ot_DB, ot_FIST, ot_FISTP, ot_FBLD, ot_FILD, ot_FBSTP, ot_FISTP
};

///////////////////////////////////////////////////////////////////////////
/*  < ADDRESSING METHOD >
E = ModRegRm가 레지스터 또는 메모리로 쓰임.
G = ModRegRm의 Reg 필드.
! = 암시적 레지스터. (No ModRegRm)
I = 상수값
^ = Warp Up (Default Bit에 따라 명령의 뒤에 D가 붙는다. PUSHA -> PUSHAD )
S = Reg 필드가 세그먼트 레지스터로 사용된다.
D = Reg 필드가 디버그 레지스터로 사용된다.
C = Reg 필드가 컨트롤 레지스터로 사용된다.
A = Direct Address (No ModRegRm)
F = Flag Register
J = Relative Offset
M = ModRegRm 바이트가 항상 메모리로 사용된다.
O = Displacement only (No ModRegRm)	MOV(A0-A3) MOV AX,[100]
R = ModRegRm 바이트가 항상 레지스터로 사용된다.
T = Reg 필드가 테스트 레지스터로 사용된다.
X = Memory Addressed by DS:SI
Y = Memory Addressed by ES:DI

    < OPERAND TYPE >
a = 두개의 Word (BOUND에서만 사용)
b = byte
c = byte, word (depending on operand size)
d = double word
p = 4바이트 포인터 DWORD PTR (depending on operand size)
6 = 6바이트 포인터 FWORD PTR (depending on operand size)
q = qword
s = 6byte pseudo descriptor	(V와 처리 방법이 동일하다.)
v = word or double word (depending on operand size)
w = word
*/
//////////////////////////////////////////////////////////////////////////
OpDataStt OneByteTbl[] = {
	{ ot_ADD,  {"Eb", "Gb",""}},            // 00	
	{ ot_ADD,  {"Ev", "Gv",""}},            // 01	
    { ot_ADD,  {"Gb", "Eb",""}},            // 02
    { ot_ADD,  {"Gv", "Ev",""}},            // 03
    { ot_ADD,  {{'!', rAL, 0}, "Ib", ""}},  // 04   ! = Direct Register
    { ot_ADD,  {{'!', eAX, 0}, "Iv", ""}},  // 05       ( No ModRegRm byte )
    { ot_PUSH, {{'!', rES, 0}, "",   ""}},  // 06
    { ot_POP,  {{'!', rES, 0}, "",   ""}},  // 07
    { ot_OR,   {"Eb", "Gb",""}},            // 08
	{ ot_OR,   {"Ev", "Gv",""}},            // 09
    { ot_OR,   {"Gb", "Eb",""}},            // 0A
    { ot_OR,   {"Gv", "Ev",""}},            // 0B
    { ot_OR,   {{'!', rAL, 0}, "Ib", ""}},  // 0C
    { ot_OR,   {{'!', eAX, 0}, "Iv", ""}},  // 0D
    { ot_PUSH, {{'!', rCS, 0}, "",   ""}},  // 0E
    { ot_DB,   {"", "", ""}             },  // 0F  Two byte OpCode Table

    { ot_ADC,  {"Eb", "Gb",""}},            // 10
    { ot_ADC,  {"Ev", "Gv",""}},            // 11
    { ot_ADC,  {"Gb", "Eb",""}},            // 12
    { ot_ADC,  {"Gv", "Ev",""}},            // 13
    { ot_ADC,  {{'!', rAL, 0}, "Ib", ""}},  // 14
    { ot_ADC,  {{'!', eAX, 0}, "Iv", ""}},  // 15
    { ot_PUSH, {{'!', rSS, 0}, "",   ""}},  // 16
    { ot_POP,  {{'!', rSS, 0}, "",   ""}},  // 17
    { ot_SBB,  {"Eb", "Gb",""}},            // 18
    { ot_SBB,  {"Ev", "Gv",""}},            // 19
    { ot_SBB,  {"Gb", "Eb",""}},            // 1A
    { ot_SBB,  {"Gv", "Ev",""}},            // 1B
    { ot_SBB,  {{'!', rAL, 0}, "Ib", ""}},  // 1C
    { ot_SBB,  {{'!', eAX, 0}, "Iv", ""}},  // 1D
    { ot_PUSH, {{'!', rDS, 0}, "",   ""}},  // 1E
    { ot_POP,  {{'!', rDS, 0}, "",   ""}},  // 1F

    { ot_AND,  {"Eb", "Gb",""}},            // 20
    { ot_AND,  {"Ev", "Gv",""}},            // 21
    { ot_AND,  {"Gb", "Eb",""}},            // 22
    { ot_AND,  {"Gv", "Ev",""}},            // 23
    { ot_AND,  {{'!', rAL, 0}, "Ib", ""}},  // 24
    { ot_AND,  {{'!', eAX, 0}, "Iv", ""}},  // 25
    { ot_SEG_PRX, {{'!', rES, 0}, "",   ""}},  // 26
    { ot_DAA,  {"", "",   ""}},             // 27
    { ot_SUB,  {"Eb", "Gb",""}},            // 28
    { ot_SUB,  {"Ev", "Gv",""}},            // 29
    { ot_SUB,  {"Gb", "Eb",""}},            // 2A
    { ot_SUB,  {"Gv", "Ev",""}},            // 2B
    { ot_SUB,  {{'!', rAL, 0}, "Ib", ""}},  // 2C
    { ot_SUB,  {{'!', eAX, 0}, "Iv", ""}},  // 2D
    { ot_SEG_PRX, {{'!', rCS, 0}, "",   ""}},  // 2E
    { ot_DAS,  {"", "",   ""}},             // 2F

    { ot_XOR,  {"Eb", "Gb",""}},            // 30
    { ot_XOR,  {"Ev", "Gv",""}},            // 31
    { ot_XOR,  {"Gb", "Eb",""}},            // 32
    { ot_XOR,  {"Gv", "Ev",""}},            // 33
    { ot_XOR,  {{'!', rAL, 0}, "Ib", ""}},  // 34
    { ot_XOR,  {{'!', eAX, 0}, "Iv", ""}},  // 35
    { ot_SEG_PRX, {{'!', rSS, 0}, "",   ""}},  // 36
    { ot_AAA,  {"", "",   ""}},             // 37
    { ot_CMP,  {"Eb", "Gb",""}},            // 38
    { ot_CMP,  {"Ev", "Gv",""}},            // 39
    { ot_CMP,  {"Gb", "Eb",""}},            // 3A
    { ot_CMP,  {"Gv", "Ev",""}},            // 3B
    { ot_CMP,  {{'!', rAL, 0}, "Ib", ""}},  // 3C
    { ot_CMP,  {{'!', eAX, 0}, "Iv", ""}},  // 3D
    { ot_SEG_PRX, {{'!', rDS, 0}, "",   ""}},  // 3E
    { ot_AAS,  {"", "",   ""}},             // 3F

    { ot_INC,  {{'!', eAX ,0}, "",   ""}},  // 40
    { ot_INC,  {{'!', eCX ,0}, "",   ""}},  // 41
    { ot_INC,  {{'!', eDX ,0}, "",   ""}},  // 42
    { ot_INC,  {{'!', eBX ,0}, "",   ""}},  // 43
    { ot_INC,  {{'!', eSP ,0}, "",   ""}},  // 44
    { ot_INC,  {{'!', eBP ,0}, "",   ""}},  // 45
    { ot_INC,  {{'!', eSI ,0}, "",   ""}},  // 46
    { ot_INC,  {{'!', eDI ,0}, "",   ""}},  // 47
    { ot_DEC,  {{'!', eAX ,0}, "",   ""}},  // 48
    { ot_DEC,  {{'!', eCX ,0}, "",   ""}},  // 49
    { ot_DEC,  {{'!', eDX ,0}, "",   ""}},  // 4A
    { ot_DEC,  {{'!', eBX ,0}, "",   ""}},  // 4B
    { ot_DEC,  {{'!', eSP ,0}, "",   ""}},  // 4C
    { ot_DEC,  {{'!', eBP ,0}, "",   ""}},  // 4D
    { ot_DEC,  {{'!', eSI ,0}, "",   ""}},  // 4E
    { ot_DEC,  {{'!', eDI ,0}, "",   ""}},  // 4F

    { ot_PUSH, {{'!', eAX ,0}, "",   ""}},  // 50
    { ot_PUSH, {{'!', eCX ,0}, "",   ""}},  // 51
    { ot_PUSH, {{'!', eDX ,0}, "",   ""}},  // 52
    { ot_PUSH, {{'!', eBX ,0}, "",   ""}},  // 53
    { ot_PUSH, {{'!', eSP ,0}, "",   ""}},  // 54
    { ot_PUSH, {{'!', eBP ,0}, "",   ""}},  // 55
    { ot_PUSH, {{'!', eSI ,0}, "",   ""}},  // 56
    { ot_PUSH, {{'!', eDI ,0}, "",   ""}},  // 57
    { ot_POP,  {{'!', eAX ,0}, "",   ""}},  // 58
    { ot_POP,  {{'!', eCX ,0}, "",   ""}},  // 59
    { ot_POP,  {{'!', eDX ,0}, "",   ""}},  // 5A
    { ot_POP,  {{'!', eBX ,0}, "",   ""}},  // 5B
    { ot_POP,  {{'!', eSP ,0}, "",   ""}},  // 5C
    { ot_POP,  {{'!', eBP ,0}, "",   ""}},  // 5D
    { ot_POP,  {{'!', eSI ,0}, "",   ""}},  // 5E
    { ot_POP,  {{'!', eDI ,0}, "",   ""}},  // 5F

    { ot_PUSHA,{"^",  "",   ""}         },  // 60  ^ : Warp Up ex)PUSHA->PUSHAD
    { ot_POPA, {"^",  "",   ""}         },  // 61
    { ot_BOUND,{"Gv", "Ma", ""}         },  // 62
    { ot_ARPL, {"Ew", "Gw", ""}         },  // 63
    { ot_SEG_PRX,  {{'!', rFS, 0}, "",  ""}},  // 64
    { ot_SEG_PRX,  {{'!', rGS, 0}, "",  ""}},  // 65
    { ot_OPND_PRX, {"", "",  ""}},          // 66
    { ot_ADDR_PRX, {"", "",  ""}},          // 67
    { ot_PUSH,     {"Iv", "", ""},      },  // 68
    { ot_IMUL,     {"Gv", "Ev", "Iv"}   },  // 69
    { ot_PUSH,     {"Ib", "", ""},      },  // 6A
    { ot_IMUL,     {"Gv", "Ev", "Ib"},  },  // 6B
    { ot_INSB,     {"",   "",   ""},    },  // 6C
    { ot_INSW,     {"^",  "",   ""}     },  // 6D
    { ot_OUTSB,    {"",   "",   ""},    },  // 6E
    { ot_OUTSW,    {"^",  "",   ""}     },  // 6F

    { ot_JO,   {"Jb", "",   ""}},  // 70
    { ot_JNO,  {"Jb", "",   ""}},  // 71
    { ot_JB,   {"Jb", "",   ""}},  // 72
    { ot_JNB,  {"Jb", "",   ""}},  // 73
    { ot_JZ,   {"Jb", "",   ""}},  // 74
    { ot_JNZ,  {"Jb", "",   ""}},  // 75
    { ot_JBE,  {"Jb", "",   ""}},  // 76
    { ot_JNBE, {"Jb", "",   ""}},  // 77
    { ot_JS,   {"Jb", "",   ""}},  // 78
    { ot_JNS,  {"Jb", "",   ""}},  // 79
    { ot_JP,   {"Jb", "",   ""}},  // 7A
    { ot_JNP,  {"Jb", "",   ""}},  // 7B
    { ot_JL,   {"Jb", "",   ""}},  // 7C
    { ot_JNL,  {"Jb", "",   ""}},  // 7D
    { ot_JLE,  {"Jb", "",   ""}},  // 7E
    { ot_JNLE, {"Jb", "",   ""}},  // 7F

    { ot_GRP1, {"Eb", "Ib", ""}},  // 80
    { ot_GRP1, {"Ev", "Iv", ""}},  // 81
    { ot_GRP1, {"Eb", "Ib", ""}},  // 82	  80, 82는 완전히 동일하다.(왜그럴까???)
    { ot_GRP1, {"Ev", "Ib", ""}},  // 83

    { ot_TEST, {"Eb", "Gb", ""}          }, // 84
    { ot_TEST, {"Ev", "Gv", ""}          }, // 85
    { ot_XCHG, {"Eb", "Gb", ""}          }, // 86
    { ot_XCHG, {"Ev", "Gv", ""}          }, // 87
    { ot_MOV,  {"Eb", "Gb", ""}          }, // 88
    { ot_MOV,  {"Ev", "Gv", ""}          }, // 89
    { ot_MOV,  {"Gb", "Eb", ""}          }, // 8A
    { ot_MOV,  {"Gv", "Ev", ""}          }, // 8B
    { ot_MOV,  {"Ew", "Sw", ""}          }, // 8C
    { ot_LEA,  {"Gv", "Mv", ""}          }, // 8D
    { ot_MOV,  {"Sw", "Ew", ""}          }, // 8E
    { ot_POP,  {"Ev", "",   ""}          }, // 8F

    { ot_NOP,  {"",   "",   ""}          }, // 90
    { ot_XCHG, {{'!', eAX, 0}, {'!', eCX, 0},   ""}  }, // 91
    { ot_XCHG, {{'!', eAX, 0}, {'!', eDX, 0},   ""}  }, // 92
    { ot_XCHG, {{'!', eAX, 0}, {'!', eBX, 0},   ""}  }, // 93
    { ot_XCHG, {{'!', eAX, 0}, {'!', eSP, 0},   ""}  }, // 94
    { ot_XCHG, {{'!', eAX, 0}, {'!', eBP, 0},   ""}  }, // 95
    { ot_XCHG, {{'!', eAX, 0}, {'!', eSI, 0},   ""}  }, // 96
    { ot_XCHG, {{'!', eAX, 0}, {'!', eDI, 0},   ""}  }, // 97
    { ot_CBW,  {"^",  "",   ""}          }, // 98
    { ot_CWD,  {"^",  "",   ""}          }, // 99
    { ot_CALL, {"Ap", "",   ""}          }, // 9A
    { ot_WAIT, {"",   "",   ""}          }, // 9B
    { ot_PUSHF,{"^",  "",   ""}          }, // 9C
    { ot_POPF, {"^",  "",   ""}          }, // 9D
    { ot_SAHF, {"",   "",   ""}          }, // 9E
    { ot_LAHF, {"",   "",   ""}          }, // 9F

    { ot_MOV,  {{'!', rAL, 0}, "Ob", "" } }, // A0
    { ot_MOV,  {{'!', eAX, 0}, "Ov", "" } }, // A1
    { ot_MOV,  {"Ob", {'!', rAL, 0}, "" } }, // A2
    { ot_MOV,  {"Ov", {'!', eAX, 0}, "" } }, // A3
    { ot_MOVSB,{"",   "",   ""}           }, // A4
    { ot_MOVSW,{"^",  "",   ""}           }, // A5
    { ot_CMPSB,{"",   "",   ""}           }, // A6
    { ot_CMPSW,{"^",  "",   ""}           }, // A7
    { ot_TEST, {{'!', rAL, 0}, "Ib",  ""} }, // A8
    { ot_TEST, {{'!', eAX, 0}, "Iv",  ""} }, // A9
    { ot_STOSB,{"",   "",   ""}           }, // AA
    { ot_STOSW,{"^",  "",   ""}           }, // AB
    { ot_LODSB,{"",   "",   ""}           }, // AC
    { ot_LODSW,{"^",  "",   ""}           }, // AD
    { ot_SCASB,{"",   "",   ""}           }, // AE
    { ot_SCASW,{"^",  "",   ""}           }, // AF

    { ot_MOV,  {{'!', rAL, 0}, "Ib", ""}  }, // B0
    { ot_MOV,  {{'!', rCL, 0}, "Ib", ""}  }, // B1
    { ot_MOV,  {{'!', rDL, 0}, "Ib", ""}  }, // B2
    { ot_MOV,  {{'!', rBL, 0}, "Ib", ""}  }, // B3
    { ot_MOV,  {{'!', rAH, 0}, "Ib", ""}  }, // B4
    { ot_MOV,  {{'!', rCH, 0}, "Ib", ""}  }, // B5
    { ot_MOV,  {{'!', rDH, 0}, "Ib", ""}  }, // B6
    { ot_MOV,  {{'!', rBH, 0}, "Ib", ""}  }, // B7
    { ot_MOV,  {{'!', eAX, 0}, "Iv", ""}  }, // B8
    { ot_MOV,  {{'!', eCX, 0}, "Iv", ""}  }, // B9
    { ot_MOV,  {{'!', eDX, 0}, "Iv", ""}  }, // BA
    { ot_MOV,  {{'!', eBX, 0}, "Iv", ""}  }, // BB
    { ot_MOV,  {{'!', eSP, 0}, "Iv", ""}  }, // BC
    { ot_MOV,  {{'!', eBP, 0}, "Iv", ""}  }, // BD
    { ot_MOV,  {{'!', eSI, 0}, "Iv", ""}  }, // BE
    { ot_MOV,  {{'!', eDI, 0}, "Iv", ""}  }, // BF

    { ot_GRP2, {"Eb", "Ib", "" }          }, // C0
    { ot_GRP2, {"Ev", "Ib", "" }          }, // C1
    { ot_RETN, {"Iw", "",   "" }          }, // C2
    { ot_RETN, {"",   "",   "" }          }, // C3
    { ot_LES,  {"Gv", "Mv", "" }          }, // C4
    { ot_LDS,  {"Gv", "Mv", "" }          }, // C5
    { ot_MOV,  {"Eb", "Ib", "" }          }, // C6
    { ot_MOV,  {"Ev", "Iv", "" }          }, // C7
    { ot_ENTER,{"Iw", "ib", "" }          }, // C8  'i' -> to Disp
    { ot_LEAVE,{"",   "",   "" }          }, // C9
    { ot_RETF, {"IW", "",   "" }          }, // CA
    { ot_RETF, {"",   "",   "" }          }, // CB
    { ot_INT3, {"",   "",   "" }          }, // CC
    { ot_INT , {"Ib", "",   "" }          }, // CD
    { ot_INTO, {"", "",   "" }            }, // CE
    { ot_IRET, {"^","",   "" }            }, // CF
    
    { ot_GRP2, {"Eb", {'%', 1, 0},   "" } }, // D0  '%' -> Implicit Constant
    { ot_GRP2, {"Ev", {'%', 1, 0},   "" } }, // D1
    { ot_GRP2, {"Eb", {'!', rCL, 0}, "" } }, // D2
    { ot_GRP2, {"Ev", {'!', rCL, 0}, "" } }, // D3
    { ot_AAM,  {"", "", "" } },              // D4
    { ot_AAD,  {"", "", "" } },              // D5
    { ot_DB,   {"", "", "" } },              // D6
    { ot_XLAT, {"", "", "" } },              // D7
    { ot_ESC8, {"", "", "" } },              // D8
    { ot_ESC9, {"", "", "" } },              // D9
    { ot_ESCa, {"", "", "" } },              // DA
    { ot_ESCb, {"", "", "" } },              // DB
    { ot_ESCc, {"", "", "" } },              // DC
    { ot_ESCd, {"", "", "" } },              // DD
    { ot_ESCe, {"", "", "" } },              // DE
    { ot_ESCf, {"", "", "" } },              // DF

    { ot_LOOPNE,{"Jb", "", "" } },           // E0
    { ot_LOOPE, {"Jb", "", "" } },           // E1
    { ot_LOOP,  {"Jb", "", "" } },           // E2
    { ot_JCXZ,  {"Jb", "", "" } },           // E3
    { ot_IN,    {{'!', rAL, 0}, "Ib", "" } },// E4
    { ot_IN,    {{'!', eAX, 0}, "Ib", "" } },// E5
    { ot_OUT,   {"Ib", {'!', rAL, 0}, "" } },// E6
    { ot_OUT,   {"Ib", {'!', eAX, 0}, "" } },// E7
    { ot_CALL,  {"Jv", "", "" } },           // E8
    { ot_JMP,   {"Jv", "", "" } },           // E9
    { ot_JMP,   {"Ap", "", "" } },           // EA
    { ot_JMP,   {"Jb", "", "" } },           // EB
    { ot_IN,    {{'!', rAL, 0}, {'!', rDX, 0}, "" } }, // EC
    { ot_IN,    {{'!', eAX, 0}, {'!', rDX, 0}, "" } }, // ED
    { ot_OUT,   {{'!', rDX, 0}, {'!', rAL, 0}, "" } }, // EE
    { ot_OUT,   {{'!', rDX, 0}, {'!', eAX, 0}, "" } }, // EF

    { ot_LOCK,  {"", "", "" } },             // F0
    { ot_DB,    {"", "", "" } },             // F1
    { ot_REPNE, {"", "", "" } },             // F2
    { ot_REP,   {"", "", "" } },             // F3
    { ot_HLT,   {"", "", "" } },             // F4
    { ot_CMC,   {"", "", "" } },             // F5
    { ot_GRP3,  {"Eb", "", "" } },           // F6
    { ot_GRP3,  {"Ev", "", "" } },           // F7
    { ot_CLC,   {"", "", "" } },             // F8
    { ot_STC,   {"", "", "" } },             // F9
    { ot_CLI,   {"", "", "" } },             // FA
    { ot_STI,   {"", "", "" } },             // FB
    { ot_CLD,   {"", "", "" } },             // FC
    { ot_STD,   {"", "", "" } },             // FD
    { ot_GRP4,  {"Eb", "", "" } },           // FE
    { ot_GRP5,  {"E?", "", "" } },           // FF  ? = w, p

    { ot_DB }
};

OpData2Stt TwoByteTbl[] = {
    { 0x00, ot_GRP6,    {"Ew", "", ""}          },
    { 0x01, ot_GRP7,    {"", "", ""}            },
    { 0x02, ot_LAR,     {"Gv", "Ew", ""}        },
    { 0x03, ot_LSL,     {"Gv", "Ew", ""}        },
    { 0x06, ot_CLTS,    {"", "", ""}            },
    { 0x08, ot_INVD,    {"", "", ""}            },
    { 0x09, ot_WINVD,   {"", "", ""}            },

    { 0x10, ot_MOV,   {"Eb", "Gb", ""}          },
    { 0x11, ot_MOV,   {"Gv", "Ev", ""}          },
    { 0x12, ot_MOV,   {"Gb", "Eb", ""}          },
    { 0x13, ot_MOV,   {"Ev", "Gv", ""}          },

    { 0x20, ot_MOV,   {"Rd", "Cd", ""}          },
    { 0x21, ot_MOV,   {"Rd", "Dd", ""}          },
    { 0x22, ot_MOV,   {"Cd", "Rd", ""}          },
    { 0x23, ot_MOV,   {"Dd", "Rd", ""}          },
    { 0x24, ot_MOV,   {"Rd", "Td", ""}          },
    { 0x26, ot_MOV,   {"Td", "Rd", ""}          },

    { 0x80, ot_JO,    {"Jv", "", ""}            },
    { 0x81, ot_JNO,   {"Jv", "", ""}            },
    { 0x82, ot_JB,    {"Jv", "", ""}            },
    { 0x83, ot_JNB,   {"Jv", "", ""}            },
    { 0x84, ot_JZ,    {"Jv", "", ""}            },
    { 0x85, ot_JNZ,   {"Jv", "", ""}            },
    { 0x86, ot_JBE,   {"Jv", "", ""}            },
    { 0x87, ot_JNBE,  {"Jv", "", ""}            },
    { 0x88, ot_JS,    {"Jv", "", ""}            },
    { 0x89, ot_JNS,   {"Jv", "", ""}            },
    { 0x8A, ot_JP,    {"Jv", "", ""}            },
    { 0x8B, ot_JNP,   {"Jv", "", ""}            },
    { 0x8C, ot_JL,    {"Jv", "", ""}            },
    { 0x8D, ot_JNL,   {"Jv", "", ""}            },
    { 0x8E, ot_JLE,   {"Jv", "", ""}            },
    { 0x8F, ot_JNLE,  {"Jv", "", ""}            },

    { 0x90, ot_SETO,   {"Eb", "", ""}           },
    { 0x91, ot_SETNO,  {"Eb", "", ""}           },
    { 0x92, ot_SETB,   {"Eb", "", ""}           },
    { 0x93, ot_SETNB,  {"Eb", "", ""}           },
    { 0x94, ot_SETZ,   {"Eb", "", ""}           },
    { 0x95, ot_SETNZ,  {"Eb", "", ""}           },
    { 0x96, ot_SETBE,  {"Eb", "", ""}           },
    { 0x97, ot_SETNBE, {"Eb", "", ""}           },
    { 0x98, ot_SETS,   {"Eb", "", ""}           },
    { 0x99, ot_SETNS,  {"Eb", "", ""}           },
    { 0x9A, ot_SETP,   {"Eb", "", ""}           },
    { 0x9B, ot_SETNP,  {"Eb", "", ""}           },
    { 0x9C, ot_SETL,   {"Eb", "", ""}           },
    { 0x9D, ot_SETNL,  {"Eb", "", ""}           },
    { 0x9E, ot_SETLE,  {"Eb", "", ""}           },
    { 0x9F, ot_SETNLE, {"Eb", "", ""}           },

    { 0xA0, ot_PUSH,   {{'!', rFS, 0}, "", ""}  },
    { 0xA1, ot_POP,    {{'!', rFS, 0}, "", ""}  },
    { 0xA2, ot_CPUID,  {"", "", ""} },
    { 0xA3, ot_BT,     {"Ev", "Gv", ""} },
    { 0xA4, ot_SHLD,   {"Ev", "Gv", "Ib"} },
    { 0xA5, ot_SHLD,   {"Ev", "Gv", {'!', rCL, 0} } },
    { 0xA6, ot_CMPXCHG,{"Eb", "Gb", "" }        },
    { 0xA7, ot_CMPXCHG,{"Ev", "Gv", "" }        },
    { 0xA8, ot_PUSH,   {{'!', rGS, 0}, "", ""}  },
    { 0xA9, ot_POP,    {{'!', rGS, 0}, "", ""}  },
    { 0xAA, ot_RSM,    {{'!', rGS, 0}, "", ""}  },
    { 0xAB, ot_BTS,    {"Ev", "Gv", ""}  },
    { 0xAC, ot_SHRD,   {"Ev", "Gv", "Ib"}  },
    { 0xAD, ot_SHRD,   {"Ev", "Gv", {'!', rCL, 0}}  },
    { 0xAF, ot_IMUL,   {"Gv", "Ev", ""  } },

    { 0xB0, ot_CMPXCHG,{"Eb", "Gb", ""  } },
    { 0xB1, ot_CMPXCHG,{"Ev", "Gv", ""  } },
    { 0xB2, ot_LSS,    {"Mp", "",   ""  } },
    { 0xB3, ot_BTR,    {"Ev", "Gv", ""  } },
    { 0xB4, ot_LFS,    {"Mp", "", ""  }   },
    { 0xB5, ot_LGS,    {"Mp", "", ""  }   },
    { 0xB6, ot_MOVZX,  {"Gv", "Eb", ""  } },
    { 0xB7, ot_MOVZX,  {"Gv", "Ew", ""  } },
    { 0xBA, ot_GRP8,   {"Ev", "Ib", ""  } },
    { 0xBB, ot_BTC,    {"Ev", "Gv", ""  } },
    { 0xBC, ot_BSF,    {"Gv", "Ev", ""  } },
    { 0xBD, ot_BSR,    {"Gv", "Ev", ""  } },
    { 0xBE, ot_MOVSX,  {"Gv", "Eb", ""  } },
    { 0xBF, ot_MOVSX,  {"Gv", "Ew", ""  } },

    { 0xC0, ot_XADD,   {"Eb", "Gb", ""  } },
    { 0xC1, ot_XADD,   {"Ev", "Gv", ""  } },
    { 0xC7, ot_DB,     {"", "", ""  }     }, // Group 9
    { 0xC8, ot_BSWAP,  {{'!', rEAX, 0}, "", ""  } },
    { 0xC9, ot_BSWAP,  {{'!', rECX, 0}, "", ""  } },
    { 0xCA, ot_BSWAP,  {{'!', rEDX, 0}, "", ""  } },
    { 0xCB, ot_BSWAP,  {{'!', rEBX, 0}, "", ""  } },
    { 0xCC, ot_BSWAP,  {{'!', rESP, 0}, "", ""  } },
    { 0xCD, ot_BSWAP,  {{'!', rEBP, 0}, "", ""  } },
    { 0xCE, ot_BSWAP,  {{'!', rESI, 0}, "", ""  } },
    { 0xCF, ot_BSWAP,  {{'!', rEDI, 0}, "", ""  } },

    { 0xFF } // End Of Table
};

UINT16 wSearchTwoByteTbl( OpDataStt* pOData, UINT16 wNo )
{
    UINT16 wI;

    memset( pOData, 0, sizeof( OpDataStt ) );

    pOData->wType = ot_DB;
    for( wI = 0; TwoByteTbl[wI].wNo != 0xFF; wI++ )
    {
        if( TwoByteTbl[wI].wNo == wNo )
        {
            pOData->wType = TwoByteTbl[wI].wType;
            memcpy( pOData->szOperand, TwoByteTbl[wI].szOperand, sizeof( pOData->szOperand ) );
            break;
        }
    }
    return(0);
}

CHAR* spMnemonic[] = {
    "DB",

    "AAA",
    "AAD",
    "AAM",
    "AAS",
    "ADC",
    "ADD",
    "ADDR_PRX",
    "AND",
    "ARPL",
    "BOUND",
    "BSF",
    "BSR",
    "BSWAP",
    "BT",
    "BTC",
    "BTR",
    "BTS",
    "CALL",
    "CBW",
    "CDQ",
    "CLC",
    "CLD",
    "CLI",
    "CLTS",
    "CMC",
    "CMP",
    "CMPSB",
    "CMPSD",
    "CMPSW",
    "CMPXCHG",
    "CPUID",
    "CWD",
    "CWDE",
    "DAA",
    "DAS",
    "DEC",
    "DIV",
    "ENTER",
    "ESC8",
    "ESC9",
    "ESCa",
    "ESCb",
    "ESCc",
    "ESCd",
    "ESCe",
    "ESCf",
    "F2XM1",
    "FABS",
    "FADD",
    "FADDP",
    "FBLD",
    "FBSTP",
    "FCHS",
    "FCOM",
    "FCOMP",
    "FCOS",
    "FDECSTP",
    "FDIV",
    "FDIVP",
    "FDIVR",
    "FDIVRP",
    "FFREE",
    "FIADD",
    "FICOM",
    "FICOMP",
    "FIDIV",
    "FIDIVR",
    "FILD",
    "FIMUL",
    "FINCSTP",
    "FIST",
    "FISTP",
    "FISUB",
    "FISUBR",
    "FLD",
    "FLD1",
    "FLDCW",
    "FLDENV",
    "FLDL2E",
    "FLDL2T",
    "FLDLG2",
    "FLDLN2",
    "FLDPI",
    "FLDZ",
    "FMUL",
    "FMULP",
    "FNOP",
    "FPATAN",
    "FPREM",
    "FPREM1",
    "FPTAN",
    "FRNDINT",
    "FRSTOR",
    "FSAVE",
    "FSCALE",
    "FSIN",
    "FSINCOS",
    "FSQRT",
    "FST",
    "FSTCW",
    "FSTENV",
    "FSTP",
    "FSTSW",
    "FSUB",
    "FSUBP",
    "FSUBR",
    "FSUBRP",
    "FTST",
    "FUCOM",
    "FUCOMP",
    "FUCOMPP",
    "FXAM",
    "FXCH",
    "FXTRACT",
    "FYL2X",
    "FYL2XP1",
    "GRP1",
    "GRP2",
    "GRP3",
    "GRP4",
    "GRP5",
    "GRP6",
    "GRP7",
    "GRP8",
    "GRP9",
    "HLT",
    "IDIV",
    "IMUL",
    "IN",
    "INC",
    "INSB",
    "INSD",
    "INSW",
    "INT",
    "INT3",
    "INTO",
    "INVD",
    "INVLPG",
    "IRET",
    "IRETD",
    "JB",
    "JBE",
    "JCXZ",
    "JL",
    "JLE",
    "JMP",
    "JNB",
    "JNBE",
    "JNL",
    "JNLE",
    "JNO",
    "JNP",
    "JNS",
    "JNZ",
    "JO",
    "JP",
    "JS",
    "JZ",
    "LAHF",
    "LAR",
    "LDS",
    "LEA",
    "LEAVE",
    "LES",
    "LFS",
    "LGDT",
    "LGS",
    "LIDT",
    "LLDT",
    "LMSW ",
    "LOCK",
    "LODSB",
    "LODSD",
    "LODSW",
    "LOOP",
    "LOOPE",
    "LOOPNE",
    "LSL",
    "LSS",
    "LTR",
    "MOV",
    "MOVSB",
    "MOVSD",
    "MOVSW",
    "MOVSX",
    "MOVZX",
    "MUL",
    "NEG",
    "NOP",
    "NOT",
    "OPND_PRX",
    "OR",
    "OUT",
    "OUTSB",
    "OUTSD",
    "OUTSW",
    "POP",
    "POPA",
    "POPAD",
    "POPF",
    "POPFD",
    "PUSH",
    "PUSHA",
    "PUSHAD",
    "PUSHF",
    "PUSHFD",
    "RCL",
    "RCR",
    "REP",
    "REPNE",
    "RETF",
    "RETN",
    "ROL",
    "ROR",
    "RSM",
    "SAHF",
    "SAR",
    "SBB",
    "SCASB",
    "SCASD",
    "SCASW",
    "SEG_PRX",
    "SETB",
    "SETBE",
    "SETL",
    "SETLE",
    "SETNB",
    "SETNBE",
    "SETNL",
    "SETNLE",
    "SETNO",
    "SETNP",
    "SETNS",
    "SETNZ",
    "SETO",
    "SETP",
    "SETS",
    "SETZ",
    "SGDT",
    "SHL",
    "SHLD",
    "SHR",
    "SHRD",
    "SIDT  ",
    "SLDT",
    "SMSW ",
    "STC",
    "STD",
    "STI",
    "STOSB",
    "STOSD",
    "STOSW",
    "STR",
    "SUB",
    "TEST",
    "VERR ",
    "VERW",
    "WAIT",
    "WINVD",
    "XADD",
    "XCHG",
    "XLAT",
    "XOR",

	"!@#$%"		// 끝이다.
};

CHAR* pRegStr( UINT16 wR )
{
    CHAR* pStr = "";

    switch( wR )
    {
        case rAH  :           pStr = "AH";             break;
        case rAL  :           pStr = "AL";             break;
        case rBH  :           pStr = "BH";             break;
        case rBL  :           pStr = "BL";             break;
        case rCH  :           pStr = "CH";             break;
        case rCL  :           pStr = "CL";             break;
        case rDH  :           pStr = "DH";             break;
        case rDL  :           pStr = "DL";             break;
        case rAX  :           pStr = "AX";             break;
        case rBX  :           pStr = "BX";             break;
        case rCX  :           pStr = "CX";             break;
        case rDX  :           pStr = "DX";             break;
        case rSI  :           pStr = "SI";             break;
        case rDI  :           pStr = "DI";             break;
        case rBP  :           pStr = "BP";             break;
        case rSP  :           pStr = "SP";             break;
        case rEAX :           pStr = "EAX";            break;
        case rEBX :           pStr = "EBX";            break;
        case rECX :           pStr = "ECX";            break;
        case rEDX :           pStr = "EDX";            break;
        case rESI :           pStr = "ESI";            break;
        case rEDI :           pStr = "EDI";            break;
        case rEBP :           pStr = "EBP";            break;
        case rESP :           pStr = "ESP";            break;
        case rCS  :           pStr = "CS";             break;
        case rDS  :           pStr = "DS";             break;
        case rES  :           pStr = "ES";             break;
        case rFS  :           pStr = "FS";             break;
        case rGS  :           pStr = "GS";             break;
        case rSS  :           pStr = "SS";             break;
        case rC0  :           pStr = "CR0";            break;
        case rC1  :           pStr = "CR1";            break;
        case rC2  :           pStr = "CR2";            break;
        case rC3  :           pStr = "CR3";            break;
        case rC4  :           pStr = "CR4";            break;
        case rC5  :           pStr = "CR5";            break;
        case rC6  :           pStr = "CR6";            break;
        case rC7  :           pStr = "CR7";            break;
        case rD0  :           pStr = "DR0";            break;
        case rD1  :           pStr = "DR1";            break;
        case rD2  :           pStr = "DR2";            break;
        case rD3  :           pStr = "DR3";            break;
        case rD4  :           pStr = "DR4";            break;
        case rD5  :           pStr = "DR5";            break;
        case rD6  :           pStr = "DR6";            break;
        case rD7  :           pStr = "DR7";            break;
        case rT0  :           pStr = "TR0";            break;
        case rT1  :           pStr = "TR1";            break;
        case rT2  :           pStr = "TR2";            break;
        case rT3  :           pStr = "TR3";            break;
        case rT4  :           pStr = "TR4";            break;
        case rT5  :           pStr = "TR5";            break;
        case rT6  :           pStr = "TR6";            break;
        case rT7  :           pStr = "TR7";            break;
        case rST  :           pStr = "ST";             break;
        case rST0 :           pStr = "ST(0)";          break;
        case rST1 :           pStr = "ST(1)";          break;
        case rST2 :           pStr = "ST(2)";          break;
        case rST3 :           pStr = "ST(3)";          break;
        case rST4 :           pStr = "ST(4)";          break;
        case rST5 :           pStr = "ST(5)";          break;
        case rST6 :           pStr = "ST(6)";          break;
        case rST7 :           pStr = "ST(7)";          break;
    }
    return( pStr );
}

