/*                 Universidad Simon Bolivar
 * Departamento de Computacion y Tecnologia de la Informacion
 *             CI-3825: Sistemas de Operacion I
 *                Prof. Fernando Torre Mora
 *
 * 				 	->  PROYECTO 1 ENERO - MARZO 2023   <-
 *                     Transporte USB
 * Estudiantes:
 * Astrid Alvarado 18-10938
 * Laura Parilli   17-10778
 * Junior Lara     17-10303
 *
 * -> Idea General:
 *
 * Error en servicio de transporte para año 2007, falta el transporte HDP
 * Hoyo de la puerta.
 *
*/

#include "standard_lib.h"

int main(int argc, char *argv[]){
	if(argc > 1 && argc < 5){ 
		open_files(argc, argv);
		initial_structs();
		
		int rows_c = num_of_lines(charge_file);
		int rows_s = num_of_lines(services_file);
		
		ReadCacCharge();
		ReadCacService();
		
		// Checks de primera hora y ultima hora.
		printf("first : %d\n", first_arrival);
		printf("last : %d\n", last_arrival);
		
		// Check de cargas.
		printf("\n CODE       NAMES         Recorr   6   7   8   9   10  11  12  13\n");
		FOR1(r, rows_c){
			struct charge aux = total_cha[r];
			if(aux.empty == 0) break;
			printf("%4s", aux.code);
			printf("%20s", aux.name);
			printf("%4d mins", aux.min_travel);
			for(int k = first_arrival; k-1 < last_arrival; k++){
				printf("%4d", aux.queue_per[k]);
			}
			printf("\n");
		}
		
		// Check de servicios.
		printf("\n CODE       LEAVINGS       \n");
		FOR1(r, rows_s){
			int c = 0;
			struct services aux = total_ser[r][c++];
			if(aux.empty == 0) continue;

			printf("%4s", aux.code);
			while(aux.empty == 1){
				printf(" %d:%d(%d) ",aux.leaveing.hour, aux.leaveing.min, aux.c_capacity);
				aux = total_ser[r][c++];
			}
			printf("\n");
		}

		return EXIT_SUCCESS;
	}
	ErrorArgument(argc, argv);
	return EXIT_SUCCESS;
}

/*******************************************************************/
/*   Definiciones y encabezados de funciones en "standard_lib.h"   */
/*******************************************************************/



void ReadCacCharge(){
	int rows = num_of_lines(charge_file);
	int c, hours, mins;
	char buf[BUFFER_SIZE];
	char *ptr;
	struct charge *aux;

	// Iteramos por las filas del archivo.
	FOR0(r, rows){
		aux = &total_cha[r];
		if(fgets(buf, sizeof(buf), charge_file)){
			aux->empty = 1;
			ptr = strtok(buf, ",");
			c = 0;
			// Iteramos por las columnas.
			while(ptr != NULL){
				if(r == 0){
					while(c++ < 3)
						ptr = strtok(NULL, ",");
					
					if(first_arrival == 0)
						first_arrival = atoi(ptr);

					last_arrival = atoi(ptr);
					ptr = strtok(NULL, ",");

				}	else {
					strncpy(aux->code, ptr, 4);
					ptr = strtok(NULL, ",");

					strncpy(aux->name, ptr, 64);
					ptr = strtok(NULL, ",");

					sscanf(ptr, " %d:%d", &hours, &mins);
					aux->min_travel = hours*60 + mins;
					ptr = strtok(NULL, ",");

					for(int k = first_arrival; k-1 < last_arrival; k++){
						aux->queue_per[k] = atoi(ptr);
						ptr = strtok(NULL, ",");
					}
				}
			}
		}
	}
	// Cerrar el archivo.
	fclose(charge_file);
}

void ReadCacService(){
	int rows = num_of_lines(services_file);
	int c, hours, mins, cap;
	char buf[BUFFER_SIZE];
	char cod[4];
	char *ptr;
	struct services aux;
	
	// Iteramos por las filas del archivo.
	FOR1(r, rows){
		c = 0;
		if(fgets(buf, sizeof(buf), services_file)){
			ptr = strtok(buf, " ");

			strncpy(cod, ptr, 4);
			ptr = strtok(NULL, " ");
			
			// Iteramos por las columnas.
			while( ptr != NULL){
				total_ser[r][c].empty = 1;
				strncpy(total_ser[r][c].code, cod, 4);
				sscanf(ptr, " %d:%d(%d)", &hours, &mins, &cap);
				total_ser[r][c].leaveing.hour = hours;
				total_ser[r][c].leaveing.min = mins;
				total_ser[r][c].c_capacity = cap;
				ptr = strtok(NULL, " ");
				c++;
			}
		}
	}
	// Cerrar el archivo.
	fclose(services_file);
}

int num_of_lines(FILE *file){
	int rows;
	char buf[BUFFER_SIZE];
	fseek(file, 0, SEEK_SET);
	while(fgets(buf, BUFFER_SIZE, file))
		rows++;
	fseek(file, 0, SEEK_SET);
	return rows;
}

void initial_structs(){
	// Init de "total_cha" con identificador vacio.
	FOR0(i, n_routes)
		total_cha[i].empty = 0;
	
	// Init de "total_set" con identificador vacio.
	FOR0(r, n_routes){
		FOR0(c, max_bus)
			total_ser[r][c].empty = 0;
	}
}

void open_files(int argc, char *argv[]){
	// Se ingreso solo el archivo "Caracteritica de Servicio".
	if(argc == 2){
		services_file = fopen(argv[1], "r");
		charge_file = fopen("carga.csv", "r");
		if(charge_file == NULL || services_file == NULL)
			perror("Error: Fail to open files.");
	
	// Se ingreso archivo "Caracteristica de Servicio" mas entero 
	// "Tiempo a simular" o archivo "Caracteristica de Carga".
	} else if(argc == 3) {
		services_file = fopen(argv[1], "r");
		
		charge_file = fopen(argv[2], "r");
		if(charge_file == NULL){
			charge_file = fopen("carga.csv", "r");
			sscanf(argv[2], "%f", &Time);
		}
		if(charge_file == NULL || services_file == NULL)
			perror("Error: Fail to open files.");

	// Se ingreso ambos archivos mas el tiempo a simular.
	} else {
		sscanf(argv[3], "%f", &Time);
		services_file = fopen(argv[1], "r");
		charge_file = fopen(argv[2], "r");
		if(charge_file == NULL || services_file == NULL)
			perror("Error: Fail to open files.");
	}
}

void ErrorArgument(int argc, char *argv[]){
	printf("Error: Number/Names or arguments is incorrect.\n");
	printf("Usage: %s", argv[0]);
	for(int k=1; k < argc; k++)
		printf("%s ", argv[k]);
	printf("\nCorrect: %s servicio.txt carga.csv\n", argv[0]);
}






