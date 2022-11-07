#ifndef BELLONA2_CRITICAL_SECTION_jj
#define BELLONA2_CRITICAL_SECTION_jj

struct CritSectTag {
    int         nInitialized;       // 초기화 되었는지의 여부.
    int         nFlag;              // FLAG 값( 1로 초기화 되어야 한다.)
    EventStt    e;                  // 이벤트 구조체를 이용한다(?)
};
typedef struct CritSectTag CritSectStt;

extern int enter_cs( CritSectStt *pCS );
extern int leave_cs( CritSectStt *pCS );

#endif