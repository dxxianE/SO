#include "myutils.h"

struct timeval start[100];

void startTimer(int i) {
    if(i>=100) printf("Warning only 100 timers available!!");
    gettimeofday(&start[i], NULL);
}

long endTimer(int i) {
    long mtime, seconds, useconds;    
    struct timeval end;

    gettimeofday(&end, NULL);
    seconds  = end.tv_sec  - start[i].tv_sec;
    useconds = end.tv_usec - start[i].tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
    return mtime;
}


int read_split(int fin, char * buff, int maxlen, char * ch_end) {
  int i;
  char * is_split = 0;
  for (i = 0; i < maxlen && !is_split && read(fin, ch_end, 1); i++) {
    is_split = strchr(" \n\t;&", *ch_end); //c is one of ' ' or '\n' or '\t' or '&'
    if (!is_split) buff[i] = *ch_end;
  }
  if (is_split) buff[i - 1] = '\0';
  return i;
}



void my_sem_init(my_semaphore* sem, int i) {
  sem->i = i;
  pthread_mutex_init(&sem->lock, NULL);
  pthread_cond_init(&sem->cond, NULL);
}


void my_sem_wait(my_semaphore* sem) {
   pthread_mutex_lock(&sem->lock);
   while(sem->i == 0) {
    pthread_cond_wait(&sem->cond, &sem->lock);
   }
   sem->i--;
   pthread_mutex_unlock(&sem->lock);
}


void my_sem_signal(my_semaphore* sem) {
   pthread_mutex_lock(&sem->lock);
   sem->i++;
   pthread_cond_signal(&sem->cond);
   pthread_mutex_unlock(&sem->lock);
}



int file_lock(int fd, int start, off_t len)
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = start;
    fl.l_len = len;
    return fcntl(fd, F_SETLK, &fl);
}

int file_unlock(int fd, int start, off_t len)
{
    struct flock fl;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = start;
    fl.l_len = len;
    return fcntl(fd, F_SETLK, &fl);
}


void mon_lock_init(monitor_lock* ml) {
  ml->bInUse=false;
  pthread_mutex_init(&ml->lock, NULL);
  pthread_cond_init(&ml->cond_free, NULL);
}

void mon_lock(monitor_lock* ml) {
  pthread_mutex_lock(&ml->lock);
  while(ml->bInUse) {
    pthread_cond_wait(&ml->cond_free,&ml->lock);
  }
  ml->bInUse = true;
  pthread_mutex_unlock(&ml->lock);
}


void mon_unlock(monitor_lock* ml) {
  pthread_mutex_lock(&ml->lock);
  ml->bInUse = false;
  pthread_cond_signal(&ml->cond_free);
  pthread_mutex_unlock(&ml->lock);
}


void fill_addr(char *ip, char* port, struct sockaddr_in* addr) {
    int portnum;
    struct hostent *server = NULL;

    if(ip != NULL) {
        server = gethostbyname(ip);
        if (server == NULL) { fprintf(stderr,"ERROR, no such host\n"); exit(0); }
    }

    memset((char *)addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    
    if(ip == NULL) addr->sin_addr.s_addr = INADDR_ANY;
    else { 
        memcpy((char *)&addr->sin_addr.s_addr, 
               (char *)server->h_addr, 
               server->h_length);
    }

    portnum = atoi(port);
    addr->sin_port = htons(portnum); // convert port num to network byte order
}



void printCountry(country c) {
    printf("----------------------\n");
    printf("Country: %s\n",c.name);
    printf("  populat: %ld\n",c.population);
    printf("     area: %d\n",c.area);
    printf("    coast: %d\n",c.coastline);
}