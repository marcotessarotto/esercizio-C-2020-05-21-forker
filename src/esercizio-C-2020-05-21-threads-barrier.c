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
#include <sys/random.h>


#define M 10
#define CHECK_ERR(a,msg) {if ((a) == -1) { perror((msg)); exit(EXIT_FAILURE); } }
#define CHECK_ERR2(a,msg) {if ((a) != 0) { perror((msg)); exit(EXIT_FAILURE); } }

pthread_barrier_t thread_barrier;
int fd;
char * file_name = "log.txt";

void * thread_function(void * arg);


int main(int argc, char * argv []) {
	int s;

	fd = open(file_name,
				  O_CREAT | O_TRUNC | O_WRONLY,
				  S_IRUSR | S_IWUSR // l'utente proprietario del file avrà i permessi di lettura e scrittura sul nuovo file
				 );
	CHECK_ERR(fd, "open")

	// STDOUT_FILENO (fd == 1) si riferisce al file
    if (dup2(fd, STDOUT_FILENO) == -1) {
    	perror("dup2");

    	exit(EXIT_FAILURE);
    }

    s = close(fd);
    CHECK_ERR(s, "close")

	s = pthread_barrier_init(&thread_barrier, NULL, M);
	CHECK_ERR(s,"pthread_barrier_init")

	pthread_t * threads = calloc(M, sizeof(pthread_t));
	if (threads == NULL){
		perror("calloc()\n");
		exit(EXIT_FAILURE);
	}

	// create M threads
	for (int i = 0; i < M; i++) {
		s = pthread_create(&threads[i], NULL, thread_function, NULL);
		CHECK_ERR2(s,"pthread_create")
	}

	// join M threads
	for(int i=0 ; i<M ; i++) {
		s = pthread_join(threads[i], NULL);
		CHECK_ERR2(s,"pthread_join")
	}

	s = pthread_barrier_destroy(&thread_barrier);
	CHECK_ERR(s,"pthread_barrier_destroy")

	printf("bye\n");

	return 0;
}


void * thread_function(void * arg) {
	int res;
	unsigned int random_value;

/*
"fase 1" vuol dire: dormire per un intervallo random di tempo compreso tra 0 e 3 secondi,
poi scrivere nel file il messaggio: "fase 1, thread id=, sleep period= secondi"

"fase 2" vuol dire: scrivere nel file il messaggio "fase 2, thread id=, dopo la barriera"
poi dormire per 10 millisecondi, scrivere nel file il messggio "thread id= bye!".

per ogni thread: effettuare "fase 1", poi aspettare che tutti i thread abbiano completato
la fase 1 (barriera: little book of semaphores, pag. 29); poi effettuare "fase 2" e terminare il thread.
 */

	res = getrandom(&random_value,
		  sizeof(random_value),
		  //GRND_RANDOM |
		  //GRND_NONBLOCK |
		  0);
	CHECK_ERR(res, "getrandom")

	random_value = random_value % 4;

	sleep(random_value);

	pthread_t tid;

	tid = pthread_self();

	printf("fase 1, thread id=%ld, sleep period=%u\n", (long) tid, random_value);

	// https://linux.die.net/man/3/pthread_barrier_wait
	pthread_barrier_wait(&thread_barrier);
	/*
	 The pthread_barrier_wait() function shall synchronize participating threads at
	 the barrier referenced by barrier. The calling thread shall block until
	 the required number of threads have called pthread_barrier_wait() specifying the barrier.
	*/

	printf("fase 2, thread id=%ld, dopo la barriera\n", (long) tid);

	struct timespec t;

	t.tv_sec = 0;  // seconds
	t.tv_nsec = 10 * 1000; // nanoseconds

	nanosleep(&t, NULL);

	printf("thread id=%ld bye!\n", (long) tid);

	return NULL;
}
