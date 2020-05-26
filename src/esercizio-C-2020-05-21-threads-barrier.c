#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <errno.h>
#include <semaphore.h>
#include <pthread.h>

#define M 10
sem_t * process_semaphore;
pthread_barrier_t thread_barrier;
int number_of_threads = M;

void * thread_function(void * arg) {

	long int id_thread = pthread_self();
	int rand_time = (int)((double)random() / RAND_MAX * 3000000);

	usleep(rand_time); // like sleep() but using microseconds as unit

	//fase 1, thread id=, sleep period= secondi
	// 3.4.2 Mutual exclusion solution, pag. 19
			if (sem_wait(process_semaphore) == -1) {
				perror("sem_wait");
				exit(EXIT_FAILURE);
			}

			double sec = (double) rand_time / 1000000;
			printf("fase 1, thread id=%lu, sleep period=%lfsecondi\n", id_thread, sec);


			if (sem_post(process_semaphore) == -1) {
				perror("sem_post");
				exit(EXIT_FAILURE);
			}

	// https://linux.die.net/man/3/pthread_barrier_wait

	pthread_barrier_wait(&thread_barrier);

	/*
	 The pthread_barrier_wait() function shall synchronize participating threads at
	 the barrier referenced by barrier. The calling thread shall block until
	 the required number of threads have called pthread_barrier_wait() specifying the barrier.
	*/


	printf("fase 2, thread id=%lu, dopo la barriera\n", id_thread);
	usleep(10000);

	printf("thread id=%lu bye!\n", id_thread);

	return NULL;
}




#define CHECK_ERR(a,msg) {if ((a) == -1) { perror((msg)); exit(EXIT_FAILURE); } }


int main() {

	int s;
	char * file_name = "output.txt";

	int fd = open(file_name,
				  O_CREAT | O_TRUNC | O_WRONLY,
				  S_IRUSR | S_IWUSR // l'utente proprietario del file avrà i permessi di lettura e scrittura sul nuovo file
				 );
	if (fd == -1) {
		perror("open()");
		exit(EXIT_FAILURE);
	}

	if (dup2(fd, STDOUT_FILENO) == -1) {
		perror("problema con dup2");
		exit(EXIT_FAILURE);
	}


	if (close(fd) == -1) {
			perror("close");
			exit(EXIT_FAILURE);
		}

	process_semaphore = malloc(sizeof(sem_t));

	s = sem_init(process_semaphore,
						0, // 1 => il semaforo è condiviso tra processi,
						   // 0 => il semaforo è condiviso tra threads del processo
						1 // valore iniziale del semaforo (se mettiamo 0 che succede?)
					  );

	CHECK_ERR(s,"sem_init")

	pthread_t threads[M];

	s = pthread_barrier_init(&thread_barrier, NULL, M);
	CHECK_ERR(s,"pthread_barrier_init")

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

	s = pthread_barrier_destroy(&thread_barrier);
	CHECK_ERR(s,"pthread_barrier_destroy")



	printf("bye\n");

	return 0;
}

