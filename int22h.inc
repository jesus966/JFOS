;********************************************
;***            INTERRUPCION 22H          ***
;********************************************

int_init:		;Iniciador de la interrupcion

    pushf
    cli
    
    xor     ax,ax
    mov     es,ax
    
    mov     al,22h
    mov     bl,04h
    mul     bl
    
    mov     bx,ax
    mov     si,interr
    mov     [es:bx],si
    add     bx,2
    mov     ax,cs
    mov     [es:bx],ax

    sti
    popf
    ret

interr:

    pusha
    push    cs
    pop     ds
    
    or	    ah,ah
    je	    imp_pant
    
    cmp     ah,01h
    je	    cre_vect

    cmp     ah,02h
    je	    restart

    stc
    popa
    iret	    

;=============================================
;=== AH-> 00h - Muestra texto en pantalla  ===
;===                                       ===
;=== Entrada:                              ===
;=== SI-> Cadena de texto a mostrar        ===
;===                                       ===
;=== Salida:                               ===
;=== Nada                                  ===
;=============================================
    
imp_pant:
    lodsb
    or	    al,al
    je	    final
    mov     ah,0Eh   ;Funcion teletype
    int     10h

    jmp     imp_pant

final:	    
    popa
    iret   

;=============================================
;=== AH-> 01h - Carga una interrupcion     ===
;===                                       ===
;=== Entrada:                              ===
;=== DL-> Numero de la interrupcion        ===
;=== SI-> Contenido de la interrupcion     ===
;===                                       ===
;=== Salida:                               ===
;=== Nada                                  ===
;=============================================
    
cre_vect:
    pushf
    cli
    
    xor     ax,ax
    mov     es,ax
    
    mov     al,dl
    mov     bl,04h
    mul     bl
    
    mov     bx,ax
    mov     [es:bx],si
    add     bx,2
    mov     ax,cs
    mov     [es:bx],ax

    sti
    popf
    popa
    clc
    iret

;=============================================
;=== AH-> 02h - Reinicia el sistema        ===
;===                                       ===
;=== Entrada:                              ===
;=== Nada                                  ===
;===                                       ===
;=== Salida:                               ===
;=== Nada                                  ===
;=============================================
    
restart:
    mov     ax,0040h
    mov     ds,ax
    mov     word [0072h], 0000h
    jmp     0FFFFh:0000h
    jmp     $