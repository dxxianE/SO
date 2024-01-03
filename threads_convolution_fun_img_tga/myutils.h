#ifndef MYUTILS
#define MYUTILS

#include <sys/time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>     // unix-like system calls read and write
#include <fcntl.h>      // unix-like file handling : open
#include <stdlib.h>     // standard C lib input output basic functions compatible with Windows
#include <stdio.h>      // standard C lib input output basic functions compatible with Windows
#include <stdbool.h>
#include <string.h>     // also from standard C lib : basic string functions like strlen
#include <pthread.h>


void startTimer(int i);   // start timer i
long endTimer(int i);     // returns millis since timer i started

int read_split( int fin, char* buff, int maxlen, char* ch_end );

// Semaphores implementation Using pthreads monitors
typedef struct semaphore_struct {
	int i;
	pthread_mutex_t lock;
	pthread_cond_t cond;
} my_semaphore;

void my_sem_init(my_semaphore* sem, int i);
void my_sem_wait(my_semaphore* sem);
void my_sem_signal(my_semaphore* sem);


typedef struct struct_monitor_lock {
	bool bInUse;                // the ressource is being used?
	pthread_mutex_t lock;      // lock to use/modify vars
	pthread_cond_t  cond_free;  // condition for waiters of the ressource
} monitor_lock;

void mon_lock_init(monitor_lock* ml);
void mon_lock(monitor_lock* ml);
void mon_unlock(monitor_lock* ml);

// File Lock Region functions : it returns if it is occupied
int file_lock(int fd, int start, off_t len);
int file_unlock(int fd, int start, off_t len);


// Sockets: Builds an address given the ip & port both as a string (char*)
void fill_addr(char *ip, char* port, struct sockaddr_in* addr);


typedef struct country_struct {
	char name[80];
	long population;
	int area;	
	int coastline;
} country;


void printCountry(country c);

#endif