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
 *
 */

#include "standard_lib.h"

int main(int argc, char *argv[])
{
    // Verificacion de argumentos de entrada.
    if (argc > 1 && argc < 5)
    {
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
        FOR(n, 1, num_of_process)
        {
            PROCESS_ID++;
            child_pids[PROCESS_ID] = fork();
            if (!child_pids[PROCESS_ID])
                break;
        }

        if (!child_pids[PROCESS_ID])
        { // PROCESOS HIJOS.
            child_funtion(PROCESS_ID, files_desc);
        }
        else
        {                                         // PROCESO PADRE.
            close(files_desc[1][0]);              // Cerramos primer pipe de lectura.
            close(files_desc[PROCESS_ID + 1][1]); // Cerramos el ultimo pipe escritura.
            char buf[14*n_routes];
            int minutes = 0;
            struct timeval tm_begin, tm_end;
            long t_ms = 1e6 * Min_Simul;
            long curr_tms = 0;
            long diff = 0;
            long scs = 0;

            int cnt = 0;

            while (TRUE)
            {
                tracker_hour(Hour_Simul);
                sprintf(buf, "%d", Hour_Simul);
                if (Hour_Simul == Hour_Final)
                {
                    write(files_desc[1][1], "-1", 3);
                }
                else
                {
                    gettimeofday(&tm_begin, NULL);
                    write(files_desc[1][1], buf, 14*n_routes);
                }
                read(files_desc[PROCESS_ID + 1][0], buf, 14*n_routes);
                gettimeofday(&tm_end, NULL);
                if( buf[0] == '-' ){
                    printf("\nCODE  Ineficientes  Eficientes\n");
                    printf("%s", &buf[2]);
                    printf("\n\n");
                    break;
                } else {
                    sscanf(buf, "%d", &minutes);
                }

                scs = (tm_end.tv_sec - tm_begin.tv_sec);
                curr_tms = (scs * 1e6) + tm_end.tv_usec - tm_begin.tv_usec;

                if (curr_tms < t_ms)
                {
                    t_ms -= curr_tms;
                    gettimeofday(&tm_begin, NULL);
                    usleep(t_ms);
                    Hour_Simul++;
                    gettimeofday(&tm_end, NULL);
                    scs = (tm_end.tv_sec - tm_begin.tv_sec);
                    curr_tms = (scs * 1e6) + tm_end.tv_usec - tm_begin.tv_usec;

                    diff = curr_tms - t_ms;
                    t_ms = 1e6 * Min_Simul;
                    if (diff < t_ms)
                        t_ms = t_ms - diff;
                }
                else
                    Hour_Simul++;

                // Aumento de tiempo para probar el codigo.
                /*cnt++;
                if (cnt == 160)
                    Hour_Simul = Hour_Final;
                */
            }
            
           /* FOR(n, 1, num_of_process){
                printf(" %5s", total_cha[n].code);
                printf("       %d ", total_cha[n].peopleThatDidnotGetTheBus);
                printf("       %d\n", total_cha[n].totalPersonInRoute - total_cha[n].peopleThatDidnotGetTheBus);
            }*/

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

void *showBus(void *data)
{
    struct services *bus = (struct services *)data;
    int leavingTime = getMinutesOfBusWithMinutesAndHours(bus->leaveing);
    int timer = leavingTime;
    int routeTimeOfBus = bus->travel_time;
    int busAlreadyArrived = 0;

    while (TRUE)
    {
        if (timer < Hour_Simul)
        {

            if (leavingTime + routeTimeOfBus > Hour_Simul)
            {
                bus->progressPercentage = getPercentageOfNumber(timer - leavingTime, routeTimeOfBus);
            }
            else if (leavingTime + routeTimeOfBus + 10 > Hour_Simul)
            {
                if (busAlreadyArrived == 0)
                {
                    bus->progressPercentage = 0;
                    bus->isWaitingForPeople = 1;
                    busAlreadyArrived = 1;
                }
            }
            else
            {
                bus->isWaitingForPeople = 0;
                bus->isReturningToUniversity = 1;
                break;
            }
            timer++;
        }
    }
}

void child_funtion(int ID, int pipes[][2])
{
    close(pipes[ID][1]);     // Cerramos escritura pipe izquierdo.
    close(pipes[ID + 1][0]); // Cerramos lectura pipe derecho.
    int minutes = 0;
    int hours = 0;
    int lastHourWithPeopleInBus = 0;
    char buf[14*n_routes];
    char buf_data[64];
    char buf_aux[14*n_routes];
    // int amountOfBusesSentInCurrentRoute = 0;
    int positionInServiceMatrixOfCurrentProcess = servicePositionInMatrixByRoute[ID];
    int timeOfArriveToUniversityOfNextBus = 0;
    int peopleWaiting = 0;
    int amountOfPeopleThatWillJoinToBus = 0;

    while (TRUE)
    {
        read(pipes[ID][0], buf, 14*n_routes);
        if( buf[0] != '-' ){
            sscanf(buf, "%d", &minutes);
        } else { 
            int late = total_cha[ID].peopleThatDidnotGetTheBus;
            int onTime = (total_cha[ID].totalPersonInRoute - total_cha[ID].peopleThatDidnotGetTheBus);
            sprintf(buf_data, "\n %s %9d %11d", total_cha[ID].code, late, onTime );
            strcat(buf, buf_data);
            write(pipes[ID + 1][1], buf, 14*n_routes);
            break;
        }

        Hour_Simul = minutes;
        hours = convertMinutesToHours(minutes);

        // HILOS A EJECUTAR//

        if (minutes == getMinutesOfBusWithMinutesAndHours(total_ser[positionInServiceMatrixOfCurrentProcess][amountOfBusesUsedByRoute[ID]].leaveing))
        {
            total_ser[positionInServiceMatrixOfCurrentProcess][amountOfBusesUsedByRoute[ID]].travel_time = total_cha[ID].min_travel;
            pthread_create(&listOfPthreads[ID][amountOfBusesUsedByRoute[ID]], NULL, &showBus, (void *)&total_ser[positionInServiceMatrixOfCurrentProcess][amountOfBusesUsedByRoute[ID]]);
            amountOfBusesUsedByRoute[ID]++;
        }

        if (amountOfBusesUsedByRoute[ID] - amountOfBusesFinishedByRoute[ID] > 0)
        {
            printf("%s: ", total_cha[ID].code);
            
            int updateQueue = 0;
            if(first_arrival < 14 && hours > 5){
                if(60*hours == Hour_Simul){
                    amountOfPeopleThatWillJoinToBus += total_cha[ID].queue_per[hours];
                    updateQueue = 1;
                }
               // printf("PP: %d ", amountOfPeopleThatWillJoinToBus);
                
                if(first_arrival <= Hour_Simul) {

		        // Pasable a funcion
		        if( ((Hour_Simul - 91) % 60 == 0) && hours > 6){
		            int HourInefficent = ((Hour_Simul - 91) / 60);
                if(total_cha[ID].queue_per[HourInefficent] > 0){
		                total_cha[ID].peopleThatDidnotGetTheBus += total_cha[ID].queue_per[HourInefficent];
                }
                    //printf("PP: %d ---- ", total_cha[ID].peopleThatDidnotGetTheBus);                  
		        }

		        for (int i = amountOfBusesFinishedByRoute[ID]; i < amountOfBusesUsedByRoute[ID]; i++)
		        {
		            if (total_ser[positionInServiceMatrixOfCurrentProcess][i].isWaitingForPeople == 1)
		            {


		                int amountOfAvailableSpaceInTheBus = total_ser[positionInServiceMatrixOfCurrentProcess][i].c_capacity - total_ser[positionInServiceMatrixOfCurrentProcess][i].peopleCharged;
                        if(updateQueue == 0){
                            amountOfPeopleThatWillJoinToBus -=  amountOfAvailableSpaceInTheBus;
                        }
		                
		                if(amountOfPeopleThatWillJoinToBus == -amountOfAvailableSpaceInTheBus  ){
		                    amountOfPeopleThatWillJoinToBus = 0;

		                } else if ( amountOfPeopleThatWillJoinToBus < 0 ){
		                    total_ser[positionInServiceMatrixOfCurrentProcess][i].peopleCharged += total_cha[ID].queue_per[first_arrival];
		                    total_cha[ID].queue_per[first_arrival] = 0;
		                    amountOfPeopleThatWillJoinToBus = 0;
		                } else {
		                    total_cha[ID].queue_per[first_arrival] -= amountOfAvailableSpaceInTheBus;
		                    total_ser[positionInServiceMatrixOfCurrentProcess][i].peopleCharged += amountOfAvailableSpaceInTheBus;
		                }

		                if( total_cha[ID].queue_per[first_arrival] == 0 ){
		                    first_arrival++;
		                    if(first_arrival > Hour_Simul)
		                        break;
		                }
		            }
		        }
		  }
            }

            printf("%d ", amountOfPeopleThatWillJoinToBus);

            for (int i = amountOfBusesFinishedByRoute[ID]; i < amountOfBusesUsedByRoute[ID]; i++)
            {
                if (total_ser[positionInServiceMatrixOfCurrentProcess][i].isWaitingForPeople == 1)
                {
                    printf("[..........] ");
                }

                else if (total_ser[positionInServiceMatrixOfCurrentProcess][i].isReturningToUniversity == 0)
                {
                    print_bus(total_ser[positionInServiceMatrixOfCurrentProcess][i].progressPercentage, total_ser[positionInServiceMatrixOfCurrentProcess][i].isReturningToUniversity);
                }
                else if (total_ser[positionInServiceMatrixOfCurrentProcess][i].isReturningToUniversity == 1)
                {
                    int leavingTimeOfBusAfterWaitForPeople = getMinutesOfBusWithMinutesAndHours(total_ser[positionInServiceMatrixOfCurrentProcess][i].leaveing) + 10 + total_ser[positionInServiceMatrixOfCurrentProcess][i].travel_time;
                    total_ser[positionInServiceMatrixOfCurrentProcess][i].progressPercentage = getPercentageOfNumber(Hour_Simul - leavingTimeOfBusAfterWaitForPeople, total_ser[positionInServiceMatrixOfCurrentProcess][i].travel_time);
                    print_bus(total_ser[positionInServiceMatrixOfCurrentProcess][i].progressPercentage, total_ser[positionInServiceMatrixOfCurrentProcess][i].isReturningToUniversity);
                }
            }

            printf(" \n");
        } else if( amountOfPeopleThatWillJoinToBus > 0 ) {
            printf("%s: ", total_cha[ID].code);
            printf("%d ", amountOfPeopleThatWillJoinToBus);
            printf(" \n"); 
        }

        timeOfArriveToUniversityOfNextBus = getMinutesOfBusWithMinutesAndHours(total_ser[positionInServiceMatrixOfCurrentProcess][amountOfBusesFinishedByRoute[ID]].leaveing) + 10 + total_ser[positionInServiceMatrixOfCurrentProcess][amountOfBusesFinishedByRoute[ID]].travel_time * 2;

        if (amountOfBusesUsedByRoute[ID] > amountOfBusesFinishedByRoute[ID] && timeOfArriveToUniversityOfNextBus <= Hour_Simul)
        {
            amountOfBusesFinishedByRoute[ID]++;
        }

        /*FOR(n, 0, amountOfBusesWaitingForPeopleInCurrentProcess)
        {
            printf("[..........] ");
        }*/

        /*for (int i = 0; i < amountOfBusesGoingToBusStationInCurrentProcess; i++)
        {
            print_bus(total_ser[positionInServiceMatrixOfCurrentProcess][i].progressPercentage, 0);

        }*/

        write(pipes[ID + 1][1], buf, 14*n_routes);
    }
    close(pipes[ID][0]);
    close(pipes[ID + 1][0]);
    exit(0);
}

void tracker_hour(int hour)
{
    int h = hour / 60;
    int m = hour % 60;
    char format[6];
    time_t now = time(NULL);
    struct tm *t_a = localtime(&now);
    t_a->tm_hour = h;
    t_a->tm_min = m;
    strftime(format, 6, "%H:%M", t_a);
    printf("Simulation Time: %s\n", format);
}

int getPercentageOfNumber(int a, int b)
{
    if ((a * 100) / b >= 0)
    {
        return a * 100 / b;
    }
    return 100;
}

int getMinutesOfBusWithMinutesAndHours(struct time_b leavingTimeOfCurrentBus)
{
    return leavingTimeOfCurrentBus.hour * 60 + leavingTimeOfCurrentBus.min;
}

void print_bus(int percentage, int direction)
{
    int rest;
    int count = percentage / 10;
    printf("[");

    if (count > 10)
    {
        count = 10;
    }
    else if (count < 0)
    {
        count = 0;
    }

    if (direction == 0)
    {
        for (int i = 0; i < count; ++i)
        {
            printf("<");
        }
    }
    else
    {
        for (int i = 0; i < count; ++i)
        {
            printf(">");
        }
    }
    rest = 10 - count;
    for (int i = 0; i < rest; ++i)
    {
        printf(" ");
    }
    printf("] ");
}

void ReadCacCharge()
{
    int rows = num_of_lines(charge_file);
    int c, hours, mins;
    char buf[BUFFER_SIZE];
    char *ptr;
    struct charge *aux;

    // Iteramos por las filas del archivo.
    FOR(r, 0, rows)
    {
        aux = &total_cha[r];
        aux->totalPersonInRoute = 0;
        if (fgets(buf, sizeof(buf), charge_file))
        {
            aux->empty = 1;
            ptr = strtok(buf, ",");
            c = 0;
            // Iteramos por las columnas.
            while (ptr != NULL)
            {
                if (r == 0)
                {
                    while (c++ < 3)
                        ptr = strtok(NULL, ",");

                    if (first_arrival == 0)
                        first_arrival = atoi(ptr);

                    last_arrival = atoi(ptr);
                    ptr = strtok(NULL, ",");
                }
                else
                {
                    // Por cada parada leida aumentamos el numero de procesos.
                    num_of_process++;

                    strncpy(aux->code, ptr, 4);
                    ptr = strtok(NULL, ",");

                    strncpy(aux->name, ptr, 64);
                    ptr = strtok(NULL, ",");

                    sscanf(ptr, " %d:%d", &hours, &mins);
                    aux->min_travel = hours * 60 + mins;
                    ptr = strtok(NULL, ",");

                    for (int k = first_arrival; k - 1 < last_arrival; k++)
                    {
                        aux->queue_per[k] = atoi(ptr);
                        aux ->totalPersonInRoute += aux ->queue_per[k];
                        ptr = strtok(NULL, ",");

                    }
                }
            }
        }
    }
    fclose(charge_file); // Cerrar el archivo.
}

void ReadCacService()
{
    int rows = num_of_lines(services_file);
    int c, hours, mins, cap;
    char buf[BUFFER_SIZE];
    char cod[4];
    char *ptr;
    struct services aux;
    int f;

    // Iteramos por las filas del archivo.
    FOR(r, 1, rows)
    {
        c = 0;
        f = 0;
        if (fgets(buf, sizeof(buf), services_file))
        {
            ptr = strtok(buf, " ");

            strncpy(cod, ptr, 4);
            ptr = strtok(NULL, " ");

            // Verificamos que coincidan los CODES.
            if (strcmp(total_cha[r].code, cod))
            {
                // Buscamos la fila correspondiente al CODE de carga.
                FOR(x, 1, num_of_process)
                {
                    if (strcmp(total_cha[x].code, cod))
                        continue;
                    f = x;
                    break;
                }
            }
            else
            {
                f = r;
            }

            // Verificar si la estructura esta vacia.
            if (!total_cha[f].empty)
                continue;

            // Iteramos por las columnas.
            while (ptr != NULL)
            {
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

int num_of_lines(FILE *file)
{
    int rows = 0;
    char buf[BUFFER_SIZE];
    fseek(file, 0, SEEK_SET);
    while (fgets(buf, BUFFER_SIZE, file))
        rows++;
    fseek(file, 0, SEEK_SET);
    return rows;
}

int convertMinutesToHours(int minutes)
{
    return minutes / 60;
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

void update_structs(){
    // Inicializar array que controla cantidad de buses usados, actualmente usado y que ya cumplieron su ruta
    FOR(n, 1, num_of_process + 1) {
        amountOfBusesUsedByRoute[n] = 0;
        amountOfBusesFinishedByRoute[n] = 0;
        total_cha[n].peopleThatDidnotGetTheBus = 0;
        //total_cha[n].totalPersonInRoute = 0;
    }

    FOR(i, 0, num_of_process + 2) {
        FOR(j, 0, num_of_process + 2) {
            if (strcmp(total_ser[i][1].code, total_cha[j].code) == 0){
                // printf("%s: ", total_cha[j].code);
                /*FOR(k, 0, max_bus) {
                    total_ser[i][k].travel_time = total_cha[j].min_travel;
                    }
                    travelTimeByBusRoute[i] = total_cha[j].min_travel;*/
                servicePositionInMatrixByRoute[j] = i;
                routePositionInMatrixByService[i] = j;
            }
        }
    }

    FOR(i, 0, num_of_process + 2) { 
        FOR(j, 0, max_bus) {
            total_ser[i][j].numberOfBusInRoute = j;
            total_ser[i][j].progressPercentage = 0;
            total_ser[i][j].isWaitingForPeople = 0;
            total_ser[i][j].isReturningToUniversity = 0;
            total_ser[i][j].peopleLate = 0;
            total_ser[i][j].peopleOnTime = 0;
            total_ser[i][j].peopleCharged = 0;
        }
    }
}

void open_files(int argc, char *argv[])
{
    // Se ingreso solo el archivo "Caracteritica de Servicio".
    if (argc == 2)
    {
        services_file = fopen(argv[1], "r");
        charge_file = fopen("carga.csv", "r");
        if (charge_file == NULL || services_file == NULL)
            perror("Error: Fail to open files.");

        // Se ingreso archivo "Caracteristica de Servicio" mas entero
        // "Tiempo a simular" o archivo "Caracteristica de Carga".
    }
    else if (argc == 3)
    {
        services_file = fopen(argv[1], "r");

        charge_file = fopen(argv[2], "r");
        if (charge_file == NULL)
        {
            charge_file = fopen("carga.csv", "r");
            sscanf(argv[2], "%f", &Min_Simul);
        }
        if (charge_file == NULL || services_file == NULL)
            perror("Error: Fail to open files.");

        // Se ingreso ambos archivos mas el tiempo a simular.
    }
    else
    {
        sscanf(argv[3], "%f", &Min_Simul);
        services_file = fopen(argv[1], "r");
        charge_file = fopen(argv[2], "r");
        if (charge_file == NULL || services_file == NULL)
            perror("Error: Fail to open files.");
    }
}

void ErrorArgument(int argc, char *argv[])
{
    printf("Error: Number/Names or arguments is incorrect.\n");
    printf("Usage: %s", argv[0]);
    for (int k = 1; k < argc; k++)
        printf("%s ", argv[k]);
    printf("\nCorrect: %s servicio.txt carga.csv\n", argv[0]);
}
