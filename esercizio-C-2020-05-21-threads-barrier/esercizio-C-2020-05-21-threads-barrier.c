#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>


#define M 10
#define nanosec 10

#define CHECK_ERR(a,msg) {if ((a) == -1) { perror((msg)); exit(EXIT_FAILURE); } }
#define CHECK_ERR2(a,msg) {if ((a) != 0) { perror((msg)); exit(EXIT_FAILURE); } }

//#define DEBUG

pthread_barrier_t thread_barrier;

struct timespec nano;

unsigned int seed = 2;
sem_t mutex;

int file_fd;

int single_use1; //quando ha valore 1 vuol dire che ho messo il \n per formattare
int conta_id;	 //quando ha valore 10 vuol dire che ho messo il \n per formattare


void * fase();

int main(int argc, char * argv[]) {
	int s;

	s = sem_init(&mutex,
						0, // 1 => il semaforo è condiviso tra processi,
						   // 0 => il semaforo è condiviso tra threads del processo
						1 // valore iniziale del semaforo
					  );
	CHECK_ERR(s,"sem_init")

	nano.tv_nsec = nanosec;
	nano.tv_sec = 0;

	char * fileName;

	pthread_t threads[M];

	if(argc == 1){
		printf("nome file non valido");
		exit(EXIT_FAILURE);
	}

	fileName = argv[1];


	file_fd = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	CHECK_ERR(file_fd, "open");

	printf("vado ad aprire il file %s con fd %d\n", fileName, file_fd);

	s = pthread_barrier_init(&thread_barrier, NULL, M);
		CHECK_ERR(s,"pthread_barrier_init")

	for (int i=0; i < M; i++) {
			s = pthread_create(&threads[i], NULL, fase, NULL);
			CHECK_ERR2(s,"pthread_create")
		}

	for (int i=0; i < M; i++) {
			s = pthread_join(threads[i], NULL);
			CHECK_ERR2(s,"pthread_join")
		}

	s = pthread_barrier_destroy(&thread_barrier);
		CHECK_ERR(s,"pthread_barrier_destroy")

	s = sem_destroy(&mutex);
		CHECK_ERR(s,"sem_destroy")

	close(file_fd);
	return 0;
}

void * fase(){

	int s;
	int time;
	struct timespec rem;
	pthread_t pthread_id;

	char * id = malloc(sizeof(unsigned long));
	char * sec = malloc(sizeof(int));
	char * frase = malloc(256);

	srand(seed);
	time = rand()%3;

	if (sem_wait(&mutex) == -1) {
			perror("sem_wait");
			exit(EXIT_FAILURE);
		}

	seed++;

#ifdef DEBUG
	printf("seed %d, rand %d\n", seed, time);
#endif

	if (sem_post(&mutex) == -1) {
		perror("sem_post");
		exit(EXIT_FAILURE);
	}



	pthread_id = pthread_self();


	sprintf(id, "%lu", pthread_id);
	sprintf(sec, "%d", time);


	sleep(time);

	strcpy(frase, "\"fase 1\"(quarantena), thread_id = ");
	strcat(frase, id);
	strcat(frase, ", sleep period = ");
	strcat(frase, sec);
	strcat(frase, "secondi \n");


#ifdef DEBUG
	printf("%s", frase);
#endif


	if (sem_wait(&mutex) == -1) {
		perror("sem_wait");
		exit(EXIT_FAILURE);
	}

	write(file_fd, frase, strlen(frase));

	if (sem_post(&mutex) == -1) {
		perror("sem_post");
		exit(EXIT_FAILURE);
	}


	s = pthread_barrier_wait(&thread_barrier);
	if(s == PTHREAD_BARRIER_SERIAL_THREAD || s == 0){

		//per una formattazione migliore
		if(single_use1 == 0){
			if (sem_wait(&mutex) == -1) {
				perror("sem_wait");
				exit(EXIT_FAILURE);
			}

			write(file_fd, "\n", strlen("\n"));
			single_use1++;

			if (sem_post(&mutex) == -1) {
				perror("sem_post");
				exit(EXIT_FAILURE);
			}
		}


		strcpy(frase, "\"fase 2\"(riapertura), thread_id = ");
		strcat(frase, id);
		strcat(frase, ", dopo la barriera\n");

#ifdef DEBUG
		printf("%s", frase);
#endif

		if (sem_wait(&mutex) == -1) {
			perror("sem_wait");
			exit(EXIT_FAILURE);
		}

		write(file_fd, frase, strlen(frase));

		//formattazione 2
		conta_id++;

		if(conta_id == 10){
			write(file_fd, "\n", strlen("\n"));
		}

		if (sem_post(&mutex) == -1) {
			perror("sem_post");
			exit(EXIT_FAILURE);
		}

		nanosleep(&nano, &rem);
		while(rem.tv_nsec >0 || rem.tv_sec > 0){
			nanosleep(&rem, &rem);
		}

		strcpy(frase, "thread_id = ");
		strcat(frase, id);
		strcat(frase, ", bye!\n");

#ifdef DEBUG
		printf("%s", frase);
#endif

		if (sem_wait(&mutex) == -1) {
			perror("sem_wait");
			exit(EXIT_FAILURE);
		}

		write(file_fd, frase, strlen(frase));

		if (sem_post(&mutex) == -1) {
			perror("sem_post");
			exit(EXIT_FAILURE);
		}

		nanosleep(&nano, &rem);
		while(rem.tv_nsec >0 || rem.tv_sec > 0){
			nanosleep(&rem, &rem);
		}


		return NULL;

	}else{
		perror("pthread_barrier_wait");
		exit(EXIT_FAILURE);
	}


}
