
.386P
.MODEL FLAT

.DATA
.CODE 
;=======================================;
_dwReadCMOS     Proc NEAR               ; 
                Public _dwReadCMOS      ; 
;=======================================;
             	PUSH EBP
				MOV  EBP, ESP
			    PUSHFD

			   ; [EBP]       <- EBP
			   ; [EBP+4] 	 <- RET Addr
			   ; [EBP+8]     <- �о���� ��Ʈ��ȣ
		
			    MOV EAX, [EBP+8]		; �о���� ��Ʈ��ȣ

                OR  AL,80H				; NMI�� �Ұ��� ����� ���� ��Ʈ 7�� 1�� ����
                CLI						; ���ͷ�Ʈ�� �Ұ� ���·� ���´�.
                OUT 70H,AL              ; ���� CMOS��Ʈ�� �����ϰ� NMI�� �Ұ����·� �ȴ�.
                NOP
                NOP						; �ణ�� �ð� ������ �д�.
				NOP
				IN  AL,71H				; ��Ʈ 71h�κ��� ����Ÿ�� �о���δ�.

				PUSH EAX
					XOR  AL,AL
					OUT  70H,AL         ; NMI�� �ٽ� �㰡 ���·� �Ѵ�.
				POP EAX

                POPFD
				POP EBP					; EAX�� ���� ���� �����Ѵ�.
                RETN
_dwReadCMOS      ENDP
;=======================================;
_dwWriteCMOS    Proc NEAR               ; 
                Public _dwWriteCMOS     ; 
;=======================================;
				PUSH EBP
				MOV  EBP, ESP
                PUSHFD
					  
			   ; [EBP]       <- EBP
			   ; [EBP+4] 	 <- RET Addr
			   ; [EBP+8]     <- ����� ��Ʈ��ȣ
			   ; [EBP+14]    <- ����� ����Ÿ

				MOV  AL,[EBP+8]
                OR   AL,80H
                CLI
                OUT  70H,AL             ; NMI�� �Ұ� ���·� ���� ����� ��Ʈ�� �����Ѵ�.
                MOV  AL,AH
				NOP
				NOP
				NOP

				MOV  AL,[EBP+14]        ; ����� ����Ÿ
                OUT  71H,AL				; ���� ��Ʈ�� ����Ѵ�.
                MOV  AL,0
                NOP
				NOP
				NOP
                OUT 70H,AL              ; NMI�� �ٽ� �㰡 ���·� �����.

                POPFD
				POP EBP
                RETN
_dwWriteCMOS    ENDP

				END
















;=======================================;
_vGetCMOSInfo Proc NEAR              	;
              Public _vGetCMOSInfo   	;
;===================================-===;   
				PUSH EBP
				MOV  EBP,ESP
				PUSHAD
			   ; [EBP]       <- ESP
			   ; [EBP+4] 	 <- RET Addr
			   ; [EBP+8]     <- CMOSInfoStt

                MOV  ECX,[EBP+8]         ; ����� ������ �ּ�
                MOV  AL,10H              ; Disk Drive A,B�� ���� ������ �д´�.
                Call _dwReadCMOS
                MOV  AL,AH
                SHR  AH,4                ; AH = A ����̺꿡 ���� ����
                AND  AL,0FH              ; AL = B ����̺꿡 ���� ����
                MOV  [ECX],AH
                MOV  [ECX+1],AL	  

				POPAD
				POP  BP
				RETN
_vGetCMOSInfo	Endp

;-----------------------------------;
