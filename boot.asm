;*****************************************
;***  SECTOR DE ARRANQUE DE JFCONSOLE  ***
;*** Copyright (c) Jesus Fdez. Gamito  ***
;*****************************************

#make_boot#

#fasm#          ;Es codigo de FASM

ORG 7C00h                         

jmp inicio

nop
db  'JFCONSOLE      '       ;Etiqueta del disco
dw  8Ah                     ;Identificador de JFSA
dw  01h                     ;Version de JFSA

inicio:

mov     ax,07C0h            ;Lo primero que hay que hacer es establecer la 
mov     ss,ax               ;longitud de la pila a 255 palabras
mov     sp,03FEh            

xor     ax,ax               ;Establecemos la direccion del segmento de datos
mov     ds,ax

mov     al,03h              ;Establecemos el modo de video a 80x25
int     10h                                                       

mov     si,inici            ;Mostrar el mensaje de cargando
call    sali_pant
            
xor     ax,ax               ;Reiniciar controlador del disquete
int     13h
jc      error               ;Si hay error informar

mov     cx,0600h            ;Cargar los datos en el area de 0600:0200
mov     es,cx
xor     cx,cx

buscar_sector:

mov     ah,02h              ;Leer el sector
mov     al,1
xor     ch,ch
mov     cl,[sect]
xor     dx,dx
xor     bx,bx

push    dx
stc                         ;Esto se hace asi para evitar un BUG que tienen
int 13h                     ;Algunas BIOS
jc error
sti                                       
pop     dx

cld                         
mov     si,nomnuc           ;Cargar el nombre de la bios y compararlo con
mov     di,bx               ;el del sector, si coinciden lo hemos encontrado
mov     cx,21               ;sino, no lo encontramos y tenemos que seguir
repe    cmpsb               ;buscando...
jnz     noenc

push    ds                  ;Guardar el segmento de datos para luego liberarlo

mov     ax,0600h            ;Mover el segmento de datos hacia el sector que hemos leido
mov     ds,ax   

mov     si,0015h            ;Mover el dato de el sector logico
mov     bx,[si]

mov     si,0017h            ;Mover dato de la longitud de datos
mov     dx,[si]

pop     ds

mov     ax,bx               ;Movemos los datos leidos a las variables
mov     [larc],dx 

call    calc_seca           ;Calculamos la direccion C-H-S del sector logico de inicio

mov     ax,[larc]
mov     ah,02h              ;Leemos los sectores del kernel...
xor     dl,dl
mov     bx,0000h            ;Y lo cargamos en la posicion 0600:0000

push    dx
stc                         ;Esto se hace asi para evitar un BUG que tienen
int 13h                     ;Algunas BIOS
jc error
sti                                       
pop     dx 

JMP     0600h:0000h         ;SALTAMOS AL KERNEL!!!

hlt                         ;Si algo va mal y retorna aqui, lo paramos
 
noenc:
cmp [sect],17
je  error                   ;Si llegamos al final de la tabla, damos ERROR

inc [sect]                  ;Incrementar nº de sector
jmp buscar_sector

error:
mov si,fallad
call sali_pant

xor ax,ax
int 16h

int 19h                     ;Continuar con el inicio del sistema
hlt

sali_pant:
push ax     ;Guardar registros 
push si
jmp bucl

bucl:
lodsb
or      al,al
je      final
mov     ah,0Eh   ;Funcion teletype
int     10h

jmp     bucl

final:      
pop ax      ;Liberar registros   
pop si
ret

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


inici  DB 'Cargando JFConsole...',0Dh,0Ah,0
fallad DB 'ERROR cargando el kernel de JFConsole.',0Dh,0Ah
       DB 'Extraiga el disquete y presione cualquier tecla para continuar... ',0Dh,0Ah,0
nomnuc DB 'JFCONSOLE.NUC        '
sect   DB 1   
larc   DW 00h
times ((0x1fe-$) and 0xFF) db 00h  ;Rellenar el resto del sector de arranque hasta llegar a la firma
db 55,0AAh                         ;Firma del sector de arranque