

#ifndef ____FS_H
#define ____FS_H    1


struct directory_facility_d
{
    unsigned long dir_address;
    char          dir_name[9]; //8+1
    int name_size;
    int initialized;
};

// Found in the base/ of the project.
struct directory_facility_d directory_facility_RootDir;
struct directory_facility_d directory_facility_EFIDir;
struct directory_facility_d directory_facility_GramadoDir;
struct directory_facility_d directory_facility_ProgramsDir;
struct directory_facility_d directory_facility_ubaseDir;
struct directory_facility_d directory_facility_usersDir;
// ...

// Found in the system partition.
// struct directory_facility_d directory_facility_???;
// ...


//
// == pwd =============================
//

// See: landos/kernel/include/rtl/fs/path.h
// A string do diretório de trabalho.


struct cwd_d 
{
    //char current_workingdiretory_string[WORKINGDIRECTORY_STRING_MAX];
    //int pwd_initialized;

    char path[WORKINGDIRECTORY_STRING_MAX];
    int size;
    int initialized;
};
struct cwd_d CWD;



// -----------------

// #todo
// Talves possamos usar enum.

// #bugbug
// Essas definições provocam problemas de compilação.

/*
// file types
#define BIN_FILE    1000
#define TXT_FILE    1001
#define CPP_FILE    1002
#define ASM_FILE    1003
// ...

// file extensions
#define BIN_FILE    2000
#define TXT_FILE    2001
#define CPP_FILE    2002
#define ASM_FILE    2003
// ...
*/

// -----------------

#ifndef DIRECTORY_SEPARATOR
# define DIRECTORY_SEPARATOR '/'
#endif

#ifndef ISSLASH
# define ISSLASH(C) ((C) == DIRECTORY_SEPARATOR)
#endif


// Filesystem types.
#define  FS_TYPE_NULL   0
#define  FS_TYPE_GVFS   1000  // Gramado Virtual File System.
#define  FS_TYPE_VFS    1001
#define  FS_TYPE_FAT12  1002 
#define  FS_TYPE_FAT16  1003 
#define  FS_TYPE_FAT32  1004 
#define  FS_TYPE_EXT2   1005
// No more types. 

// deveria is para disk.h
#define SECTOR_SIZE    512  
//#define SECTOR_SIZE  4096  

//#define MBR_BOOTABLE        0x80
//#define MBR_SIGNATURE       0xAA55


// short.
//#define FS_I386_IMAGE  0x014C

// pwd support
#define  FS_VFSWORKINGDIRECTORY_ID      0
#define  FS_BOOTWORKINGDIRECTORY_ID     1
#define  FS_SYSTEMWORKINGDIRECTORY_ID   2
#define  FS_UNKNOWNWORKINGDIRECTORY_ID  (-1)

#define CACHE_SAVED        1
#define CACHE_NOT_SAVED    0

#define CACHE_LOADED        1
#define CACHE_NOT_LOADED    0



//
//  == Variables =================================
//

// Boot partition.
int fat_cache_saved;
int fat_cache_loaded;


// gcc -Wall Defined but not used!
static char *____root_name = "/";


//
// == Search path ================================
//

// See:
// https://en.wikipedia.org/wiki/PATH_(variable)
// ...

// #importante
// + Quando uma função para eecutar uma programa é chamada
// o kernel deve tomar suas decisões sobre qual diretório
// procurar. Não é seguro obedecer o search path indicado pelo
// processo que chamou a função.
// Então o kernel pode começar procurando em /BIN/ e /USER/BIN/.
// O kernel só vai procurar no cwd se o pathname desejado começar
// explicitamente com "./". 


// Multics originated the idea of a search path. 
// The early Unix shell only looked for program names in /bin, 
// but by Version 3 Unix the directory was too large and /usr/bin, 
// and a search path, became part of the operating system.


//char search_path[?];
unsigned long search_path_dir_address;
unsigned long search_path_dir_entries;


// ?? - Contagem de diretórios.
//int dirCount;  


// List of clusters. 
// Usado na rotina de carregamento de arquivo.
unsigned short file_cluster_list[1024]; 




//
// == Structures ====================================
//


/*
 * 
 * Obs: Dentro do kernel base somente teremos primitivas
 *      Operações complexas devem ir para servidores utilitários.
 *
 * Descrição:
 *     Header para o File System Manager. (FSM)
 *     File system header for kernel file management.
 *     Inicialmente suporte ao sistema de arquivos FAT16.
 *     Posteriormente suporte aos sistemas FAT12, FAT32, EXT2.     
 *
 * History:
 *     2014 - Created by Fred Nora.
 */

//suporte ao diretório alvo que desejamos acessar.
//com base em um pathname passado via argumento.

struct target_dir_d
{
    int used;
    int magic;

    // Buffer where the directory was loaded.
    unsigned long current_dir_address;

	//ponteiro para a string do caminho
	//char *pwd_string;  

    //file name 8.3 (11 bytes;)
    char name[32];


    // ??
    // The number of entries ?

    // ...
    
    int initialized;
};

struct target_dir_d current_target_dir;
 
// links para arquivos ou diretórios 
// dentro do mesmo sistema de arquivos. 

struct hardlink_d
{
    int used;
    int magic;
	//..
};



// links para arquivos e diretórios em 
// volumes espalhados por vários discos. 
struct softlink_d
{
    int used;
    int magic;
	//..
};



struct fat_d
{
    unsigned long address;
    int type;
	//...
};

struct fat_d *fat;

 
/*
 **************************************
 * dir_d:
 *     Estrutura para diretório de arquivo.
 *     #importante: 
 *     Entrada de diretório baseada em streams. 
 */

struct dir_d
{
    int id;

    int used;
    int magic;

    // #todo: 
    // Precisaremos dessas coisas.

    struct inode_d *inode;
    file *_file;

    pid_t pid;
    uid_t uid;
    gid_t gid;


    // número de bytes em uma entrada.
    int entry_size_in_bytes;

    //numero total de bytes no diretório.
    int totalentries_size_in_bytes;

    int number_of_entries;

    // Númetro máximo de arquivos em um diretório.
    int fileMax;

	// Número total de arquivos presentes no diretório.
    int fileTotal;

	//Endreço onde o arquivo foi carregado.
    unsigned long address;
   

    //flag, se esta ou nao na memoria.
    int inMemory;

    struct dir_d *next;
};





/*
 * filesystem_d:
 *     Informações sobre um sistema de arquivos.
 */

struct filesystem_d
{
    // Object info.

    object_type_t  objectType;
    object_class_t objectClass;

    //#todo:
    //int id;

    int used;
    int magic;

    int type;

    char *name;


    // Sectors per cluster.
    int spc; 


    // Number of entries in the root dir.
    //int rootdir_entries;
    int dir_entries;
    
    // Size of the entry in bytes.
    int entry_size; 

    //================    

    // #bugbug
    // Thats specific for fat16.
    // fat16.
    
    unsigned long rootdir_address;   //endereço do rootdir
    unsigned long rootdir_lba;       //lba
    unsigned long fat_address;       //endereço da fat  
    unsigned long fat_lba;           //lba
    unsigned long dataarea_address;  //endereço do inicio da area de dados.
    unsigned long dataarea_lba;      //lba

    //...

    // struct filesystem_d *next;
};

struct filesystem_d *root;
// ...


//
// == file context ===========================================
//

// file context.
// It is gonna help us to load a file.
// Let's put in here all the data we need to handle the file.

struct file_context_d
{
    //
    // security
    //
    
    pid_t pid;
    uid_t uid;
    gid_t gid;

    //
    // storage
    //

    //#todo
    // Em que disco o arquivo esta.
    //struct disk_d   *disk;
    
    // Em que volume o arquivo esta.
    struct volume_d *volume;
    
    struct filesystem_d *FileSystem;

    struct dir_d *Directory;

    file *File;

    struct inode_d *inode;
    
    // The FAT is alread loaded in this address.
    unsigned long fat_address;

    // The directory is alread loaded in this address.
    unsigned long dir_address;
    // Number of entries.
    int dir_entries;
    
    unsigned char *file_name;
    
    // Buffer to load the file;
    unsigned long file_address;
    
    // The size of the buffer
    unsigned long buffer_limit;
    
    //...

    struct file_context_d *next;
};



//
// == prototypes =====================================
//



int get_free_slots_in_the_file_table(void);
int get_free_slots_in_the_inode_table(void);

int get_filesystem_type (void);
void set_filesystem_type (int type);

unsigned long fs_count_path_levels (unsigned char *path);

// manipulando a lista de arquivos do kernel.
void *get_file(int Index);
void set_file( void *file, int Index);

#endif    



