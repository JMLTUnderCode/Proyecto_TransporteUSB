#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<pthread.h>
#include<semaphore.h>
#include<time.h>
#include<sys/time.h>
#include<unistd.h>

#define FOR(i, x, n) for(int i = x; i-x < n; i++)

#ifndef TRUE
	#define TRUE (0==0)
#endif
#ifndef FALSE
	#define FALSE !TRUE
#endif

int Hour_Simul = 0;

void time_simulation(double a){
	struct timeval g_start, g_end;
	unsigned int t_ms = a*1e6;
	unsigned int diff = 0;
	unsigned int acum = 0;
	int x = 0;

	while(TRUE){
		gettimeofday(&g_start, NULL);
		usleep(t_ms);
		gettimeofday(&g_end, NULL);

		long sg = (g_end.tv_sec - g_start.tv_sec);
		diff = (unsigned int)( ((sg*1e6)+g_end.tv_usec) - g_start.tv_usec );  
		acum+= diff-t_ms;

		printf("diff: %d\n", diff);
	
		if(acum < t_ms){
			printf("acum < t_ms\n");
			printf("%f %d\n", a*1e6, diff);
			if(diff<a*1e6){
				t_ms = a*1e6 - diff;
			}else{
				t_ms = diff - a*1e6;
			}
			printf("t: %d\n", t_ms);
		}else{
			printf("acum >= t_ms\n");
			x++;
			printf("%f %d\n", a*1e6, diff);
			if(diff<a*1e6){
				t_ms = a*1e6 - diff;
			}else{
				t_ms = diff -a*1e6;
			}
			printf("acumulado: %d\n", acum);
			printf("t: %d\n", t_ms);
			acum = 0;
			Hour_Simul++;
			printf("Hora simulada: %d\n", Hour_Simul);
		}

		if(x==10) break;
	}
}

int main(int argc, char* argv[]){
	
	double t = atof(argv[1]);
	printf("%f\n", t);
	time_simulation(t);

	return 	EXIT_SUCCESS;
}
