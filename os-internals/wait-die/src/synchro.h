#ifndef WAIT_DIE_SYNCHRO_H
#define WAIT_DIE_SYNCHRO_H

#include <pthread.h>

struct lock {
	pthread_mutex_t mutex;
};

void lock_init(struct lock *lock);
void lock(struct lock *lock);
void unlock(struct lock *lock);

struct condition {
	pthread_cond_t cond;
};

void condition_init(struct condition *cv);
void wait(struct condition *cv, struct lock *lock);
void notify_one(struct condition *cv);
void notify_all(struct condition *cv);

#endif // WAIT_DIE_SYNCHRO_H
