/***********************************************************************************/
// Inclusiones de librerias C.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>

/***********************************************************************************/
// Descripcion, definicion, creacion e inicializacion de variables globales.

// Directivas For.
#define FOR(i, x, n) for (int i = x; i - x < n; i++)

// Macros True and False.
#ifndef TRUE
	#define TRUE (0 == 0)
#endif
#ifndef FALSE
	#define FALSE !TRUE
#endif

// Definicion de funcion min/max para dos valores x e y.
#define min(x, y) \
	({ typeof (x) _x = (x); \
			typeof (y) _y = (y); \
			_x < _y ? _x : _y; })
#define max(x, y) \
	({ typeof (x) _x = (x); \
			typeof (y) _y = (y); \
			_x > _y ? _x : _y; })

// Buffers y Constantes Macros.
#define BUFFER_SIZE 1024   // Buffer para lectura de filas en archivo.
#define n_routes 50		     // Numero maximo de rutas en la universidad.
#define max_hour 24        // Maximo de horas en un dia.
#define max_bus 200        // Maximo de Buses en una ruta.

FILE *charge_file, *services_file; // Variable para archivo de carga y servicio.
int first_arrival = 0;          // Hora Inicial de llegada de estudiantes.
int last_arrival = 23;          // Hora final de llegada de etudiantes.
int num_of_process = 0;         // Numero dep rocesos(Paradas) a crear.
int Hour_Simul = 1440;          // Hora degault para la simulacion.
int Hour_Final = 0;             // Hora final de la simulacion.
float Min_Simul = 0.25;         // Minuto default para la simulacion.
int PROCESS_ID = 0;             // Identificador de los procesos creados.

// Estructura para guardar los tiempos usados en simulacion.
struct time_b {
	int hour;
	int min;
};

//  Estructura para guardar la informacion del archivo de carga.
//	- empty: Indica si la estructura esta en uso.
//	- queue_per: Cola de personas en una parada.
//	- code: Codigo de la parada.
//	- name: Nombre de la parada.
//	- min_travel: Tiempo de viaje del bus.
//  - peopleThatDidnotGetTheBus: Cantidad de personas que no lograron subir.
//  - totalPersonInRoute: Cantidad total de gente correspondiente a la ruta.
struct charge {
	int empty;
	int queue_per[max_hour];
	char code[4];
	char name[64];
	int min_travel;
	int peopleThatDidnotGetTheBus;
	int totalPersonInRoute;
};

// Estructura para guardar la informacion del archivo de servicio.
//	- empty:Indica si la estructura esta en uso.
//	- code: Codigo de la parada.
//	- leaveing: Hora de salida del bus desde la universidad.
//	- c_capacity: Capacidad de carga del bus.
//	- travel_time: Tiempo que tarda el bus en cumplir su ruta
//	- progressPercentage: Porcentaje de la ruta que el bus ha recorrido
//	- isWaitingForPeople: Si el bus esta en la parada esperando gente o no
//	- isReturningToUniversity: Si el bus esta camino a la parada o a la universidad.
//	- peopleCharged: Cantidad de gente que se ha montado en el bus.
struct services {
	int empty;
	char code[4];
	struct time_b leaveing;
	int c_capacity;
	int travel_time;
	int progressPercentage;
	int isWaitingForPeople;
	int isReturningToUniversity;
	int peopleCharged;
};

// Cantidad de buses que se han enviado por ruta.
int amountOfBusesUsedByRoute[n_routes];

// Cantidad de buses que han terminado recorrido por ruta.
int amountOfBusesFinishedByRoute[n_routes];

// Arreglo de unidimensional de estructuras de chargas. Permite
// indexar por ruta leida en archivo de carga. Cada indice contiene
// la informacion por ruta, codigo, la cola de persona y toda informacion
// de la parada.
struct charge total_cha[n_routes];

// Arreglo bidimensional de estructuras servicios. Permite indexar
// por fila cada ruta/parada los los autobuses asignados para dicha
// parada. Cada columna contiene la informacion de un bus respectivo.
struct services total_ser[n_routes][max_bus];

// Estructura matricial que contiene los hilos de cada proceso respectivo
// donde las columnas representan los buses.
pthread_t listOfPthreads[n_routes][max_bus];

/***********************************************************************************/
// Descripcion y definicion de funciones.

// Funncion encargada de mostrar un bus en la terminal, toma 2 valores,
// el porcentaje del recorrido del bus y su direccion.
// Si la dirrecion es 1 el bus esta Parada -> Universidad, si es 0 es
// USB -> Parada. Se obtendra una variable count que sera el numero de
// simbolos(flechas) que se mostrara en el print diviendo el porcentaje
// entre 10, luego se muestra un simbolo por cada valor de count y este
// sera la cantidad de simbolos(flechas) que se mostrara en el bus.
void print_bus(int, int);

// Funcion encargada de determinar el porcentaje de un numero en base a 
// otro numero.Aplicar una regla de tres simplex.
int getPercentageOfNumber(int, int);

// Funcion que convierte tiempo de un struct time_b (tiempo en horas 
// y minutos) a tiempo solo en minutos.
int getMinutesOfBusWithMinutesAndHours(struct time_b);

// Funcion que convierte minutos a horas.
int convertMinutesToHours(int);

// Funcion encargada simular un bus, es la funcion especifica para 
// un hilo creado.
void *showBus(void*);

// Funcion encargada de mostrar en consola la hora simulada dado un
// numero entero que representa la cantidad de minutos.
void tracker_hour(int);

// Funcion descrita para el trabajo de un proceso hijo creado por fork.
void child_funtion(int, int[][2]);

// Funcion encargada de obtener/extraer la informacion del
// archivo "Caracteristica de Carga" a la estructura
// "total_cha".
void ReadCacCharge();

// Funcion encargada de obtener/extraer la informacion del archivo 
// "Caracteristica de Servicio" a la estructura "total_ser".
void ReadCacService();

// Funcion encargada de obtener la cantidad de lineas de un para un 
// archivo dado.
int num_of_lines(FILE *);

// Funcion encargada de inicializar todas las estructuras macros
// de informacion como "total_cha" y "total_ser" en sub_estructuras
// por default.
void initial_structs();

// Funcion encargada de actualizar estructuras luego de obtener ciertos
// datos de los archivos carga y servicios.
void update_structs();

// Funcion encargada de abrir los archivos dados como argumento en la 
// funcion main.
void open_files(int, char **);

// Funcion encargada de mostrar errores de llamado al programa.
// Cuando no se ingresan los archivos o la cantidad correcta.
void ErrorArgument(int, char **);

/***********************************************************************************/

