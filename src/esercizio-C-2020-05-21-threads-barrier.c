#include <stdio.h>
#include <stdlib.h>
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
int random_number(int min_num, int max_num);


#define M 2

sem_t semaphore;

pthread_barrier_t thread_barrier;

int main() {

	// open a file
	int fd = open("log.txt",
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

	while(1){

	}


	if(close(fd) == -1){
		perror("close()\n");
		exit(1);
	}

	exit(0);
}


void * thread_function(){


	int intervall = (int)((double)random() / RAND_MAX * 3000000);
	printf("%d\n", intervall);

	usleep(intervall); // like sleep()

	if (sem_wait(&semaphore) == -1) {
		perror("sem_wait");
		exit(EXIT_FAILURE);
	}

	printf("Thread : %lu\n", pthread_self());


	if (sem_post(&semaphore) == -1) {
		perror("sem_post");
		exit(EXIT_FAILURE);
	}


	int s = pthread_barrier_wait(&thread_barrier);

	printf("critical points\n");


	return NULL;
}

int random_number(int min_num, int max_num)
{
    int result = 0, low_num = 0, hi_num = 0;

    if (min_num < max_num)
    {
        low_num = min_num;
        hi_num = max_num + 1; // include max_num in output
    } else {
        low_num = max_num + 1; // include max_num in output
        hi_num = min_num;
    }

    srand(time(NULL));
    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}
