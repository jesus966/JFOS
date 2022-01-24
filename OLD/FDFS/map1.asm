;************************************************************************************************
;*****                      SECTOR 1 DEL MAPA DE BITS DE FDFS                               *****
;************************************************************************************************
#fasm#
db 01010101b
;        --
;        ||---> 1 si esta ocupado, 0 si libre
;        |----> 1 si esta daniado, 0 si no
;Los 4 primeros clusteres, por lo general siempre estan ocupados y sin dañar (3210)
buf db 511 dup 0
;times  db 00h  ;Rellenar el resto del sector
;dw 0000h                                
                                                                                                               
