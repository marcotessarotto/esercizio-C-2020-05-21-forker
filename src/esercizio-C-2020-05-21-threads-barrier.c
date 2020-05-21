#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// for open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fcntl.h>
// for close
#include <unistd.h>
// for pthread_create & barrier_init
#include <pthread.h>
// for semaphore
#include <semaphore.h>


void * thread_function();
char * concat(const char *s1, const char *s2);


#define M 10

sem_t semaphore;
pthread_barrier_t thread_barrier;
int fd;

int main() {

	// open a file
	fd = open("log.txt",
			  O_CREAT | O_TRUNC | O_WRONLY,
			  S_IRUSR | S_IWUSR // l'utente proprietario del file avrà i permessi di lettura e scrittura sul nuovo file
			 );
	if(fd == -1){
		perror("open()\n");
		exit(1);
	}

	int s = pthread_barrier_init(&thread_barrier, NULL, M);
	if(s != 0){
		perror("pthread_barrier_init()\n");
		exit(1);
	}

	pthread_t * threads = calloc(M, sizeof(pthread_t));
	if(threads == NULL){
		perror("calloc()\n");
		exit(1);
	}

	int res = sem_init(&semaphore,
			0, // 1 => il semaforo è condiviso tra processi, 0 => il semaforo è condiviso tra threads del processo
			1 // valore iniziale del semaforo
		  );
	if(res == -1){
		perror("sem_init()");
		exit(1);
	}

	// create M threads
	for ( int i = 0 ; i < M ; i++){
		int res2 = pthread_create(&threads[i], NULL, thread_function, NULL);
		if(res2 != 0){
			perror("pthread_create()\n");
			exit(1);
		}
	}

	// join M threads
	for(int i=0 ; i<M ; i++){
		int res3 = pthread_join(threads[i], NULL);
		if(res3 != 0){
			perror("pthread_join()\n");
			exit(1);
		}
	}

	if(close(fd) == -1){
		perror("close()\n");
		exit(1);
	}

	printf("bye\n");

	exit(0);
}


void * thread_function(){

	int intervall = (int)((double)random() / RAND_MAX * 3000000);

	usleep(intervall); // like sleep() but using microseconds as unit

	if (sem_wait(&semaphore) == -1) {
		perror("sem_wait");
		exit(EXIT_FAILURE);
	}
	// start critical section

	char * str1 = "fase 1, thread id=";

	char str2 [20];
	long unsigned id = pthread_self();
	int res = sprintf(str2, "%lu", id);
	if(res < 0){
		perror("sprintf()\n");
		exit(1);
	}

	char * str3 = ", sleep period=";

	double sec = (double)intervall / 1000000;
	char str4[20];
	res = sprintf(str4, "%lf", sec);
	if(res < 0){
		perror("sprintf()\n");
		exit(1);
	}

	char * str5 = " secondi\n";

	char * final_str1 = concat(concat(concat(concat(str1, str2), str3), str4), str5);

	res = write(fd, final_str1, strlen(final_str1));
	if(res == -1){
		perror("write()\n");
		exit(1);
	}

	// end critical section
	if (sem_post(&semaphore) == -1) {
		perror("sem_post");
		exit(EXIT_FAILURE);
	}

	int s = pthread_barrier_wait(&thread_barrier);
	// phase 2

	char * str6 = "fase 2, thread id=";

	char * str7 = ", dopo la barriera\n";

	char * final_str2 = concat(concat(str6, str2), str7);

	res = write(fd, final_str2, strlen(final_str2));
	if(res == -1){
		perror("write()\n");
		exit(1);
	}


	usleep(10000); // like sleep() for 10 milliseconds

	char * str8 = "thread id=";

	char * str9 = " bye!\n";

	char * final_str3 = concat(concat(str8, str2), str9);
	res = write(fd, final_str3, strlen(final_str3));
	if(res == -1){
		perror("write()\n");
		exit(1);
	}

	return NULL;
}

char * concat(const char *s1, const char *s2){
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    if(result == NULL){
    	perror("malloc()");
    	exit(1);
    }
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
