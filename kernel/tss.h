#ifndef BELLONA2_TSS_HEADER_oh
#define BELLONA2_TSS_HEADER_oh

typedef struct TSSTag {
	DWORD  dwBackLink;
	DWORD  dwESP0;
	UINT16 wSS0, rsv0;
	DWORD  dwESP1;
	UINT16 wSS1, rsv1;
	DWORD  dwESP2;
	UINT16 wSS2, rsv2;
	DWORD  dwCR3;
	DWORD  dwEIP;
	DWORD  dwEFLAG;
	DWORD  dwEAX;
	DWORD  dwECX;
	DWORD  dwEDX;
	DWORD  dwEBX;
	DWORD  dwESP;
	DWORD  dwEBP;
	DWORD  dwESI;
	DWORD  dwEDI;
    UINT16 wES,  rsv3;
    UINT16 wCS,  rsv4;
    UINT16 wSS,  rsv5;
    UINT16 wDS,  rsv6;
    UINT16 wFS,  rsv7;
    UINT16 wGS,  rsv8;
    UINT16 wLDT, rsv9;
	UINT16 wTBit;
	UINT16 wBitField;
	UCHAR  byIOBit[3];
    UCHAR  byEndMark;
};
typedef struct TSSTag TSSStt;

typedef struct V86TSSTag {
	DWORD  dwBackLink;
	DWORD  dwESP0;
	UINT16 wSS0, rsv0;
	DWORD  dwESP1;
	UINT16 wSS1, rsv1;
	DWORD  dwESP2;
	UINT16 wSS2, rsv2;
	DWORD  dwCR3;
	DWORD  dwEIP;
	DWORD  dwEFLAG;
	DWORD  dwEAX;
	DWORD  dwECX;
	DWORD  dwEDX;
	DWORD  dwEBX;
	DWORD  dwESP;
	DWORD  dwEBP;
	DWORD  dwESI;
	DWORD  dwEDI;
    UINT16 wES,  rsv3;
    UINT16 wCS,  rsv4;
    UINT16 wSS,  rsv5;
    UINT16 wDS,  rsv6;
    UINT16 wFS,  rsv7;
    UINT16 wGS,  rsv8;
    UINT16 wLDT, rsv9;
	UINT16 wTBit;
	UINT16 wBitField;
	UCHAR  byIOBit[8192];
    UCHAR  byEndMark;
};
typedef struct V86TSSTag V86TSSStt;

extern TSSStt dbg_tss, init_tss, pf_tss;

extern int		nDispTSS			( int argc, char *argv[] );
extern int		nAppendTSSTbl		( char *pName, DWORD dwTSSAddr );

extern void		vMakeV86TSS			( V86TSSStt *pTSS, DWORD dwEIP, DWORD dwESP );
extern void		vMakeTSS			( TSSStt *pTSS, DWORD dwEIP, DWORD dwESP );
extern void		make_tlb_tss		();
extern TSSStt	*get_current_tss	();

#endif