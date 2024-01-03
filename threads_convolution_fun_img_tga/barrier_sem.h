#ifndef MYBARRIER
#define MYBARRIER

#include <unistd.h>     // unix-like system calls read and write
#include <fcntl.h>      // unix-like file handling : open
#include <stdio.h>      // standard C lib input output basic functions compatible with Windows
#include <string.h>     // also from standard C lib : basic string functions like strlen
#include <pthread.h>    //

typedef struct barrier_struct {
	int n;
	pthread_mutex_t lock;
	pthread_cond_t cond;
} barrier_type;

void barrier_init(barrier_type* b, int i);
void barrier_wait_all(barrier_type* b);

#endif