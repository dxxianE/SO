#include "barrier_sem.h"


void barrier_init(barrier_type* b, int i) {
	b->n = i;
	pthread_mutex_init(&b->lock, NULL);
    pthread_cond_init(&b->cond, NULL);
}


void barrier_wait_all(barrier_type* b) {
	pthread_mutex_lock(&b->lock);
	b->n--;
	if(b->n > 0) {
		pthread_cond_wait(&b->cond, &b->lock);
	}
	pthread_cond_broadcast(&b->cond);
	pthread_mutex_unlock(&b->lock);
}