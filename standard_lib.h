#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

// Directivas Fors.
#define FOR0(i, n) for(int i = 0; i < n; i++)
#define FOR1(i, n) for(int i = 1; i-1 < n; i++)

// Macros True and False.
#ifndef TRUE
	#define TRUE (0==0)
#endif
#ifndef FALSE
	#define FALSE !TRUE
#endif

#define BUFFER_SIZE 1024
#define n_routes 50
#define max_hour 24
#define max_bus 200

float Time = 0.25;
int first_arrival = 0;
int last_arrival = 23;
int rows_file;
FILE *charge_file, *services_file;

struct time_b{
	int hour;
	int min;
};

struct charge{
	int empty;
	int queue_per[max_hour];
	char code[4];
	char name[64];
	int min_travel;
};

struct services{
	int empty;
	char code[4];
	struct time_b leaveing;
	int c_capacity;
};

struct charge total_cha[n_routes];
struct services total_ser[n_routes][max_bus];

// Funcion encargada de obtener/extraer la informacion del 
// archivo "Caracteristica de Carta" a la estructura 
// "total_cha".
void ReadCacCharge();

// Funcion encargada de obtener/extraer la informacion del
// archivo "Caracteristica de Servicio" a la estructura 
// "total_ser".
void ReadCacService();

// Funcion encargada de obtener la cantidad de lineas de un
// para un archivo dado.
int num_of_lines(FILE*);

// Funcion encargada de inicializar todas las estructuras macros
// de informacion como "total_cha" y "total_set" en sub_estructuras
// por default.
void initial_structs();

// Funcion encargada de abrir los archivos dados como argumento
// en la funcion main.
void open_files(int, char**);

// Funcion encargada de mostrar errores de llamado al programa.
// Cuando no se ingresan los archivos o la cantidad correcta.
void ErrorArgument(int, char**);

