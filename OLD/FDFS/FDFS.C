//****************************************************
//**	BYTE load_map(BYTE * map, BYTE drive)		**
//**	Carga el mapa de bits de la unidad 			**
//**	indicada. Retorna !=0 si error				**
//****************************************************
BYTE load_map(BYTE * map, BYTE drive){
	BYTE res = 0;

	res = load_clusters(1, drive, map, 2);

	return res;
}

//****************************************************
//**	BYTE write_map(BYTE * map, BYTE drive)		**
//**	Escribe el mapa de bits de la unidad 		**
//**	indicada. Retorna !=0 si error				**
//****************************************************
BYTE write_map(BYTE * map, BYTE drive){
	BYTE res = 0;

	res = write_clusters(1, drive, map, 2);

	return res;
}

//****************************************************************
//**	void set_free_cluster(BYTE * map, UINT cluster)   		**
//**	Establece el cluster indicado como libre en el mapa.    **
//****************************************************************
void set_free_cluster(BYTE * map, UINT cluster){
	BYTE agrupacion = 0;
	BYTE pos = 0;
	//Obtenemos el Byte del mapa en el que esta el cluster que queremos
	//mapear
	agrupacion = map[(cluster/4)];
	
	//Posicion del cluster dentro de los grupos de 4 clusteres 
	pos = (cluster%4)*2;
	
	//Ponemos un 0 en la posicion
	agrupacion &= ~(1<<pos);
	
	//Lo metemos en la tabla
	map[(cluster/4)] =  agrupacion;
	
}

//****************************************************************
//**	void set_used_cluster(BYTE * map, UINT cluster)   		**
//**	Establece el cluster indicado como usado en el mapa.    **
//****************************************************************
void set_used_cluster(BYTE * map, UINT cluster){
	BYTE agrupacion = 0;
	BYTE pos = 0;
	//Obtenemos el Byte del mapa en el que esta el cluster que queremos
	//mapear
	agrupacion = map[(cluster/4)];
	
	//Posicion del cluster dentro de los grupos de 4 clusteres 
	pos = (cluster%4)*2;
	
	//Ponemos un 1 en la posicion
	agrupacion |= (1<<pos);
	
	//Lo metemos en la tabla
	map[(cluster/4)] =  agrupacion;
	
}

//****************************************************************
//**	void set_bad_cluster(BYTE * map, UINT cluster)   		**
//**	Establece el cluster indicado como malo en el mapa.     **
//****************************************************************
void set_bad_cluster(BYTE * map, UINT cluster){
	BYTE agrupacion = 0;
	BYTE pos = 0;
	//Obtenemos el Byte del mapa en el que esta el cluster que queremos
	//mapear
	agrupacion = map[(cluster/4)];
	
	//Posicion del cluster dentro de los grupos de 4 clusteres 
	pos = ((cluster%4)*2)+1;
	
	//Ponemos un 1 en la posicion
	agrupacion |= (1<<pos);
	
	//Lo metemos en la tabla
	map[(cluster/4)] =  agrupacion;
	
}

//****************************************************************
//**	void set_good_cluster(BYTE * map, UINT cluster)   		**
//**	Establece el cluster indicado como bueno en el mapa.    **
//****************************************************************
void set_good_cluster(BYTE * map, UINT cluster){
	BYTE agrupacion = 0;
	BYTE pos = 0;
	//Obtenemos el Byte del mapa en el que esta el cluster que queremos
	//mapear
	agrupacion = map[(cluster/4)];
	
	//Posicion del cluster dentro de los grupos de 4 clusteres 
	pos = ((cluster%4)*2)+1;
	
	//Ponemos un 0 en la posicion
	agrupacion &= ~(1<<pos);
	
	//Lo metemos en la tabla
	map[(cluster/4)] =  agrupacion;
	
}
//****************************************************
//**	UINT find_free_cluster(BYTE * map)   		**
//**	Devuelve el primer cluster libre    		**
//**	del mapa. 0 si no hay mas libres.		    **
//****************************************************
UINT find_free_cluster(BYTE * map){
	UINT res = 0;
	UINT cls_totales = 0;
	int i,j =0;
	int encontrado = 0;
	BYTE agrupacion = 0;
	BYTE cls[4];

	//Obtenemos el maximo de clusters de la unidad. OJO si tamclus es par estamos desperdiciando un cluster
	cls_totales = ((FSCB.head * FSCB.cyl * FSCB.sec)/FSCB.tamclus);

	for (i=0; (i<cls_totales/4 && encontrado == 0); i++){

		agrupacion = map[i];	//Obtenemos el BYTE que tiene agrupado 4 clusteres

		//Obtenemos los datos de cada uno
		cls[0] = agrupacion & 3;
		cls[1] = (agrupacion & 12) >> 2;
		cls[2] = (agrupacion & 48) >> 4;
		cls[3] = (agrupacion & 192) >> 6;

		//Vamos actualizando las estadisticas
		for (j=0; (j<4 && encontrado == 0); j++){

			if (cls[j] == 0){
				encontrado = 1;
			}else{
				res++;
			}
		}

	}

	if (encontrado == 0 ) res = 0;

	return res;
}

//********************************************************************
//**	UINT find_free_clusters(BYTE * map, UINT * tamanio)   		**
//**	Busca el primer bloque de clusters libres consecutivos    	**
//**	del mapa. 0 si no hay mas libres, devuelve la posicion		**
//**	del primero en otro caso, y en tamanio indica cuantos		**
//**	hay libres consecutivos a ese								**
//********************************************************************
UINT find_free_clusters(BYTE * map, UINT * tamanio){
	UINT res = 0;
	UINT cls_totales = 0;
	int i,j =0;
	int encontrado, contando = 0;
	BYTE agrupacion = 0;
	BYTE cls[5];
	int bandera = 0;
	UINT tam = 0;
	*tamanio = 0;
	encontrado = 0;
	contando = 0;
	res = 0;

	//Obtenemos el maximo de clusters de la unidad. OJO si tamclus es par estamos desperdiciando un cluster
	cls_totales = ((FSCB.head * FSCB.cyl * FSCB.sec)/FSCB.tamclus);

	for (i=0; (i<cls_totales/4 && encontrado == 0); i++){


		agrupacion = map[i];	//Obtenemos el BYTE que tiene agrupado 4 clusteres

		//Obtenemos los datos de cada uno
		cls[0] = agrupacion & 3;
		cls[1] = (agrupacion & 12) >> 2;
		cls[2] = (agrupacion & 48) >> 4;
		cls[3] = (agrupacion & 192) >> 6;
		cls[4] = 1;

		//Vamos actualizando las estadisticas
		for (j=0; ((contando != 0 && cls[j] == 0 && j<4) || j<4 && encontrado == 0); j++){

			if (cls[j] == 0){
				bandera = 1;
				contando = 1;
				tam++;
			}else{
				if (contando != 0){
					encontrado = 1;
				}else{
					res++;
				}

			}
		}

	}

	if (!bandera) res = 0;

	*tamanio = tam;
	return res;
}

//********************************************************************************************************************
//**	void fdfs_stats(BYTE * map, UINT * cls_lib, UINT * cls_ocp, UINT * cls_bad, UINT * bad_ocp, UINT * bad_lib)	**			**
//**																												**
//**	Devuelve las estadisticas del mapa de bits cargado en memoria. CLS_LIB indica los clusteres libres totales  **
//**	CLS_OCP los ocupados, CLS_BAD el numero total de defectuosos, BAD_OCP los defectuosos ocupados, 			**
//**	BAD_LIB los defectuosos libres, cabe decir que CLS_OCP = BAD_OCP + BAD_LIB									**
//********************************************************************************************************************
void fdfs_stats(BYTE * map, UINT * cls_lib, UINT * cls_ocp, UINT * cls_bad, UINT * bad_ocp, UINT * bad_lib){

	UINT cls_totales = 0;
	int i = 0;
	int j = 0;
	BYTE agrupacion = 0;
	BYTE cls[4];

	*cls_lib = 0;
	*cls_ocp = 0;
	*cls_bad = 0;
	*bad_ocp = 0;
	*bad_lib = 0;

	//Obtenemos el maximo de clusters de la unidad. OJO si tamclus es par estamos desperdiciando un cluster
	cls_totales = ((FSCB.head * FSCB.cyl * FSCB.sec)/FSCB.tamclus);

	for (i=0; i<cls_totales/4; i++){

		agrupacion = map[i];	//Obtenemos el BYTE que tiene agrupado 4 clusteres

		//Obtenemos los datos de cada uno
		cls[0] = agrupacion & 3;
		cls[1] = (agrupacion & 12) >> 2;
		cls[2] = (agrupacion & 48) >> 4;
		cls[3] = (agrupacion & 192) >> 6;

		//Vamos actualizando las estadisticas
		for (j=0; j<4; j++){
			if (cls[j] == 0){
				*cls_lib=(*cls_lib)+1;
			}else if (cls[j] == 1){
				*cls_ocp=(*cls_ocp)+1;
			}else if (cls[j] == 2){
				*cls_bad=(*cls_bad)+1;
				*bad_lib=(*bad_lib)+1;
			}else if (cls[j] == 3){
				*cls_bad=(*cls_bad)+1;
				*bad_ocp=(*bad_ocp)+1;
			}
		}

	}


}

//****************************************
//**	UINT load_FSCB(BYTE drive)		**
//**	Carga el FSCB de la unidad 		**
//**	indicada. Retorna !=0 si error	**
//**	Siendo 65535 si no se detecto   **
//**	un sistema de archivos FDFS 	**
//****************************************
UINT load_FSCB(BYTE drive){
	UINT res = 0;
	BYTE buf[512];

	res = reset_disk(drive);

	if (res == 0){

		res=read_sectors(1, 0, 1, 0, drive, buf); //Leemos el sector de arranque

		if (res==0){

			if (!(buf[2] == 'F' && buf[3] == 'D' && buf[4] == 'F' && buf[5] == 'S')){
				res = 65535;
			}else{
				memcpy(&FSCB, buf+6, 32);
			}
		}
	}
	return res;

}