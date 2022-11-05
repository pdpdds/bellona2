#include "lib.h"

int  is_gui_mode()
{
	return( (int)syscall_stub( SCTYPE_IS_GUI_MODE ) );
}

void set_xy( int nX, int nY )
{
	syscall_stub( SCTYPE_SET_CURSOR_XY, nX, nY );
}

void get_xy( int *pX, int *pY )
{
	syscall_stub( SCTYPE_GET_CURSOR_XY, pX, pY );
}

int direct_disp_str( int nX, int nY, char *pS )
{
    int nR;

    if( pS == NULL || pS[0] == 0 )
        return( 0 );

    nR = syscall_stub( SCTYPE_DIRECT_DISPSTR, nX, nY, pS );
    return( nR );
}

// nX ���� nY ������ ������ �����.
int del_line( int nX, int nY )
{
	syscall_stub( SCTYPE_DEL_LINE, nX, nY );

	return( 0 );
}

static int insert_char( char *pS, int nI, int nChar )
{
    char    ch, ch_next;
    int     nLength, nX;

    nLength = strlen( pS );
    if( nI >= nLength )
    {   // ���� ���� �߰��ϸ� �ȴ�.
        pS[nI] = (char)nChar;
        pS[nI+1] = 0;
        return( nI +1 );
    }

    // �߰��� �����Ѵ�.
    ch = (char)nChar;
    for( nX = nI;; )
    {
        ch_next = pS[nX];
        pS[nX] = ch;
        if( ch == 0 )
            break;
        ch = ch_next;
        nX++;
    }

    return( nI+1 );
}

static int delete_char( char *pS, int nI )
{
    int nX;

    if( pS[nI] == 0 )
        return( 0 );

    for( nX = nI; ; nX++ )
    {
        pS[nX] = pS[nX+1];
        if( pS[nX] == 0 )
            break;    
    }

    return( 0 );
}

// '*'�� nLength�� ������ŭ ����Ѵ�.
static int print_star( int nLength )
{
	char	szT[260];

	memset( szT, '*', nLength );
	szT[nLength] = 0;
	printf( szT );
	
	return(0 );
}

//pStr�� ���ݱ��� �Էµ� ���ڿ�
int internal_input_str( char *pStr, int nSize, int nFlag )
{
	char	szT[260];
    int		nLineSize, nIPos, nDispX, nDispLen, nX, nY, nR, nChar, nLength;

    // Ŀ���� ���� ��ġ�� �˾Ƴ���.
    get_xy( &nX, &nY );

	nIPos = strlen( pStr );
	nLineSize = 79 - nX;
	nDispX = 0;

    for( ;; )
    {
		nLength = strlen( pStr );

		if( nDispX + nLength > nLineSize )
			nDispX = nLength - nLineSize;

		if( nIPos < nDispX )
			nDispX = nIPos;

		if( nIPos - nDispX > nLineSize )
			nDispX += nLineSize - (nIPos - nDispX);

        // ���ݱ����� ��Ʈ���� ����Ѵ�.
		set_xy( nX, nY );
		del_line( nX, nY ); 
		if( (int)strlen( &pStr[nDispX] ) > nLineSize )
		{
			memcpy( szT, &pStr[nDispX], nLineSize );
			szT[nLineSize] = 0;
			if( nFlag == 0 )
				printf( szT );
			else
				print_star( strlen( szT ) );
		}
		else
		{
			if( nFlag == 0 )
				printf( &pStr[nDispX] );
			else
				print_star( strlen( &pStr[nDispX] ) );
		}

        // ��µ� ���ڿ� ���� �÷����� ���� ������ �����. 
		nDispLen = strlen( &pStr[nDispX] );
		//if( nX+nDispLen < 79 )
		//	del_line( nX+nDispLen, nY ); 

        // Ŀ�� ��ġ�� �ٽ� ��´�.
		if( nIPos - nDispX > nLineSize )
			set_xy( nX + nLineSize, nY );
		else
			set_xy( nX + nIPos - nDispX , nY );

        // �� ���ڸ� �о���δ�.
        nChar = getch();
        if( nChar == 9 )
            nChar = ' ';              // TAB -> SPACE

        if( nChar == BK_DEL )
        {
            delete_char( pStr, nIPos );
        }
        else if( nChar == BK_LEFT )
        {
            if( nIPos > 0 )
                nIPos--;
        }
        else if( nChar == BK_RIGHT )
        {
            if( nIPos < nLength )
                nIPos++;            
        }
        else if( nChar == BK_HOME )
        {
            nIPos = 0;
        }
        else if( nChar == BK_END )
        {
            nIPos = nLength;            
        }
        else if( nChar == BK_UP )
        {

        }
        else if( nChar == BK_DOWN )
        {
        
        }
        else if( nChar == 13 )			// ENTER
        {  
            return( 0 );				// �Էµ� ���� ó���Ѵ�.
        }
        else if( nChar == 27 )			// ESC
        {
			pStr[0] = 0;
			nDispX  = 0;
			nIPos   = 0;
        }
        else if( nChar == 8 )			// BACKSPACE
        {
            if( nIPos > 0 )
            {
                delete_char( pStr, nIPos-1);
                nIPos--;
            }
        }
        else // ���ۿ� �߰��Ѵ�.
        {   
			if( nLength < nSize )
			{	// �Է¹��� �� �ִ� ������ �ִ� ��쿡�� �߰��Ѵ�.
	            nR = insert_char( pStr, nIPos, nChar );
		        nIPos++;
			}
        }
    }   

    return( 0 );
}

int input_str( char *pStr, int nSize )
{
	int nR;
	nR = internal_input_str( pStr, nSize, 0  );
	return( nR );
}

int input_pass( char *pStr, int nSize )
{
	int nR;
	nR = internal_input_str( pStr, nSize, 1  );
	return( nR );
}

