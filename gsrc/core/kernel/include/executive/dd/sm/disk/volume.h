/*
 * File: include\executive\dd\sm\disk\volume.h
 *
 * Descri��o:
 *     Gerenciador de volumes.
 *     Header para o Volume Manager.
 *     Ger�ncia de volumes.
 *
 *
 * Vers�o 1.0, 2015.
 */

 
#define VOLUME_COUNT_MAX 16


/*
 *****************************************************
 * volume_type_t:
 *     Enumerando os tipos de volume.
 *
 */
typedef enum {
	
	VOLUME_TYPE_NULL,            // Null. 
	
	// Parti��o em disco f�sico.
	VOLUME_TYPE_DISK_PARTITION,  

	// Parti��o em disco f�sico.
	VOLUME_TYPE_VIRTUAL_DISK_PARTITION,  

    // Arquivo.
	// Um pequeno arquivo qualquer. N�o tem MBR.
	// Esse foi carregado de algum disco.
	VOLUME_TYPE_FILE,           
	
	// Buffer.
	// Um pequeno buffer qualquer. N�o tem MBR.
	// Obs: Esse n�o foi carregado de disco nenhum.
	// O volume 0, ser� desse tipo. (conductor://)
	VOLUME_TYPE_BUFFER
	
	//...
	
}volume_type_t;




/*
 ******************************************************
 * volumeinfo_d
 *
 * Volume info struct.
 * @todo: deve existir algum padr�o para isso.
 */
typedef struct volumeinfo_d volumeinfo_t; 
struct volumeinfo_d
{
	object_type_t objectType;
	object_class_t objectClass;
		
    //
	// Volume info.
	//
	
	int id;  
	
	int used; 
	int magic; 
	
	char *name; 
	
	int flag;
	int error;
	
	//
	// File System Type
	//
	
	int file_system_type;
	
	
	//
	// VBR info. 
	//
	
    struct vbr_d *vbr;
	unsigned long Signature;   
	
	//
	// Hidden Sectors. (Quantos setores reservados antes da fat).
	//
	
    unsigned long HiddenSectors;	
	
	//
	// FATs.
	//
	int cFAT;  
	unsigned long szFAT;

	
	//
	// ROOT.
	//
	unsigned long szROOT;
	
	
	
//
	// Hidden Sectors. (Quantos setores reservados antes da fat, ou do vbr da primeira parti��o.).
	//
	
    //unsigned long HiddenSectors;	
	
    //
    // FAT.
    //
	
	//unsigned long FatType;              //FAT12, FAT16, FAT32, FATX16 or FATX32.	
    //unsigned long FatSectorStart;       //Starting sector of 1st FAT table.
    //unsigned long ActiveFatSectorStart; //Starting sector of active FAT table.
    //unsigned long NumberOfFats;         //Number of FAT tables.
    //unsigned long SectorsPerFat;        //Sectors per FAT table.

	
    //
	// ROOT.
	//
    //unsigned long RootDirSectorStart;   //Starting sector of the root directory (non-fat32).
    //unsigned long RootDirSectors;       //Number of sectors of the root directory (non-fat32).
   // unsigned long RootDirStartCluster;  //Starting cluster number of the root directory (fat32 only).

	//
	// DATA AREA.
	//
	//unsigned long DataSectorStart;      //Starting sector of the data area.
	
};
volumeinfo_t *volumeinfo_conductor;

volumeinfo_t *CurrentVolumeInfo;
volumeinfo_t *CurrentVolume;
volumeinfo_t *volumeinfo;
volumeinfo_t *SystemVolume;
//...



/*
 * volume_d:
 *     Estrutura para acesso r�pido a volumes.
 *     Deve ser simples e com poucos elementos.
 */
typedef struct volume_d volume_t; 
struct volume_d
{ 
	object_type_t objectType;
	object_class_t objectClass;
	
	volume_type_t volumeType;
	
	int id;
	
	int used;
	int magic;
	
	char *name; 

    struct volumeinfo_d *volume_info;	
};
volume_t *volume_conductor;  //O volume 0.
//...


//
// Lista de volumes.
//

unsigned long volumeList[VOLUME_COUNT_MAX];



void *volume_get_volume_handle( int number );
void *volume_get_current_volume_info();


int volume_init();


//
//fim.
//

