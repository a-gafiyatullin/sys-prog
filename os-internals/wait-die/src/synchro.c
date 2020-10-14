#include "synchro.h"

void lock_init(struct lock *lock)
{
	pthread_mutex_init(&lock->mutex, NULL);
}

void lock(struct lock *lock)
{
	pthread_mutex_lock(&lock->mutex);
}

void unlock(struct lock *lock)
{
	pthread_mutex_unlock(&lock->mutex);
}

void condition_init(struct condition *cv)
{
	pthread_cond_init(&cv->cond, NULL);
}

void wait(struct condition *cv, struct lock *lock)
{
	pthread_cond_wait(&cv->cond, &lock->mutex);
}

void notify_one(struct condition *cv)
{
	pthread_cond_signal(&cv->cond);
}

void notify_all(struct condition *cv)
{
	pthread_cond_broadcast(&cv->cond);
}
