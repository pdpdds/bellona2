#include "bellona2.h"

int enter_cs( CritSectStt *pCS )
{
	// 초기화는 한 번만 하면 된다.
    if( init_cond_value( &pCS->nInitialized ) == 0 )
    {   // 초기화
        pCS->nFlag = 1;
        // 이벤트에는 이름만 설정한다.
        init_event( &pCS->e, "CS" );
    }

	for( ;; )
    {
		if( down_cond_value( &pCS->nFlag ) == 0 )
			break;
	    // 대기 상태로 들어간다.
		kdbg_printf( "Enter CS: wait\n" );
		wait_event( &pCS->e, 0 );  // time out = 0
    }   

    return( 0 );
}

int leave_cs( CritSectStt *pCS )
{
	// 플래그를 '1'로 설정한다.
    pCS->nFlag = 1;

    // 쓰레드 스위칭은 일어나지 않는다.
    inc_event_count( &pCS->e );

    return( 0 );
}
