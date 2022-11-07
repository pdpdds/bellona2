;===============================================================================;
;									ASM.ASM										;
;===============================================================================;
;  VC++������ SGDT, DB ���� ����� �� �����Ƿ� ����� �� �̿��Ͽ�     ;
;  OBJ���·� ��ũ�� �־�� �Ѵ�.                                                ;
;  Parameter�� C���� fn(A,B,C)�� ���� �ϸ� C,B,A������ ���ÿ� Ǫ���ǰ� CALL��   ;
;  ����ȴ�.                                                                    ;
;  MASM 6.14�� ����� �ϸ� OMF����(?)�� OBJ�� �����ȴ�.  �̰��� �ٷ� ��ũ�ϸ�	;
;  ��� �߻��ϹǷ� LIB�� ��ȯ�Ͽ� ��ũ�Ѵ�.                                   ;
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
		   ; [EBP+16]    <- �� gdt�� ũ��

		   ; ���� GDTR�� ���� ����.
		   MOV  EAX,[EBP+12]
		   SGDT	FWORD PTR [EAX]

		   ; �� GDT�� ��Ʈ�� ���� �����Ѵ�.
		   MOV EDI,[EBP+8]
		   MOV ESI,[EAX+2]	 ; <- bell.gdtr.dwAddr
		   XOR ECX,ECX
		   MOV CX, [EAX]	 ; <- bell.gdtr.wSize
		   
		   REP MOVSB  ; ��Ʈ�� ���� �� �Ű��.

		   ; �� ��ġ�� ũ�⸦ �����ϰ� ������ �ȴ�.
		   MOV EDI,[EBP+8]
		   MOV [EAX+2],EDI	 ; �� gdt ���̽��� �����Ѵ�.
		   MOV EDX,[EBP+16]  ; �� gdt�� ũ��
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
				; [EBP+8]	   <- IRQ ��ȣ
		 
                 MOV AL,20H				        ; EOI Signal�� ������.
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
		   ; [EBP+8]      <- 8259 �ʱ�ȭ ����Ÿ

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
_vReadPort		Proc NEAR							;  // ��Ʈ���� �ѹ���Ʈ�� �д´�.
				public _vReadPort     				;
;===================================================;
				PUSH  EBP
				MOV   EBP,ESP
				PUSH  EAX
				PUSH  EDX

			    ; [EBP]        <- EBP
			    ; [EBP+4] 	   <- RET Addr
			    ; [EBP+8]      <- ��Ʈ ��ȣ
			    ; [EBP+12]	   <- ���� ����Ʈ�� ������� ������ 

				MOV EDX,[EBP+8]

				IN  AL,DX			; ��Ʈ���� ���� �д´�.
			    MOV EDX,[EBP+12]
				CMP EDX,0
				JZ  GOBACK0			; NULL�̸� �������� �ʴ´�.
					MOV [EDX],AL	; ���� ����Ÿ�� �����Ѵ�.

	GOBACK0:	POP EDX
				POP EAX
				POP EBP
				RET
_vReadPort		Endp
;===================================================;
_vReadPortWord  Proc NEAR							;  // ��Ʈ���� �ι���Ʈ�� �д´�.
				public _vReadPortWord  				;
;===================================================;
				PUSH  EBP
				MOV   EBP,ESP
				PUSH  EAX
				PUSH  EDX

			    ; [EBP]        <- EBP
			    ; [EBP+4] 	   <- RET Addr
			    ; [EBP+8]      <- ��Ʈ ��ȣ
			    ; [EBP+12]	   <- ���� ���带 ������� ������ 

				MOV EDX,[EBP+8]

				IN  AX,DX			; ��Ʈ���� ���� �д´�.
			    	MOV EDX,[EBP+12]
				MOV [EDX],AX		; ���� ����Ÿ�� �����Ѵ�.

				POP EDX
				POP EAX
				POP EBP
				RET
_vReadPortWord	Endp
;===================================================;
_vReadPortMultiWord  Proc NEAR							;  // ��Ʈ���� �ι���Ʈ�� �д´�.
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
			    ; [EBP+8]      <- ��Ʈ ��ȣ
			    ; [EBP+12]	   <- ���� ���带 ������� ������ 
				; [EBP+16]     <- ���� ���� ��.

				MOV EDX, [EBP+8]
				MOV EBX, [EBP+12]
				MOV ECX, [EBP+16]
NEXT:
				IN   AX,DX			; ��Ʈ���� ���� �д´�.
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
_vReadPortDword  Proc NEAR							;  // ��Ʈ���� �ι���Ʈ�� �д´�.
				public _vReadPortDword  				;
;===================================================;
				PUSH  EBP
				MOV   EBP,ESP
				PUSH  EAX
				PUSH  EDX

			    ; [EBP]        <- EBP
			    ; [EBP+4] 	   <- RET Addr
			    ; [EBP+8]      <- ��Ʈ ��ȣ
			    ; [EBP+12]	   <- ���� ���带 ������� ������ 

				MOV EDX,[EBP+8]

				IN  EAX,DX			; ��Ʈ���� ���� �д´�.
			    MOV EDX,[EBP+12]
				MOV [EDX],EAX		; ���� ����Ÿ�� �����Ѵ�.

				POP EDX
				POP EAX
				POP EBP
				RET
_vReadPortDword	Endp
;===================================================;
_vWritePort		Proc NEAR							;  // ��Ʈ�� �ѹ���Ʈ�� ����Ѵ�.
				public _vWritePort     				;
;===================================================;
				PUSH  EBP
				MOV   EBP,ESP
				PUSH  EAX
				PUSH  EDX

			    ; [EBP]        <- EBP
			    ; [EBP+4] 	   <- RET Addr
			    ; [EBP+8]      <- ��Ʈ ��ȣ
			    ; [EBP+12]	   <- ����� ����Ʈ
				
				MOV EDX,[EBP+8]
				MOV AL, [EBP+12]
                OUT DX,AL           ; ��Ʈ�� ���� ����.

				POP EDX
				POP EAX
				POP EBP
				RET
_vWritePort		Endp
;===================================================;
_vWritePortWord Proc NEAR							;  // ��Ʈ�� 2����Ʈ�� ����Ѵ�.
				public _vWritePortWord				;
;===================================================;
				 PUSH  EBP
				 MOV   EBP,ESP
				 PUSH  EAX
				 PUSH  EDX

			     ; [EBP]        <- EBP
			     ; [EBP+4] 	    <- RET Addr
			     ; [EBP+8]      <- ��Ʈ ��ȣ
			     ; [EBP+12]	    <- ����� ����Ʈ
				
				 MOV EDX,[EBP+8]
				 MOV EAX, [EBP+12]
                 OUT DX,AX    ; ��Ʈ�� ���� ����.

				 POP EDX
				 POP EAX
				 POP EBP
				 RET
_vWritePortWord  Endp
;===================================================;
_vWritePortDword Proc NEAR							;  // ��Ʈ�� 4����Ʈ�� ����Ѵ�.
				 public _vWritePortDword			;
;===================================================;
				 PUSH  EBP
				 MOV   EBP,ESP
				 PUSH  EAX
				 PUSH  EDX

			     ; [EBP]        <- EBP
			     ; [EBP+4] 	    <- RET Addr
			     ; [EBP+8]      <- ��Ʈ ��ȣ
			     ; [EBP+12]	    <- ����� ����Ʈ
				
				 MOV EDX,[EBP+8]
				 MOV EAX,[EBP+12]
                 OUT DX,EAX    ; ��Ʈ�� ���� ����.

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

				MOV    ECX,0FFFFFh   ; ���ѷ����� ������ �ʵ��� �Ѵ�.
WAIT_IN:        IN     AL,64H	     ; Write ������ �������� �˻��Ѵ�.
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

				MOV    ECX,0FFFFFh   ; ���ѷ����� ������ �ʵ��� �Ѵ�.
WAIT_OUT:       IN     AL,64H	     ; Write ������ �������� �˻��Ѵ�.
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
	                MOV  AH,AL			; DMA1 ��
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
_vAsmSetKBDLed	Proc NEAR							;  // Ű���� �߱����̿��带 �����Ѵ�.
				public _vAsmSetKBDLed  				;
;===================================================;
				PUSH  EBP
				MOV   EBP,ESP

			    ; [EBP]        <- EBP
			    ; [EBP+4]  	   <- RET Addr
			    ; [EBP+8]      <- LED Mask Bit			B2-Caps, B1-NUM, B0-Scroll

				MOV  EAX,1					; IRQ1�� UnMASK�Ѵ�.
				PUSH EAX
				CALL _MaskIRQ
				ADD  ESP,4

                CALL WaitInBufferEmpty        
                MOV  AL,0EDh                ; LED ������ 
                OUT  60h,AL                 

                CALL WaitOutBufferFull      ; ������ �����Ѵ�.
                IN   AL,64h

                CALL WaitInBufferEmpty      ; �Է¹��۰� ��⸦ ��ٸ���.
				MOV  EAX,[EBP+8]
                OUT  60h,AL                 ; ���ο� LED���� ������.

                CALL WaitOutBufferFull      ; ������ �����Ѵ�.
                IN   AL,64h										
				
				MOV  EAX,1					; IRQ1�� UnMASK�Ѵ�.
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
                MOV  AL,0FEh                ; �ý��� ����Ʈ
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

		   STI           ;  ���ͷ�Ʈ�� �ٷ� Enable���� ������.

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


