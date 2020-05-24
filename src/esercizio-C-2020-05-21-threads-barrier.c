#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define M 10

pthread_barrier_t barrier;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void err_exit(char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

void *thread_func(void *ptr)
{
	int *fd = (int *) ptr;
	pthread_t tid = pthread_self();
	time_t t;
	time(&t);
	if (t == ((time_t) -1))
		err_exit("time() error");

	srand((unsigned) t);
	char seconds = rand() % 3;
	sleep(seconds);

	if (pthread_mutex_lock(&mutex))
		err_exit("pthread_mutex_lock() error");
	int written = dprintf(*fd, "fase 1, thread id = %lu, sleep period = %d\n", tid, seconds);
	if (written < 0 )
		err_exit("dprintf() error");
	if (pthread_mutex_unlock(&mutex))
		err_exit("pthread_mutex_unlock() error");

	struct timespec tspec = {
			.tv_sec = 0,
			.tv_nsec = 10000000,
	};

	int ret = pthread_barrier_wait(&barrier);
	if (!(ret == PTHREAD_BARRIER_SERIAL_THREAD || ret == 0))
		err_exit("pthread_barrier_wait() error");

	if (pthread_mutex_lock(&mutex))
		err_exit("pthread_mutex_lock() error");
	written = dprintf(*fd, "fase 2, thread id = %lu, dopo la barriera\n", tid);
	if (written < 0 )
		err_exit("dprintf() error");
	if (pthread_mutex_unlock(&mutex))
		err_exit("pthread_mutex_unlock() error");

	if (nanosleep(&tspec, NULL))
		err_exit("nanosleep() error");

	if (pthread_mutex_lock(&mutex))
		err_exit("pthread_mutex_lock() error");
	written = dprintf(*fd, "thread id = %lu, bye!\n", tid);
	if (written < 0 )
		err_exit("dprintf() error");
	if (pthread_mutex_unlock(&mutex))
		err_exit("pthread_mutex_unlock() error");
	return NULL;
}

int main(int argc, char *argv[])
{
	int fd = open(argv[1], O_RDWR | O_TRUNC | O_CREAT, 00644);
	if (fd == -1)
		err_exit("open() error");

	if (pthread_barrier_init(&barrier, NULL, M))
		err_exit("pthread_barrier_init() error");

	pthread_t threads[M];
	for (int i = 0; i < M; i++) {
		if (pthread_create(&threads[i], NULL, thread_func, &fd))
			err_exit("pthread_create() error");
	}

	for (int i = 0; i < M; i++) {
		if (pthread_join(threads[i], NULL))
			err_exit("pthread_join() error");
	}

	if (pthread_barrier_destroy(&barrier))
		err_exit("pthread_barrier_destroy() error");

	close(fd);
	return 0;
}
