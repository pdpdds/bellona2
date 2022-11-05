CODE                SEGMENT
                    ASSUME CS:CODE, DS:CODE

                    ORG 0H
;=========================================;
G_wMagic            DW 0
G_wFunc             DW 0
G_wResult           DW 0
G_dwParam           DD 0
;=========================================;

                    ORG 100H
;-----------------------------------------;
START:              JMP     XXX
LIB_MAGIC           DB      "V86LIB", 0
FUNC_TBL			DW		0
					DW		OFFSET F_LINES_50
					DW		OFFSET F_GET_VESA_INFO
					DW		OFFSET F_GET_MODE_INFO
					DW		OFFSET F_SET_VESA_MODE
					DW		OFFSET F_LINES_25
BUFF_START			DB		'BUFF'
BUFF				DT	    50 DUP(0)
                    DB      12 DUP(0)
;-----------------------------------------;
XXX:                PUSH    CS
                    POP     DS
                    PUSH    CS
                    POP     ES

                    CMP     WORD PTR G_wMagic, 01128H
                    JE      CHK_PARAM
                            MOV  G_wResult, 1    ; Invalid parameter magic id.
                            JMP END_OF_SVC

CHK_PARAM:          MOV     WORD PTR G_wResult,0 ; The default return value is zero.
				    
					MOV     SI, OFFSET FUNC_TBL
					MOV     BX, G_wFunc
					SHL		BX, 1
					MOV     DI, [BX+SI]
					CALL    DI					 ; Get the function address and call that.
					CMP     AL, 4FH
					JE      CHK_AH
							MOV WORD PTR G_wResult, 0FFFFh	; Function not supported
							JMP END_OF_SVC
	CHK_AH:			MOV		CL, 8
					SHR		AX, CL				 ; Shift the AH value to the AL register
					MOV		G_wResult, AX		 ; The AL value 0 means success but others mean failure.

END_OF_SVC:			INT     51H					 ; Return to the Flat Protected Mode

                    JMP     XXX

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
F_LINES_50			PROC NEAR
					
					MOV     AX,1202H    
					MOV     BL,30H      
					INT     10H         
					                    
					MOV     AX,0003H    
					INT     10H         
					                    
					MOV     AX,1112H    
					MOV     BL,00       
					INT     10H 
					
					MOV     AX, 004FH        

					RETN
F_LINES_50			ENDP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
F_GET_VESA_INFO	    PROC NEAR

					MOV     DI,OFFSET BUFF
				    MOV     WORD PTR [DI],  4256H	; VB
				    MOV     WORD PTR [DI+2],3245H	; E2			    
				    MOV     AX,4F00H				; VBE information
				    INT     10H

					RETN
F_GET_VESA_INFO     ENDP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
F_GET_MODE_INFO		PROC NEAR

					MOV  AX,4F01H					; VBE Mode Information
					MOV  CX,WORD PTR G_dwParam
					MOV  DI,OFFSET BUFF
					INT  10H

					RETN
F_GET_MODE_INFO		ENDP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
F_SET_VESA_MODE		PROC NEAR

					MOV  AX,4F02H					
					MOV  BX,WORD PTR G_dwParam
					MOV  DI,0
					INT  10H

					RETN
F_SET_VESA_MODE		ENDP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
F_LINES_25			PROC NEAR

					MOV  AX,02H	  				
					MOV  BX,2	  ; video mode
					MOV  DI,0
					INT  10H
					
					MOV  AX,004FH

					RETN
F_LINES_25			ENDP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

CODE                ENDS
                    END     START
