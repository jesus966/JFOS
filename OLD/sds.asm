;********************************************
;****               SDS.ASM              ****
;****  FUNCIONES DEL SISTEMA DE ARCHIVOS ****
;********************************************

use16

#fasm#

;Macros
macro int13h{
    push dx     ;Se hace asi para arreglar el bug de ciertas bios
    stc
    int 13h
    sti
    pop dx 
}

mov  di,victor
call leearchivo
hlt
victor db 'VICTORESTONTO.XE   '


;Constantes y definiciones internas

heads db 2  ;Estos datos definen los limites maximos
cyls  db 80 ;de la unidad que se este usando
sect  db 18
tsec  dw 2880
tshed dw 1440 ;Total de sectores/Cabezales
 

;Descriptor del medio
sdsid  db   '   '           ;Identificador del formato Sistema de Disquete Simple (SDS)
sdsver db   00h             ;Identificador de version de SDS (version 0.0)
sdsetq db '                ';Etiqueta del disquete (16 caracteres max)
sdstip db   00h             ;Tipo de disquete (0-360K 1-1,2M 2-720K 3-1,44M)
sdstot dw   0000            ;Numero total de sectores del disquete (1,44M -> 2880)
sdssta dw   0000            ;Numero del sector en el que empieza la tabla de archivos
sdsfec dd   000000h         ;Fecha de creacion del disquete DD/MM/AA
sdshor dd   000000h         ;Hora de creacion HHMMSS  

etqcor db 'SDS'             ;Etiqueta para comparacion 

taind db 00h                ;Entrada de tabla de archivos
tanom db '                   '
taatr db 0
tafec dd 000000h
tahor dd 000000h
tacom dw 0000h
tatam db 0
;---------------------------------------------
;---            LEEARCHIVO                 ---
;--- Lee una determinado archivo.          ---
;---                                       ---
;--- Entradas:                             ---  
;--- ES:DI -> Nombre del archivo a leer    ---  
;--- ES:BX -> Direccion de memoria donde   ---
;--- dejar el contenido del archivo.       ---
;--- DL -> Unidad                          ---
;--- Ninguna                               ---
;--- Salidas:                              ---  
;--- AH:                                   ---   
;---    0 si no hubo un error              ---
;---    1 si no se encontro el archivo     --- 
;---    2 si el disco no tiene for. valido ---
;---    3 si error 13h                     ---
;--- AL -> INT13 err si AH es 3            ---  
;--- AL -> Ano de archivo                  ---
;--- BH -> Mes de archivo                  ---
;--- BL -> Dia de archivo                  ---
;--- CH -> Hora de archivo                 ---
;--- CL -> Minuto de archivo               ---
;--- DH -> Segundo de archivo              ---
;--- DL -> Atributo de archivo             ---
;--- CF -> 1 Si hubo error                 ---
;---------------------------------------------
leearchivo:
    pusha        
    
    mov [ladi],di   ;Guardamos los registros
    mov [labx],bx                           
    
    call leetabarch ;Cargamos la tabla del archivo
    jc laterr       ;Si hay error comprobamos
    
    mov ax,[tacom]  ;Calculamos donde esta el primer sector
    mov [lasig],ax
    
    mov ax,[lasig]  ;Cargamos el siguiente sector
    call calchs  
    
    mov ah,02h      ;Leemos el sector correspondiente
    mov al,1
    mov bx,secaux
    call intdisco 
    jc ltaderr 
    

    
 
laterr:
    cmp ah,01h      ;Si no se encontro salimos
    je lanoenc
    
    cmp ah,02h      ;Si el formato no es valido decimos
    je lafor
            
    mov [mdbt],ax   ;Si no, es error de disco
    popa
    mov ax,[mdbt]
    mov al,ah
    mov ah,03h   
    stc
    ret 
    
lanoenc:
    popa
    mov ah,01h
    stc
    ret

lafor:
    popa
    mov ah,02h
    stc
    ret

ladi dw 0
labx dw 0   
lasig dw 0
latam db 0
;---------------------------------------------
;---            LEETABARCH                 ---
;--- Lee una tabla de determinado archivo. ---
;---                                       ---
;--- Entradas:                             ---  
;--- ES:DI -> Nombre del archivo a leer    ---
;--- DL -> Unidad                          ---
;--- Ninguna                               ---
;--- Salidas:                              ---
;--- AH:                                   ---   
;---    0 si no hubo un error              ---
;---    1 si no se encontro el archivo     --- 
;---    2 si el disco no tiene for. valido ---
;---    3 si error 13h                     ---
;--- AL -> INT13 err si AH es 3            ---
;--- CF -> 1 Si hubo error                 ---
;---------------------------------------------
leetabarch:
    pusha 
    
    mov [ltanar],di ;Guardamos los datos
    mov [ltadrv],dl
    
    call actdes     ;Antes que nada, actualizamos el descriptor de medios
    jc ltacerr      ;Si hay error, comprobar
    
    mov ax,[sdssta] ;Cargamos los valores CHS del sector donde comienza la tabla
ltao:call calchs
    
    mov ah,02h      ;Leemos el sector correspondiente
    mov al,1
    mov bx,secaux
    call intdisco 
    jc ltaderr
    
    cld                 ;Leemos la tabla
    mov si,secaux
    mov di,taind
    mov cx,32           ;Movemos 32 datos
    rep movsb   
    
    mov bx,secaux
    
    mov al,[taind]
        
ltabuc:    
    cmp al,4Ah ;Comprobamos si la entrada de la tabla es 4Ah
    je  ltacomp     ;Si es asi, hemos encontrado una entrada valida
        
ltasig:add bx,1Eh      ;Comprobamos los sig. 32 bytes   
    mov si,bx
    cld             ;Leemos la tabla
    mov di,taind
    mov cx,32           ;Movemos 33 datos
    rep movsb
    
    cmp [ltacont],0 ;Comprobamos si hemos llegado al final del sector
    je ltacamsec    ;Si es asi, cambiamos o comprobamos si es el final
    
    dec [ltacont]   ;Drecementamos el contador
    jmp ltabuc      ;Saltamos
    

    
ltacomp:

    mov di,[ltanar]  ;Cargamos el nombre de archivo a buscar
    mov si,tanom     ;Lo comparamos con la cadena de la tabla de archivo
    mov cx,19        ;Comparamos los 19 caracteres
    repe cmpsb
    jne ltasig       ;Si no, este archivo no es, seguimos comprobando mas archivos
    
    popa             ;Lo hemos encontrado, asi que salimos
    xor ah,ah
    clc
    ret 
    

ltaderr:        
    mov [mdbt],ax
    popa
    mov ax,[mdbt]
    mov al,ah
    mov ah,03h   
    stc
    ret 


    
ltacamsec:
    cmp [taind],0FFh ;Si es FFh, hemos llegado al final de la tabla de archivos 
    je  ltanoenc    ;sin encontrar una entrada escrita, con lo cual no hay ningun archivo
    
    mov [ltacont],15 ;Reseteamos el contador
    
    mov ax,word [tanom] ;Si no, cargamos el siguiente sector de la tabla y vuelta a buscar 
    mov dl,[ltadrv]    
    jmp ltao        
    
ltacont db 15   
ltadrv  db 0
ltanar  dw 0

ltanoenc:
    popa
    mov ah,01h
    stc
    ret

ltacerr:
    cmp ah,01h
    je ltaefor
        
    mov [mdbt],ax
    popa
    mov ax,[mdbt]
    mov al,ah
    mov ah,03h   
    stc
    ret 

ltaefor:
    popa
    mov ah,02h   
    stc
    ret

;---------------------------------------------
;---                 MAROC                 ---
;--- Marca un determinado sector logico    ---
;--- como ocupado.                         ---
;---                                       ---
;--- Entradas:                             ---  
;--- AX -> Sector logico a marcar          ---
;--- DL -> Unidad                          ---
;--- Ninguna                               ---
;--- Salidas:                              ---
;--- AH -> 0 si no error, 1 - Err 13H,     ---
;--- AL -> INT13 err si AH es 1            ---
;--- CF -> 1 Si hubo error                 ---
;---------------------------------------------
maroc:
    pusha
      
    mov [mdbt],ax   ;Guardamos el sector logico a marcar
    mov [mdun],dl   ;y la unidad
    
    mov ah,02h      ;Leemos el sector 2
    mov al,1
    xor ch,ch
    mov cl,2
    xor dh,dh
    mov bx,secaux
    call intdisco
    jc  moerr       ;Si hay error lo decimos  
    
    mov ax,[mdbt]   ;Lo recuperamos
      
    mov cx,8
    div cx   ;Dividimos ax entre 8 para obtener
             ;en que byte estara el bit a modificar
             ;DL contiene el bit que hay que modificar
    
    mov [mdbt],ax ;Lo guardamos
    
    mov bx,secaux      ;Ponemos la direccion de memoria del
    add bx,[mdbt]      ;sector auxiliar + la posicion del byte 
    
    mov ax,[bx] 
    
    mov ah,01h         ;Creamos la mascara OR
    mov cl,dl
    rol ah,cl          ;Con esto hemos desplazado el bit 1 dl veces                      
    
    or al,ah           ;PONEMOS EL BIT EN 1
    
    mov bx,secaux      ;Ponemos la direccion de memoria del
    add bx,[mdbt]      ;sector auxiliar + la posicion del byte 
    
    mov [bx],al 
    
    mov ah,03h      ;Escribimos el sector 2
    mov al,1
    xor ch,ch
    mov cl,2
    xor dh,dh
    mov dl,[mdun]
    mov bx,secaux
    call intdisco
    jc moerr
    
    popa
    xor ah,ah
    clc
    ret 
    
moerr:  
    mov [mdbt],ax
    popa
    mov ax,[mdbt]
    mov al,ah
    mov ah,01h   
    stc
    ret 


;---------------------------------------------
;---                 OBTLI                 ---
;--- Obtiene si un determinado n. logico   ---
;--- esta marcado como libre.              ---
;---                                       ---
;--- Entradas:                             ---  
;--- AX -> Sector logico a ver             ---
;--- DL -> Unidad                          ---
;--- Ninguna                               ---
;--- Salidas:                              ---
;--- AH -> 0 si li, 1 - si no, 2-Err 13h   ---
;--- AL -> INT13 err si AH es 2            ---
;--- CF -> 1 Si hubo error                 ---
;---------------------------------------------
obtli:
    pusha
    
    mov [mdbt],ax   ;Guardamos el sector logico a marcar
    mov [mdun],dl   ;y la unidad
    
    mov ah,02h      ;Leemos el sector 2
    mov al,1
    xor ch,ch
    mov cl,2
    xor dh,dh
    mov bx,secaux
    call intdisco
    jc  lierr       ;Si hay error lo decimos 
    
    mov ax,[mdbt]   ;Lo recuperamos
    
    mov cx,8
    div cx   ;Dividimos ax entre 8 para obtener
             ;en que byte estara el bit a ver
             ;DL contiene el bit que hay que ver
             
    mov [mdbt],ax ;Lo guardamos
    
    mov bx,secaux      ;Ponemos la direccion de memoria del
    add bx,[mdbt]      ;sector auxiliar + la posicion del byte 
    
    mov ax,[bx]   
    
    mov ah,01h         ;Creamos la mascara AND
    mov cl,dl
    rol ah,cl          ;Con esto hemos desplazado el bit 1 dl veces                      
    
    and al,ah          ;Comprobamos
    
    cmp al,0           ;Comprobamos si ese sector esta libre... 
    jne limal          ;Si no es 0, es que esta ocupado
    
    popa               ;Si no esta libre
    xor ah,ah
    clc
    ret   
    
limal:
    popa               ;Esta ocupado, lo decimos
    mov ah,01h
    stc
    ret 

lierr:              
    mov [mdbt],ax
    popa
    mov ax,[mdbt]
    mov al,ah
    mov ah,02h   
    stc
    ret

;---------------------------------------------
;---              INTDISCO                 ---
;--- Ejecuta la INT13h y sirve como control---
;--- de errores.                           ---
;---                                       ---
;--- Entradas:                             ---  
;--- Entradas de int 13h                   ---
;--- Ninguna                               ---
;--- Salidas:                              ---
;--- AH -> 0 si no error                   ---
;--- CF -> 1 Si hubo error                 ---
;---------------------------------------------

intdisco:  
    int13h
    jc iderr
ider:ret                 ;Si no hubo error volvemos
    
iderr:mov [idau],ax

idbuc:cmp [cont],0       ;Si el contador es 0, se han acabado los reintentos
    je ider              ;Saltamos a la comprobacion de errores  
    
    xor ax,ax           ;Reseteamos el disco antes de probar otra vez
    int13h
    jc iddec      
    
    mov ax,[idau]
    int13h
    jc iddec            ;Decrementamos el contador
    
    mov [cont], 3       ;Reseteamos el contador
    ret                 ;Volvemos

iddec:
    dec [cont]         
    jmp idbuc 


cont db 3               ;Contador de intentos fallidos
idau dw 0               ;Auxiliar

;---------------------------------------------
;---                 OBTDA                 ---
;--- Obtiene si un determinado n. logico   ---
;--- esta marcado como danado.             ---
;---                                       ---
;--- Entradas:                             ---  
;--- AX -> Sector logico a ver             ---
;--- DL -> Unidad                          ---
;--- Ninguna                               ---
;--- Salidas:                              ---
;--- AH -> 0 si no da, 1 - da, 2-Err 13h   ---
;--- AL -> INT13 err si AH es 2            ---
;--- CF -> 1 Si hubo error                 ---
;---------------------------------------------
obtda:
    pusha
    
    mov [mdbt],ax   ;Guardamos el sector logico a marcar
    mov [mdun],dl   ;y la unidad
    
    mov ah,02h      ;Leemos el sector 3
    mov al,1
    xor ch,ch
    mov cl,3
    xor dh,dh
    mov bx,secaux
    call intdisco
    jc  oderr       ;Si hay error lo decimos 
    
    mov ax,[mdbt]   ;Lo recuperamos
    
    mov cx,8
    div cx   ;Dividimos ax entre 8 para obtener
             ;en que byte estara el bit a ver
             ;DL contiene el bit que hay que ver
             
    mov [mdbt],ax ;Lo guardamos
    
    mov bx,secaux      ;Ponemos la direccion de memoria del
    add bx,[mdbt]      ;sector auxiliar + la posicion del byte 
    
    mov ax,[bx]   
    
    mov ah,01h         ;Creamos la mascara AND
    mov cl,dl
    rol ah,cl          ;Con esto hemos desplazado el bit 1 dl veces                      
    
    and al,ah          ;Comprobamos
    
    cmp al,0           ;Comprobamos si ese sector no tiene fallos... 
    jne odmal          ;Si no es 0, es que esta danado
    
    popa               ;Si no esta bien
    xor ah,ah
    clc
    ret   
    
odmal:
    popa               ;Esta danado, lo decimos
    mov ah,01h
    stc
    ret 

oderr:              
    mov [mdbt],ax
    popa
    mov ax,[mdbt]
    mov al,ah
    mov ah,02h   
    stc
    ret
    

;---------------------------------------------
;---                 MARDA                 ---
;--- Marca un determinado sector logico    ---
;--- como danado.                          ---
;---                                       ---
;--- Entradas:                             ---  
;--- AX -> Sector logico a marcar          ---
;--- DL -> Unidad                          ---
;--- Ninguna                               ---
;--- Salidas:                              ---
;--- AH -> 0 si no error, 1 - Err 13H,     ---
;--- AL -> INT13 err si AH es 1            ---
;--- CF -> 1 Si hubo error                 ---
;---------------------------------------------
marda:
    pusha
      
    mov [mdbt],ax   ;Guardamos el sector logico a marcar
    mov [mdun],dl   ;y la unidad
    
    mov ah,02h      ;Leemos el sector 3
    mov al,1
    xor ch,ch
    mov cl,3
    xor dh,dh
    mov bx,secaux
    call intdisco
    jc  mderr       ;Si hay error lo decimos  
    
    mov ax,[mdbt]   ;Lo recuperamos
      
    mov cx,8
    div cx   ;Dividimos ax entre 8 para obtener
             ;en que byte estara el bit a modificar
             ;DL contiene el bit que hay que modificar
    
    mov [mdbt],ax ;Lo guardamos
    
    mov bx,secaux      ;Ponemos la direccion de memoria del
    add bx,[mdbt]      ;sector auxiliar + la posicion del byte 
    
    mov ax,[bx] 
    
    mov ah,01h         ;Creamos la mascara OR
    mov cl,dl
    rol ah,cl          ;Con esto hemos desplazado el bit 1 dl veces                      
    
    or al,ah           ;PONEMOS EL BIT EN 1
    
    mov bx,secaux      ;Ponemos la direccion de memoria del
    add bx,[mdbt]      ;sector auxiliar + la posicion del byte 
    
    mov [bx],al 
    
    mov ah,03h      ;Escribimos el sector 3
    mov al,1
    xor ch,ch
    mov cl,3
    xor dh,dh
    mov dl,[mdun]
    mov bx,secaux
    call intdisco
    jc mderr
    
    popa
    xor ah,ah
    clc
    ret 
    
mderr:  
    mov [mdbt],ax
    popa
    mov ax,[mdbt]
    mov al,ah
    mov ah,01h 
    stc
    ret 
    
mdbt dw 0
mdun db 0
;---------------------------------------------
;---                 ACTDES                ---
;--- Actualiza la tabla de descriptor de   ---
;--- medios.                               ---
;---                                       ---
;--- Entradas:                             ---
;--- DL -> Unidad                          ---
;--- Ninguna                               ---
;--- Salidas:                              ---
;--- AH -> 0 si no error, 1 - Err Compb,   ---
;--- AL -> INT13 err si AH es 2            ---
;--- CF -> 1 Si hubo error                 ---
;---------------------------------------------
actdes:
    pusha
    
    mov ah,02h          ;Leemos el primer sector de todos
    mov al,1
    xor ch,ch
    mov cl,1
    xor dh,dh
    mov bx,secaux 
    call intdisco           
    jc aderr            ;Si hay error lo decimos
                       
    cld                 ;Leemos la tabla
    mov si,secaux+2
    mov di,sdsid
    mov cx,32           ;Movemos 32 datos
    rep movsb
    
    mov di,sdsid        ;Comprobamos si el descriptor
    mov si,etqcor       ;coincide con SDS
    mov cx,3
    repe cmpsb 
    jnz act_err_des     ;Si no, dar error
    
    popa
    xor ah,ah
    clc
    ret

act_err_des:
    
    popa
    mov ah,01h
    stc
    ret 
    
aderr:
    popa
    mov al,ah
    mov ah,02h
    stc
    ret
   
       
;---------------------------------------------
;---                 CALCHS                ---
;--- Calcula el valor CHS correspondiente a---
;--- un sector logico.                     ---
;---                                       ---
;--- Entradas:                             ---
;--- AX -> N. de sector logico.            ---
;--- Salidas:                              ---
;--- CH -> Cilindro                        ---
;--- CL -> Sector                          ---
;--- DH -> Cabezal                         ---
;--- CF -> 1 Si hubo error                 ---
;---------------------------------------------

calchs:

    pusha          ;Guardamos los registros
    
    cmp ax, [tsec] ;Comprobamos si ax>=TSEC
    jae calchs_e   ;Si es asi, error 
    
    mov [ccnum],ax   ;Guardamos el n. logico
    
    div [tshed]    ;Dividimos AX entre tscab
    
    mov [cched], ax  ;Tenemos los cabezales 
    
    mul [tshed]    ;Multiplicamos los cabezales * tshed
    
    sub [ccnum], ax ;Le restamos a ccnum el resultado de la mult.
    
    cmp [cched], 0 ;Comprobamos si cched != 0
    jne otrcyl     ;Nos vamos a otro calculo
    
    mov ax,[ccnum] ;Dividmos num. / sec_pista
    div [sect]

ccrt:
    
    mov [cccyl], ax  ;Tenemos las pistas
    
    mul [sect]     ;ccyl*sect
    
    mov dx,ax      ;Obtenemos el sector
    mov ax,[ccnum]
    sub ax,dx
    add ax,1
    mov byte [ccaux],al  
    
    popa
    
    mov cl,byte [ccaux] ;Ponemos los datos
    mov ch,byte [cccyl] 
    mov dh,byte [cched]    
    
    clc         ;No hubo error
    
    ret         ;Retornamos de la subrutina

otrcyl:

    mov ax, [cched] ;Cabezales*sec_pista
    mul [sect]
    
    mov [ccaux],ax  ;Lo dividimos para obtener el n. de pistas
    mov ax,[ccnum]
    div [ccaux]
    
    jmp ccrt

calchs_e:
    
    popa        ;Liberamos registros
       
    stc         ;CF=1
    
    ret   
    
ccnum dw 0    
cched dw 0      ;Variables temporales
cccyl dw 0
ccaux dw 0

secaux:         ;Direccion para guardar sectores temporales