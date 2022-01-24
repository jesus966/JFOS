;***********************************************
;***        SISTEMA DE ARCHIVOS JFSA         ***
;***  Copyright (C) Jesus Fernandez Gamito   ***
;***********************************************

;************************************************************
;***  leer_arch - LEE UN ARCHIVO DEL DISQUETE             ***
;***  ENTRADA:                                            ***
;***  SI -> Nombre del archivo a leer (21 caracteres max) ***
;***  ES:BX -> Offset de memoria donde cargar el archivo  ***
;***  SALIDA:                                             ***
;***  AH -> Estado de la INT 13h                          ***
;***  AL -> 0 si se encontro el archivo, 1 si no          ***
;***  CF=1 si hubo algun error                            ***
;************************************************************
leer_arch:

pusha      

mov [daclar],bx

xor ax,ax
int 13h
jc  larerr

larbus:

mov     ah,02h              ;Leer el sector
mov     al,1
xor     ch,ch
mov     cl,[sectlar]
xor     dx,dx

push    dx
stc                         ;Esto se hace asi para evitar un BUG que tienen
int     13h                     ;Algunas BIOS
jc      larerr
sti                                       
pop     dx

cld                         
mov     di,bx               ;el del sector, si coinciden lo hemos encontrado
mov     cx,21               ;sino, no lo encontramos y tenemos que seguir
repe    cmpsb               ;buscando...
jnz     noenclar

push    ds                  ;Guardar el segmento de datos para luego liberarlo

mov     ax,0600h            ;Mover el segmento de datos hacia el sector que hemos leido
mov     ds,ax   

mov     si,0015h            ;Mover el dato de el sector logico
mov     bx,[si]

mov     si,0017h            ;Mover dato de la longitud de datos
mov     dx,[si]

pop     ds

mov     ax,bx               ;Movemos los datos leidos a las variables
mov     [larclar],dx 

call    calc_seca           ;Calculamos la direccion C-H-S del sector logico de inicio

mov     ax,[larclar]
mov     ah,02h              ;Leemos los sectores del kernel...
xor     dl,dl
mov     bx,[daclar]

push    dx
stc                         ;Esto se hace asi para evitar un BUG que tienen
int     13h                     ;Algunas BIOS
jc      larerr
sti                                       
pop     dx 

popa
clc
ret

larerr:
popa
stc
ret

noenclar:
cmp [sectlar],17
je  errorlar                ;Si llegamos al final de la tabla, damos ERROR

inc [sectlar]               ;Incrementar nº de sector

jmp larbus

errorlar:
popa            
mov     ax,0001h
stc
ret


sectlar DB 0
larclar DW 0   
daclar  DW 0
;************************************************************
;***  calc_seca - TRADUCE LOS SECTORES LOGICOS A C-H-S    ***
;***  ENTRADA:                                            ***
;***  AX -> N. de sector logico (de 1 hasta 2880)         ***
;***  SALIDA:                                             ***
;***  CH -> N. de cilindro                                ***
;***  CL -> N. de sector                                  ***
;***  DH -> N. de cabezal                                 ***
;***  CF=1 si hubo algun error                            ***
;************************************************************

calc_seca:                     

pusha                                  ;Guardamos todos los registros 

cmp     ax,0                           ;Comprobar si el sector logico es =>0 o <2880
jbe     fincalcseca                    ;y si es asi dar error

cmp     ax,2880
ja      fincalcseca

cmp     ax,1440                        ;Comprobamos si es mayor de 1440
jna     ntcancsa

mov     [cabcsa],1                     ;Si es asi, esta en el cabezal 2

sub     ax,1440                        ;Quitar 1440 para posicionarlos como si fueran
mov     [sectcsa],ax                   ;del cabezal 1.

sigpacsa:
mov     ax,[sectcsa]                   ;Dividir el n. de sectores logicos entre 18
mov     bl,18
div     bl 

mov     [cilcsa],al                    ;El resultado es el n. de cilindro

mov     bl,18                          ;Multiplicar por 18 el cilindro
mul     bl
                    
sub     [sectcsa],ax                   ;Restar al sector logico la multiplicacion anterior

mov     ax,[sectcsa]                   ;Con eso sabemos en que sector esta
mov     [seccsa],al

cmp     [seccsa],0                     ;Si la resta no es igual a 0 finalizamos
jne     finrutcsa

mov     [seccsa],18                    ;Si es asi, el sector es el 18 y el cilindro es
sub     [cilcsa],1                     ;uno menos

finrutcsa:   

popa                                   ;Liberamos los registros y limpiamos CF
clc

mov     dh,[cabcsa]
mov     ch,[cilcsa]                    ;Posicionar los datos C-H-S y salir
mov     cl,[seccsa]

ret

ntcancsa:
mov     [cabcsa],0                     ;El cabezal es 0
mov     [sectcsa],ax
jmp     sigpacsa                       ;Volvemos

fincalcseca:

popa                                   ;Liberamos todos los registros, CF=1 y volvemos
stc
ret


sectcsa    DW 0000
cilcsa     DB 00 
seccsa     DB 00   
cabcsa     DB 00