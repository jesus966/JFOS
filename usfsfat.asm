first:
DB 1h ;File
DB 'KERNEL.SYF' ;File Name
DB 10 dup (0) 
DB 1            ;Total cluster number
DB 4            ;First cluster
DB 4            ;End cluster
DB (512*2 - ($ - offset first)) DUP (0) ;Fill with 0s