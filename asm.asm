;===============================================================================;
;									ASM.ASM										;
;===============================================================================;
;  VC++에서는 SGDT, DB 예약어를 사용할 수 없으므로 어셈블리 언어를 이용하여     ;
;  OBJ형태로 링크해 주어야 한다.                                                ;
;  Parameter는 C에서 fn(A,B,C)로 콜을 하면 C,B,A순으로 스택에 푸쉬되고 CALL이   ;
;  수행된다.                                                                    ;
;  MASM 6.14로 어셈블 하면 OMF포맷(?)의 OBJ가 생성된다.  이것을 바로 링크하면	;
;  경고가 발생하므로 LIB로 변환하여 링크한다.                                   ;
;===============================================================================;
   
.386P
.387
.model flat

DMA_PICU1       EQU 0020h
DMA_PICU2       EQU 00A0h

.DATA

.CODE 

;===================================;
_vAsmInitFPU Proc NEAR				;
			 Public _vAsmInitFPU	;
;===================================;		   
			 FWAIT
			 FSETPM
			 FWAIT
			 FNINIT

			 RET
_vAsmInitFPU Endp

;===================================;
_vResetGDT Proc NEAR				;
           Public _vResetGDT		;
;===================================;		   
		   PUSH EBP
		   MOV  EBP,ESP
		   PUSHAD
		   PUSHFD

		   ; [EBP]       <- EBP
		   ; [EBP+4] 	 <- RET Addr
		   ; [EBP+8]     <- bell.gdt
		   ; [EBP+12]    <- bell.gdtr
		   ; [EBP+16]    <- 새 gdt의 크기

		   ; 이전 GDTR을 구해 본다.
		   MOV  EAX,[EBP+12]
		   SGDT	FWORD PTR [EAX]

		   ; 새 GDT로 엔트리 들을 복사한다.
		   MOV EDI,[EBP+8]
		   MOV ESI,[EAX+2]	 ; <- bell.gdtr.dwAddr
		   XOR ECX,ECX
		   MOV CX, [EAX]	 ; <- bell.gdtr.wSize
		   
		   REP MOVSB  ; 엔트리 들은 다 옮겼다.

		   ; 새 위치와 크기를 설정하고 나가면 된다.
		   MOV EDI,[EBP+8]
		   MOV [EAX+2],EDI	 ; 새 gdt 베이스를 설정한다.
		   MOV EDX,[EBP+16]  ; 새 gdt의 크기
		   MOV [EAX],DX
		   LGDT FWORD PTR[EAX]

		   POPFD
		   POPAD
		   POP  EBP
           retn
_vResetGDT Endp

;***************************************************;
_vSendEOI  PROC NEAR								;
           public _vSendEOI			 				;
;***************************************************;
                 PUSH EBP
				 MOV  EBP, ESP
				 PUSH EAX

			    ; [EBP]        <- EBP
			    ; [EBP+4]  	   <- RET Addr
				; [EBP+8]	   <- IRQ 번호
		 
                 MOV AL,20H				        ; EOI Signal을 보낸다.
                 OUT DMA_PICU1,AL
                 
				 CMP BYTE PTR [EBP+8],7
				 JBE END_OF_EOI
					 OUT DMA_PICU2,AL            ;Send to 2 also

END_OF_EOI:		 POP EAX
				 POP EBP
				 RET
_vSendEOI        ENDP                
;===================================================;
_vReprogramPIC Proc NEAR							;
			   public _vReprogramPIC				;
;===================================================;
			PUSH EBP
			MOV EBP,ESP
			PUSH EAX
			PUSH ESI
			PUSH EDX

		   ; [EBP]        <- EBP
		   ; [EBP+4] 	  <- RET Addr
		   ; [EBP+8]      <- 8259 초기화 데이타

			CLD
            MOV ESI,[EBP+8]        ; REPROGRAMMING 8259 PIC
			XOR DX,DX
SET_PIC:    LODSB                  ; IRQ0 = 20H
            CMP AL,0               ; IRQ8 = 28H
            JE  END_PIC_SET
                MOV DL,AL
                LODSB
                OUT	DX,AL
                JMP SET_PIC
END_PIC_SET:

			
            ;MOV AL,11111010b                        ;Masked all but cascade
			MOV  AL, 0
            OUT  DMA_PICU1+1,AL                  ;MASK - MASTER (0= Ints ON)
            PUSH EAX
            POP  EAX
            OUT  DMA_PICU2+1,AL                  ;MASK - SLAVE

			POP EDX
			POP ESI
			POP EAX
			POP EBP
			RET
_vReprogramPIC Endp	 
;===================================================;
_vReadPort		Proc NEAR							;  // 포트에서 한바이트를 읽는다.
				public _vReadPort     				;
;===================================================;
				PUSH  EBP
				MOV   EBP,ESP
				PUSH  EAX
				PUSH  EDX

			    ; [EBP]        <- EBP
			    ; [EBP+4] 	   <- RET Addr
			    ; [EBP+8]      <- 포트 번호
			    ; [EBP+12]	   <- 읽은 바이트를 집어넣을 포인터 

				MOV EDX,[EBP+8]

				IN  AL,DX			; 포트에서 값을 읽는다.
			    MOV EDX,[EBP+12]
				CMP EDX,0
				JZ  GOBACK0			; NULL이면 저장하지 않는다.
					MOV [EDX],AL	; 읽은 데이타를 저장한다.

	GOBACK0:	POP EDX
				POP EAX
				POP EBP
				RET
_vReadPort		Endp
;===================================================;
_vReadPortWord  Proc NEAR							;  // 포트에서 두바이트를 읽는다.
				public _vReadPortWord  				;
;===================================================;
				PUSH  EBP
				MOV   EBP,ESP
				PUSH  EAX
				PUSH  EDX

			    ; [EBP]        <- EBP
			    ; [EBP+4] 	   <- RET Addr
			    ; [EBP+8]      <- 포트 번호
			    ; [EBP+12]	   <- 읽은 워드를 집어넣을 포인터 

				MOV EDX,[EBP+8]

				IN  AX,DX			; 포트에서 값을 읽는다.
			    	MOV EDX,[EBP+12]
				MOV [EDX],AX		; 읽은 데이타를 저장한다.

				POP EDX
				POP EAX
				POP EBP
				RET
_vReadPortWord	Endp
;===================================================;
_vReadPortMultiWord  Proc NEAR							;  // 포트에서 두바이트를 읽는다.
				public _vReadPortMultiWord  				;
;===================================================;
				PUSH  EBP
				MOV   EBP,ESP
				PUSH  EAX
				PUSH  EBX
				PUSH  ECX
				PUSH  EDX

			    ; [EBP]        <- EBP
			    ; [EBP+4] 	   <- RET Addr
			    ; [EBP+8]      <- 포트 번호
			    ; [EBP+12]	   <- 읽은 워드를 집어넣을 포인터 
				; [EBP+16]     <- 읽을 워드 수.

				MOV EDX, [EBP+8]
				MOV EBX, [EBP+12]
				MOV ECX, [EBP+16]
NEXT:
				IN   AX,DX			; 포트에서 값을 읽는다.
				MOV  [EBX],AX
				INC  EBX
				INC  EBX
				LOOP NEXT

				POP EDX
				POP ECX
				POP EBX
				POP EAX
				POP EBP
				RETN
_vReadPortMultiWord	Endp
;===================================================;
_vReadPortDword  Proc NEAR							;  // 포트에서 두바이트를 읽는다.
				public _vReadPortDword  				;
;===================================================;
				PUSH  EBP
				MOV   EBP,ESP
				PUSH  EAX
				PUSH  EDX

			    ; [EBP]        <- EBP
			    ; [EBP+4] 	   <- RET Addr
			    ; [EBP+8]      <- 포트 번호
			    ; [EBP+12]	   <- 읽은 워드를 집어넣을 포인터 

				MOV EDX,[EBP+8]

				IN  EAX,DX			; 포트에서 값을 읽는다.
			    MOV EDX,[EBP+12]
				MOV [EDX],EAX		; 읽은 데이타를 저장한다.

				POP EDX
				POP EAX
				POP EBP
				RET
_vReadPortDword	Endp
;===================================================;
_vWritePort		Proc NEAR							;  // 포트에 한바이트를 기록한다.
				public _vWritePort     				;
;===================================================;
				PUSH  EBP
				MOV   EBP,ESP
				PUSH  EAX
				PUSH  EDX

			    ; [EBP]        <- EBP
			    ; [EBP+4] 	   <- RET Addr
			    ; [EBP+8]      <- 포트 번호
			    ; [EBP+12]	   <- 기록할 바이트
				
				MOV EDX,[EBP+8]
				MOV AL, [EBP+12]
                OUT DX,AL           ; 포트에 값을 쓴다.

				POP EDX
				POP EAX
				POP EBP
				RET
_vWritePort		Endp
;===================================================;
_vWritePortWord Proc NEAR							;  // 포트에 2바이트를 기록한다.
				public _vWritePortWord				;
;===================================================;
				 PUSH  EBP
				 MOV   EBP,ESP
				 PUSH  EAX
				 PUSH  EDX

			     ; [EBP]        <- EBP
			     ; [EBP+4] 	    <- RET Addr
			     ; [EBP+8]      <- 포트 번호
			     ; [EBP+12]	    <- 기록할 바이트
				
				 MOV EDX,[EBP+8]
				 MOV EAX, [EBP+12]
                 OUT DX,AX    ; 포트에 값을 쓴다.

				 POP EDX
				 POP EAX
				 POP EBP
				 RET
_vWritePortWord  Endp
;===================================================;
_vWritePortDword Proc NEAR							;  // 포트에 4바이트를 기록한다.
				 public _vWritePortDword			;
;===================================================;
				 PUSH  EBP
				 MOV   EBP,ESP
				 PUSH  EAX
				 PUSH  EDX

			     ; [EBP]        <- EBP
			     ; [EBP+4] 	    <- RET Addr
			     ; [EBP+8]      <- 포트 번호
			     ; [EBP+12]	    <- 기록할 바이트
				
				 MOV EDX,[EBP+8]
				 MOV EAX,[EBP+12]
                 OUT DX,EAX    ; 포트에 값을 쓴다.

				 POP EDX
				 POP EAX
				 POP EBP
				 RET
_vWritePortDword Endp
;===================================================;
WaitInBufferEmpty	Proc	near					;
;===================================================;
				PUSH   EAX
				PUSH   ECX

				MOV    ECX,0FFFFFh   ; 무한루프에 빠지지 않도록 한다.
WAIT_IN:        IN     AL,64H	     ; Write 가능한 상태인지 검사한다.
                TEST   AL,2
                LOOPNZ WAIT_IN
				
				POP    ECX
				POP    EAX
				RET	   
WaitInBufferEmpty Endp
;===================================================;
WaitOutBufferFull Proc	near						;
;===================================================;
				PUSH   EAX
				PUSH   ECX

				MOV    ECX,0FFFFFh   ; 무한루프에 빠지지 않도록 한다.
WAIT_OUT:       IN     AL,64H	     ; Write 가능한 상태인지 검사한다.
                TEST   AL,1
                LOOPZ  WAIT_OUT
				
				POP    ECX
				POP    EAX
				RET	   
WaitOutBufferFull Endp
;===================================================;
_MaskIRQ		Proc	near						;
				public _MaskIRQ						;
;===================================================;
                PUSH EBP
                MOV  EBP, ESP
                PUSHFD
				PUSHAD
                CLI

			    ; [EBP]        <- EBP
			    ; [EBP+4]  	   <- RET Addr
			    ; [EBP+8]      <- IRQ Number  to Mask

                MOV EAX, 1
                MOV ECX, [EBP+8]
                AND ECX, 0Fh
                SHL EAX, CL
                AND AL,AL
                JZ MIRQ2	 
	                MOV  AH,AL			; DMA1 쪽
		            IN   AL, DMA_PICU1+1
			        PUSH EAX
				    POP  EAX
					OR   AL, AH
	                OUT  DMA_PICU1+1, AL
		            JMP  SHORT MIRQEnd

MIRQ2:          IN   AL, DMA_PICU2+1
                PUSH EAX
                POP  EAX
                OR   AL, AH
                OUT  DMA_PICU2+1, AL

MIRQEnd:        POPAD
				POPFD
                POP  EBP
                RET        
_MaskIRQ		Endp
;===================================================;
_UnMaskIRQ		Proc	near						;
				public _UnMaskIRQ					;
;===================================================;
                PUSH EBP
                MOV  EBP, ESP
                PUSHFD
				PUSHAD
                CLI

			    ; [EBP]        <- EBP
			    ; [EBP+4]  	   <- RET Addr
			    ; [EBP+8]      <- IRQ Number  to Mask

                MOV EAX,1
                MOV ECX,[EBP+08h]
                AND ECX, 0Fh
                SHL EAX, CL
                AND AL,AL
                JZ  UMIRQ2
	                MOV  AH, AL
		            IN   AL, DMA_PICU1+1
			        PUSH EAX
				    POP  EAX
					NOT  AH
	                AND  AL, AH
		            OUT  DMA_PICU1+1, AL
			        JMP  SHORT UMIRQEnd

UMIRQ2:         IN AL, DMA_PICU2+1
                PUSH EAX
                POP  EAX
                NOT  AH
                AND  AL, AH
                OUT  DMA_PICU2+1, AL

UMIRQEnd:       POPAD
				POPFD
                POP  EBP
                RET 
_UnMaskIRQ		Endp				       
;===================================================;
_vAsmSetKBDLed	Proc NEAR							;  // 키보드 발광다이오드를 설정한다.
				public _vAsmSetKBDLed  				;
;===================================================;
				PUSH  EBP
				MOV   EBP,ESP

			    ; [EBP]        <- EBP
			    ; [EBP+4]  	   <- RET Addr
			    ; [EBP+8]      <- LED Mask Bit			B2-Caps, B1-NUM, B0-Scroll

				MOV  EAX,1					; IRQ1을 UnMASK한다.
				PUSH EAX
				CALL _MaskIRQ
				ADD  ESP,4

                CALL WaitInBufferEmpty        
                MOV  AL,0EDh                ; LED 변경명령 
                OUT  60h,AL                 

                CALL WaitOutBufferFull      ; 응답을 무시한다.
                IN   AL,64h

                CALL WaitInBufferEmpty      ; 입력버퍼가 비기를 기다린다.
				MOV  EAX,[EBP+8]
                OUT  60h,AL                 ; 새로운 LED값을 보낸다.

                CALL WaitOutBufferFull      ; 응답을 무시한다.
                IN   AL,64h										
				
				MOV  EAX,1					; IRQ1을 UnMASK한다.
				PUSH EAX
				CALL _UnMaskIRQ
				ADD  ESP,4

                POP  EBP
				RET
_vAsmSetKBDLed  Endp
;===================================================;
_vRebootSystem	Proc								;
				public _vRebootSystem				;
;===================================================;
				PUSH EAX
                CALL WaitInBufferEmpty        
                MOV  AL,0FEh                ; 시스템 리부트
                OUT  64h,AL                 
				POP  EAX
				RET
_vRebootSystem	Endp

;===================================================;
_vEnableInterrupt  Proc NEAR		                ;
				   Public _vEnableInterrupt         ;
;===================================================;		   
		   PUSH EBP
		   MOV  EBP,ESP
		   PUSHAD

		   ; [EBP]        <- EBP
		   ; [EBP+4] 	  <- RET Addr
		   ; [EBP+8]      <- bell.idtr

		   MOV  EAX,[EBP+8]
		   LIDT	FWORD PTR [EAX]

		   STI           ;  인터럽트를 바로 Enable시켜 버린다.

		   POPAD
		   POP  EBP
           retn
_vEnableInterrupt  Endp

;===================================================;
_vSetTimerInterval	Proc NEAR						;
				public _vSetTimerInterval 			;
;===================================================;
				PUSH  EBP
				MOV   EBP,ESP
				PUSH  EAX

			    ; [EBP]        <- EBP
			    ; [EBP+4]  	   <- RET Addr
			    ; [EBP+8]      <- Interval

			    MOV  AL,00110100b
				OUT  43h,AL

				MOV  AX,[EBP+8]
				OUT  40h,AL
				MOV  AL,AH
				OUT  40h,AL

				POP  EAX
				POP  EBP
				RETN
_vSetTimerInterval	Endp
;*****************************************************************************;
END


