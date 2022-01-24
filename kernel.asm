;******************************************************************************************
;****                              KERNEL PRINCIPAL DE JFCONSOLE                       ****
;****                          Copyright (c) Jesus Fernandez Gamito                    ****
;******************************************************************************************

USE16			;Crear el kernel de 16 bits

ORG	0h		;Empezamos en el segmento 0000

jmp	inicio		;Saltar al inicio del kernel

msgbien DB 'Bienvenido a JFConsole',0Dh,0Ah,0Dh,0Ah,0

lincom	DB 'DQ1=> ',0

tamcmd	EQU 20		;Tamano de la linea de comando

cmdbuf	DB tamcmd DUP("b")
cmdlmp	DB tamcmd DUP(" "),0 

desc	DB 0Dh,0Ah,0Dh,0Ah,'Comando o nombre de archivo no reconocido',0Dh,0Ah,0Dh,0Ah,0

creini	DB 'reiniciar'
creini_cola:

inicio:

mov	ax,0600h
mov	ds,ax

call int_init		;Iniciar interrupcion 22h

mov	ax,0600h
mov	es,ax

xor	ah,ah		;Establecer modo de video
mov	al,03h
int	10h

xor	ax,ax		;Ponemos el mensaje de bienvenida
mov	si,msgbien
int	22h  
						       
bucle_eterno:		;Hacemos el bucle eterno principal de JFConsole

call	cojer_comando

call	procesar_comando
						       
jmp	bucle_eterno	   

cojer_comando:	      

xor	ax,ax		;Mostramos el prompt
mov	si,lincom
int	22h

mov	dx,tamcmd	;Cojer el texto
mov	di,cmdbuf
call	cojer_texto   

ret  

procesar_comando:	;Procesamos los comandos

push	ds
pop	es

cld

mov	si,cmdbuf	;Comprobamos si es este comando
mov	di,creini
mov	cx,creini_cola - creini
repe	cmpsb
je	csalir_cmd	;Saltar al comando si es ese


;Si llega a aqui no se conoce el comando
xor	ax,ax
mov	si,desc
int	22h

ret

csalir_cmd:
jmp $

cojer_texto:
push	ax
push	cx
push	di
push	dx

mov	cx, 0			; char counter.

cmp	dx, 1			; buffer too small?
jbe	vb			;

dec	dx			; reserve space for last zero.


eut:

mov	ah, 0			; get pressed key.
int	16h

cmp	al, 0Dh 		; 'return' pressed?
jz	exitct


cmp	al, 8			; 'backspace' pressed?
jne	aab
jcxz	eut			; nothing to remove!
dec	cx
dec	di
	
mov	al, 8
mov	ah, 0eh
int	10h			  ; backspace.

mov	al,' '
mov	ah, 0eh
int	10h			  ; backspace.

mov	al, 8
mov	ah, 0eh
int	10h			  ; backspace.

jmp	eut

aab:

	cmp	cx, dx		; buffer is full?
	jae	eut	     ; if so wait for 'backspace' or 'return'...

	mov	[di], al
	inc	di
	inc	cx
	
	; print the key:
	mov	ah, 0eh
	int	10h

jmp	eut

exitct:

; terminate by null:
xor	di,di

vb:

pop	dx
pop	di
pop	cx
pop	ax
ret	     

include "funciones.inc" ;Incluimos todas las funciones del kernel