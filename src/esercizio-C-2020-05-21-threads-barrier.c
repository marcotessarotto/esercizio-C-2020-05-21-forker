/*
 * esercizio-C-2020-05-21-threads-barrier.c
 *
 *  Created on: May 19, 2020
 *      Author: marco
 *      un processo apre un file in scrittura (se esiste già sovrascrive i
 *       contenuti del file),
 *       poi lancia M (=10) threads.

"fase 1" vuol dire: dormire per un intervallo random di tempo compreso
tra 0 e 3 secondi,
 poi scrivere nel file il messaggio:
 "fase 1, thread id=, sleep period= secondi"

"fase 2" vuol dire: scrivere nel file il messaggio
"fase 2, thread id=, dopo la barriera"
poi dormire per 10 millisecondi, scrivere nel file il messggio
"thread id= bye!".

per ogni thread: effettuare "fase 1", poi aspettare che tutti
i thread abbiano completato
la fase 1 (barriera: little book of semaphores, pag. 29);
poi effettuare "fase 2" e terminare il thread.

vedere anche:
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h> // nanosleep
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <errno.h>
#include <pthread.h>



#define M 10


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t ta;
sem_t* barrier;


#define CHECK_ERR(a,msg) {if ((a) != 0) { perror((msg)); exit(EXIT_FAILURE); } }
char* msg1 = "fase 1, thread id=%ld, sleep period = %f secondi\n";
char* msg2 = "fase 2, thread id=%ld, dopo la barriera\n";

int number_of_threads = M;
int counter;


void * thread_function(void* arg) {

	int* msec = (int*)arg;
	// fase 1: dormire per un interv casuale
	CHECK_ERR(pthread_mutex_lock(&mutex),"pthread_mutex_lock")
	// sezione critica: dormi casualmente
		//srandom(time(0));
		//int msec = random() % 3000;

		//printf("thread id= %ld; msec sleep %d\n", pthread_self(), *msec);
		counter++;
		ta = pthread_self();
		sleep(*msec*1e-3);
		//PRINTF scrive su STDIN che è diventato ;
		printf(msg1,ta,(*msec*1e-3));

	CHECK_ERR(pthread_mutex_unlock(&mutex),"pthread_mutex_unlock")


	// https://linux.die.net/man/3/pthread_barrier_wait
	//CHECK_ERR(pthread_barrier_wait(&thread_barrier),"pthread_barrier_wait")
	if(counter == M){
		//CHECK_ERR(pthread_mutex_unlock(&barrier),"pthread_mutex_unlock")
		if(sem_post(barrier) == -1) {perror("sem_post barrier!"); exit(EXIT_FAILURE);}
	}

	//CHECK_ERR(pthread_mutex_lock(&barrier),"pthread_mutex_unlock")
	//CHECK_ERR(pthread_mutex_unlock(&barrier),"pthread_mutex_unlock")
	if (sem_wait(barrier) == -1) {
		perror("sem_wait barrier");
		exit(EXIT_FAILURE);
	}

	if (sem_post(barrier) == -1) {
				perror("sem_wait barrier");
				exit(EXIT_FAILURE);
	}


	ta = pthread_self();
	printf(msg2,ta);

	/*
	 The pthread_barrier_wait() function shall synchronize participating threads at
	 the barrier referenced by barrier. The calling thread shall block until
	 the required number of threads have called pthread_barrier_wait() specifying the barrier.
	*/

	//printf("critical point\n");
	sleep(0.010);
	//printf("thread id=%ld,bye!\n",ta);


	return NULL;
}

int main(int argc, char * argv[]) {

	/*
	if( argc == 1){
		perror("Not enought arguments!\n");
		exit(EXIT_FAILURE);
	}
	 */

	int s;
	pthread_t threads[M];
	int* msec;

	msec = malloc(M*sizeof(int));
	if(msec == NULL){
		perror("malloc()!");
		exit(EXIT_FAILURE);
	}

	srandom(time(NULL));
	for (int i = 0; i< M; i++){
		msec[i] = random() % 3000;
		//printf("msec[%d] = %d\n",i,msec[i]);
	}
	// apro file in modalita scrittura

	char * fileName;

	//fileName = (argc == 1) ? "/home/utente/output_2020-05-21.txt" : argv[1];
	fileName = "/home/utente/output_2020-05-21.txt";
	int fd = open(fileName,
				  O_CREAT | O_TRUNC | O_WRONLY,
				  S_IRUSR | S_IWUSR // l'utente proprietario del file avrà i permessi di lettura e scrittura sul nuovo file
				 );

	// l'utente proprietario del file sarà lo stesso utente che ha eseguito questo programma
	// se il file non viene apert, errore!
	if (fd == -1) { // errore!
		perror("open()");
		exit(EXIT_FAILURE);
	}

	if(dup2(fd,STDOUT_FILENO)==-1){
		perror("dup2()");
		exit(EXIT_FAILURE);
	}
	// chiude file descrptor fd
	close(fd);

	// https://linux.die.net/man/3/pthread_barrier_init
	//s = pthread_barrier_init(&thread_barrier, NULL, M);
	//CHECK_ERR(s,"pthread_barrier_init")


	barrier = malloc(sizeof(sem_t));
	s = sem_init(barrier,
						0, // 1 => il semaforo è condiviso tra processi,
						   // 0 => il semaforo è condiviso tra threads del processo
						0  // valore iniziale del semaforo (se mettiamo 0 che succede?)
					  );

	CHECK_ERR(s,"sem_init barrier")

	// lancio i threads
	for (int i = 0; i < M; i++){
		s = pthread_create(&threads[i], NULL, thread_function, &msec[i]);

		if (s != 0) {
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}

	}
	// aspetto i threads
	for (int i=0; i < M; i++) {
		s = pthread_join(threads[i], NULL);

		if (s != 0) {
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}

	}

	// https://linux.die.net/man/3/pthread_barrier_init
	//s = pthread_barrier_destroy(&thread_barrier);
	//CHECK_ERR(s,"pthread_barrier_destroy")
	s = sem_destroy(barrier);
	CHECK_ERR(s,"sem_destroy")

	printf("Programa terminato! Bye\n");

	exit(EXIT_SUCCESS);
}
