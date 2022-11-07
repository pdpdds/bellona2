#include "bellona2.h"

int enter_cs( CritSectStt *pCS )
{
	// �ʱ�ȭ�� �� ���� �ϸ� �ȴ�.
    if( init_cond_value( &pCS->nInitialized ) == 0 )
    {   // �ʱ�ȭ
        pCS->nFlag = 1;
        // �̺�Ʈ���� �̸��� �����Ѵ�.
        init_event( &pCS->e, "CS" );
    }

	for( ;; )
    {
		if( down_cond_value( &pCS->nFlag ) == 0 )
			break;
	    // ��� ���·� ����.
		kdbg_printf( "Enter CS: wait\n" );
		wait_event( &pCS->e, 0 );  // time out = 0
    }   

    return( 0 );
}

int leave_cs( CritSectStt *pCS )
{
	// �÷��׸� '1'�� �����Ѵ�.
    pCS->nFlag = 1;

    // ������ ����Ī�� �Ͼ�� �ʴ´�.
    inc_event_count( &pCS->e );

    return( 0 );
}
