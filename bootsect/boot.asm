
CODE            SEGMENT
                ASSUME CS:CODE, DS:CODE
                ORG 0H
.386

START:          JMP XXX
		ORG 3H
SECTOR_TO_READ  DW  0		; �о���� ���ͼ�	
GOTO_BLOAD      DW  0, 0    ; Bload.com�� NOP���ӵ� �κ�. �ּ�
LOADING_MESG    DB  'Loading Bellona2 Kernel',0Dh, 0Ah, 0
CRITICAL_MESG   DB  'Critical error occurred. system halted.', 0Dh, 0Ah, 0
DOT		DB  '.',0
TRACK		DB  0
HEAD		DB  0
SECTOR		DB  2
;-------------------------------------------;
PRINT		PROC NEAR
		PUSHA
		MOV AH,0EH
		MOV BX,14
NEXT_PRINT:	LODSB
		CMP AL,0
		JE  END_PRINT
		    INT 10H
		    JMP NEXT_PRINT

END_PRINT:	POPA
		RET
PRINT		ENDP
;-------------------------------------------;
PROGRESS	PROC	NEAR
		PUSHA
		MOV SI,OFFSET DOT
		CALL PRINT		
		POPA
		RET
PROGRESS	ENDP
;-------------------------------------------;
READ_SECTOR	PROC	NEAR
		PUSHA
		
		MOV AX,0201H
		MOV CH,TRACK
		MOV CL,SECTOR
		MOV DH,HEAD
		MOV DL,0
		INT 13H
		PUSHF
			INC BYTE PTR SECTOR		; ���͹�ȣ 1 ����
			CMP BYTE PTR SECTOR, 19
			JB  END_RSECT 
			    MOV BYTE PTR SECTOR, 1	; HEAD ��ȣ 1 ����
			    INC BYTE PTR HEAD
			    CMP BYTE PTR HEAD, 2
			    JB  END_RSECT
				MOV BYTE PTR HEAD, 0
				INC BYTE PTR TRACK	; TRACK ��ȣ 1 ����
				;.... ���
				CALL PROGRESS
		
END_RSECT:	POPF
           	POPA
		RET
READ_SECTOR	ENDP
;-------------------------------------------;
XXX:    CLI
		MOV AX,07C0H
        MOV DS,AX
        MOV ES,AX
		
		; �ΰ� ����Ѵ�.
		MOV SI,OFFSET LOADING_MESG
		CALL PRINT

		; ����̺긦 �ʱ�ȭ�Ѵ�.
		XOR AH,AH
		XOR DL,DL
		INT 13H

		; ������ ���鼭 ���͵��� �о���δ�.
		PUSH ES
		MOV CX,SECTOR_TO_READ
		MOV BX,07E0H
READ_NEXT:	MOV ES,BX
		XOR BX,BX

		CALL READ_SECTOR
		JC   NEXT
		     MOV BX,ES
		     ADD BX,20H
		     LOOP READ_NEXT
NEXT:
		POP ES

		; ȭ���� �����.
		PUSH ES
		MOV  DI,0b800h
		MOV  ES,DI
		MOV  DI,0
		MOV  AX,0720h
		MOV  CX,80*25
		REP  STOSW
		POP  ES

		; SS, SP�� �����Ѵ�.
		MOV AX,09000h
		MOV SS,AX
		MOV AX,0FFFEh
		MOV SP,AX

		; BLOAD.COM���� �����Ѵ�.
		PUSH WORD PTR GOTO_BLOAD+2
		PUSH WORD PTR GOTO_BLOAD
		RETF

		;mov si,0b800h
		;mov es,si
		;mov si,0a80h	
		;mov ds,si
		;xor si,si
		;xor di,di
		;mov cx,2
		;mov ah,15
		;lodsb
		;stosw
		;lodsb
		;stosw
		;HLT
		
CRITICAL_ERROR:	MOV SI,OFFSET CRITICAL_MESG	; �����޽����� ����ϰ� ���ѷ����� ����.
		CALL PRINT

INFINITE:	JMP INFINITE

ORG 510
        DW 0AA55H

CODE    ENDS
        END START

