
;Format of DOS directory entry:
;Offset	Size	Description	(Table 1007)
; 00h  8 BYTEs	blank-padded filename
; 08h  3 BYTEs	blank-padded file extension
; 0Bh	 BYTE	attributes
; 0Ch 10 BYTEs	(MS-DOS 1.0-6.22) reserved
;		(DR-DOS) used to store file password
;		(MS-DOS 7/Windows95) additional file times (see #1008)
; 16h	WORD	time of creation or last update (see #1317 at AX=5700h)
; 18h	WORD	date of creation or last update (see #1318 at AX=5700h)
; 1Ah	WORD	starting cluster number (see also AX=440Dh/CX=0871h)
; 1Ch	DWORD	file size

;================================================;
CODE            SEGMENT
                ASSUME CS:CODE, DS:CODE
                ORG 0H
.386

START:          JMP XXX
				ORG 3H
;================================================;
BOOT_DATA 		DB 04Dh, 053h, 044h, 04Fh, 053h, 035h, 02Eh, 030h, 000h, 002h, 001h, 001h, 000h
             	DB 002h, 0E0h, 000h, 040h, 00Bh, 0F0h, 009h, 000h, 012h, 000h, 002h, 000h, 000h, 000h, 000h
GOTO_BLOAD      DW  0, 0    ; Bload.com�� NOP���ӵ� �κ�. �ּ�
LOADING_MESG    DB  'Bellona2 Kernel',0Dh, 0Ah, 0
CRITICAL_MESG   DB  0Dh, 0Ah, 'Error!', 0
HI_MESG         DB  'Hi~', 0
KERN_IMG		DB  'BELLONA2IMG',0
DOT				DB  '.',0
TRACK			DB  0
HEAD			DB  0
SECTOR			DB  11  ; 0, 0, 11 -> FAT2�� ����Ŵ
;-------------------------------------------;
PRINT			PROC NEAR
				PUSHA
				MOV  AH,0EH
				MOV  BX,14
NEXT_PRINT:		LODSB
				CMP  AL,0
				JE   END_PRINT
		    	INT  10H
		    	JMP  NEXT_PRINT

END_PRINT:		POPA
				RET
PRINT			ENDP
;-------------------------------------------;
PROGRESS		PROC	NEAR
				PUSHA
				MOV   SI,OFFSET DOT
				CALL  PRINT
				POPA
				RET
PROGRESS		ENDP
;-------------------------------------------;
READ_SECTOR		PROC	NEAR
				PUSHA

				MOV AX,0201H
				MOV CH,TRACK
				MOV CL,SECTOR
				MOV DH,HEAD
				MOV DL,0
				INT 13H
				PUSHF
		    	INC BYTE PTR SECTOR			; ���͹�ȣ 1 ����

		    		CMP BYTE PTR SECTOR, 19
		    		JB  END_RSECT
		    		MOV BYTE PTR SECTOR, 1	; HEAD ��ȣ 1 ����
		    		INC BYTE PTR HEAD

		    		CMP BYTE PTR HEAD, 2
		    		JB  END_RSECT
	            	MOV BYTE PTR HEAD, 0
		    		INC BYTE PTR TRACK		; TRACK ��ȣ 1 ����

END_RSECT:     	CALL PROGRESS

				POPF
           		POPA
				RET
READ_SECTOR		ENDP
;-------------------------------------------;
FIND_KERN  		PROC NEAR
				PUSH SI

				MOV DI, OFFSET KERN_IMG

CHK_FK:			LODSB

				CMP AL, 'a'			; �빮�ڷ� �����Ѵ�.
				JB  FK1				;
				CMP AL, 'z'			;
				JA  FK1				;
		    	SUB AL, 'a' - 'A'	;
		    	
FK1:			CMP AL, [DI]
				JNE FK_NOT_FOUND

				INC DI
				CMP BYTE PTR [DI], 0
				JNE CHK_FK

				; ���� Ŭ������ ��ȣ�� �����Ѵ�.
				POP SI
				MOV AX, [SI+1AH]
				RETN

FK_NOT_FOUND:	XOR  AX,AX
				POP  SI
				RETN
FIND_KERN  		ENDP
;-------------------------------------------;
XXX:            CLI
				CLD
				MOV AX,07C0H
                MOV DS,AX
                MOV AX, 09000h				; 640K �Ʒ��� 64k
                MOV SS,AX
                MOV SP,0FFFEH
                STI

				; �ΰ� ����Ѵ�.
				MOV SI,OFFSET LOADING_MESG
				CALL PRINT

				; ����̺긦 �ʱ�ȭ�Ѵ�.
				XOR AH,AH
				XOR DL,DL
				INT 13H

		;======================================;
		; FAT2, ROOT_DIRECTORY �� �о� ���δ�. ;
		;======================================;
				MOV CX,9 + 16     ; FAT2 + ROOT DIR
				MOV BX,07E0H
READ_NEXT:		MOV ES,BX
				XOR BX,BX

				CALL READ_SECTOR
				JNC  INC_ES
		     	JMP CRITICAL_ERROR
INC_ES:
				MOV  BX,ES
				ADD  BX,20H
				LOOP READ_NEXT
		
		;==========================================;
		; ��Ʈ ���丮���� BELLONA2.IMG �� ã�´�.;
		;==========================================;
				PUSH CX
				MOV  CX, 256    ; ���丮 ��Ʈ�� �ִ� 256��
				MOV  SI, 1200H + 200H ; ��Ʈ ���丮�� ����.
FIND_KERN_LOOP: CALL FIND_KERN
                CMP  AX, 0
                JNE  GOT_IT

                     ADD SI, 32  ; ���� ��Ʈ��
                     LOOP FIND_KERN_LOOP
                     
                     JMP  CRITICAL_ERROR
        ;==========================================;
        ;       FAT CHAIN�� XXX�� ���Ѵ�.          ;
        ;==========================================;
GOT_IT:			POP  CX		; FAT CHAIN (=AX)�� ���� �д�.
                XOR DI, DI
                MOV SI, 200H    ; FAT2 �� ����.

NEXT_CHAIN:     MOV SS:[DI], AX
                CMP AX, 0FF7H
                JAE READ_IMG

                ADD DI, 2
                MOV BX, 3	; AX ���� �����´�.
                CLC
                RCR AX,1	; 2�� ������.
                JC  ODD

        		; ¦�� (3���Ѵ�.)
				MUL BX
				MOV BX, AX
				MOV AX, [BX+SI]
				AND AX, 0FFFH
				JMP NEXT_CHAIN

ODD:			; Ȧ�� (3���ؼ� 1���Ѵ�.)
				MUL BX
				INC AX
				MOV BX, AX
				MOV AX, [BX+SI]
				SHR AX, 4
				JMP NEXT_CHAIN

		;============================;
		; BELLONA2.IMG�� �о���δ�. ;
		;============================;
READ_IMG:		XOR SI, SI
				MOV BX,07E0H

READ_IMG2:		MOV ES,BX
				XOR BX,BX

				MOV AX, SS:[SI]
				ADD SI,2
				CMP AX, 0FF7H
				JAE JMP_BLOAD

				ADD AX, 31		; Loical 2 = Physical 36
				MOV CH, 36
				DIV CH
				MOV TRACK, AL

				SHR AX, 8		; AH -> AL
				MOV CH, 18
				DIV CH
				MOV HEAD,   AL
				INC AH			; ���ʹ� 0�� �ƴ϶� 1������ ������.
				MOV SECTOR, AH

				CALL READ_SECTOR
				JNC  CHK_ADDR
		     	JMP CRITICAL_ERROR

CHK_ADDR: 		MOV BX,ES
				CMP  DWORD PTR GOTO_BLOAD, 0
				JNE  INC_ES2
			 	; �̰� �����൵ �� �Ͱ�����...
			 	MOV DX, ES:[3]
			 	MOV DS:[3],DX
		     	; 5, 6, 7, 8 �ּҸ� �ű��.
		     	MOV EDX, ES:[5]
		     	MOV DWORD PTR GOTO_BLOAD, EDX
		     	JMP READ_IMG2		

INC_ES2:		ADD BX,20H
				JMP READ_IMG2

JMP_BLOAD:		; ȭ���� �����.
				;PUSH ES
				;MOV  DI,0b800h
				;MOV  ES,DI
				;MOV  DI,0
				;MOV  AX,0720h
				;MOV  CX,80*25
				;REP  STOSW
				;POP  ES
		
				; �������� ȭ�����,		
				;KKK:	MOV DI, 0b800h
				;		MOV ES, DI
				;		XOR DI, DI
				;		LDS SI, GOTO_BLOAD
				;		MOV CX, 25*80
				;		MOV AH, 7
				;KKK2:	LODSB
				;		STOSW
				;		LOOP KKK2
				;LLL:	JMP LLL

				; SS, SP�� ������ �� ������ 9000:FFFEh�� �̿��Ѵ�.
				
				; BLOAD.COM���� �����Ѵ�.
				CLI
				PUSH WORD PTR GOTO_BLOAD+2
				PUSH WORD PTR GOTO_BLOAD
				RETF

CRITICAL_ERROR:	; �����޽����� ����ϰ� ���ѷ����� ����.
				MOV SI,OFFSET CRITICAL_MESG

CRITICAL_ERROR2:
				CALL PRINT

INFINITE:		JMP INFINITE

ORG 510
        		DW 0AA55H

CODE    		ENDS
        		END START

