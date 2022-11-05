#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>

#include <string.h>
#include <mem.h>
#include <alloc.h>
#include <fcntl.h>
#include <sys\stat.h>

#include "gdt.h"

void vPutChar( char c )
{
    if( c == '\n' )
    {
        asm{
        PUSH AX
        PUSH BX
        MOV  AH,0x0E
        MOV  BX,14
        MOV  AL,0x0d
        INT  0x10
        MOV  AL,0x0a
        INT  0x10
        POP  BX
        POP  AX }
    }
    else
    {
        asm{
        PUSH AX
        PUSH BX
        MOV  AH,0x0E
        MOV  BX,14
        MOV  AL,c
        INT  0x10
        POP  BX
        POP  AX }
    }
}
void vMesg( char *pS )
{
    int nI;
//    printf( pS );
    for( nI = 0; pS[nI] != 0; nI++ )
        vPutChar( pS[nI] );

}
//-------------------------------------------------------------//
int KBD_CMD()
{
    int nR;
                asm{
                PUSH CX
                XOR CX,CX}
    CMD_WAIT:   asm{
                IN   AL,0x64
                NOP
                NOP
                NOP
                NOP
                NOP
                TEST AL,2
                JZ CMD_SEND
                LOOP CMD_WAIT
                    MOV nR,0xFFFF  //STC
                    JMP END_CMD}
    CMD_SEND:   asm{
                MOV AL,BL
                OUT 0x64,AL
                NOP
                NOP
                NOP
                NOP
                NOP
                MOV nR,0  // CLC
                }
    END_CMD:    asm{
                POP CX}

    return(nR);
}

int KBD_READ()
{
    int nR;
                asm{
                PUSH CX
                XOR  CX,CX}
    READ_LOOP:  asm{
                IN   AL,0x64
                NOP
                NOP
                NOP
                NOP
                NOP
                TEST AL,1
                JNZ  READ_NOW
                LOOP READ_LOOP
                     MOV nR, -1 //STC
                     JMP END_READ}

    READ_NOW:   asm{
                IN   AL,0x60
                MOV  nR, 0} //CLC
    END_READ:   asm{
                POP  CX}
    return(nR);
}


int KBD_WRITE()
{
    int nR;
                asm{
                PUSH CX
                PUSH DX

                MOV  DL,AL
                XOR  CX,CX}
    WRITE_LOOP: asm{
                IN   AL,0x64
                NOP
                NOP
                NOP
                NOP
                NOP
                TEST AL,2
                JZ   WRITE_NOW
                LOOP WRITE_LOOP
                     MOV nR,-1 //STC
                     JMP END_WRITE}
    WRITE_NOW:  asm{MOV  AL,DL
                OUT  0x60,AL
                MOV nR,0 }  //CLC
    END_WRITE:  asm{POP  DX
                POP  CX}
    return(nR);
}

int ENABLE_A20()
{
    int nR = -1;
                asm{
                PUSH AX
                PUSH BX
                CLI
                MOV  BL,0xD0
                }    // READ

                if( KBD_CMD() != 0 )
                    goto END_A20;

                if( KBD_READ() != 0 )
                    goto END_A20;

                asm{
                PUSH AX         // WRITE
                MOV  BL,0xD1}
                nR = KBD_CMD();
                asm{POP  AX}

                if( nR != 0 )
                    goto END_A20;

                asm{OR   AL,2}

                if( KBD_WRITE() != 0 )
                    goto END_A20;

                nR = 0;

END_A20:        asm{
                STI
                POP BX
                POP AX}
    return(nR);
}

int SIMPLE_ENABLE_A20()
{

                asm{XOR CX,CX}
IBEmm0:
                asm{
                IN AL,64h
                TEST AL,02h
                LOOPNZ IBEmm0
                MOV AL,0D1h
                OUT 64h,AL
                XOR CX,CX
                }
IBEmm1:
                asm{
                IN AL,64h
                TEST AL,02h
                LOOPNZ IBEmm1
                MOV AL,0DFh
                OUT 60h,AL
                XOR CX,CX
                }
/*IBEmm2:
                asm{
                IN AL,64h
                TEST AL,02h
                LOOPNZ IBEmm2
                MOV AL,0D1h
                OUT 64h,AL
                XOR CX,CX
                }*/
IBEmm3:
                asm{
                IN AL,64h
                TEST AL,02h
                LOOPNZ IBEmm3
                MOV AL,0AEh
                OUT 64h,AL
                }
    return(0);
}
//--------------------------------------------------------------------------//
static GDTRStt gdtr = { 0, 0 };
static DescriptorStt gdt[64] = {
    {0,0,0,0,0,0,0,0}, //  First Entry Must be filled with 0
    {0xFF,0xFF,0x00,0x00,0x00,0x9E,0x0F,0x00},   // CODE16
    {0xFF,0xFF,0x00,0x00,0x00,0x92,0x8F,0x00},   // DATA16
    {0xFF,0xFF,0x00,0x00,0x00,0x92,0x8F,0x00},   // STACK16
    {0xFF,0xFF,0x00,0x00,0x00,0x9A,0xCF,0x00},   // CODE32 R0
    {0xFF,0xFF,0x00,0x00,0x00,0x92,0xCF,0x00},   // DATA32 R0
    {0x00,0x00,0x20,0x00,0x00,0x8C,0x00,0x00},   // CALLGATE32
    {0xFF,0xFF,0x00,0x00,0x00,0xFA,0xCF,0x00},   // CODE32 R3
    {0xFF,0xFF,0x00,0x00,0x00,0xF2,0xCF,0x00},   // DATA32 R3
	  //   DW 0000H          ; OFFSET (0-15)
	  //   DW GSEL_CODE32    ; TARGET CS SELECTOR
	  //   DB 0              ; 000, PARAMETERS
          //   DB 10001100B      ; TYPE
          //   DW 0000H          ; OFFSET (16-31)
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};
static UINT16 wDosCS = 0, wDosDS = 0, wDosSS = 0;

#define BLOCK_SIZE (65536-1024)
static int   nBuff = 0;   // Total Allocated 60k Buffers

static char    szStart[] = {3,3,3,3,3,3,3,3,3,3}; //**********************
// pBuff must follow nRBuff////////////////
static int     nRBlock = -1;             // Total Read Blocks
static UCHAR   far *pBuff[10] = {NULL};  // 10 pointers = 63k * 10 = 630K
static UINT16  wRead = 0xFFFF;           // Last Fragment Size
///////////////////////////////////////////

static int     nFileHandle = 0;
static long    lFileSize = 0;
void vEnterIntoPMode()
{
    ULONG  dwT;
    ULONG  far *pDW;
    ULONG  far *pDX;
    UINT16 far *p_elfanew;
    ULONG  dwSizeOfHeader, dwCodeVirtualAddress, dwCodePointerToRawData;

    UINT16 wDS, wTemp, *pW;
    UCHAR  *pB, *pX;

    // Open A20 Gate to Access All Memory Area
    if( SIMPLE_ENABLE_A20() == 0 )
	    vMesg("Enable a20 gate - ok.\n" );
    else
    {
        vMesg("Enable a20 gate - error.\n");
        exit(0);
    }

    p_elfanew = (UINT16 far*)&pBuff[0][60];

    // Calculate Jmp Address
    asm JMP DIRTY_1
    asm JMP JMP_OFFSET
DIRTY_1:
    asm DB 0xE8, 0, 0   // CALL $+3
    asm POP AX
    asm MOV pB,AX

    pB -= 5;
    pW = (UINT16*)pB;
    wTemp = (UINT16)pB + 2;
    pB = (UCHAR*)( wTemp + *pW );

    // MAKE JMP Code
    pDX = (ULONG far*)&pBuff[0][ (int)p_elfanew[0] + (int)260 ];  // text.CodeVirtualAddress
    dwCodeVirtualAddress = pDX[0];
    pDX = (ULONG far*)&pBuff[0][ (int)p_elfanew[0] + (int)268 ];  // text.CodePointerToRawData
    dwCodePointerToRawData = pDX[0];
    pDW = (ULONG far*)&pBuff[0][ (int)p_elfanew[0] + (int)40 ];	  //  Address Of Entry Point
    dwT = pDW[0]  + (ULONG)0x200000 + dwCodePointerToRawData - dwCodeVirtualAddress;
    pX  = (UCHAR*)&dwT;
    pB[1] = pX[0];
    pB[2] = pX[1];
    pB[3] = pX[2];
    pB[4] = pX[3];

//    printf( "%04X %04X\n", (UINT16)*pDW, (UINT16)(*pDW >> 16) );
//    printf( "%02X %02X %02X %02X \n", pB[1], pB[2], pB[3], pB[4] );
//    exit(0);
    //asm int 3

    // Make GDT
    vClearGDTR( &gdtr );
    wSetGDTRSize( &gdtr, sizeof( gdt ) );
    // Make GDT Base
    asm{ MOV wDS, DS }
    asm{ MOV wTemp, offset gdt }
    dwT = (ULONG)wDS;
    dwT = dwT << 4;
    dwT += (ULONG)wTemp;
    dwSetGDTRAddr( &gdtr, dwT );

    // Make Code Descriptor Base
    asm{ MOV wTemp, CS }
    dwT = (ULONG)wTemp;
    dwT = dwT << 4;
    dwSetDescriptorAddr( &gdt[1], dwT );
    // Make Data Descriptor Base
    asm{ MOV wTemp, DS }
    dwT = (ULONG)wTemp;
    dwT = dwT << 4;
    dwSetDescriptorAddr( &gdt[2], dwT );
    // Make Stack Descriptor Base
    asm{ MOV wTemp, SS }
    dwT = (ULONG)wTemp;
    dwT = dwT << 4;
    dwSetDescriptorAddr( &gdt[3], dwT );

//    gotoxy( 1, 1 );

    asm{
    CLI
    }

    // Save Org Segments CS,DS,SS
    asm {
    MOV wDosCS,CS
    MOV wDosDS,DS
    MOV wDosSS,SS
    }

    // Get PMODE000 ADDR
    asm{
    DB  0xE8, 0x00, 0x00 // CALL 103
    POP BX               // BX IS the OFFSET of "POP BX"

    MOV CS:[BX+26],BX               // 0x2E, 0x89, 0x5F, 26
    ADD WORD PTR CS:[BX+26],30      // 0x2E, 0x83, 0x47, 26, 30
    }

    asm{  // Switching to PMODE
    DB 0x0F,0x01,0x16             // LGDT
    DW offset gdtr                // FWORD  PTR gdtr
    DB 0x0F,0x20,0xC0             // MOV     EAX,CR0
    DB 0x66,0x83,0xC8,0x01        // OR      EAX,01
    DB 0x0F,0x22,0xC0             // MOV     CR0,EAX

    DB 0xEA
    DW 0                          // Offset PMODE000
    DW GSEL_CODE16                // CODE16 Selector
    }

PMODE000:
    asm{  // Initialize DS, SS
    MOV AX,GSEL_DATA16
    MOV DS,AX
    MOV AX,GSEL_STACK16
    MOV SS,AX

    //  Get Address of nBuff
    DB 0x66,0x33,0xD2        //     XOR     EDX,EDX
    DB 0x66,0x33,0xC9        //     XOR     ECX,ECX
    MOV CX,wDosDS
    MOV DX,offset pBuff
    DB 0x66,0xC1,0xE1,0x04   //     SHL     ECX,04
    DB 0x66,0x03,0xD1        //     ADD     EDX,ECX
    // EDX <- Flat Address of pBuff

    MOV SI,nRBlock           // SI <- Read Block Count

    DB 0x66,0x33,0xC0        //     XOR     EAX,EAX
    DB 0x66,0x33,0xC9        //     XOR     ECX,ECX

    MOV AX,wDosCS
    MOV BX,offset gdt + GSEL_CALLGATE32
////// Do Not Insert or Alter Bellow Codes !!!
    DB 0xE8,0x00,0x00             //     CALL    0106
    DB 0x59                       //     POP     CX        ; Calc Dest Offset
    DB 0x81,0xC1,0x1A,0x00        //     ADD     CX,001A

    DB 0x66,0xC1,0xE0,0x04        //     SHL     EAX,04
    DB 0x66,0x03,0xC1             //     ADD     EAX,ECX

    DB 0x89,0x07                  //     MOV     [BX],AX   ; Set Callgate Offset
    DB 0x66,0xC1,0xE8,0x10        //     SHR     EAX,10
    DB 0x89,0x47,0x06             //     MOV     [BX+06],AX

    DB 0x9A,0x00,0x00,0x30,0x00   //     CALL    GSEL_CALLGATE32:0x0000
    DB 0x90                       //     NOP
////////////////////////////////////

////////////////////////////////////////
///  Code32 Default Bit = 1          ///
///  Normal 16 Bit Code is Useless.  ///
////////////////////////////////////////
DB 0x66,0xB8,0x28,0x00                 //   MOV     AX,0028
DB 0x66,0x8E,0xD8                      //   MOV     DS,AX
DB 0x66,0x8E,0xC0                      //   MOV     ES,AX
DB 0x66,0x8E,0xD0                      //   MOV     SS,AX
DB 0xC6,0x05,0x00,0x80,0x0B,0x00,0x30  //   MOV     BYTE  PTR [000B8000],30
DB 0xFC                                //   CLD
DB 0x33,0xC9                           //   XOR     ECX,ECX
DB 0xBF,0x00,0x00,0x20,0x00            //   MOV     EDI,00200000
DB 0x66,0x8B,0xCE                      //   MOV     CX,SI
DB 0x33,0xC0                           //   XOR     EAX,EAX
DB 0x33,0xDB                           //   XOR     EBX,EBX
DB 0x66,0x8B,0x02                      //   MOV     AX,[EDX]
DB 0x66,0x8B,0x5A,0x02                 //   MOV     BX,[EDX+02]
DB 0xC1,0xE3,0x04                      //   SHL     EBX,04
DB 0x03,0xD8                           //   ADD     EBX,EAX
DB 0x8B,0xF3                           //   MOV     ESI,EBX
DB 0x51                                //   PUSH    ECX
DB 0xB9,0x00,0x3F,0x00,0x00            //   MOV     ECX,00003F00
DB 0xF3                                //   REP
DB 0xA5                                //   MOVSD
DB 0x59                                //   POP     ECX
DB 0x83,0xC2,0x04                      //   ADD     EDX,04
DB 0xFE,0x05,0x00,0x80,0x0B,0x00       //   INC     BYTE  PTR [000B8000]
DB 0xE2,0xDA                           //   LOOP    021F
}

JMP_OFFSET:
asm{
//*************************//
DB 0xB8,0x00,0x00,0x00,0x00            //   MOV     EAX,00000000
//*************************//
DB 0xFF,0xE0                           //   JMP     EAX

DB 0xF4                                //   HLT
}
    return;
}

///////////////////////////////////////////////////////////////////////////
void vBootEntry()
{
    asm{NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;}
    asm{
       MOV AX,CS
       MOV DS,AX
       MOV ES,AX

       MOV SS,AX
    }

    vMesg( "\nInitializing...\n" );
    vEnterIntoPMode();
}
///////////////////////////////////////////////////////////////////////////

void vEscapeToRMode()
{
    return;
}

//-------------- Load Image File ------------------------------//
int nLoadImage( char *pS )    // return nBuffs
{
    int    nI, nR;
    UINT16 wR;

    // Allocate Total Memory

    nBuff = 0;
    for( nI = 0; nI < 10; nI++ )
    {
        pBuff[nI] = (UCHAR far *)farmalloc( BLOCK_SIZE );
        if( pBuff[nI] == NULL )
            break;

        nBuff++;
    }

    printf( "available memory : %d * %dk bytes\n", nBuff, (int)(BLOCK_SIZE/1024) );
    if( nBuff == 0 )  // Error No Memory
        return( -1 );

    // Open File
    printf( "open image : %s", pS );
    if( _dos_open( pS, O_BINARY | O_RDONLY, &nFileHandle ) != 0 )
    {
        printf( "   (Error!)\n" );
        return( -2 );
    }
        printf( "   (ok)\n" );

    // Get File Size
    lFileSize = lseek( nFileHandle, 0, SEEK_END );
    printf( "image size : %dk\n", (int)(lFileSize/1024) );

    // ReWind File
    lseek( nFileHandle, 0, SEEK_SET );

    // Read Image Blocks
    nRBlock = 0;
    wRead = 0;
    for( nI = 0; nI < nBuff; nI++ )
    {
        nR = _dos_read( nFileHandle, pBuff[nI], (UINT16)BLOCK_SIZE, &wR );
        if( nR == 0 && wR > (UINT16)0 )
            nRBlock++;
        else
            break;
        wRead = wR;
    }
    printf("total %d blocks + (%u) bytes,\n", nRBlock, wRead );

    _dos_close( nFileHandle );

    return( nBuff );
}




//-------------------------------------------------------------//
void main(int argc, char *argv[] )
{
    vMesg( "Bellona2 Kernel Loader\n" );

    nLoadImage( "bellona2.bin" );

    // Enter into Protected Mode
    vEnterIntoPMode();

    // Escape to RealMode
//    vEscapeToRMode();

    vMesg("ok.\n");
}
