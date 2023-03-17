/*                 Universidad Simon Bolivar
 * Departamento de Computacion y Tecnologia de la Informacion
 *             CI-3825: Sistemas de Operacion I
 *                Prof. Fernando Torre Mora
 *
 * 				 	->  PROYECTO 1 ENERO - MARZO 2023   <-
 *                     Transporte USB
 * Estudiantes:
 * Astrid Alvarado  18-10938
 * Laura Parilli    17-10778
 * Jhonaiker Blanco 18-10784
 * Junior Lara      17-10303
 *
 * -> Idea General:
 *
 * Error en servicio de transporte para año 2007, falta el transporte HDP
 * Hoyo de la puerta.
 *
*/

#include "standard_lib.h"

int main(int argc, char *argv[]){
	// Verificacion de argumentos de entrada.
	if(argc > 1 && argc < 5){ 
		open_files(argc, argv);
		initial_structs();
		
		//int rows_c = num_of_lines(charge_file);
		//int rows_s = num_of_lines(services_file);
		
		ReadCacCharge();
		ReadCacService();
		
		int child_pids[num_of_process+1];
		int PROCESS_ID = 0;
		int files_desc[num_of_process+1][2];
		
		// Apertura de todos los pipes a usar.
		FOR(n, 1, num_of_process){
			pipe(files_desc[n]);
		}
		
		// Creacion de los procesos. En caso de ser proceso hijo hacer un break.
		// No se quiere crear un arbol de procesos sino un abanico. Por tanto
		// mientras sea el proceso padre entonces el for sigue.
		FOR(n, 1, num_of_process){
			PROCESS_ID++;
			child_pids[PROCESS_ID] = fork();
			if(!child_pids[PROCESS_ID]) break;
		}
		
		// Procesos hijo va a su funcion con su respectivo ID.
		if(!child_pids[PROCESS_ID]){
			child_funtion(PROCESS_ID, files_desc);

		} else { // Proceso padre.
			close(files_desc[1][0]); 	// Cerramos primer pipe de lectura.
			char buf[10];
			int cnt = 0;

			while(TRUE){
				// funcion aumentar time//
				sprintf(buf, "%d", Hour_Simul);
				if(Hour_Simul == Hour_Final) {
					// Escribir -1 en pipe es señal de terminar procesos.
					write(files_desc[1][1], "-1", 3); break;
				} else {
					write(files_desc[1][1], buf, 10);
				}
				sleep(2);
				
				// Aumento de tiempo para probar el codigo.
				cnt++;
				if(cnt == 2) {
					Hour_Simul = Hour_Final;
				}else{
					Hour_Simul++;
				}
			}

			close(files_desc[1][1]);
			

			// Wait de cada proceso.
			int status;
			FOR(n, 1, num_of_process){
				wait(&status);
				if(!WEXITSTATUS(status))
					printf("Hijo %d ha terminadoo.\n", PROCESS_ID);
				else
					printf("Hijo %d NO ha terminado.\n", PROCESS_ID);
			}
		}

		// Checks de primera hora y ultima hora.
		printf("first : %d\n", first_arrival);
		printf("last : %d\n", last_arrival);
		
		return EXIT_SUCCESS;
	}
	ErrorArgument(argc, argv);
	return EXIT_SUCCESS;
}

/*******************************************************************/
/*   Definiciones y encabezados de funciones en "standard_lib.h"   */
/*******************************************************************/

void child_funtion(int ID, int pipes[][2]){
	
	if(ID == num_of_process){ // Ultimo hijo.
		close(pipes[ID][1]); // Cerramos el file de escritura.
		int minutos = 0;
		char buf[10];
				
		while(TRUE){
			read(pipes[ID][0], buf, 10);
			printf("->Soy el proceso hijo %d PID: %d\n", ID, getpid());
			sscanf(buf, "%d", &minutos);
			if(minutos == -1) break;
			printf("Leido por pipe: %d\n\n", minutos);
		}

		close(pipes[ID][0]);
			
	} else {
		close(pipes[ID][1]);   // Cerramos escritura pipe izquierdo.
		close(pipes[ID+1][0]); // Cerramos lectura pipe derecho.
		int minutos = 0;
		char buf[10];
				
		while(TRUE){
			read(pipes[ID][0], buf, 10);
			printf("->Soy el proceso hijo %d PID: %d\n", ID, getpid());
			sscanf(buf, "%d", &minutos);
			if(minutos == -1) break;
			printf("Leido por pipe: %d\n\n", minutos);
			
			write(pipes[ID+1][1], buf, 10);
		}

		close(pipes[ID][0]);
		close(pipes[ID+1][0]);
	}
	exit(0);
}


void tracker_hour(int hour){
	int h = hour/60;
	int m = hour % 60;
	char format[6];
	time_t now = time(NULL);
	struct tm* t_a = localtime(&now);
	t_a->tm_hour = h;
	t_a->tm_min = m;
	strftime(format, 6, "%H:%M", t_a);
	printf("Simulation Time: %s\n", format);
}

void ReadCacCharge(){
	int rows = num_of_lines(charge_file); 
	int c, hours, mins;
	char buf[BUFFER_SIZE];
	char *ptr;
	struct charge *aux;

	// Iteramos por las filas del archivo.
	FOR(r, 0, rows){
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
					// Por cada parada leida aumentamos el numero de procesos.
					num_of_process++;

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
	fclose(charge_file); // Cerrar el archivo.
}

void ReadCacService(){
	int rows = num_of_lines(services_file);
	int c, hours, mins, cap;
	char buf[BUFFER_SIZE];
	char cod[4];
	char *ptr;
	struct services aux;
	int f;

	// Iteramos por las filas del archivo.
	FOR(r, 1, rows){
		c = 0; f = 0;
		if(fgets(buf, sizeof(buf), services_file)){
			ptr = strtok(buf, " ");

			strncpy(cod, ptr, 4);
			ptr = strtok(NULL, " ");
			
			// Verificamos que coincidan los CODES.
			if(strcmp(total_cha[r].code, cod)){
				// Buscamos la fila correspondiente al CODE de carga.
				FOR(x, 1, num_of_process){
					if(strcmp(total_cha[x].code, cod)) continue;
					f = x; break;	
				}	
			} else { f = r; }
			
			// Verificar si la estructura esta vacia.
			if(!total_cha[f].empty) continue;

			// Iteramos por las columnas.
			while( ptr != NULL){
				total_ser[f][c].empty = 1;
				strncpy(total_ser[f][c].code, cod, 4);
				sscanf(ptr, " %d:%d(%d)", &hours, &mins, &cap);
				
				// Verificamos la hora inicial simulada.
				Hour_Simul = min(hours*60 + mins, Hour_Simul);
				Hour_Final = max(hours*60 + mins, Hour_Final);

				total_ser[f][c].leaveing.hour = hours;
				total_ser[f][c].leaveing.min = mins;
				total_ser[f][c].c_capacity = cap;
				ptr = strtok(NULL, " ");
				c++;
			}
		}
	}
	Hour_Simul -= 5; // Iniciamos 5 minutos antes.
	fclose(services_file); // Cerramos el archivo.
}

int num_of_lines(FILE *file){
	int rows = 0;
	char buf[BUFFER_SIZE];
	fseek(file, 0, SEEK_SET);
	while(fgets(buf, BUFFER_SIZE, file))
		rows++;
	fseek(file, 0, SEEK_SET);
	return rows;
}

void initial_structs(){
	// Init de "total_cha" con identificador vacio.
	FOR(i, 0, n_routes)
		total_cha[i].empty = 0;
	
	// Init de "total_set" con identificador vacio.
	FOR(r, 0, n_routes){
		FOR(c, 0, max_bus)
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
			sscanf(argv[2], "%f", &Min_Simul);
		}
		if(charge_file == NULL || services_file == NULL)
			perror("Error: Fail to open files.");

	// Se ingreso ambos archivos mas el tiempo a simular.
	} else {
		sscanf(argv[3], "%f", &Min_Simul);
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
