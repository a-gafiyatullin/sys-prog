#ifndef WAIT_DIE_WD_LOCK_H
#define WAIT_DIE_WD_LOCK_H

#include "synchro.h"
#include "stdatomic.h"

#define atomic_ullong unsigned long long

struct wdlock_ctx;

/* Lock */
struct wdlock {
	struct wdlock *next;
	const struct wdlock_ctx *owner;
	struct lock wd_lock_guard;

	int num;

	struct lock lock;
	struct condition cv;
};

/* Context */
struct wdlock_ctx {
	unsigned long long timestamp;
	struct wdlock *locks; // locked
};

/* Initialize new context  */
void wdlock_ctx_init(struct wdlock_ctx *ctx);

/* Initialize new lock */
void wdlock_init(struct wdlock *lock);

/* Capture lock with context. Return 0 on fail, 1 on success */
int wdlock_lock(struct wdlock *l, struct wdlock_ctx *ctx);

/* Free all locks in the context */
void wdlock_unlock(struct wdlock_ctx *ctx);

#endif //WAIT_DIE_WD_LOCK_H
