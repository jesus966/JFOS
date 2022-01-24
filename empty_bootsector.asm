;******************************************
;****           EMPTY BOOTSECTOR        ***
;******************************************


#make_boot#                 ;Indicate that this is a BOOT SECTOR
     
     ;Constants
     SPDIR      EQU 7E00h   ;Stack segment memory possition
     NEW_LINE   EQU 0Dh,0Ah 

pushall MACRO    
    PUSH    AX
    PUSH    CX
    PUSH    DX
    PUSH    BX
    PUSH    SP
    PUSH    BP
    PUSH    SI
    PUSH    DI 
    PUSHF
ENDM     

popall MACRO
    POPF    
    POP     DI
    POP     SI
    POP     BP
    POP     SP
    POP     BX
    POP     DX
    POP     CX
    POP     AX
ENDM
     ORG 7C00h              ;Segment 7C00h     
     
first:    
     JMP start              ;Skip USFS Header
     
     DB     'USFS'          ;Ultra Simple File System
     DB     20 dup (0)      ;Disk label   
     DB     2               ;Number of heads
     DB     80              ;Number of cylinders
     DB     15              ;Number of sectors per cylinder
     DB     1               ;Number of sectors that forms one cluster
     DB     2               ;Start of file table cluster
     DB     2               ;Length of file table

start: 
     XOR    AX,AX           ;Clear DS an ES
     MOV    DS,AX
     MOV    ES,AX      
     
     MOV    [OLDSS], SS     ;Save old Stack Pointers
     MOV    [OLDSP], SP
     
     MOV    SS,AX           ;Initialize Stack
     MOV    SP,SPDIR
     
     MOV    AH, 0Eh         ;Make a BEEP
     MOV    AL, 07h
     INT    10h
     
     LEA    SI,errmsg       ;Print blinking ERROR message first
     CALL   PRINT_BLK_MSG
         
     LEA    SI,msg          ;Print empty bootsector message on screen
     CALL   PRINT_MSG 
    
     XOR    AH,AH           ;Wait for any key
     INT    16h
     
     MOV    SS, [OLDSS]     ;Restore old Stack Pointers
     MOV    SP, [OLDSP]
     
     INT    19h             ;Try to boot from another disk 

halt:                       ;If anything went wrong, halt the system
     HLT
     JMP halt

errmsg  DB 'ERROR:',0     
msg     DB ' This floppy disk is not bootable.', NEW_LINE
        DB 'Insert system disk and press enter...', NEW_LINE, 0


    ;Print a string on screen that must be NULL terminated
    ;The address of the string is loaded in SI 
    PRINT_MSG     PROC 
             
             pushall
             
             MOV  AH,0Eh
             
        next_char:
             CMP  b.[SI], 0    ; check for zero to stop
             JE   stop         ;
        
             MOV  AL, [SI]     ; next get ASCII char.
        
             INT  10h          ; using interrupt to print a char in AL.
        
             ADD  SI, 1        ; advance index of string array.
        
             JMP  next_char    ; go back, and type another char.
        
        stop:            
             popall
             RET                   ; return to caller.
    PRINT_MSG     ENDP
    
    PRINT_BLK_MSG   PROC
             pushall
             
             MOV    AX, 1003h  ;Turn ON blinking
             XOR    BX,BX
             MOV    BL, 01h
             INT    10h        
             
             MOV    AH, 03h    ;Get inicial cursor position
             XOR    BH, BH
             INT    10h                                    

             
             MOV    BL, 0F0h   ;Blink attribute
             MOV    CX, 01h    ;Write only one character  
                              
             nxt_char:    
                CMP     b.[SI], 0  ;Check for zero to stop
                JE      stp                          
                
                MOV     AH, 09h    ;Write charater with attribute 
                MOV     AL,[SI]    ;Next character                    
                INT     10h        ;Write to screen
                
                INC     DL         ;Advance cursor
                INC     SI         ;Advance string
                
                MOV     AH, 02h    ;Set new cursor position
                INT     10h                                
                
                JMP     nxt_char   ;Loop until 0 is found
                
             stp:
                popall
                RET
                
    PRINT_BLK_MSG   ENDP 
    
    OLDSS DW 0
    OLDSP DW 0
    
    DB (510 - ($ - offset first)) DUP (0) ;Fill with 0s
    DW 0AA55h                             ;Boot signature           
    
    END