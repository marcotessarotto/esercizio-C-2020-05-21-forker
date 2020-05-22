#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>       /* time */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define N 10
#define A 0
#define B 3000000

sem_t mutex;
sem_t barrier;

// variabili condivise tra i thread
int count;
int number_of_threads = N;

void * thread_function(void * arg) {

	long unsigned tid = pthread_self();
	int random_number = rand() % B + A;
	usleep(random_number);
	//	mutex.wait()
	//	   count = count + 1
	//	mutex.signal()
	if (sem_wait(&mutex) == -1) {
		perror("sem_wait");
		exit(EXIT_FAILURE);
	}

	count++;
	double sec = (double)random_number / 1000000;
	printf("fase 1, thread id=%lu, sleep period=%lf secondi\n", tid, sec);

	if (sem_post(&mutex) == -1) {
		perror("sem_post");
		exit(EXIT_FAILURE);
	}
	//

	//	if count == n :
	//	   barrier.signal()
	if (count == number_of_threads) {
		if (sem_post(&barrier) == -1) {
			perror("sem_post");
			exit(EXIT_FAILURE);
		}
	}

	// turnstile (tornello)
	//	barrier.wait()
	//	barrier.signal()
	if (sem_wait(&barrier) == -1) {
		perror("sem_wait");
		exit(EXIT_FAILURE);
	}
	if (sem_post(&barrier) == -1) {
		perror("sem_post");
		exit(EXIT_FAILURE);
	}
	//

	printf("fase 2, thread id=%lu, dopo la barriera\n", tid);
	// dorme per 10 ms
	usleep(10000);
	printf("thread id=%lu bye!\n", tid);

	return NULL;
}

#define CHECK_ERR(a,msg) {if ((a) == -1) { perror((msg)); exit(EXIT_FAILURE); } }

int main() {
	char * file_name = "output.txt";

	int fd = open(file_name,
					  O_CREAT | O_WRONLY | O_TRUNC,
					  S_IRUSR | S_IWUSR // l'utente proprietario del file avrà i permessi di lettura e scrittura sul nuovo file
					 );

	CHECK_ERR(fd, "open()")

	if (dup2(fd, STDOUT_FILENO) == -1) {
		perror("problema con dup2");
		exit(EXIT_FAILURE);
	}

	close(fd);

	int s;
	pthread_t threads[N];

	s = sem_init(&mutex,
					0, // 1 => il semaforo è condiviso tra processi,
					   // 0 => il semaforo è condiviso tra threads del processo
					1 // valore iniziale del semaforo
				  );

	CHECK_ERR(s,"sem_init")

	s = sem_init(&barrier,
						0, // 1 => il semaforo è condiviso tra processi,
						   // 0 => il semaforo è condiviso tra threads del processo
						0 // valore iniziale del semaforo
					  );

	CHECK_ERR(s,"sem_init")

	for (int i=0; i < number_of_threads; i++) {
		s = pthread_create(&threads[i], NULL, thread_function, NULL);

		if (s != 0) {
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}

	for (int i=0; i < number_of_threads; i++) {
		s = pthread_join(threads[i], NULL);

		if (s != 0) {
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}

	}

	s = sem_destroy(&mutex);
	CHECK_ERR(s,"sem_destroy")

	s = sem_destroy(&barrier);
	CHECK_ERR(s,"sem_destroy")

	return 0;
}
