#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

// Directivas For.
#define FOR(i, x, n) for(int i = x; i-x < n; i++)

// Macros True and False.
#ifndef TRUE
	#define TRUE (0==0)
#endif
#ifndef FALSE
	#define FALSE !TRUE
#endif

// Buffers y Constantes Macros.
#define BUFFER_SIZE 1024 // Buffer para lectura de filas en archivo.
#define n_routes 50      // Numero maximo de rutas en la universidad.
#define max_hour 24      // Maximo de horas en un dia.
#define max_bus 200      // Maximo de Buses en una ruta.

float Time = 0.25;     // Tiempo default para la simulacion.
int first_arrival = 0; // Hora Inicial de llegada de estudiantes.
int last_arrival = 23; // Hora final de llegada de etudiantes.
FILE 	*charge_file,    // Variable para archivo de carga
			*services_file;  // Variable para archivo de servicio.

// Estructura para guardar los tiempos usados en simulacion.
struct time_b{
	int hour;
	int min;
};

//  Estructura para guardar la informacion del archivo de carga.
//	- empty: Indica si la estructura esta en uso.
//	- queue_per: Cola de personas en una parada.
//	- code: Codigo de la parada.
//	- name: Nombre de la parada.
//	- min_travel: Tiempo de viaje del bus.
struct charge{
	int empty;
	int queue_per[max_hour];
	char code[4];
	char name[64];
	int min_travel;
};

// Estructura para guardar la informacion del archivo de servicio.
//	- empty:Indica si la estructura esta en uso.
//	- code: Codigo de la parada.
//	- leaveing: Hora de salida del bus desde la universidad.
//	- c_capacity: Capacidad de carga del bus. 
struct services{
	int empty;
	char code[4];
	struct time_b leaveing;
	int c_capacity;
};

// Arreglo de unidimensional de estructuras de chargas. Permite 
// indexar por ruta leida en archivo de carga. Cada indice contiene
// la informacion por ruta, codigo, la cola de persona y toda informacion
// de la parada.
struct charge total_cha[n_routes];

// Arreglo bidimensional de estructuras servicios. Permite indexar
// por fila cada ruta/parada los los autobuses asignados para dicha
// parada. Cada columna contiene la informacion de un bus respectivo.
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
