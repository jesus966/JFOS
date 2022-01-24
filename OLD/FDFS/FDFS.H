//Fichero de cabecera para el sistema FDFS

//Definicion de estructuras, variables globales y defines.
#define RETRY 3
typedef unsigned char BYTE;
typedef unsigned int  UINT;

//File System Control Block : Es una estructura global que almacena todos los datos importantes
//que están contenidos en el sector de arranque, hay que llamar a la funcion load_FSCB para cargar
//sus parametros y que el resto de funciones funcionen correctamente
typedef struct{
	BYTE version;
	char etiqueta[18]; //El ultimo se reserva para el byte de etiqueta adicional
	BYTE head;
	BYTE cyl;
	BYTE sec;
	BYTE tamclus;
	UINT cdirr;
	BYTE hora;
	BYTE minuto;
	BYTE segundo;
	BYTE dia;
	BYTE mes;
	BYTE anio;
	UINT cinfad;
} FSCBs;

FSCBs FSCB;

BYTE fs_map[1024];	//Aqui guardamos el mapa de bits de la unidad actual

//Definicion de funciones

//Funciones relacionadas con DISKIO
BYTE verify_sectors(BYTE sector_start, BYTE sector_total, BYTE track, BYTE head, BYTE drive);
BYTE format_track(BYTE track, BYTE head, BYTE tracks, BYTE sectors, BYTE drive);
BYTE read_sectors(BYTE number_sectors_read, BYTE track, BYTE sector, BYTE head, BYTE drive, BYTE * buffer);
BYTE write_sectors(BYTE number_sectors_write, BYTE track, BYTE sector, BYTE head, BYTE drive, BYTE * buffer);
BYTE reset_disk(BYTE drive);
void cluster_to_CHS(UINT cluster, BYTE * C, BYTE * H, BYTE * S);
UINT CHS_to_cluster(BYTE C, BYTE H, BYTE S);
BYTE load_cluster(UINT cluster, BYTE drive, BYTE * buffer);
BYTE load_clusters(UINT cluster, BYTE drive, BYTE * buffer, UINT clst_tot);
BYTE write_cluster(UINT cluster, BYTE drive, BYTE * buffer);
BYTE write_clusters(UINT cluster, BYTE drive, BYTE * buffer, UINT clst_tot);

//Funciones relacionadas con el sistema de archivos en si
void set_free_cluster(BYTE * map, UINT cluster);
void set_used_cluster(BYTE * map, UINT cluster);
void set_bad_cluster(BYTE * map, UINT cluster);
void set_good_cluster(BYTE * map, UINT cluster);
UINT find_free_cluster(BYTE * map);
UINT find_free_clusters(BYTE * map, UINT * tamanio);
void fdfs_stats(BYTE * map, UINT * cls_lib, UINT * cls_ocp, UINT * cls_bad, UINT * bad_ocp, UINT * bad_lib);
UINT load_FSCB(BYTE drive);
BYTE write_map(BYTE * map, BYTE drive);
BYTE load_map(BYTE * map, BYTE drive);

//Includes del codigo
#include "DISKIO.C"			//Incluye todas las funciones relacionadas con el acceso al disco a bajo nivel
#include "FDFS.C"			//Incluye las funciones que manejan el sistema de archivos FDFS