/*                 Universidad Simon Bolivar
 * Departamento de Computacion y Tecnologia de la Informacion
 *             CI-3825: Sistemas de Operacion I
 *                Prof. Fernando Torre Mora
 *
 *          ->  PROYECTO 1 ENERO - MARZO 2023   <-
 *                     Transporte USB
 * Estudiantes:
 * Astrid Alvarado  18-10938
 * Laura Parilli    17-10778
 * Jhonaiker Blanco 18-10784
 * Junior Lara      17-10303
 *
 */

#include "standard_lib.h"

int main(int argc, char *argv[]) {
	// Verificacion de argumentos de entrada.
	if (argc > 1 && argc < 5) {
		open_files(argc, argv); // Apertura de archivos.
		initial_structs();      // Inicializacion de estructuras.

		ReadCacCharge();  // Lectura del archivo de carga.
		ReadCacService(); // Lectura del archivo de servicios.

		update_structs(); // Actualizacion de estructuras.

		// Arreglo de PID de cada proceso creado.
		int child_pids[num_of_process + 2];

		// Arreglo de files descriptos o bien sea PIPES.
		int files_desc[num_of_process + 2][2];

		// Apertura de todos los pipes a usar.
		FOR(n, 1, num_of_process + 2)
		pipe(files_desc[n]);

		// Creacion de procesos hijos.
		FOR(n, 1, num_of_process) {
			PROCESS_ID++;
			child_pids[PROCESS_ID] = fork();
			if (!child_pids[PROCESS_ID]) 
				break;
		}

		if (!child_pids[PROCESS_ID]) { // PROCESOS HIJOS.
			child_funtion(PROCESS_ID, files_desc);
		} else { // PROCESO PADRE.
			close(files_desc[1][0]);              // Cerramos primer pipe de lectura.
			close(files_desc[PROCESS_ID + 1][1]); // Cerramos el ultimo pipe escritura.
			int minutes = 0; 		      // Cantidad de minutos simulados.
			char buf[14 * n_routes];              // Buffer para lectura de pipe.
			long t_ms = 1e6 * Min_Simul;          // Tiempo en milisegundos.
			long scs = 0;                         // Segundos transcurridos.
			long t_trans_ms = 0;                  // Tiempo transcurrido en milisegundos.
			long diff = 0;                        // Diferencia entre el tiempo transcurrido y el tiempo simulado.
			struct timeval tm_begin, tm_end;      // Estructuras para el tiempo inicial y final.
			
			while (TRUE) {
				// Mostrado de hora simulada y creacion del buffer para el pipe.
				tracker_hour(Hour_Simul);
				sprintf(buf, "%d", Hour_Simul);

				// Condicional para deterner el ciclo de comunicacion con los
				// procesos hijos o transmitir la hora simulada.
				if (Hour_Simul == Hour_Final) {
					write(files_desc[1][1], "-1", 3);
				} else {
					gettimeofday(&tm_begin, NULL); // Inicio del recorrido pipe.
					write(files_desc[1][1], buf, 14 * n_routes);
				}
				read(files_desc[PROCESS_ID + 1][0], buf, 14 * n_routes);
				gettimeofday(&tm_end, NULL); // Fin del recorrido pipe.

				// Se debe terminar el proceso si el buff contiene 
				// simbolo "-" al comienzo.
				if (buf[0] == '-') {
					printf("\nCODE  Ineficientes  Eficientes\n");
					printf("%s", &buf[2]);
					printf("\n\n");
					break;
				} else 
					sscanf(buf, "%d", &minutes);
				
				// Se determina el tiempo transcurrido en recorrer
				// todos los pipes(cada proceso hijo).
				scs = (tm_end.tv_sec - tm_begin.tv_sec);
				t_trans_ms = (scs * 1e6) + tm_end.tv_usec - tm_begin.tv_usec;

				// En caso de obtener un tiempo menor al minuto similado
				// entonces se duerme el sistema por la diferencia de tiempo
				// restada por el tiempo usado para los pipes.
				if (t_trans_ms < t_ms) {
					t_ms -= t_trans_ms;
					gettimeofday(&tm_begin, NULL);
					usleep(t_ms);
					Hour_Simul++;
					gettimeofday(&tm_end, NULL);
					scs = (tm_end.tv_sec - tm_begin.tv_sec);
					t_trans_ms = (scs * 1e6) + tm_end.tv_usec - tm_begin.tv_usec;
					
					// Se verifica que el cual fue el tiempo conciso de sleep
					// para el sistema y el resto se debe restar con el minuto
					// simulado para la proxima iteracion.
					diff = t_trans_ms - t_ms;
					t_ms = 1e6 * Min_Simul;
					if (diff < t_ms)
						t_ms = t_ms - diff;

				// En caso de tener un retraso el pipe se debe iterar 
				// instantaneamente la hora simulada.
				} else
					Hour_Simul++;
			}

			close(files_desc[1][1]);
			close(files_desc[PROCESS_ID + 1][0]); // Cerramos el ultimo pipe escritura.
		}
		return EXIT_SUCCESS;
	}
	ErrorArgument(argc, argv);
	return EXIT_SUCCESS;
}

/*******************************************************************/
/*   Definiciones y encabezados de funciones en "standard_lib.h"   */
/*******************************************************************/

void print_bus(int percentage, int direction) {
	int rest = 0;
	int count = percentage / 10;
	
	printf("[");
	// Cantidad de simbolos(flechas).
	if (count > 10) {
		count = 10;
	} else if (count < 0) {
		count = 0;
	}
	
	// Direccion parada.
	if (direction == 0) {
		FOR(i, 0, count)
			printf("<");
	// Direccion universidad.
	} else {
		FOR(i, 0, count)
			printf(">");
	}
	
	// Cantidad de espacios en blanco sobrantes.
	rest = 10 - count;
	FOR(i, 0, rest)
		printf(" ");
		
	printf("] ");
}

int getPercentageOfNumber(int a, int b) {
	if ((a * 100) / b >= 0)
		return a * 100 / b;
	return 100;
}

int getMinutesOfBusWithMinutesAndHours(struct time_b leavingTimeOfCurrentBus) {
	return leavingTimeOfCurrentBus.hour * 60 + leavingTimeOfCurrentBus.min;
}

int convertMinutesToHours(int minutes) {
	return minutes / 60;
}

void *showBus(void *data) {
	struct services *bus = (struct services *)data;                      // Data del bus contertidad otra vez al struct necesario.
	int leavingTime = getMinutesOfBusWithMinutesAndHours(bus->leaveing); // Hora de partidad del bus en minutos.
	int timer = leavingTime;                                             // Hora del bus en minutos, es usado como un auxiliar para actualizar la data del bus.
	int routeTimeOfBus = bus->travel_time;                               // Tiepo que toma el viaje del bus.
	int busAlreadyArrived = 0;                                           // Si vale 0 el bus no ha llegado a la parada, si vale 1 si.
	
	while (TRUE) {
		// Cada segundo Hour_Simul aumenta en una entonces se cumplira el if, se activara lo de adentro y
		// aumentaremos en 1 el valor de timer y volveran a valer lo mismo
		if (timer < Hour_Simul) {
		
			// Si el tiempo de partida del bus + su tiempo de recorrido es mayor al tiempo actual el bus no ha llegado a la parada y
			// solo hay que aumentar su porcentaje de recorrido
			if (leavingTime + routeTimeOfBus > Hour_Simul) {
				bus->progressPercentage = getPercentageOfNumber(timer - leavingTime, routeTimeOfBus);
			
			//  Si el tiempo de partida del bus + su tiempo de recorrido + 10 es mayor al tiempo actual es porque el bus esta
			// esperando gente en la parada y hay que actualizar su data para que esto se vea
			} else if (leavingTime + routeTimeOfBus + 10 > Hour_Simul) {
			
			// Este if se usa para solo actualizar la data la primera vez que el bus entra en el for
				if (busAlreadyArrived == 0) {
					bus->progressPercentage = 0;
					bus->isWaitingForPeople = 1;
					busAlreadyArrived = 1;
				}
			
			// Si ninguna de las 2 cosas anteriores es verdad el bus empezo a devolverse a la universidad y hay que actualizar su data para
			// mostrar esto
			} else {
				bus->isWaitingForPeople = 0;
				bus->isReturningToUniversity = 1;
				break;
			}
			timer++;
		}
	}
}

void tracker_hour(int hour) {
	int h = hour / 60;      // Numero de horas..
	int m = hour % 60;      // Numero de minutos.
	char format[6];         // String para el formato de salida por consola.
	time_t now = time(NULL);
	struct tm *t_a = localtime(&now);
	t_a->tm_hour = h;
	t_a->tm_min = m;
	strftime(format, 6, "%H:%M", t_a);
	printf("Simulation Time: %s\n", format);
}

void child_funtion(int ID, int pipes[][2]) {
	close(pipes[ID][1]);            // Cerramos escritura pipe izquierdo.
	close(pipes[ID + 1][0]);        // Cerramos lectura pipe derecho.
	int positionInServiceMatrixOfCurrentProcess = 0; // Posicion en matrix de servicios de array con buses de ruta actual.
	if(total_ser[ID][0].empty){
		positionInServiceMatrixOfCurrentProcess = ID;
	} else {
		positionInServiceMatrixOfCurrentProcess = -1;
	}
	//int positionInServiceMatrixOfCurrentProcess = servicePositionInMatrixByRoute[ID]; // Posicion en matrix de servicios de array con buses de ruta actual.
	int minutes = 0;                                // Tiempo actual en minutos.
	int hours = 0;                                  // Tiempo actual en horas.
	int timeOfArriveToUniversityOfNextBus = 0;      // Momento cuando llegara el siguiente bus.
	int amountOfPeopleThatWillJoinToBus = 0;        // Cantidad de personas esperando a montarse en el bus.
	int leavingTimeOfBusAfterWaitForPeople = 0;     // Tiempo de partida luego de esperar por persona.
	int amountOfAvailableSpaceInTheBus = 0;         // Capacidad disponible en los autobuses.
	int updateQueue = 0;                            // Variable que indica actualización de la cola cada hora.
	int HourInefficent = 0;                         // Hora a la que las personas son calificadas como "ineficientes".
	int late = 0;                                   // Cantidad de personas que llegaron tarde a la universidad.
	int onTime = 0;                                 // Cantidad de personas que llegaron a tiempo.
	char buf[14 * n_routes];                        // Buffer para lectura de pipe.
	char buf_data[64];                              // Buffer para creacion de datos por proceso.
	
	while (TRUE) {
		read(pipes[ID][0], buf, 14 * n_routes);
		if (buf[0] != '-') {
			sscanf(buf, "%d", &minutes);
		} else {
			// En caso de no tener una linea de servicio asociada a la parada entonces 
			// se debe decir que todas las personas en cola de esa ruta nunca llegaron
			// a la universidad.
			if(positionInServiceMatrixOfCurrentProcess != -1){
				late = total_cha[ID].peopleThatDidnotGetTheBus;
				onTime = (total_cha[ID].totalPersonInRoute - total_cha[ID].peopleThatDidnotGetTheBus);
				sprintf(buf_data, "\n %s %9d %11d", total_cha[ID].code, late, onTime);
				strcat(buf, buf_data);
			} else {
				sprintf(buf_data, "\n %s %9d %11d", total_cha[ID].code, total_cha[ID].totalPersonInRoute, 0);
				strcat(buf, buf_data);
			}

			write(pipes[ID + 1][1], buf, 14 * n_routes);
			break;
		}

		Hour_Simul = minutes;
		hours = convertMinutesToHours(minutes);

		// En caso de no tener una linea de servicio asociada a la ruta entonces
		// skip los calculos.
		if(positionInServiceMatrixOfCurrentProcess == -1){
			write(pipes[ID + 1][1], buf, 14 * n_routes);	
			continue;
		}

		// HILOS A EJECUTAR//

		// Si el tiempo de partida del siguiente bus de la ruta es igual al tiempo actual se creara un hilo que controlara el movimiento
		// de ese bus, tambien se le dara a ese bus el tiempo de viaje correspondiente a su ruta y se aumentara el valor que guarda la cantidad
		// de buses que ha enviado esa ruta en 1
		if (minutes == getMinutesOfBusWithMinutesAndHours(total_ser[positionInServiceMatrixOfCurrentProcess][amountOfBusesUsedByRoute[ID]].leaveing)) {
			total_ser[positionInServiceMatrixOfCurrentProcess][amountOfBusesUsedByRoute[ID]].travel_time = total_cha[ID].min_travel;
			pthread_create(&listOfPthreads[ID][amountOfBusesUsedByRoute[ID]], NULL, &showBus, (void *)&total_ser[positionInServiceMatrixOfCurrentProcess][amountOfBusesUsedByRoute[ID]]);
			amountOfBusesUsedByRoute[ID]++;
		}

		// Pasable a funcion
		if (((Hour_Simul - 91) % 60 == 0) && hours > 6) {
			HourInefficent = ((Hour_Simul - 91) / 60);
			if (total_cha[ID].queue_per[HourInefficent] > 0) {
				total_cha[ID].peopleThatDidnotGetTheBus += total_cha[ID].queue_per[HourInefficent];
			}
		}

		// Este if es para verificar que en una ruta hay buses activos
		if (amountOfBusesUsedByRoute[ID] - amountOfBusesFinishedByRoute[ID] > 0) {
			// Muestra el codigo de la ruta
			printf("%s: ", total_cha[ID].code);

			updateQueue = 0;
			if (first_arrival < 14 && hours > 5) {
				if (60 * hours == Hour_Simul) {
					amountOfPeopleThatWillJoinToBus += total_cha[ID].queue_per[hours];
					updateQueue = 1;
				}

				if (first_arrival <= Hour_Simul) {


					for (int i = amountOfBusesFinishedByRoute[ID]; i < amountOfBusesUsedByRoute[ID]; i++) {
						if (total_ser[positionInServiceMatrixOfCurrentProcess][i].isWaitingForPeople == 1) {

							amountOfAvailableSpaceInTheBus = total_ser[positionInServiceMatrixOfCurrentProcess][i].c_capacity - total_ser[positionInServiceMatrixOfCurrentProcess][i].peopleCharged;
							if (updateQueue == 0) {
								amountOfPeopleThatWillJoinToBus -= amountOfAvailableSpaceInTheBus;
							}

							if (amountOfPeopleThatWillJoinToBus == -amountOfAvailableSpaceInTheBus) {
								amountOfPeopleThatWillJoinToBus = 0;
							} else if (amountOfPeopleThatWillJoinToBus < 0) {
								total_ser[positionInServiceMatrixOfCurrentProcess][i].peopleCharged += total_cha[ID].queue_per[first_arrival];

								if( total_ser[positionInServiceMatrixOfCurrentProcess][i].leaveing.hour*60+total_ser[positionInServiceMatrixOfCurrentProcess][i].leaveing.min + 2*total_ser[positionInServiceMatrixOfCurrentProcess][i].travel_time + 10 > first_arrival*60 + 90){
									total_cha[ID].peopleThatDidnotGetTheBus += total_cha[ID].queue_per[first_arrival];
								}

								total_cha[ID].queue_per[first_arrival] = 0;
								amountOfPeopleThatWillJoinToBus = 0;
							} else {
								total_cha[ID].queue_per[first_arrival] -= amountOfAvailableSpaceInTheBus;
								total_ser[positionInServiceMatrixOfCurrentProcess][i].peopleCharged += amountOfAvailableSpaceInTheBus;
							}

							if (total_cha[ID].queue_per[first_arrival] == 0) {
								first_arrival++;
								if (first_arrival > Hour_Simul)
									break;
							}
						}
					}
				}
			}

			printf("%d ", amountOfPeopleThatWillJoinToBus);

			// For que ira desde el primer bus activo del array (que no ha terminado su ruta) hasta el ultimo bus enviado
			for (int i = amountOfBusesFinishedByRoute[ID]; i < amountOfBusesUsedByRoute[ID]; i++) {
				// Si un bus esta esperando se mostrara esto
				if (total_ser[positionInServiceMatrixOfCurrentProcess][i].isWaitingForPeople == 1) {
					printf("[..........] ");
				
				// Si un bus esta bajando se mostrara con la function print_bus su imagen correspondiente
				} else if (total_ser[positionInServiceMatrixOfCurrentProcess][i].isReturningToUniversity == 0) {
					print_bus(total_ser[positionInServiceMatrixOfCurrentProcess][i].progressPercentage, total_ser[positionInServiceMatrixOfCurrentProcess][i].isReturningToUniversity);
				

			// Si un bus esta subiendo se actualizara el porcentaje de la ruta que ha cubierto y se mostrara con la function print_bus su imagen correspondiente
				} else if (total_ser[positionInServiceMatrixOfCurrentProcess][i].isReturningToUniversity == 1) {
					leavingTimeOfBusAfterWaitForPeople = getMinutesOfBusWithMinutesAndHours(total_ser[positionInServiceMatrixOfCurrentProcess][i].leaveing) + 10 + total_ser[positionInServiceMatrixOfCurrentProcess][i].travel_time;
					total_ser[positionInServiceMatrixOfCurrentProcess][i].progressPercentage = getPercentageOfNumber(Hour_Simul - leavingTimeOfBusAfterWaitForPeople, total_ser[positionInServiceMatrixOfCurrentProcess][i].travel_time);
					print_bus(total_ser[positionInServiceMatrixOfCurrentProcess][i].progressPercentage, total_ser[positionInServiceMatrixOfCurrentProcess][i].isReturningToUniversity);
				}
			}
			printf(" \n");
			
		} else if (amountOfPeopleThatWillJoinToBus > 0) {
			printf("%s: ", total_cha[ID].code);
			printf("%d ", amountOfPeopleThatWillJoinToBus);
			printf(" \n");
		}

		timeOfArriveToUniversityOfNextBus = getMinutesOfBusWithMinutesAndHours(total_ser[positionInServiceMatrixOfCurrentProcess][amountOfBusesFinishedByRoute[ID]].leaveing) + 10 + total_ser[positionInServiceMatrixOfCurrentProcess][amountOfBusesFinishedByRoute[ID]].travel_time * 2;

		if (amountOfBusesUsedByRoute[ID] > amountOfBusesFinishedByRoute[ID] && timeOfArriveToUniversityOfNextBus <= Hour_Simul) {
			amountOfBusesFinishedByRoute[ID]++;
		}

		write(pipes[ID + 1][1], buf, 14 * n_routes);
	}
	close(pipes[ID][0]);
	close(pipes[ID + 1][0]);
	exit(0);
}

void ReadCacCharge() {
	int rows = num_of_lines(charge_file); // Numero de filas del archivo.
	int c, hours, mins;                   // Variables varias, para correcta lectura del archivo.
	char buf[BUFFER_SIZE];                // Buffer para la linea leida por archivo.
	char *ptr;                            // Marcador de lectura.
	struct charge *aux;                   // Estrectura carga auxiliar para la lectura del archivo.
	
	// Iteramos por las filas del archivo.
	FOR(r, 0, rows) {
		aux = &total_cha[r];
		aux->totalPersonInRoute = 0;
		if (fgets(buf, sizeof(buf), charge_file)) {
			aux->empty = 1;
			ptr = strtok(buf, ",");
			c = 0;
			// Iteramos por las columnas.
			while (ptr != NULL) {
				if (r == 0) {
					while (c++ < 3)
						ptr = strtok(NULL, ",");

					if (first_arrival == 0)
						first_arrival = atoi(ptr);

					last_arrival = atoi(ptr);
					ptr = strtok(NULL, ",");
				
				} else {
					// Por cada parada leida aumentamos el numero de procesos.
					num_of_process++;

					strncpy(aux->code, ptr, 4);
					ptr = strtok(NULL, ",");

					strncpy(aux->name, ptr, 64);
					ptr = strtok(NULL, ",");

					sscanf(ptr, " %d:%d", &hours, &mins);
					aux->min_travel = hours * 60 + mins;
					ptr = strtok(NULL, ",");

					for (int k = first_arrival; k - 1 < last_arrival; k++) {
						aux->queue_per[k] = atoi(ptr);
						aux->totalPersonInRoute += aux->queue_per[k];
						ptr = strtok(NULL, ",");
					}
				}
			}
		}
	}
	fclose(charge_file); // Cerrar el archivo.
}

void ReadCacService() {
	int rows = num_of_lines(services_file); // Numero de filas del archivo.
	int c, hours, mins, cap, f;             // Variables varias, para correcta lectura del archivo.
	char buf[BUFFER_SIZE];                  // Buffer para la linea leida por archivo.
	char cod[4];                            // Buffer para el codigo de la ruta.
	char *ptr;                              // Marcador de lectura.
	struct services aux;                    // Estrectura servicio auxiliar para la lectura del archivo.
	
	// Iteramos por las filas del archivo.
	FOR(r, 1, rows) {
		c = 0;
		f = 0;
		if (fgets(buf, sizeof(buf), services_file)) {
			ptr = strtok(buf, " ");

			strncpy(cod, ptr, 4);
			ptr = strtok(NULL, " ");

			// Verificamos que coincidan los CODES.
			if (strcmp(total_cha[r].code, cod)) {
				// Buscamos la fila correspondiente al CODE de carga.
				FOR(x, 1, num_of_process) {
					if (strcmp(total_cha[x].code, cod))
						continue;
					f = x;
					break;
				}
			} else
				f = r;
			
			// Verificar si la estructura esta vacia.
			if (!total_cha[f].empty)
				continue;

			// Iteramos por las columnas.
			while (ptr != NULL) {
				total_ser[f][c].empty = 1;
				strncpy(total_ser[f][c].code, cod, 4);
				sscanf(ptr, " %d:%d(%d)", &hours, &mins, &cap);

				// Verificamos la hora inicial simulada.
				Hour_Simul = min(hours * 60 + mins, Hour_Simul);
				Hour_Final = max(hours * 60 + mins + 2 * total_cha[f].min_travel + 10, Hour_Final);

				total_ser[f][c].leaveing.hour = hours;
				total_ser[f][c].leaveing.min = mins;
				total_ser[f][c].c_capacity = cap;
				ptr = strtok(NULL, " ");
				c++;
			}
		}
	}
	Hour_Final++;
	Hour_Simul -= 5;       // Iniciamos 5 minutos antes.
	fclose(services_file); // Cerramos el archivo.
}

int num_of_lines(FILE *file) {
	int rows = 0;   // Numero de filas inicial del archivo "file".
	char buf[BUFFER_SIZE];
	
	fseek(file, 0, SEEK_SET);
	while (fgets(buf, BUFFER_SIZE, file))
		rows++;
		
	fseek(file, 0, SEEK_SET);
	return rows;
}


void initial_structs() {
	// Init de "total_cha" con identificador vacio.
	FOR(i, 0, n_routes)
		total_cha[i].empty = 0;

	// Init de "total_set" con identificador vacio.
	FOR(r, 0, n_routes) {
		FOR(c, 0, max_bus)
			total_ser[r][c].empty = 0;
	}
}

void update_structs() {
	// Inicializar array que controla cantidad de buses enviados, cantidad de buses que terminaron su ruta y cantidad de personas
	// que no lograron subirse al bus
	FOR(n, 1, num_of_process + 1) {
		amountOfBusesUsedByRoute[n] = 0;
		amountOfBusesFinishedByRoute[n] = 0;
		total_cha[n].peopleThatDidnotGetTheBus = 0;
	}
	
	// Inicializa valores numericos de cada bus en 0 excepto por el numero del bus que va a ser correspondiente a su posicion en el
	// array de la matriz que contiene los buses de dicha ruta
	FOR(i, 0, num_of_process + 2) {
		FOR(j, 0, max_bus) {
			total_ser[i][j].progressPercentage = 0;
			total_ser[i][j].isWaitingForPeople = 0;
			total_ser[i][j].isReturningToUniversity = 0;
			total_ser[i][j].peopleCharged = 0;
		}
	}
}

void open_files(int argc, char *argv[]) {
	// Se ingreso solo el archivo "Caracteritica de Servicio".
	if (argc == 2) {
		services_file = fopen(argv[1], "r");
		charge_file = fopen("carga.csv", "r");
		if (charge_file == NULL || services_file == NULL)
			perror("Error: Fail to open files.");

	// Se ingreso archivo "Caracteristica de Servicio" mas entero
	// "Tiempo a simular" o archivo "Caracteristica de Carga".
	} else if (argc == 3) {
		services_file = fopen(argv[1], "r");

		charge_file = fopen(argv[2], "r");
		if (charge_file == NULL) {
			charge_file = fopen("carga.csv", "r");
			sscanf(argv[2], "%f", &Min_Simul);
		}
		if (charge_file == NULL || services_file == NULL)
			perror("Error: Fail to open files.");

		// Se ingreso ambos archivos mas el tiempo a simular.
	} else {
		sscanf(argv[3], "%f", &Min_Simul);
		services_file = fopen(argv[1], "r");
		charge_file = fopen(argv[2], "r");
		if (charge_file == NULL || services_file == NULL)
			perror("Error: Fail to open files.");
	}
}

void ErrorArgument(int argc, char *argv[]) {
	printf("Error: Number/Names or arguments is incorrect.\n");
	printf("Usage: %s", argv[0]);
	for (int k = 1; k < argc; k++)
		printf("%s ", argv[k]);
	printf("\nCorrect: %s servicio.txt carga.csv\n", argv[0]);
}
