#ifndef BELLONA2_CRITICAL_SECTION_jj
#define BELLONA2_CRITICAL_SECTION_jj

struct CritSectTag {
    int         nInitialized;       // �ʱ�ȭ �Ǿ������� ����.
    int         nFlag;              // FLAG ��( 1�� �ʱ�ȭ �Ǿ�� �Ѵ�.)
    EventStt    e;                  // �̺�Ʈ ����ü�� �̿��Ѵ�(?)
};
typedef struct CritSectTag CritSectStt;

extern int enter_cs( CritSectStt *pCS );
extern int leave_cs( CritSectStt *pCS );

#endif