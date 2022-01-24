;******************************************************************
;****           SECTOR DE ARRANQUE DEL SISTEMA OPERATIVO        ***
;******************************************************************


                                          
#fasm#              ;Usamos FASM

#make_boot#

     ORG 7C00h       ;Cargamos en el segmento 7C00h
    
     jmp ini         ;Vamos al codigo ejecutable

db   'SDS'           ;Identificador del formato Sistema de Disquete Simple (SDS)
db   00h             ;Identificador de version de SDS (version 0.0)
db '                ';Etiqueta del disquete (16 caracteres max)
db   03h             ;Tipo de disquete (0-360K 1-1,2M 2-720K 3-1,44M)
dw   2880            ;Numero total de sectores del disquete (1,44M -> 2880)
dw   0003            ;Numero del sector en el que empieza la tabla de archivos
dd   081115h         ;Fecha de creacion del disquete DD/MM/AA
dd   145101h         ;Hora de creacion HHMMSS


ini: xor ax,ax      ;Iniciamos los segmentos en la posicion correcta
     mov ds,ax
     mov es,ax  
     
     mov ss,ax      ;Iniciamos la pila justo debajo del sector de arranque
     mov sp,7D00h 
    
     mov si,ierr    ;Mostramos el mensaje de error
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


times ((0x1FE-$) and 0xFF) db 00h  ;Rellenar el resto del sector de arranque hasta llegar a la firma
dw 0AA55h                          ;Firma del sector de arranque           

;*******************
;TABLA DE ARCHIVOS:
;Cada entrada contiene los siguientes datos:
;4Ah -> Si hay una entrada, 00 -> si la entrada esta vacia
;19 bytes con el nombre de archivo
;0 -> Atributos del archivo (0 - Normal, 1- Oculto, 2 - Sistema)
;4 bytes con la fecha de creacion
;4 bytes con la hora de creacion
;2 bytes con el primer sector en el que empieza el archivo   
;1 byte con el n. de sectores que ocupa ese bloque

;Cada tabla ocupa 32 bytes, lo que nos deja un total de 15 archivos por sector
;Recordemos que la tabla de archivos esta en los primeros 480 bytes, los 32 ultimos
;son para lo de aqui abajo:
;Los 32 ultimos bytes del sector de tabla indican:
;FF -> Fin de la tabla, 00->La tabla no se acaba todavia
;2 bytes -> Siguiente sector donde esta la siguiente tabla
;Resto -> 06h hasta el final como medida para comprobar que el sector es correcto.

;Luego en cada sector leido, los primeros 2 bytes indican donde esta ubicado el siguiente bloque del archivo 
;y el siguiente byte indica cuantos sectores consecutivos ocupa ese bloque, siendo el minimo 1 y el maximo 
;125 (64K). Si es FFFF, entonces es que se trata del ultimo sector del archivo. 
;Es decir, para leer un bloque, es necesario leer el byte 3, y luego leer el n. de sector actual + el contenido del byte 3
;y luego seguir leyendo en el siguiente bloque indicado por los 2 primeros bytes o si por el contrario es FFFF, terminar
;de leer el archivo.
;CUIDADO: En el 3 byte, el bit mas significativo indica si el bloque esta roto (tiene algun sector defectuoso), si es 0
;no hay problema, si es 1 entonces el bloque esta roto.   

;Hay dos sectores especiales, el sector 1 y 2, que son los siguientes del sector de arranque.
;El sector 1, los primeros 360 bytes indican un mapa de sectores que estan siendo utilizados y los libres. Cada byte representa
;los 8 sectores correspondientes a su posicion, de MSB a LSB. Se pone un 1 si el sector esta libre y un 0 si esta ocupado.
;El sector 2, los primeros 360 bytes indican un mapa de sectores defectuosos. Si un sector esta defectuoso se marca con un 1,
;mientras que si no esta a 0