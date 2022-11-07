/*
	HISTORY
	=======
	2003-08-27:  JMP DWORD PTR [ABS_ADDR] FF 25 00 20 00 80 버그 수정.
*/

#include "bellona2.h"

static UINT16 wDefaultBit = 1;

UINT16	wSetDefaultBit( UINT16 wDBit )
{
	UINT16 wT;
	wT = wDefaultBit;

	wDefaultBit = wDBit;

	return( wT );		// 이전 것을 리턴한다.
}

UCHAR* pByte2Hex( UCHAR* pS, UCHAR byLo )
{
    UCHAR byHi = (UCHAR)( (UCHAR)byLo >> 4 );
    byLo = (UCHAR)( (UCHAR)byLo & 0x0F );

    if( byHi >= 10 )
         byHi += ('A' - 10);
    else
        byHi += '0';

    if( byLo >= 10 )
         byLo += ('A' - 10);
    else
        byLo += '0';

    pS[0] = byHi;
    pS[1] = byLo;
    pS[2] = 0;

    return( &pS[2] );
}

CHAR* pMakeOperandStr( CHAR* pBuff, OperandStt* pOprnd, OpStt* pOp )
{
    UINT16  wDispSize =  pOp->wDispSize;
    ULONG   dwDisp    =  pOp->dwDisp;
    UINT16  wImmSize  =  pOp->wImmSize;
    ULONG   dwImm     =  pOp->dwImm;

    pBuff[0] = 0;
    switch( pOprnd->wType )
    {
        case oc_REG :
            strcat( pBuff, pRegStr( pOprnd->wRegBase ) );
            break;

        case oc_MEM :
            strcat( pBuff, "[" );

            if( pOprnd->wRegBase != 0 )
                strcat( pBuff, pRegStr( pOprnd->wRegBase ) );  // Base

            if( pOprnd->wIndex != 0 )                      // Index
            {
                strcat( pBuff, "+" );
                strcat( pBuff, pRegStr( pOprnd->wIndex ) );
            }

            switch( pOprnd->wScale )                       // Scale
            {
                case 1 : strcat( pBuff, "*2" ); break;
                case 2 : strcat( pBuff, "*4" ); break;
                case 3 : strcat( pBuff, "*8" ); break;
            }

/*            if( wDispSize > 0 )
            {
                UCHAR* pDisp = (UCHAR*)&dwDisp;
                UCHAR  szT[3];
                UINT16   wI;

                if( pOprnd->wRegBase || pOprnd->wIndex )
                    strcat( pBuff, "+" );

                for( wI = wDispSize; wI > 0; wI-- )
                {
                    pByte2Hex( szT, pDisp[wI-1] );
                    strcat( pBuff, (CHAR*)szT );
                }
            }
            strcat( pBuff, "]" );
            break;
*/
           if( wDispSize > 0 )
           {
               UCHAR* pDisp = (UCHAR*)&dwDisp;
               UCHAR  szT[3];
               UINT16 wI;
               char   szSign[2];

               szSign[1] = 0;

			    // Base 또는 Index가 없으면 변위를 절대값으로 인식한다.
			    // JMP DWORD PTR [7FFFE000] FF 25 00 20 00 80 이 아니라
			    // JMP DWORD PTR [80002000]으로 역어셈블한다.  2003-08-27
               if( ( pOprnd->wIndex != 0 || pOprnd->wRegBase != 0 ) && dwDisp & (DWORD)0x80000000 )
               {
                   szSign[0] = '-';
                   dwDisp = (DWORD)(~dwDisp );
                   dwDisp += 1;
               }
               else
                   szSign[0] = '+';

               if( pOprnd->wRegBase || pOprnd->wIndex )
                   strcat( pBuff, szSign );

               for( wI = wDispSize; wI > 0; wI-- )
               {
                   pByte2Hex( szT, pDisp[wI-1] );
                   strcat( pBuff, (CHAR*)szT );
               }
           }
           strcat( pBuff, "]" );
           break;

        case oc_IMMdisp :
            if( wDispSize > 0 )
            {
                UCHAR* pDisp = (UCHAR*)&dwDisp;
                UCHAR  szT[3];
                UINT16   wI;

                for( wI = wDispSize; wI > 0; wI-- )
                {
                    pByte2Hex( szT, pDisp[wI-1] );
                    strcat( pBuff, (CHAR*)szT );
                }
            }
            break;

        case oc_IMM :
            if( wImmSize > 0 )
            {
                UCHAR* pImm = (UCHAR*)&dwImm;
                UCHAR   szT[3];
                UINT16   wI;

                for( wI = wImmSize; wI > 0; wI-- )
                {
                    pByte2Hex( szT, pImm[wI-1] );
                    strcat( pBuff, (CHAR*)szT );
                }
            }
            break;

        case oc_REL :
            if( wImmSize > 0 )
            {
                UCHAR* pImm;
                UCHAR  szT[3];
                UINT16   wI;
				DWORD  dwOverflow = 0;

                wI = wImmSize;
				if( wI == 1 && dwImm >= (DWORD)0x80 )
					dwOverflow = (DWORD)0x100;

                dwImm = (ULONG)( dwImm + pOp->dwIP - dwOverflow + pOp->wLength );
                pImm = (UCHAR*)&dwImm;
                if( wI == 1 )
                    wI = 2;
                for( ; wI > 0; wI-- )
                {
                    pByte2Hex( szT, pImm[wI-1] );
                    strcat( pBuff, (CHAR*)szT );
                }
            }
            break;

        default :
            break;
    }

    return( pBuff );
}

char* pDispOp( char *strArr[], UCHAR* pS, OpStt* pOp )
{
    char	szOperandStr[40];
	int		nI;

	(strArr[0])[0]= (strArr[1])[0]= (strArr[2])[0]= (strArr[3])[0]= (strArr[4])[0]= (strArr[5])[0] = 0;

	// 1번 항목 IP
	sprintf( strArr[1], "%08X", (ULONG)pOp->dwIP );

	// 2번 항목 OpCode
    if( pOp->wType == ot_SEG_PRX )
    {
        strcpy( strArr[2], pRegStr( pOp->Oprnd[0].wRegBase ) );
        strcat( strArr[2], ":");
    }
    else
    {
        strcpy( strArr[2], spMnemonic[ pOp->wType] );

		// 3번 항목 Operand
		strArr[3][0] = 0;
        if( pOp->Oprnd[0].wType == oc_MEM && pOp->Oprnd[1].wType != oc_REG )
        {
            switch( pOp->Oprnd[0].wSize )
            {
                case 1 : strcat( strArr[3], "BYTE  PTR " ); break;
                case 2 : strcat( strArr[3], "WORD  PTR " ); break;
                case 4 : strcat( strArr[3], "DWORD PTR " ); break;
                case 6 : strcat( strArr[3], "FWORD PTR " ); break;
                case 8 : strcat( strArr[3], "QWORD PTR " ); break;
                case 10: strcat( strArr[3], "TBYTE PTR " ); break;
            }
        }

        pMakeOperandStr( szOperandStr, &pOp->Oprnd[0], pOp );
        strcat( strArr[3], szOperandStr );
    }

    if( pOp->Oprnd[1].wType != 0 )
    {
        strcat( strArr[3], "," );

        if( pOp->Oprnd[1].wType == oc_MEM && pOp->Oprnd[0].wType != oc_REG )
        {
            switch( pOp->Oprnd[0].wSize )
            {
                case 1 : strcat( strArr[3], "BYTE  PTR " ); break;
                case 2 : strcat( strArr[3], "WORD  PTR " ); break;
                case 4 : strcat( strArr[3], "DWORD PTR " ); break;
                case 6 : strcat( strArr[3], "FWORD PTR " ); break;
                case 8 : strcat( strArr[3], "QWORD PTR " ); break;
                case 10: strcat( strArr[3], "TBYTE PTR " ); break;
            }
        }
        pMakeOperandStr( szOperandStr, &pOp->Oprnd[1], pOp );
        strcat( strArr[3], szOperandStr );
    }

    if( pOp->Oprnd[2].wType != 0 )
    {
        strcat( strArr[3], "," );
        pMakeOperandStr( szOperandStr, &pOp->Oprnd[2], pOp );
        strcat( strArr[3], szOperandStr );
    }

	sprintf( strArr[4], "%d", pOp->wLength );  // 4번 항목 명령 길이

	// 5번 항목 HexaCodeDump
	strArr[5][0] = 0;
	for( nI = 0; nI < (int)pOp->wLength; nI++ )
	{
		char szTemp[8];

		sprintf( szTemp, "%02X ", (UCHAR)pS[nI] );
		strcat( strArr[5], szTemp );
	}

    return( strArr[2] );
}

UINT16 wMRR16( UCHAR* pS, ModRegRmStt* pM )  // Get ModRegRm 16
{
    pM->wMod = (UINT16)( (UINT16)pS[0] >> 6 );
    pM->wReg = (UINT16)( (UINT16)pS[0] & 0x0038 ) >> 3;
    pM->wRm  = (UINT16)( (UINT16)pS[0] & 0x0007 );

    return( 1 );
}
UINT16 wMRR32( UCHAR* pS, ModRegRmStt* pM )  // Get ModRegRm 32
{
    UINT16 wSize = 1;

    pM->wMod = (UINT16)( (UINT16)pS[0] >> 6 );
    pM->wReg = (UINT16)( (UINT16)pS[0] & 0x0038 ) >> 3;
    pM->wRm  = (UINT16)( (UINT16)pS[0] & 0x0007 );

    if( pM->wMod != 3 && pM->wRm == 0x0004 )
    {
        wSize++;     // Sibling Extension
        pM->wScale = (UINT16)( (UINT16)pS[1] >> 6 );
        pM->wIndex = (UINT16)( (UINT16)pS[1] & 0x0038 ) >> 3;
        pM->wBase  = (UINT16)( (UINT16)pS[1] & 0x0007 );
    }

    return( wSize );
}

UINT16 wDecodeModRegRm( UCHAR* pS, UINT16 wDBit, UCHAR byAddrPrx, ModRegRmStt* pM )
{
    UINT16 wSize;
    memset( pM, 0, sizeof( ModRegRmStt ) );

    if( wDBit == 0 )
        if( byAddrPrx == 0 )
            wSize = wMRR16( pS, pM );
        else
            wSize = wMRR32( pS, pM );
    else
        if( byAddrPrx != 0 )
            wSize = wMRR16( pS, pM );
        else
            wSize = wMRR32( pS, pM );

    pM->wSize = wSize;
    return( wSize );
}

UINT16 wOperandSize( OperandStt* pOp, UCHAR byCH, UINT16 wDBit, UCHAR byAddrPrx,
/**/                                                        UCHAR byOprndPrx )
{
    if( (wDBit && byAddrPrx) || (!wDBit && !byAddrPrx) )
        pOp->wWidth = 2;
    else
        pOp->wWidth = 4;

    if( (wDBit && byOprndPrx) || (!wDBit && !byOprndPrx) )
        pOp->wSize = 2;
    else
        pOp->wSize = 4;

    switch( byCH )
    {
        case 'a' :  // Used by BOUND Only
            pOp->wSize = pOp->wWidth;
            break;

        case 'b' :
            pOp->wSize = 1;
            break;

        case 'w' :
            pOp->wSize = 2;
            break;

        case 'd' :
            pOp->wSize = 4;
            break;

        case '6' :
            pOp->wSize = 6;
            break;

        case 'q' :
            pOp->wSize = 8;
            break;

        case 't' :
            pOp->wSize = 10;
            break;

    }

	return(0);
}

UINT16 wGetReg( UINT16 wRValue, UINT16 wSize )
{
    UINT16 wReg;
    switch( wSize )
    {
        case 1  :
            wReg = RegTbl8[  wRValue ];
            break;

        case 2 :
            wReg = RegTbl16[ wRValue ];
            break;

        case 4 :
            wReg = RegTbl32[ wRValue ];
            break;
    }
    return( wReg );
}

UINT16 wSetBaseIndexScale32( OperandStt* pOpnd, ModRegRmStt* pM )
{
    pOpnd->wRegBase = wGetReg( pM->wRm, 4 );
    if( pOpnd->wRegBase == rESP )
    {
        pOpnd->wRegBase = wGetReg( pM->wBase,  4 );
        pOpnd->wIndex   = wGetReg( pM->wIndex, 4 );
        pOpnd->wScale   = pM->wScale;

//        if( pOpnd->wRegBase == rESP )
//            pOpnd->wRegBase =  0;
        if( pOpnd->wIndex   == rESP )
            pOpnd->wIndex   =  0;
    }
    else
        return(0);

	return(0);
}

UINT16 wSetBaseIndex16( OperandStt* pOpnd, ModRegRmStt* pM )
{
    switch( pM->wRm )
    {
        case 0 : pOpnd->wRegBase  = rBX; pOpnd->wIndex = rSI; break;
        case 1 : pOpnd->wRegBase  = rBX; pOpnd->wIndex = rDI; break;
        case 2 : pOpnd->wRegBase  = rBP; pOpnd->wIndex = rSI; break;
        case 3 : pOpnd->wRegBase  = rBP; pOpnd->wIndex = rDI; break;
        case 4 : pOpnd->wRegBase  = rSI; break;
        case 5 : pOpnd->wRegBase  = rDI; break;
        case 6 : pOpnd->wRegBase  = rBP; break;
        case 7 : pOpnd->wRegBase  = rBX; break;
    }
    return(0);
}

UINT16 wSetOperand( UCHAR* pS, OpStt* pOp, UINT16 wOperandNo, ModRegRmStt* pM,
                                              OpDataStt* pO, UINT16 wDBit )
{
    UCHAR*      pOStr = pO->szOperand[ wOperandNo ];
    OperandStt* pOpnd = &pOp->Oprnd[wOperandNo];

    switch( pOStr[0] )
    {
        case '^' :   // ??W -> ??D
            memset( pM, 0, sizeof( ModRegRmStt ) );
            if( (pOp->byOprndPrx != 0 && wDBit == 0 ) || (pOp->byOprndPrx == 0 && wDBit != 0 ) )
               // pOp->wType--;
                pOp->wType++;
            break;

        case 'O' :  //--------< Memory Offset No ModRegRm >------------//
            wOperandSize( pOpnd, pOStr[1], wDBit, pOp->byAddrPrx, pOp->byOprndPrx );
            memset( pM, 0, sizeof( ModRegRmStt ) ); // ModRegRm <- 0
            pOpnd->wType = oc_MEM;
            if( pOpnd->wWidth == 4 )
            {
                pOp->wDispSize  = 4;
                pOp->dwDisp = *(ULONG*)((UCHAR*)&pS[pM->wSize]);
            }
            else
            {
                pOp->wDispSize = 2;
                pOp->dwDisp = *(UINT16*)((UCHAR*)&pS[pM->wSize]);
            }
            break;

        case 'A' :  //--------< Direct address 0000:0000 >-------------//
            if( pOpnd->wSize == 4 )
            {
                pOp->wDispSize = 2;
                pOp->wImmSize  = 2;
                pOp->dwDisp = *(UINT16*)((UCHAR*)&pS[pM->wSize]);
                pOp->dwImm  = *(UINT16*)((UCHAR*)&pS[pM->wSize+2]);
            }
            else
            {
                pOp->wDispSize = 4;
                pOp->wImmSize  = 2;
                pOp->dwDisp = *(ULONG*)((UCHAR*)&pS[pM->wSize]);
                pOp->dwImm  = *(UINT16*)((UCHAR*)&pS[pM->wSize+4]);
            }
            break;

        case 'S' :  //--------< Reg field = Segment Register >---------//
            pOpnd->wType    = oc_REG;
            pOpnd->wSize    = pOpnd->wWidth = 2;
            pOpnd->wRegBase = SegRegTbl[ pM->wReg ];
            break;

        case 'J' :  //--------< JMP Loop Operand >---------------------//
            memset( pM, 0, sizeof( ModRegRmStt ) ); // ModRegRm <- 0
            pOpnd->wType = oc_REL;
            wOperandSize( pOpnd, pOStr[1], wDBit, pOp->byAddrPrx, pOp->byOprndPrx );
            pOp->wImmSize = pOpnd->wWidth = pOpnd->wSize;
            if( pOpnd->wSize == 4 )
                pOp->dwImm = *(ULONG*)((UCHAR*)&pS[pM->wSize]);
            else if( pOpnd->wSize == 2 )
                pOp->dwImm = *(UINT16*)((UCHAR*)&pS[pM->wSize]);
            else
                pOp->dwImm = pS[pM->wSize];
            break;

        case '!' :  //--------< Direct Register    >-------------------//

            if(   ( pO->szOperand[0][0] != 'E' && pO->szOperand[0][0] != 'R' &&
                    pO->szOperand[1][0] != 'E' && pO->szOperand[1][0] != 'R' ) ||
                  ( wOperandNo == 0 && pO->szOperand[1][0] == 0 ) )
                memset( pM, 0, sizeof( ModRegRmStt ) ); // ModRegRm <- 0

            pOpnd->wType = oc_REG;

            wOperandSize( pOpnd, 0, wDBit, pOp->byAddrPrx, pOp->byOprndPrx );
            pOpnd->wWidth = pOpnd->wSize;

            if( pOStr[1] < eAX )  // Certainly defined.
            {
                pOpnd->wRegBase = (UINT16)pOStr[1];
            }
            else  // e??
            {
                if( pOpnd->wSize == 4 )
                    pOpnd->wRegBase = (UINT16)( pOStr[1] + rEAX - eAX );
                else
                    pOpnd->wRegBase = (UINT16)( pOStr[1] + rAX - eAX );
            }
            break;

        case 'i' :
        case 'I' :  //--------< Immediate Data     >-------------------//
            if( wOperandNo == 0 )
                memset( pM, 0, sizeof( ModRegRmStt ) );
            else if( pO->szOperand[0][0] != 'E' && pO->szOperand[0][0] != 'G')
                memset( pM, 0, sizeof( ModRegRmStt ) );

            wOperandSize( pOpnd, pOStr[1], wDBit, pOp->byAddrPrx, pOp->byOprndPrx );
            pOpnd->wWidth = pOpnd->wSize;

            if( pOStr[0] == 'I' )
            {
                pOpnd->wType  = oc_IMM;
                pOp->wImmSize = pOpnd->wWidth;
                if( pOpnd->wSize == 4 )
                    pOp->dwImm = *(ULONG*)((UCHAR*)&pS[pM->wSize + pOp->wDispSize]);
                else if( pOpnd->wSize == 2 )
                    pOp->dwImm = *(UINT16*)((UCHAR*)&pS[pM->wSize + pOp->wDispSize]);
                else
                    pOp->dwImm = pS[pM->wSize + pOp->wDispSize];
            }
            else           // for ENTER
            {
                pOpnd->wType   = oc_IMMdisp;
                pOp->wDispSize = pOpnd->wWidth;
                if( pOpnd->wSize == 4 )
                    pOp->dwDisp = *(ULONG*)((UCHAR*)&pS[pM->wSize + pOp->wImmSize]);
                else if( pOpnd->wSize == 2 )
                    pOp->dwDisp = *(UINT16*)((UCHAR*)&pS[pM->wSize + pOp->wImmSize]);
                else
                    pOp->dwDisp = pS[pM->wSize + pOp->wImmSize];
            }
            break;

        case 'C' :
            pOpnd->wType    = oc_REG;
            pOpnd->wSize    = pOpnd->wWidth = 4;
            pOpnd->wRegBase = rC0 + pM->wReg;
            break;

        case 'D' :
            pOpnd->wType    = oc_REG;
            pOpnd->wSize    = pOpnd->wWidth = 4;
            pOpnd->wRegBase = rD0 + pM->wReg;
            break;

        case 'T' :
            pOpnd->wType    = oc_REG;
            pOpnd->wSize    = pOpnd->wWidth = 4;
            pOpnd->wRegBase = rT0 + pM->wReg;
            break;

        case 'M' :
        case 'R' :
            if( ( pOStr[0] == 'M' && pM->wMod == 3 ) || pOStr[0] == 'R' && pM->wMod != 3 )
			{
                 // Coding Error
			}

        case 'E' :  //--------< Register or Memory >-------------------//
            // Set pOpnd's Size Width
            wOperandSize( pOpnd, pOStr[1], wDBit, pOp->byAddrPrx, pOp->byOprndPrx );

            if( pM->wMod == 3 )
                pOpnd->wType  = oc_REG;
            else
                pOpnd->wType  = oc_MEM;

            switch( pM->wMod )
            {
                case 3 :    // Mod 11 = General Register
                    pOpnd->wRegBase = wGetReg( pM->wRm, pOpnd->wSize );
                    break;

                case 2 :    // Word Displacement
                    pOp->wDispSize = pOpnd->wWidth;
                    if( pOpnd->wWidth == 4 ) // Get 32Byte Displacement
                    {
                        pOp->dwDisp = *(ULONG*)((UCHAR*)&pS[pM->wSize]);
                        wSetBaseIndexScale32( pOpnd, pM );
                    }
                    else
                    {
                        pOp->dwDisp = *(UINT16*)((UCHAR*)&pS[pM->wSize]);
                        wSetBaseIndex16( pOpnd, pM );
                    }
                    break;

                case 1 :    // Byte Displacement
                    pOp->wDispSize = 1;
                    pOp->dwDisp = (UCHAR)pS[pM->wSize];
                    if( pOpnd->wWidth == 4 ) // Get 32Byte Displacement
                        wSetBaseIndexScale32( pOpnd, pM );
                    else
                        wSetBaseIndex16( pOpnd, pM );
                    break;

                case 0 :    // No Displacement
                    if( pOpnd->wWidth == 4 )
                    {
                        wSetBaseIndexScale32( pOpnd, pM );
                        if( pOpnd->wRegBase == rEBP && pOpnd->wIndex == 0 )
                        {                         // 32bit Displacement Only
                            pOpnd->wRegBase = 0;
                            pOp->wDispSize  = 4;
                            pOp->dwDisp = *(ULONG*)((UCHAR*)&pS[pM->wSize]);
                        }
                    }
                    else
                    {
                        wSetBaseIndex16( pOpnd, pM );
                        if( pM->wRm == 6 )
                        {                         // 16bit Displacement Only
                            pOpnd->wRegBase = 0;
                            pOp->wDispSize = 2;
                            pOp->dwDisp = *(UINT16*)((UCHAR*)&pS[pM->wSize]);
                        }

                    }
                    break;
            }
            break;
        //---------------------------------------------------------------//

        case 'G' :
            pOpnd->wType  = oc_REG;
            wOperandSize( pOpnd, pOStr[1], wDBit, pOp->byAddrPrx, pOp->byOprndPrx );
            pOpnd->wRegBase = wGetReg( pM->wReg, pOpnd->wSize );
            break;

    }

    return(0);
}

UINT16 wLucifer( UCHAR* pS, OpStt* pOp )
{
    UINT16       wOpCodeSize;
    OpDataStt    OD;
    OpDataStt*   pOData = &OD;
    ModRegRmStt  Mrr;

    memset( &Mrr,   0, sizeof( ModRegRmStt ) );
    memset( pOData, 0, sizeof( OpDataStt ) );

    if( *pS == (UCHAR)0x67 )
    {
        pS++;
        pOp->byAddrPrx = 0x67;
        if( *pS == (UCHAR)0x66 )
        {
            pS++;
            pOp->byOprndPrx = 0x66;
        }
    }
    else if( *pS == (UCHAR)0x66 )
    {
        pS++;
        pOp->byOprndPrx = 0x66;
        if( *pS == (UCHAR)0x67 )
        {
            pS++;
            pOp->byAddrPrx = 0x67;
        }
    }

    if( pS[0] == 0x0F )
    {
        wOpCodeSize = 2;
        wSearchTwoByteTbl( pOData, pS[1] );
    }
    else
    {
        wOpCodeSize = 1;
        pOData =  &OneByteTbl[ pS[0] ];
    }

    if( pOData->wType == ot_DB )  // UnKnown Code  -> Define Byte
        pOp->wType = ot_DB;
    else
    {
        wDecodeModRegRm( &pS[wOpCodeSize], wDefaultBit, pOp->byAddrPrx, &Mrr );

        switch( pOData->wType )  // Process GRPs
        {
            case ot_GRP1 :
                pOp->wType = swGrp01Tbl[Mrr.wReg];
                break;

            case ot_GRP2 :
                pOp->wType = swGrp02Tbl[Mrr.wReg];
                break;

            case ot_GRP3 :
                pOData->szOperand[1][0] = 0;

                pOp->wType = swGrp03Tbl[Mrr.wReg];
                switch( pOp->wType )
                {
                    case ot_TEST :
                        pOData->szOperand[1][0] = 'I';
                        pOData->szOperand[1][1] = pOData->szOperand[0][1];
                        break;
                }
                pOData->szOperand[1][2] = 0;
                break;

            case ot_GRP4 :
                pOp->wType = swGrp04Tbl[Mrr.wReg];
                pOData->szOperand[0][0] = 'E';
                pOData->szOperand[0][1] = 'b';
                pOData->szOperand[0][2] =  0 ;
                break;

            case ot_GRP5 :
                pOp->wType = swGrp05Tbl[Mrr.wReg];
                pOData->szOperand[0][1] = 'v';
                if( Mrr.wReg == 3 || Mrr.wReg == 5 )
                    pOData->szOperand[0][1] = '6';
				else if( Mrr.wReg == 2 || Mrr.wReg == 4 )
                    pOData->szOperand[0][1] = 'p';
                break;

            case ot_GRP6 :
                pOp->wType = swGrp06Tbl[Mrr.wReg];
                break;

            case ot_GRP7 :
                pOp->wType = swGrp07Tbl[Mrr.wReg];
                if( Mrr.wReg == 7 )
                    pOData->szOperand[0][0] = 0;
                else if( Mrr.wReg == 6 || Mrr.wReg == 4 )
                {
                    pOData->szOperand[0][0] = 'E';
                    pOData->szOperand[0][1] = 'w';
                }
                else
                {
                    pOData->szOperand[0][0] = 'M';
                    pOData->szOperand[0][1] = 's';
                }
                pOData->szOperand[0][2] =  0 ;
                break;

            case ot_GRP8 :
                pOp->wType = swGrp08Tbl[Mrr.wReg];
                break;

            case ot_ESC8 :
                pOData->szOperand[0][0] = 0;
                if( Mrr.wMod != 3 )
                {
                    pOp->wType = swEscD8Tbl[Mrr.wReg];
                    pOData->szOperand[0][0] = 'M';
                    pOData->szOperand[0][1] = 'd';
                    pOData->szOperand[0][2] = 0;
                    pOData->szOperand[1][0] = 0;
                }
                else
                {
                    pOp->wType = swEscD8Tbl[Mrr.wReg];
                    pOp->Oprnd[0].wType    = oc_REG;
                    pOp->Oprnd[0].wSize    = pOp->Oprnd[0].wWidth = 4;
                    pOp->Oprnd[0].wRegBase = rST;
                    pOp->Oprnd[1].wType    = oc_REG;
                    pOp->Oprnd[1].wSize    = pOp->Oprnd[1].wWidth = 4;
                    pOp->Oprnd[1].wRegBase = rST0 + Mrr.wRm;
                }
                break;

            case ot_ESC9 :
                pOData->szOperand[0][0] = 0;
                pOData->szOperand[1][0] = 0;
                if( Mrr.wMod != 3 )
                {
                    pOp->wType = swEscD9Tbl1[Mrr.wReg];
                    pOData->szOperand[0][0] = 'M';
                    if( Mrr.wReg == 5 || Mrr.wReg == 7 )
                        pOData->szOperand[0][1] = 'w';
                    else
                        pOData->szOperand[0][1] = 'd';
                    pOData->szOperand[0][2] = 0;

                }
                else
                {
                    if( Mrr.wReg == 0 || Mrr.wReg == 1 )
                    {
                        if( Mrr.wReg == 0 )
                            pOp->wType = ot_FLD;
                        else
                            pOp->wType = ot_FXCH;

                        pOp->Oprnd[0].wType    = oc_REG;
                        pOp->Oprnd[0].wSize    = pOp->Oprnd[0].wWidth = 4;
                        pOp->Oprnd[0].wRegBase = rST;
                        pOp->Oprnd[1].wType    = oc_REG;
                        pOp->Oprnd[1].wSize    = pOp->Oprnd[1].wWidth = 4;
                        pOp->Oprnd[1].wRegBase = rST0 + Mrr.wRm;
                    }
                    else if( Mrr.wReg == 2 )
                        pOp->wType = ot_FNOP;
                    else
                      pOp->wType =swEscD9Tbl2[ (UINT16)pS[1]-(UINT16)0xE0 ];
                }
                break;

            case ot_ESCa :
                pOData->szOperand[0][0] = 0;
                if( Mrr.wMod != 3 )
                {
                    pOp->wType = swEscDaTbl[Mrr.wReg];
                    pOData->szOperand[0][0] = 'M';
                    pOData->szOperand[0][1] = 'd';
                    pOData->szOperand[0][2] = 0;
                }
                else if( pS[1] == (UCHAR)0xE9 )
                    pOp->wType = ot_FUCOMPP;
                else
                    pOp->wType = ot_DB;

                break;

            case ot_ESCb :
                pOData->szOperand[0][0] = 0;
                if( Mrr.wMod != 3 )
                {
                    pOp->wType = swEscDbTbl[Mrr.wReg];
                    pOData->szOperand[0][0] = 'M';

                    if( Mrr.wReg == 5 || Mrr.wReg == 7 )
                        pOData->szOperand[0][1] = 't';
                    else
                        pOData->szOperand[0][1] = 'd';

                    pOData->szOperand[0][2] = 0;
                }
                else if( pS[1] == (UCHAR)0xE9 )
                    pOp->wType = ot_FUCOMPP;
                else
                    pOp->wType = ot_DB;

                break;

            case ot_ESCc :
                pOData->szOperand[0][0] = 0;
                if( Mrr.wMod != 3 )
                {
                    pOp->wType = swEscD8Tbl[Mrr.wReg];
                    pOData->szOperand[0][0] = 'M';
                    pOData->szOperand[0][1] = 'q';
                    pOData->szOperand[0][2] = 0;
                    pOData->szOperand[1][0] = 0;
                }
                else
                {
                    pOp->wType = swEscDcTbl[Mrr.wReg];
                    pOp->Oprnd[0].wType    = oc_REG;
                    pOp->Oprnd[0].wSize    = pOp->Oprnd[1].wWidth = 4;
                    pOp->Oprnd[0].wRegBase = rST0 + Mrr.wRm;
                    pOp->Oprnd[1].wType    = oc_REG;
                    pOp->Oprnd[1].wSize    = pOp->Oprnd[0].wWidth = 4;
                    pOp->Oprnd[1].wRegBase = rST;
                }
                break;

            case ot_ESCd :
                pOData->szOperand[0][0] = 0;
                pOData->szOperand[1][0] = 0;
                if( Mrr.wMod != 3 )
                {
                    pOp->wType = swEscDdTbl1[Mrr.wReg];
                    pOData->szOperand[0][0] = 'M';
                    pOData->szOperand[0][1] = 'q';
                    pOData->szOperand[0][2] = 0;
                }
                else
                {
                    pOp->wType = swEscDdTbl2[Mrr.wReg];
                    pOp->Oprnd[0].wType    = oc_REG;
                    pOp->Oprnd[0].wSize    = pOp->Oprnd[1].wWidth = 4;
                    pOp->Oprnd[0].wRegBase = rST0 + Mrr.wRm;
                }
                break;

            case ot_ESCe :
                pOData->szOperand[0][0] = 0;
                pOData->szOperand[1][0] = 0;
                if( Mrr.wMod != 3 )
                {
                    pOp->wType = swEscDaTbl[Mrr.wReg];
                    pOData->szOperand[0][0] = 'M';
                    pOData->szOperand[0][1] = 'w';
                    pOData->szOperand[0][2] = 0;
                }
                else
                {
                    pOp->wType = swEscDeTbl[Mrr.wReg];
                    pOp->Oprnd[0].wType    = oc_REG;
                    pOp->Oprnd[0].wSize    = pOp->Oprnd[1].wWidth = 4;
                    pOp->Oprnd[0].wRegBase = rST0 + Mrr.wRm;
                    pOp->Oprnd[1].wType    = oc_REG;
                    pOp->Oprnd[1].wSize    = pOp->Oprnd[0].wWidth = 4;
                    pOp->Oprnd[1].wRegBase = rST;
                }
                break;


            case ot_ESCf :
                pOData->szOperand[0][0] = 0;
                if( Mrr.wMod != 3 )
                {
                    pOp->wType = swEscDfTbl[Mrr.wReg];
                    pOData->szOperand[0][0] = 'M';

                    if( Mrr.wReg == 5 || Mrr.wReg == 7 )
                        pOData->szOperand[0][1] = 'q';
                    else if( Mrr.wReg == 4 || Mrr.wReg == 6 )
                        pOData->szOperand[0][1] = 't';
                    else
                        pOData->szOperand[0][1] = 'w';

                    pOData->szOperand[0][2] = 0;
                }
                else if( pS[1] == (UCHAR)0xE0 )
                {
                    pOp->wType = ot_FSTSW;
                    pOp->Oprnd[0].wType = oc_REG;
                    pOp->Oprnd[0].wSize = pOp->Oprnd[0].wWidth = 2;
                    pOp->Oprnd[0].wRegBase = rAX;
                }
                else
                    pOp->wType = ot_DB;
                break;

            default :
                pOp->wType = pOData->wType;
                break;
        }

        if( pOData->szOperand[0][0] != 0 )   // Operand 1
        {
            wSetOperand( &pS[wOpCodeSize], pOp, 0, &Mrr, pOData, wDefaultBit );

            if( pOData->szOperand[1][0] != 0 )   // Operand 2
                wSetOperand( &pS[wOpCodeSize], pOp, 1, &Mrr, pOData, wDefaultBit );

            if( pOData->szOperand[2][0] != 0 )   // Operand 3
                wSetOperand( &pS[wOpCodeSize], pOp, 2, &Mrr, pOData, wDefaultBit );
        }
        else
        {
            if( pS[0] < (UCHAR)0xD8 ||  pS[0] > (UCHAR)0xDF )
                memset( &Mrr, 0, sizeof( ModRegRmStt ) );
        }
    }

    pOp->wLength = wOpCodeSize + Mrr.wSize + pOp->wDispSize + pOp->wImmSize;


    if( pOp->byAddrPrx != 0 )
        pOp->wLength++;
    if( pOp->byOprndPrx != 0 )
        pOp->wLength++;

    return( pOp->wLength );
}

int nDisAssembleOneCode( ULONG dwIP, OpStt *pOp, UCHAR *pBuff, char *strArr[] )
{
    UINT16 wSize;

	memset( pOp, 0, sizeof( OpStt ) );  // OpCode 구조를 초기화 한다.

    pOp->dwIP = dwIP;					// IP 값을 알아서 준다.
    wSize = wLucifer( pBuff, pOp );		// OpCode를 분석한다.

    pDispOp( strArr, pBuff, pOp );		// 출력 가능한 스트링으로 변경한다.

    return( (int)wSize );				// 명령의 길이를 리턴한다.
}

// 도스에서 디버깅을 하기 위해 사용하는 함수. 다른데서는 안쓴다.
char *pDosDispOp( CHAR* pBuff, UCHAR* pAddr, UCHAR* pS, OpStt* pOp )
{
    UCHAR	*pV;
    UINT16  wI, wJ;
    CHAR	szOperandStr[40];

    pBuff[0] = 0;
    pV = (UCHAR*)&pAddr;

    pByte2Hex( (BYTE*)&pBuff[0], (BYTE)pV[3] );
    pByte2Hex( (BYTE*)&pBuff[2], (BYTE)pV[2] );
    pBuff[4] = ':';
    pByte2Hex( (BYTE*)&pBuff[5], (BYTE)pV[1] );
    pByte2Hex( (BYTE*)&pBuff[7], (BYTE)pV[0] );
    pBuff[9] = ' ';

    for( wJ = 10, wI = 0; wI < pOp->wLength; wI++, wJ+=2 )
    {
        pByte2Hex( (BYTE*)&pBuff[wJ], (BYTE)pS[wI] );
    }
    for( ; wI < 14; wI++ )    // Fill Blank
    {
        pBuff[wJ++] = ' ';
        pBuff[wJ++] = ' ';
        pBuff[wJ]   = 0;
    }

    if( pOp->wType == ot_SEG_PRX )
    {
        strcat( &pBuff[wJ], pRegStr( pOp->Oprnd[0].wRegBase ) );
        strcat(pBuff, ":");
    }
    else
    {
        strcat( &pBuff[wJ], spMnemonic[ pOp->wType] );
        strcat( pBuff, "         " );
        pBuff[wJ+8] = 0;

        if( pOp->Oprnd[0].wType == oc_MEM && pOp->Oprnd[1].wType != oc_REG )
        {
            switch( pOp->Oprnd[0].wSize )
            {
                case 1 : strcat( pBuff, "BYTE  PTR " ); break;
                case 2 : strcat( pBuff, "WORD  PTR " ); break;
                case 4 : strcat( pBuff, "DWORD PTR " ); break;
                case 6 : strcat( pBuff, "FWORD PTR " ); break;
                case 8 : strcat( pBuff, "QWORD PTR " ); break;
                case 10: strcat( pBuff, "TBYTE PTR " ); break;
            }
        }

        pMakeOperandStr( szOperandStr, &pOp->Oprnd[0], pOp );
        strcat( pBuff, szOperandStr );
    }

    if( pOp->Oprnd[1].wType != 0 )
    {
        strcat( pBuff, "," );

        if( pOp->Oprnd[1].wType == oc_MEM && pOp->Oprnd[0].wType != oc_REG )
        {
            switch( pOp->Oprnd[0].wSize )
            {
                case 1 : strcat( pBuff, "BYTE  PTR " ); break;
                case 2 : strcat( pBuff, "WORD  PTR " ); break;
                case 4 : strcat( pBuff, "DWORD PTR " ); break;
                case 6 : strcat( pBuff, "FWORD PTR " ); break;
                case 8 : strcat( pBuff, "QWORD PTR " ); break;
                case 10: strcat( pBuff, "TBYTE PTR " ); break;
            }
        }
        pMakeOperandStr( szOperandStr, &pOp->Oprnd[1], pOp );
        strcat( pBuff, szOperandStr );
    }

    if( pOp->Oprnd[2].wType != 0 )
    {
        strcat( pBuff, "," );
        pMakeOperandStr( szOperandStr, &pOp->Oprnd[2], pOp );
        strcat( pBuff, szOperandStr );
    }

    return( pBuff );
}
