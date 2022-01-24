/********************************************************************************
**
** Autor: Jesús Fernández Gamito
** Fecha: 16/01/2018
** Proyecto: FDFS
**
**
**********************************************************************************/
#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

//********************************************************************************************************************
//**	BYTE read_sectors(BYTE number_sectors_read, BYTE track, BYTE sector, BYTE head, BYTE drive, BYTE * buffer)  **
//**																												**
//**	Lee los sectores usando la int 13h, se le pasa como parametros el nº de sectores a leer, el CHS, la unidad  **
//**	Y una tabla de BYTES (que debe ser de number_sectors_read*512 de tamanio para que no haya problemas)		**
//**    Devuelve el valor de AH de la int 13h, siendo 0 si todo fue bien o el codigo de error si lo hubo			**
//********************************************************************************************************************
BYTE read_sectors(BYTE number_sectors_read, BYTE track, BYTE sector, BYTE head, BYTE drive, BYTE * buffer){
	char far * bufptr;
    union REGS inregs, outregs;
    struct SREGS segregs;

    segread(&segregs);           /* set the segment registers */
    bufptr = (char far *) buffer;   /* need a far pointer to 'buf' */
    segregs.es = FP_SEG(bufptr); /* ...to set the address of */
    inregs.x.bx = FP_OFF(bufptr);/* ...'buf' for the BIOS */
    inregs.h.ah = 2;             /* set BIOS function number */
    inregs.h.al = number_sectors_read;             /* set # of sectors to read */
    inregs.h.ch = track;             /* set track # of boot sector */
    inregs.h.cl = sector;             /* set sector # of boot sector */
    inregs.h.dh = head;             /* set disk side number */
    inregs.h.dl = drive;             /* set drive number to A: */
	int86x(0x13, &inregs, &outregs, &segregs);  /* read sector */

	//outregs.x.cflag
	return outregs.h.ah;
}

//*************************************
//**	BYTE reset_disk(BYTE drive)  **
//**							     **
//**    Resetea la unidad indicada	 **
//*************************************
BYTE reset_disk(BYTE drive){

	union REGS inregs, outregs;	//Registros de entrada y de salida

	inregs.h.ah=0;			//Funcion de resetear el disco
	inregs.h.dl=drive;			//Unidad a resetear

	int86(0x13, &inregs, &outregs);	//Llamamos a la interrupcion 13h

	return(outregs.h.ah);			//Devolvemos el estado de la operacion

}

//****************************************************************************************************************************
//**	BYTE write_sectors(BYTE number_sectors_write, BYTE track, BYTE sector, BYTE head, BYTE drive, BYTE * buffer)		**
//**																														**
//**	Escribe los sectores usando la int 13h, se le pasa como parametros el nº de sectores a escribir, el CHS, la unidad  **
//**	Y una tabla de BYTES a escribir (que debe ser de number_sectors_write*512 de tamanio para que no haya problemas)	**
//**    Devuelve el valor de AH de la int 13h, siendo 0 si todo fue bien o el codigo de error si lo hubo			        **
//**	Así mismo, si detecta error de escritura, intenta reintentarlo un maximo de RETRY veces antes de devolver fallo		**
//****************************************************************************************************************************
BYTE write_sectors(BYTE number_sectors_write, BYTE track, BYTE sector, BYTE head, BYTE drive, BYTE * buffer){
	char far * bufptr;
    union REGS inregs, outregs;
    struct SREGS segregs;

    segread(&segregs);           /* set the segment registers */
    bufptr = (char far *) buffer;   /* need a far pointer to 'buf' */
    segregs.es = FP_SEG(bufptr); /* ...to set the address of */
    inregs.x.bx = FP_OFF(bufptr);/* ...'buf' for the BIOS */
    inregs.h.ah = 3;             /* set BIOS function number */
    inregs.h.al = number_sectors_write;             /* set # of sectors to read */
    inregs.h.ch = track;             /* set track # of boot sector */
    inregs.h.cl = sector;             /* set sector # of boot sector */
    inregs.h.dh = head;             /* set disk side number */
    inregs.h.dl = drive;             /* set drive number to A: */
	int86x(0x13, &inregs, &outregs, &segregs);  /* write sector */

	if(outregs.x.cflag){		//Si hubo un error escribiendo, reintentamos la escritura
								//con RETRY intentos, reseteando el disco entre escriturasº
		int i = 0;
		for (i=0; i<RETRY && outregs.x.cflag; i++){
			reset_disk(drive);
			int86x(0x13, &inregs, &outregs, &segregs);
		}

	}

	return outregs.h.ah;		//Pasamos resultado final (sea exito o error)
}

//*******************************************************
//**	UINT CHS_to_cluster(BYTE C, BYTE H, BYTE S)	   **
//**												   **
//**	Convierte los parametros CHS indicados a	   **
//**	clusters, FSCB tiene que ser cargado antes.    **
//*******************************************************
UINT CHS_to_cluster(BYTE C, BYTE H, BYTE S){
	UINT LBA = 0;
	UINT i, j, clus = 0;

	//Conversion LBA
	LBA = ((((C * (FSCB.head-1)) + H) * FSCB.sec) +  (S - 1));

	if (LBA <= 2 || FSCB.tamclus == 1){	//Si son los 3 primeros sectores, o el tamclus es 1, los devolvemos tal cual
		return LBA;
	}else{
		LBA = LBA - 3;

		//Recorremos hasta LBA,contando clusteres cada FSCB.tamclus
		for ( i=0, j=0; i<=LBA; i++, j++){
			 if (j >= FSCB.tamclus){
				 j=0;
				 clus++;
			 }
		}

		return clus+3;

	}

}

//****************************************************************************
//**	void cluster_to_CHS(UINT cluster, BYTE * C, BYTE * H, BYTE * S)		**
//**																		**
//**	Convierte un cluster a CHS, hace falta que el FSCB este cargado.	**
//****************************************************************************
void cluster_to_CHS(UINT cluster, BYTE * C, BYTE * H, BYTE * S){

	if (!(cluster <=2 || FSCB.tamclus == 1)){
		//Tenemos que traducir de nº de cluster a nº de LBA
		cluster = cluster-3;

		//Ahora tenemos la conversion hecha en LBA
		cluster = ((cluster*FSCB.tamclus)+3);
	}

	*C=cluster/((FSCB.head-1)*FSCB.sec);
	*H=(cluster/FSCB.sec)%(FSCB.head-1);
	*S=(cluster%FSCB.sec)+1;

}

//********************************************************************************************
//**	BYTE write_cluster(UINT cluster, BYTE drive, BYTE * buffer)							**
//**																						**
//**	Escribe clusters en el disco.														**
//**	Y los datos a escribir estan en buffer, que deberia de ser tamclus*512				**
//**	Devuelve 0 si no hubo error, o AH en caso de error									**
//********************************************************************************************
BYTE write_cluster(UINT cluster, BYTE drive, BYTE * buffer){
	BYTE C,H,S=0;
	BYTE res = 0;
	BYTE leidos = 0;

	//Primero, obtenemos el CHS del inicio del cluster
	cluster_to_CHS(cluster, &C, &H, &S);

	//A continuación, escribimos, pero hay que tener en cuenta que write_sectors no escribimos
	//entre pistas, así que tenemos que avanazar la pista (e incluso el cabezal) si
	//es necesario.

	if (FSCB.tamclus == 1 || (C==0 && H==0 && S<=3)){

		// Si los clusters son de 1 sector, simplemente lo escribimos
		res = write_sectors(1, C, S, H, drive, buffer);

	}else{
		//Si el total de los sectores a escribir supera a los sectores por pista...
		if (S+(FSCB.tamclus-1) > FSCB.sec){

			//Primero, escribimos los sectores que sean necesarios hasta la siguiente pista
			leidos = (FSCB.sec-S)+1;
			res = write_sectors(leidos, C, S, H, drive, buffer);

			if (res == 0){ //Si no hubo errores...

				S=1;	//S lo ponemos a 1

				if (C==(FSCB.cyl-1)){ //Si hemos llegado al cilindro final
					C=0;			  //Ponemos el cilindro a 0 e incrementamos el cabezal
					H++;
				}else{
					C++;
				}

				//Escribimos lo que nos falte
				res = write_sectors(FSCB.tamclus-leidos, C, S, H, drive, buffer+(512*leidos));

			}

		}else{

			//Si no supera a los sectores de la pista, simplemente leemos
			res = write_sectors(FSCB.tamclus, C, S, H, drive, buffer);
		}

	}

	return res;
}


//****************************************************************************
//**	BYTE load_cluster(UINT cluster, BYTE drive, BYTE * buffer)			**
//**																		**
//**	Carga un cluster en el buffer, que debe de ser de tamclus de tam.	**
//**	Devuelve 0 si no hubo error, o AH en caso de error					**
//****************************************************************************
BYTE load_cluster(UINT cluster, BYTE drive, BYTE * buffer){
	BYTE C,H,S=0;
	BYTE res = 0;
	BYTE leidos = 0;

	//Primero, obtenemos el CHS del inicio del cluster
	cluster_to_CHS(cluster, &C, &H, &S);

	//A continuación, leemos, pero hay que tener en cuenta que read_sectors no leemos
	//entre pistas, así que tenemos que avanazar la pista (e incluso el cabezal) si
	//es necesario.

	if (FSCB.tamclus == 1 || (C==0 && H==0 && S<=3)){

		// Si los clusters son de 1 sector, simplemente lo leemos y ya
		res = read_sectors(1, C, S, H, drive, buffer);

	}else{
		//Si el total de los sectores a leer supera a los sectores por pista...
		if (S+(FSCB.tamclus-1) > FSCB.sec){

			//Primero, leemos los sectores que sean necesarios hasta la siguiente pista
			leidos = (FSCB.sec-S)+1;
			res = read_sectors(leidos, C, S, H, drive, buffer);

			if (res == 0){ //Si no hubo errores...

				S=1;	//S lo ponemos a 1

				if (C==(FSCB.cyl-1)){ //Si hemos llegado al cilindro final
					C=0;			  //Ponemos el cilindro a 0 e incrementamos el cabezal
					H++;
				}else{
					C++;
				}

				//Leemos lo que nos falte
				res = read_sectors(FSCB.tamclus-leidos, C, S, H, drive, buffer+(512*leidos));

			}

		}else{

			//Si no supera a los sectores de la pista, simplemente leemos
			res = read_sectors(FSCB.tamclus, C, S, H, drive, buffer);
		}

	}

	return res;
}

//************************************************************************************************
//**	BYTE load_clusters(UINT cluster, BYTE drive, BYTE * buffer, UINT clst_tot)				**
//**																							**
//**	Carga varios clusteres en el buffer, el buffer deberia de ser de tamclus*512*clst_tot	**
//**	Devuelve 0 si no hubo error, o AH en caso de error										**
//************************************************************************************************
BYTE load_clusters(UINT cluster, BYTE drive, BYTE * buffer, UINT clst_tot){
	BYTE res = 0;
	UINT i, cr = 0;

	cr = cluster;

	//Vamos recorriendo el buffer, leyendo de 1 a 1 cluster
	for (i=0; (i<clst_tot && res == 0); i++){
		res = load_cluster(cr, drive, buffer+(512*FSCB.tamclus*(cr-cluster)));
		cr++;
	}

	return res;
}


//************************************************************************************************
//**	BYTE write_clusters(UINT cluster, BYTE drive, BYTE * buffer, UINT clst_tot)				**
//**																							**
//**	Escribe varios clusteres en el buffer, el buffer deberia de ser de tamclus*512*clst_tot	**
//**	Devuelve 0 si no hubo error, o AH en caso de error										**
//************************************************************************************************
BYTE write_clusters(UINT cluster, BYTE drive, BYTE * buffer, UINT clst_tot){
	BYTE res = 0;
	UINT i, cr = 0;

	cr = cluster;

	//Vamos recorriendo el buffer, escribiendo de 1 a 1 cluster
	for (i=0; (i<clst_tot && res == 0); i++){
		res = write_cluster(cr, drive, buffer+(512*FSCB.tamclus*(cr-cluster)));
		cr++;
	}

	return res;
}

//*****************************************************************************************************
//**	BYTE verify_sectors(BYTE sector_start, BYTE sector_total, BYTE track, BYTE head, BYTE drive) **
//**																								 **
//**	Verifica los sectores especificados															 **
//**	Devuelve 0 si no hubo error, o AH en caso de error										     **
//*****************************************************************************************************

BYTE verify_sectors(BYTE sector_start, BYTE sector_total, BYTE track, BYTE head, BYTE drive){
    union REGS inregs, outregs;
    struct SREGS segregs;

    inregs.h.ah = 0x04;             /* set BIOS function number */
    inregs.h.al = sector_total;             /* set # of sectors to read */
    inregs.h.ch = track;             /* set track # of boot sector */
    inregs.h.cl = sector_start;           		 /* set sector # of boot sector */
    inregs.h.dh = head;             /* set disk side number */
    inregs.h.dl = drive;             /* set drive number to A: */
	int86(0x13, &inregs, &outregs);  /* write sector */

	if(outregs.x.cflag){		//Si hubo un error escribiendo, reintentamos la escritura
								//con RETRY intentos, reseteando el disco entre escriturasº
		int i = 0;
		for (i=0; i<RETRY && outregs.x.cflag; i++){
			reset_disk(drive);
			int86x(0x13, &inregs, &outregs, &segregs);
		}

	}

	return outregs.h.ah;		//Pasamos resultado final (sea exito o error)
}

//*****************************************************************************************************
//**	BYTE format_track(BYTE track, BYTE head, BYTE tracks, BYTE sectors, BYTE drive)				 **
//**																								 **
//**	Formatea la pista indicada																	 **
//**	Devuelve 0 si no hubo error, o AH en caso de error										     **
//*****************************************************************************************************
BYTE format_track(BYTE track, BYTE head, BYTE tracks, BYTE sectors, BYTE drive){
	char far * bufptr;
	BYTE * buffer = NULL;
	typedef struct {
		BYTE trackn;
		BYTE headn;
		BYTE sectn;
		BYTE sects;
	}ENTRADA;
	ENTRADA entrada;
    union REGS inregs, outregs;
    struct SREGS segregs;
	int i = 0;
	int j = 0;

	inregs.h.ah=0x18;			//Establecer el medio a formatear
	inregs.h.ch=tracks;
	inregs.h.cl=sectors;
	inregs.h.dl=drive;			//Unidad a resetear

	int86(0x13, &inregs, &outregs);	//Llamamos a la interrupcion 13h
	
	if (outregs.h.ah != 0x80){	//Si hay un disco introducido (o no soporta la funcion)
		
		//Creamos el buffer con la info adecuada
		buffer = calloc(sectors, sizeof(ENTRADA));
		
		if (buffer == NULL){
			return 0xFF;
		}
		
		entrada.trackn = track;
		entrada.headn = head;
		entrada.sects = 0x02;
	
		for (i = 1, j = 0; i<=sectors; i++ , j=j+sizeof(ENTRADA)){
			
			entrada.sectn = i;
			
			memcpy(buffer+j,&entrada,sizeof(ENTRADA));
			
		}
	
		segread(&segregs);           /* set the segment registers */
		bufptr = (char far *) buffer;   /* need a far pointer to 'buf' */
		segregs.es = FP_SEG(bufptr); /* ...to set the address of */
		inregs.x.bx = FP_OFF(bufptr);/* ...'buf' for the BIOS */
		inregs.h.ah = 0x05;             /* set BIOS function number */
		inregs.h.al = sectors;             /* set # of sectors to read */
		inregs.h.ch = track;             /* set track # of boot sector */
		inregs.h.dh = head;             /* set disk side number */
		inregs.h.dl = drive;             /* set drive number to A: */
		int86x(0x13, &inregs, &outregs, &segregs);  /* write sector */

		if(outregs.x.cflag){		//Si hubo un error escribiendo, reintentamos la escritura
									//con RETRY intentos, reseteando el disco entre escriturasº
			int i = 0;
			for (i=0; i<RETRY && outregs.x.cflag; i++){
				reset_disk(drive);
				int86x(0x13, &inregs, &outregs, &segregs);
			}

		}		
	}			

	return outregs.h.ah;		//Pasamos resultado final (sea exito o error)
}

/*
//Main de prueba
void main(){
	UINT res = 0;
	BYTE C,H,S=0;
	UINT cluster = 0;
	UINT cls_lib = 0;
	UINT cls_ocp = 0;
	UINT cls_bad = 0;
	UINT bad_ocp = 0;
	UINT bad_lib = 0;
	UINT i = 0;
	res=load_FSCB(0);
	if (res == 0 || res == 65535){

		if (res == 65535){
			printf ("El sistema de archivos del disquete no se corresponde con el sistema FDFS.\n");
		}else{
			printf ("Informacion del sistema de archivos FDFS:\n");
			printf ("Version: %x\n", FSCB.version);
			printf ("Etiqueta: %s\n", FSCB.etiqueta);
			printf ("Cabezales: %u\n", FSCB.head);
			printf ("Cilindros: %u\n", FSCB.cyl);
			printf ("Sectores:  %u\n", FSCB.sec);
			printf ("Tamanio del cluster: %u\n", FSCB.tamclus);
			printf ("Cluster del directorio raiz: %u\n", FSCB.cdirr);
			printf ("Hora: %u\n", FSCB.hora);
			printf ("Minuto: %u\n", FSCB.minuto);
			printf ("Segundo: %u\n", FSCB.segundo);
			printf ("Dia:  %u\n", FSCB.dia);
			printf ("Mes:  %u\n", FSCB.mes);
			printf ("Anio: %u\n", FSCB.anio);
			printf ("Dir. cluster de info adicional: %u\n", FSCB.cinfad);
		}



		for (i = 0; i<1024; i++){
			fs_map[i]=0;
		}

		load_map(&fs_map, 0);
		fdfs_stats(&fs_map, &cls_lib, &cls_ocp, &cls_bad, &bad_ocp, &bad_lib);
		printf("Estadisticas: Libres: %u, Ocupados: %u, Malos: %u, Malos ocupados: %u, Malos libres: %u\n", cls_lib, cls_ocp, cls_bad, bad_ocp, bad_lib);

		printf("Primer cluster libre: %u\n",find_free_cluster(&fs_map));
		set_used_cluster(&fs_map, 4);
		printf("Primer cluster libre: %u\n",find_free_cluster(&fs_map));
		set_free_cluster(&fs_map, 4);
		printf("Primer cluster libre: %u\n",find_free_cluster(&fs_map));
		set_bad_cluster(&fs_map, 4);
		fdfs_stats(&fs_map, &cls_lib, &cls_ocp, &cls_bad, &bad_ocp, &bad_lib);
		printf("Estadisticas: Libres: %u, Ocupados: %u, Malos: %u, Malos ocupados: %u, Malos libres: %u\n", cls_lib, cls_ocp, cls_bad, bad_ocp, bad_lib);
		set_good_cluster(&fs_map, 4);
		fdfs_stats(&fs_map, &cls_lib, &cls_ocp, &cls_bad, &bad_ocp, &bad_lib);
		printf("Estadisticas: Libres: %u, Ocupados: %u, Malos: %u, Malos ocupados: %u, Malos libres: %u\n", cls_lib, cls_ocp, cls_bad, bad_ocp, bad_lib);

		cls_ocp = 0;
		cls_lib = 0;
		cls_ocp = find_free_clusters(&fs_map, &cls_lib);
		printf ("Primer cluster encontrado libre: %u, Clusters libres consecutivos: %u\n", cls_ocp, cls_lib);
		set_used_cluster(&fs_map, 11);
		cls_ocp = find_free_clusters(&fs_map, &cls_lib);
		printf ("Primer cluster encontrado libre: %u, Clusters libres consecutivos: %u\n", cls_ocp, cls_lib);

		for (i=0; i<1440; i++) set_used_cluster(&fs_map, i);

			cls_ocp = find_free_clusters(&fs_map, &cls_lib);
		printf ("Primer cluster encontrado libre: %u, Clusters libres consecutivos: %u\n", cls_ocp, cls_lib);

		system("pause");
	}else{
		printf("Ocurrio un error, AH = %X\n",res);
	}
}*/