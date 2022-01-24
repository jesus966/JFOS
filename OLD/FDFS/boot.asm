#make_boot#
;************************************************************************************************
;*****                      SECTOR DE ARRANQUE DEL SISTEMA FDS                              *****
;************************************************************************************************

ORG 7C00h
use16
#fasm#

jmp inicio

identificador db 'FDFS'                 ; Identificador de FDFS    
version       db 0                      ; Version de FDFS
etiqueta      db 'SIN NOMBRE      ',0,0 ; 16 car de etiqueta, y el 17 y 18 son 0 para indicar que no hay mas nombre
cabezales     db 2                      ; CHS del disquete (1,44M en este caso)                
pistas        db 80
sectores      db 18
tamcluster    db 1                      ; Indica cuantos sectores forman un cluster (1 en este caso)
directraiz    dw 3                      ; N. de cluster donde esta ubicado el directorio raiz (siempre >=4)
                                        ; Los 3 primeros clusteres estan reservados y son de 1 sector independientemente
                                        ; de lo indicado en tamcluster (0 sect arranque, 1 y 2 mapa de bits de clusteres)
hora          db 0                      ; Hora y fecha de la creacion del disquete    
minuto        db 0
segundo       db 0

dia           db 0
mes           db 0
ano           db 0

clsinfoadd    dw 0                      ; Info adicional si la hubiera

inicio:

     xor ax,ax      ;Iniciamos los segmentos en la posicion correcta
     mov ds,ax
     mov es,ax  
     
     mov ss,ax      ;Iniciamos la pila justo debajo del sector de arranque
     mov sp,7D00h 
    
     mov  si,ierr    ;Mostramos el mensaje de error
     call imp_pant
     
     xor ah,ah
     int 16h
     
     int 19h 
     
     hlt

ierr db 0Dh,0Ah
     db 'Error: El disquete introducido no es arrancable',0Dh,0Ah
     db 'Extraiga el disquete y presione cualquier tecla para continuar...',0Dh,0Ah,0


;*********************************************
;***    RUTINA PARA IMPRIMIR EN PANTALLA   ***
;*** Entradas:                             ***
;*** DS:SI -> Cadena a imprimir            ***
;*********************************************
imp_pant:
push ax     ;Guardar registros 
push si

ipbuc:

lodsb
or      al,al    ;Si encontramos un 0 hemos llegado al final
je      ipfinal

mov     ah,0Eh   ;Funcion teletype
int     10h

jmp     ipbuc

ipfinal:      
pop ax            ;Liberar registros   
pop si
ret               ;Volver al programa principal

relleno DB 312 dup 0                ;Si se modifica el codigo de arriba hay que recalcular el relleno
;times ((0x1FE-$) and 0xFF) db 00h  ;Rellenar el resto del sector de arranque hasta llegar a la firma
dw 0AA55h                          ;Firma del sector de arranque           
                                                                                                               
