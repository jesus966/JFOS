;************************************************************************************************
;*****                      SECTOR 1 DEL MAPA DE BITS DE FDFS                               *****
;************************************************************************************************
#fasm#
;db 00000000b
;        --
;        ||---> 1 si esta ocupado, 0 si libre
;        |----> 1 si esta daniado, 0 si no
;Los 4 primeros clusteres, por lo general siempre estan ocupados y sin dañar (3210)
buf db 512 dup 0
;times ((0x1FE-$) and 0xFF) db 00h  ;Rellenar el resto del sector
;dw 0000h                                
                                                                                                               
