TEXT            SEGMENT
                ASSUME CS:TEXT, DS:TEXT

IODELAY         MACRO
                JMP $+2
                JMP $+2
                JMP $+2
                JMP $+2
                JMP $+2
                JMP $+2
                ENDM

;-------------------------------;
KBD_CMD         PROC NEAR
                PUSH CX

                XOR CX,CX
    CMD_WAIT:   IN  AL,64H
                IODELAY
                TEST AL,2
                JZ CMD_SEND
                LOOP CMD_WAIT
                    STC
                    JMP END_CMD

    CMD_SEND:   MOV AL,BL
                OUT 64H,AL
                IODELAY

                CLC
    END_CMD:    POP CX
                RET
KBD_CMD         ENDP
;-------------------------------;
KBD_READ        PROC NEAR
                PUSH CX
                XOR  CX,CX
    READ_LOOP:  IN   AL,64H
                IODELAY
                TEST AL,1
                JNZ  READ_NOW
                LOOP READ_LOOP
                     STC
                     JMP END_READ
    READ_NOW:   IN   AL,60H
                CLC
    END_READ:   POP  CX
                RET
KBD_READ        ENDP
;-------------------------------;
KBD_WRITE       PROC NEAR
                PUSH CX
                PUSH DX

                MOV  DL,AL
                XOR  CX,CX
    WRITE_LOOP: IN   AL,64H
                IODELAY
                TEST AL,2
                JZ   WRITE_NOW
                LOOP WRITE_LOOP
                     STC
                     JMP END_WRITE
    WRITE_NOW:  MOV  AL,DL
                OUT  60H,AL
                CLC
    END_WRITE:  POP  DX
                POP  CX
                RET
KBD_WRITE       ENDP
;-------------------------------;
_ASM_ENABLE_A20 PROC NEAR
                PUBLIC _ASM_ENABLE_A20

                PUSH BX
                CLI

                MOV  BL,0D0H    ; READ
                CALL KBD_CMD
                JC   END_A20_ERROR
                CALL KBD_READ
                JC   END_A20_ERROR

                PUSH AX         ; WRITE
                MOV  BL,0D1H
                CALL KBD_CMD
                POP  AX
                JC   END_A20_ERROR
                OR   AL,2
                CALL KBD_WRITE
                JC   END_A20_ERROR

END_A20:        STI
                POP BX
                XOR AX,AX
                RET

END_A20_ERROR:  STI
                POP BX
                MOV AX,0FFFFh
                RET


_ASM_ENABLE_A20 ENDP
TEXT            ENDS

                END
