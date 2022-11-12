#include "gdt.h"

extern "C" void* memset(void* start, int value, size_t size);

#ifdef _MSC_VER
#pragma pack (push, 1)
#endif

//! processor gdtr register points to base of gdt. This helps
//! us set up the pointer
struct gdtr {

	//! size of gdt
	uint16_t		m_limit;

	//! base address of gdt
	uint32_t		m_base;
};

#ifdef _MSC_VER
#pragma pack (pop, 1)
#endif

//! global descriptor table is an array of descriptors
static struct gdt_descriptor	_gdt [MAX_DESCRIPTORS];

//! gdtr data
static struct gdtr				_gdtr;

struct Tss {
	int previousTask;				// Backlink for nested task
	int esp0, ss0;					// PL0 stack
	int esp1, ss1;					// PL1 stack
	int esp2, ss2;					// PL2 stack
	int cr3;						// Page directory base
	int eip;						// Instruction Pointer
	int eflags;					// Flags
	int eax, ecx, edx, ebx;		// General Purpose registers
	int esp, ebp;					// Stack Registers
	int esi, edi;					// Index Registers
	int es, cs, ss, ds, fs, gs;	// Selectors
	int ldt_seg;					// LDTR
	unsigned short trap_fl; 					// least sig. bit is trap flag	
	unsigned short iomap;						// IO permission map
};

struct GdtEntry {
	unsigned short limit0_15;
	unsigned short base0_15;
	unsigned char base16_23;
	unsigned char type : 4;
	unsigned char system : 1; 		// 0 = system, 1 = application
	unsigned char dpl : 2; 			// Descriptor Privilege Level
	unsigned char present : 1;
	unsigned char limit16_19 : 4;
	unsigned char unused : 1;
	unsigned char zeroBit : 1;
	unsigned char defOpSize : 1; 	// Default operation size, 1 = 32 bit
	unsigned char granularity : 1; 	// 0 = 16 bit, 1 = 32 bit
	unsigned char base24_31;
};


static Tss tss = {
	0, 0, 0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static GdtEntry gdt[] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},					// Null segment	0x0
	{0xffff, 0, 0, 0xa, 1, 0, 1, 0xf, 0, 0, 1, 1, 0},			// OS Code		0x8
	{0xffff, 0, 0, 0x2, 1, 0, 1, 0xf, 0, 0, 1, 1, 0},			// OS Data		0x10
	{0xffff, 0, 0, 0xa, 1, 3, 1, 0xf, 0, 0, 1, 1, 0},			// User Code	0x1b
	{0xffff, 0, 0, 0x2, 1, 3, 1, 0xf, 0, 0, 1, 1, 0},			// User Data	0x23
	{sizeof(tss), reinterpret_cast<unsigned int>(&tss) & 0xffff,	// main TSS		0x28
		(reinterpret_cast<unsigned int>(&tss) >> 16) & 0xff, 9, 0, 0, 1, 0, 0, 0, 1, 1,
		(reinterpret_cast<unsigned int>(&tss) >> 24) & 0xff}
};

#pragma pack( push, 1 )
struct desc {
	unsigned short limit;
	const GdtEntry* base;
};
#pragma pack( pop )

inline void LoadGdt(const GdtEntry base[], unsigned int limit)
{
	desc d;
	d.base = base;
	d.limit = limit;

	_asm
	{
		lgdt[d]
		mov ax, 0x10
		mov ds, ax
		mov es, ax
		mov gs, ax
		mov fs, ax
		mov ss, ax
		mov ax, 0x28
		ltr ax
	}
}

//! install gdtr
static void InstallGDT () 
{
	//LoadGdt(gdt, sizeof(gdt));
	_asm lgdt [_gdtr]
}

//! Setup a descriptor in the Global Descriptor Table
void gdt_set_descriptor(uint32_t i, uint64_t base, uint64_t limit, uint8_t access, uint8_t grand)
{
	if (i > MAX_DESCRIPTORS)
		return;

	//! null out the descriptor
	memset ((void*)&_gdt[i], 0, sizeof (gdt_descriptor));

	//! set limit and base addresses
	_gdt[i].baseLo	= uint16_t(base & 0xffff);
	_gdt[i].baseMid	= uint8_t((base >> 16) & 0xff);
	_gdt[i].baseHi	= uint8_t((base >> 24) & 0xff);
	_gdt[i].limit	= uint16_t(limit & 0xffff);

	//! set flags and grandularity bytes
	_gdt[i].flags = access;
	_gdt[i].grand = uint8_t((limit >> 16) & 0x0f);
	_gdt[i].grand |= grand & 0xf0;
}


//! returns descriptor in gdt
gdt_descriptor* i86_gdt_get_descriptor (int i) {

	if (i > MAX_DESCRIPTORS)
		return 0;

	return &_gdt[i];
}

typedef unsigned short UINT16;
typedef unsigned long  ULONG;
typedef unsigned char  UCHAR;
typedef unsigned long  DWORD;

#pragma pack(push, 1)
typedef struct descriptorTag {
	UCHAR  size_0, size_1;
	UCHAR  addr_0, addr_1, addr_2;
	UCHAR  byAccess;
	UCHAR  gdou_size;
	UCHAR  addr_3;
};
typedef struct descriptorTag DescriptorStt;
#pragma pack(pop)


DescriptorStt ___gdt[64] = {
	{0,0,0,0,0,0,0,0}, //  First Entry Must be filled with 0
	//{0xFF,0xFF,0x00,0x00,0x00,0x9E,0x0F,0x00},   // CODE16
	//{0xFF,0xFF,0x00,0x00,0x00,0x92,0x8F,0x00},   // DATA16
	//{0xFF,0xFF,0x00,0x00,0x00,0x92,0x8F,0x00},   // STACK16
	{0xFF,0xFF,0x00,0x00,0x00,0x9A,0xCF,0x00},   // CODE32 R0
	{0xFF,0xFF,0x00,0x00,0x00,0x92,0xCF,0x00},   // DATA32 R0
	{0xFF,0xFF,0x00,0x00,0x00,0xFA,0xCF,0x00},   // CODE32 R3
	{0xFF,0xFF,0x00,0x00,0x00,0xF2,0xCF,0x00},   // DATA32 R3
	{0x00,0x00,0x20,0x00,0x00,0x8C,0x00,0x00},   // CALLGATE32
	//   DW 0000H          ; OFFSET (0-15)
	//   DW GSEL_CODE32    ; TARGET CS SELECTOR
	//   DB 0              ; 000, PARAMETERS
		//   DB 10001100B      ; TYPE
		//   DW 0000H          ; OFFSET (16-31)
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

//GDT 초기화 및 GDTR 레지스터에 GDT 로드
int InitGDT()
{
	//GDTR 레지스터에 로드할 _gdtr 구조체의 값 초기화
	//_gdtr 구조체의 주소는 물리주소다.
	//디스크립터의 수를 나타내는 MAX_DESCRIPTORS의 값은 10이다.
	//NULL 디스크립터, 커널 코드 디스크립터, 커널 데이터 디스크립터, 유저 코드 디스크립터
	//유저 데이터 디스크립터 이렇게 총 5개이다.
	//디스크립터당 6바이트이므로 GDT의 크기는 30바이트다.
	_gdtr.m_limit = (sizeof(struct gdt_descriptor) * 64) - 1;
	_gdtr.m_base = (uint32_t)&___gdt[0];
	/*
	//NULL 디스크립터의 설정
	gdt_set_descriptor(0, 0, 0, 0, 0);

	//커널 코드 디스크립터의 설정
	gdt_set_descriptor(1, 0, 0xffffffff,
		I86_GDT_DESC_READWRITE | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_CODEDATA |
		I86_GDT_DESC_MEMORY, I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT |
		I86_GDT_GRAND_LIMITHI_MASK);

	//커널 데이터 디스크립터의 설정
	gdt_set_descriptor(2, 0, 0xffffffff,
		I86_GDT_DESC_READWRITE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY,
		I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);

	//유저모드 디스크립터의 설정
	gdt_set_descriptor(3, 0, 0xffffffff,
		I86_GDT_DESC_READWRITE | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_CODEDATA |
		I86_GDT_DESC_MEMORY | I86_GDT_DESC_DPL, I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT |
		I86_GDT_GRAND_LIMITHI_MASK);

	//유저모드 데이터 디스크립터의 설정
	gdt_set_descriptor(4, 0, 0xffffffff, I86_GDT_DESC_READWRITE | I86_GDT_DESC_CODEDATA | I86_GDT_DESC_MEMORY | I86_GDT_DESC_DPL,
		I86_GDT_GRAND_4K | I86_GDT_GRAND_32BIT | I86_GDT_GRAND_LIMITHI_MASK);*/

	//GDTR 레지스터에 GDT 로드
	InstallGDT();

	return 0;
}
