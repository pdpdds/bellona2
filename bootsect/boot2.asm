
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
GOTO_BLOAD      DW  0, 0    ; Bload.com의 NOP연속된 부분. 주소
LOADING_MESG    DB  'Bellona2 Kernel',0Dh, 0Ah, 0
CRITICAL_MESG   DB  0Dh, 0Ah, 'Error!', 0
HI_MESG         DB  'Hi~', 0
KERN_IMG		DB  'BELLONA2IMG',0
DOT				DB  '.',0
TRACK			DB  0
HEAD			DB  0
SECTOR			DB  11  ; 0, 0, 11 -> FAT2를 가리킴
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
		    	INC BYTE PTR SECTOR			; 섹터번호 1 증가

		    		CMP BYTE PTR SECTOR, 19
		    		JB  END_RSECT
		    		MOV BYTE PTR SECTOR, 1	; HEAD 번호 1 증가
		    		INC BYTE PTR HEAD

		    		CMP BYTE PTR HEAD, 2
		    		JB  END_RSECT
	            	MOV BYTE PTR HEAD, 0
		    		INC BYTE PTR TRACK		; TRACK 번호 1 증가

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

				CMP AL, 'a'			; 대문자로 변경한다.
				JB  FK1				;
				CMP AL, 'z'			;
				JA  FK1				;
		    	SUB AL, 'a' - 'A'	;
		    	
FK1:			CMP AL, [DI]
				JNE FK_NOT_FOUND

				INC DI
				CMP BYTE PTR [DI], 0
				JNE CHK_FK

				; 시작 클러스터 번호를 리턴한다.
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
                MOV AX, 09000h				; 640K 아래쪽 64k
                MOV SS,AX
                MOV SP,0FFFEH
                STI

				; 로고를 출력한다.
				MOV SI,OFFSET LOADING_MESG
				CALL PRINT

				; 드라이브를 초기화한다.
				XOR AH,AH
				XOR DL,DL
				INT 13H

		;======================================;
		; FAT2, ROOT_DIRECTORY 를 읽어 들인다. ;
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
		; 루트 디렉토리에서 BELLONA2.IMG 를 찾는다.;
		;==========================================;
				PUSH CX
				MOV  CX, 256    ; 디렉토리 엔트리 최대 256개
				MOV  SI, 1200H + 200H ; 루트 디렉토리의 시작.
FIND_KERN_LOOP: CALL FIND_KERN
                CMP  AX, 0
                JNE  GOT_IT

                     ADD SI, 32  ; 다음 엔트리
                     LOOP FIND_KERN_LOOP
                     
                     JMP  CRITICAL_ERROR
        ;==========================================;
        ;       FAT CHAIN을 XXX에 구한다.          ;
        ;==========================================;
GOT_IT:			POP  CX		; FAT CHAIN (=AX)을 구해 둔다.
                XOR DI, DI
                MOV SI, 200H    ; FAT2 의 시작.

NEXT_CHAIN:     MOV SS:[DI], AX
                CMP AX, 0FF7H
                JAE READ_IMG

                ADD DI, 2
                MOV BX, 3	; AX 값을 가져온다.
                CLC
                RCR AX,1	; 2로 나눈다.
                JC  ODD

        		; 짝수 (3곱한다.)
				MUL BX
				MOV BX, AX
				MOV AX, [BX+SI]
				AND AX, 0FFFH
				JMP NEXT_CHAIN

ODD:			; 홀수 (3곱해서 1더한다.)
				MUL BX
				INC AX
				MOV BX, AX
				MOV AX, [BX+SI]
				SHR AX, 4
				JMP NEXT_CHAIN

		;============================;
		; BELLONA2.IMG를 읽어들인다. ;
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
				INC AH			; 섹터는 0이 아니라 1번부터 시작함.
				MOV SECTOR, AH

				CALL READ_SECTOR
				JNC  CHK_ADDR
		     	JMP CRITICAL_ERROR

CHK_ADDR: 		MOV BX,ES
				CMP  DWORD PTR GOTO_BLOAD, 0
				JNE  INC_ES2
			 	; 이건 안해줘도 될 것같은데...
			 	MOV DX, ES:[3]
			 	MOV DS:[3],DX
		     	; 5, 6, 7, 8 주소를 옮긴다.
		     	MOV EDX, ES:[5]
		     	MOV DWORD PTR GOTO_BLOAD, EDX
		     	JMP READ_IMG2		

INC_ES2:		ADD BX,20H
				JMP READ_IMG2

JMP_BLOAD:		; 화면을 지운다.
				;PUSH ES
				;MOV  DI,0b800h
				;MOV  ES,DI
				;MOV  DI,0
				;MOV  AX,0720h
				;MOV  CX,80*25
				;REP  STOSW
				;POP  ES
		
				; 원시적인 화면덤프,		
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

				; SS, SP는 시작할 때 설정한 9000:FFFEh를 이용한다.
				
				; BLOAD.COM으로 점프한다.
				CLI
				PUSH WORD PTR GOTO_BLOAD+2
				PUSH WORD PTR GOTO_BLOAD
				RETF

CRITICAL_ERROR:	; 오류메시지를 출력하고 무한루프로 들어간다.
				MOV SI,OFFSET CRITICAL_MESG

CRITICAL_ERROR2:
				CALL PRINT

INFINITE:		JMP INFINITE

ORG 510
        		DW 0AA55H

CODE    		ENDS
        		END START

