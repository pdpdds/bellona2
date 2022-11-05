
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
			   ; [EBP+8]     <- 읽어들일 포트번호
		
			    MOV EAX, [EBP+8]		; 읽어들일 포트번호

                OR  AL,80H				; NMI를 불가로 만들기 위해 비트 7을 1로 설정
                CLI						; 인터럽트를 불가 상태로 놓는다.
                OUT 70H,AL              ; 읽을 CMOS포트를 설정하고 NMI가 불가상태로 된다.
                NOP
                NOP						; 약간의 시간 지연을 둔다.
				NOP
				IN  AL,71H				; 포트 71h로부터 데이타를 읽어들인다.

				PUSH EAX
					XOR  AL,AL
					OUT  70H,AL         ; NMI를 다시 허가 상태로 한다.
				POP EAX

                POPFD
				POP EBP					; EAX로 읽은 값을 리턴한다.
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
			   ; [EBP+8]     <- 기록할 포트번호
			   ; [EBP+14]    <- 기록할 데이타

				MOV  AL,[EBP+8]
                OR   AL,80H
                CLI
                OUT  70H,AL             ; NMI를 불가 상태로 놓고 기록할 포트를 설정한다.
                MOV  AL,AH
				NOP
				NOP
				NOP

				MOV  AL,[EBP+14]        ; 기록할 데이타
                OUT  71H,AL				; 실제 포트에 기록한다.
                MOV  AL,0
                NOP
				NOP
				NOP
                OUT 70H,AL              ; NMI를 다시 허가 상태로 만든다.

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

                MOV  ECX,[EBP+8]         ; 결과를 리턴할 주소
                MOV  AL,10H              ; Disk Drive A,B에 관한 정보를 읽는다.
                Call _dwReadCMOS
                MOV  AL,AH
                SHR  AH,4                ; AH = A 드라이브에 관한 정보
                AND  AL,0FH              ; AL = B 드라이브에 관한 정보
                MOV  [ECX],AH
                MOV  [ECX+1],AL	  

				POPAD
				POP  BP
				RETN
_vGetCMOSInfo	Endp

;-----------------------------------;
