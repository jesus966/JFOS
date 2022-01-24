#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "boot.c"
#include "FDFS.H"

//MAIN DEL PROGRAMA
int main(int argc, char * argv[]){
	int res = 0;
	int i = 0;
	int j = 0;
	int sal = 0;
	int verificar = 0;
	int noconfir = 0;
	int rapido = 0;
	int sectores = 0;
	//int j = 0;
	int sizeclust = 1;
	int etiquet = 0;
	time_t tiempo = NULL;
	struct tm *tlocal = NULL;
	char temp[3];
	BYTE drive = 0;
	char c = 0;
	UINT cls_lib = 0;
	UINT cls_ocp = 0;
	UINT cls_bad = 0;
	UINT bad_ocp = 0;
	UINT bad_lib = 0;
	BYTE etiqueta[18] = {'S','I','N',' ','N','O','M','B','R','E',' ',' ',' ',' ',' ',' ',' ',0};
	BYTE C = 80;
	BYTE H = 2;
	BYTE S = 18;

	//Si recibimos parametros los procesamos
	if (argc > 1){

		//Buscamos el signo de ?
		for (i = 1; i<argc && sal == 0; i++){
			if ((argv[i][0] == '-' || argv[i][0] == '/') && argv[i][1] == '?'){
				sal = 1;

				//Imprimimos la ayuda
				printf("Formatea un disquete con el sistema de archivos FDFS.\n");
				printf("\n");
				printf("Uso:\n");
				printf("\n");
				printf("FSFORMAT.EXE [unidad:] [/T:X] [/H:X] [/C:X] [/S:X] [/U:X] [/E:X] [/V] [/N] [/Q] [/B] [/?]");
				printf("\n\n");
				printf("unidad:		Indica la unidad que va a ser formateada (A: o B:)\n");
				printf("\n");
				printf("/T:X		Indica el tipo de formato del disquete (360, 1.22, 720, 1.44).\n");
				printf("/H:X		Indica el numero de cabezales del disquete. Sobreescribe a T.\n");
				printf("/C:X		Indica el numero de cilindros del disquete. Sobreescribe a T.\n");
				printf("/S:X		Indica el numero de sectores del disquete. Sobreescribe a T.\n");
				printf("/U:X		Indica el tamanio del cluster.\n");
				printf("/E:X		Indica la etiqueta del disquete recien formateado.\n");
				printf("/V		Verifica los sectores tras formatearlos.\n");
				printf("/N		No pide confirmaciones en pantalla.\n");
				printf("/Q		Hace un formato rapido del disquete.\n");
				printf("/B		No formatea el disquete, busca sectores defectuosos.\n");
				printf("/?		Muestra esta pantalla.\n");

			}
		}

		//Buscamos el resto de parámetros
		for (i = 1; i < argc; i++){

			//Es la unidad
			if ((toupper(argv[i][0]) == 'A' || toupper(argv[i][0]) == 'B') && argv[i][1] == ':'){
				if (toupper(argv[i][0]) == 'A'){
					drive = 0;
				}else{
					drive = 1;
				}
			}

			//Opcion V
			if ((argv[i][0] == '-' || argv[i][0] == '/') && toupper(argv[i][1]) == 'V'){
				verificar = 1;
			}

			//Opcion N
			if ((argv[i][0] == '-' || argv[i][0] == '/') && toupper(argv[i][1]) == 'N'){
				noconfir = 1;
			}

			//Opcion Q
			if ((argv[i][0] == '-' || argv[i][0] == '/') && toupper(argv[i][1]) == 'Q'){
				rapido = 1;
			}

			//Opcion B
			if ((argv[i][0] == '-' || argv[i][0] == '/') && toupper(argv[i][1]) == 'B'){
				sectores = 1;
			}


			//Opcion T
			if ((argv[i][0] == '-' || argv[i][0] == '/') && toupper(argv[i][1]) == 'T' && argv[i][2] == ':'){


				if(strlen(argv[i])==6){ //Estamos en el caso de 360 o 720

					if (argv[i][3] == '3' && argv[i][4] == '6' && argv[i][5] == '0'){
						C=40;
						H=2;
						S=9;
					}else if (argv[i][3] == '7' && argv[i][4] == '2' && argv[i][5] == '0'){
						C=80;
						H=2;
						S=9;
					}

				}else if(strlen(argv[i])==7){ //Caso 1.22 o 1.44

					if (argv[i][3] == '1' && argv[i][4] == '.' && argv[i][5] == '2' && argv[i][6] == '2'){
						C=80;
						H=2;
						S=15;
					}else if (argv[i][3] == '1' && argv[i][4] == '.' && argv[i][5] == '4' && argv[i][6] == '4'){
						C=80;
						H=2;
						S=18;
					}
				}

			}

			//Opcion C
			if ((argv[i][0] == '-' || argv[i][0] == '/') && toupper(argv[i][1]) == 'C' && argv[i][2] == ':'){

				//Convertimos la cadena subyancente

				if(atoi(argv[i]+3)!=0) C=atoi(argv[i]+3);

			}

			//Opcion H
			if ((argv[i][0] == '-' || argv[i][0] == '/') && toupper(argv[i][1]) == 'H' && argv[i][2] == ':'){

				//Convertimos la cadena subyancente

				if(atoi(argv[i]+3)!=0) H=atoi(argv[i]+3);

			}

			//Opcion S
			if ((argv[i][0] == '-' || argv[i][0] == '/') && toupper(argv[i][1]) == 'S' && argv[i][2] == ':'){

				//Convertimos la cadena subyancente

				if(atoi(argv[i]+3)!=0) S=atoi(argv[i]+3);

			}

			//Opcion U
			if ((argv[i][0] == '-' || argv[i][0] == '/') && toupper(argv[i][1]) == 'U' && argv[i][2] == ':'){

				//Convertimos la cadena subyancente

				if(atoi(argv[i]+3)!=0) sizeclust=atoi(argv[i]+3);

			}

			//Opcion E
			if ((argv[i][0] == '-' || argv[i][0] == '/') && toupper(argv[i][1]) == 'E' && argv[i][2] == ':'){

				//Copiamos el substring
				strncpy(etiqueta, argv[i]+3, strlen(argv[i]+3) > 17? 17: strlen(argv[i]+3));
				etiquet = 1;
			}

		}

	}

	//Dependiendo de los datos de la linea de parametros, hacemos una cosa u otra

	if (!sectores){	//Se pidio formateo rapido o normal

		//Mostramos info de inicio
		printf("FSFORMAT - Formateo de disquete con el sistema de archivos FDFS.\n\n");
		printf("Configuracion detectada:\n\n");
		printf("Unidad: %c:, Cabezales: %u, Cilindros: %u, Sectores: %u, Formato Rapido: %s, Verificar sectores: %s, Tamanio de cluster: %u\n", drive==0?'A':'B',H,C,S,rapido==0?"NO":"SI",verificar==0?"NO":"SI",sizeclust);

		if (!noconfir){

			do{

				printf("\n");
				printf("Es correcta esta informacion (s/n)?: ");
				c = toupper(getchar());

				if (c == 'N'){
					res = 1;	//Salimos
					return res;
				}

			}while(c != 'S');

		}

		if (!rapido){	//Formateo normal...
			
			printf("Formateando disco...\n");
			//Bucle para recorrer todas las pistas
			for (i = 0; i < C; i++){
				
				//Formateamos los dos cabezales
				for(j = 0; j < H; j++){
					
					int resf = 0;

					resf = format_track(i, j, C, S, drive);
					
					if (resf == 0 && verificar == 1){
						resf = verify_sectors(1, S, i, j, drive);
					}
					
					if (resf != 0){
						printf("ERROR: No se pudo formatear el disco en el cabezal %i, pista %i.\nCodigo de error 0x%x. Disquete defectuoso.\n", j, i, resf);
						return 0xFF;
					}
					
				}
			
			}
			
			printf ("Formateo normal terminado exitosamente.\n");
		}

		//Tanto al final el formateo normal, como para el rapido, nos encargamos de crear el sistema de archivos

		//Iniciamos a 0 fs_map

		for (i = 0; i < 1024; i++){
			fs_map[i]=0;
		}

		fs_map[0]=0x55;	//El primer byte indica 4 clusters ocupados

		//Si si ha pedido confirmaciones, y etiquet es 0, preguntamos por la etiqueta
		if (!noconfir && !etiquet){

			printf("\n");
			printf("Escriba el nombre de la etiqueta de la unidad (max. 17 caracteres): ");
			getchar();
			scanf("%10[^\n]", &etiqueta);
			printf("\n");
		}


		//Rellenamos el FSCB recien formateado
		FSCB.version = 0;
		strncpy(FSCB.etiqueta, etiqueta, 17);
		FSCB.head = H;
		FSCB.cyl = C;
		FSCB.sec = S;
		FSCB.tamclus = sizeclust;
		FSCB.cdirr = 3;

		//Vamos parseando la fecha y hora
		time(&tiempo);
		tlocal = localtime(&tiempo);
		strftime(temp,3,"%d",tlocal);
		FSCB.dia = atoi(temp);
		strftime(temp,3,"%m",tlocal);
		FSCB.mes = atoi(temp);
		strftime(temp,3,"%y",tlocal);
		FSCB.anio = atoi(temp);
		strftime(temp,3,"%H",tlocal);
		FSCB.hora = atoi(temp);
		strftime(temp,3,"%M",tlocal);
		FSCB.minuto = atoi(temp);
		strftime(temp,3,"%S",tlocal);
		FSCB.segundo = atoi(temp);

		FSCB.cinfad = 0;

		memcpy(boot_bin+6, &FSCB, 32);	//Escribimos el nuevo FSCB en el sector de arranque

		//Escribimos los 4 sectores clave FALTA FUNCION PARA ESCRIBIR DIRECTORIO
		if (write_sectors(1, 0, 1, 0, drive, boot_bin)!=0 && write_map(fs_map, drive) != 0){
			printf("ERROR: No se pudo escribir la informacion del sistema de archivos. Disquete defectuoso.\n");
			res = 1;
		}else{

			//Escribimos todo bien, asi que terminamos

			printf("\n");
			printf("Disquete formateado exitosamente.\n\n");
			printf("Estadisticas:\n\n");
			fdfs_stats(&fs_map, &cls_lib, &cls_ocp, &cls_bad, &bad_ocp, &bad_lib);
			printf("Clusters libres:		%u\n",cls_lib);
			printf("Clusters ocupados:		%u\n",cls_ocp);
			printf("Clusters defectuosos:		%u\n",cls_bad);
			printf("Fecha y hora de creacion:	%d/%d/%d - %d:%d:%d\n",FSCB.dia, FSCB.mes, FSCB.anio, FSCB.hora, FSCB.minuto, FSCB.segundo);
			printf("Espacio libre total:		%.2f KBytes\n",cls_lib*0.512*sizeclust);
			printf("Espacio daniado total:		%.2f KBytes\n\n",cls_bad*0.512*sizeclust);
			
			res = 0;
			system("pause");	
			
		}
		
		
		
	}else{	//Se pedio buscar sectores malos
		
	}
	
	return res;
}