#include <stdio.h>
#include <stdlib.h>
// for open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fcntl.h>
// for close
#include <unistd.h>
// for pthread_create
#include <pthread.h>

void * thread_function();


#define M 10

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

	pthread_t * threads = calloc(M, sizeof(pthread_t));
	if(threads == NULL){
		perror("calloc()\n");
		exit(1);
	}

	// create M threads
	for ( int i = 0 ; i < M ; i++){
		int res = pthread_create(&threads[i], NULL, thread_function, NULL);
		if(res != 0){
			perror("pthread_create()\n");
			exit(1);
		}
	}


	if(close(fd) == -1){
		perror("close()\n");
		exit(1);
	}

	exit(0);
}


void * thread_function(){
	printf("Thread\n");

	return NULL;
}
