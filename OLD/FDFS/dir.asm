;************************************************************************************************
;*****                      ENTRADA DE DIRECTORIO DE FDFS                                   *****
;************************************************************************************************
#fasm#          
iden DB 'DIR'    ;Identificador de entrada de directorio
cdpa DW 0        ;Cluster del directorio padre, 0 si es el raiz
atrr DW 24       ;Atributos de LSB a MSB -> Oculto, sistema, solo lectura, 3 bits con los permisos
                 ;de usuario grupo y admin RWX, un bit para indicar que es inenlazable, otro para
                 ;indicar que es el original y no un enlace
enla DW 0        ;Numero de enlaces a este directorio, a 0 si no hay ninguno   

nombre DB 256 dup 0 ;Nombre 255 car max, el 256 si es distinto de 0 indica la siguiente entrada donde
                 ;sigue el nombre

meta DW 0        ;Metadatos, 0 si no hay
horac DB 0       ;Hora y fecha de creacion mod y acc
minuc DB 0
seguc DB 0                                                  
diac DB 0        
mesc DB 0
anioc DB 0
horam DB 0       ;Hora y fecha de creacion mod y acc
minum DB 0
segum DB 0                                                  
diam DB 0        
mesm DB 0
aniom DB 0
horaa DB 0       ;Hora y fecha de creacion mod y acc
minua DB 0
segua DB 0                                                  
diaa DB 0        
mesa DB 0
anioa DB 0       

subdi DB 112 dup 0 ;Tabla de subdirectorios, 0 si no hay mas, FFFF si en ese hueco no hay, pero
                   ;continua 
archv DB 114 dup 0 ;Tabla de clusters de archivos asociados a este dir, idem que arriba
