;************************************
;****           BOOTSECTOR        ***
;************************************


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
hpc  DB     2               ;Number of heads
     DB     80              ;Number of cylinders
spt  DB     15              ;Number of sectors per cylinder
     DB     1               ;Number of sectors that forms one cluster
     DB     2               ;Start of file table cluster
     DB     2               ;Length of file table

start: 
     XOR    AX,AX           ;Clear DS an ES
     MOV    DS,AX
     MOV    ES,AX      
     
     MOV    SS,AX           ;Initialize Stack
     MOV    SP,SPDIR
     
     MOV    AX,01h
     CALL   LBATOCHS
     
; Input: ax - LBA value
; Output: cl - Sector
;	  dh - Head
;	  ch - Cylinder

    LBATOCHS        PROC        
     PUSH dx		; Save the value in dx
     XOR dx,dx		; Zero dx       
     XOR BX,BX
     MOV bl, [spt]	; Move into place STP (LBA all ready in place)
     DIV bx			; Make the divide (ax/bx -> ax,dx)
     inc dx			; Add one to the remainder (sector value)
     push dx	    ; Save the sector value on the stack
    
     XOR dx,dx		; Zero dx
     XOR BX,BX
     MOV bl, [hpc]	; Move NumHeads into place (NumTracks all ready in place)
     DIV bx			; Make the divide (ax/bx -> ax,dx)
    
     MOV cx,ax		; Move ax to cx (Cylinder)
     MOV bx,dx		; Move dx to bx (Head)
     POP ax			; Take the last value entered on the stack off.
    			    ; It doesn't need to go into the same register.
    			    ; (Sector)
     POP dx			; Restore dx, just in case something important was
    			    ; originally in there before running this.    
     MOV CL,AL
     MOV DH,BL
     XCHG CH,CL
     RET			; Return to the main function           
    LBATOCHS        ENDP        
    
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
        
    DB (510 - ($ - offset first)) DUP (0) ;Fill with 0s
    DW 0AA55h                             ;Boot signature           
    
    END