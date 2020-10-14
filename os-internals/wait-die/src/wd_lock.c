#include "wd_lock.h"

/* Initialize new context  */
void wdlock_ctx_init(struct wdlock_ctx *ctx)
{
	static atomic_ullong next;

	ctx->timestamp = atomic_fetch_add(&next, 1) + 1;
	ctx->locks = NULL;
}

/* Initialize new lock */
void wdlock_init(struct wdlock *lock)
{
	lock_init(&lock->lock);
	lock_init(&lock->wd_lock_guard);
	condition_init(&lock->cv);
	lock->owner = NULL;
}

/* Capture lock with context. Return 0 on fail, 1 on success */
int wdlock_lock(struct wdlock *l, struct wdlock_ctx *ctx)
{
	lock(&l->wd_lock_guard);

	if (l->owner != NULL) {
		if (l->owner->timestamp > ctx->timestamp) {
			lock(&l->lock);
			unlock(&l->wd_lock_guard);

			wait(&l->cv, &l->lock);

			unlock(&l->lock);
			lock(&l->wd_lock_guard);
		} else {
			unlock(&l->wd_lock_guard);
			return 0;
		}
	}

	l->owner = ctx;
	struct wdlock *next_lock = ctx->locks;
	ctx->locks = l;
	l->next = next_lock;

	unlock(&l->wd_lock_guard);
	return 1;
}

/* Free all locks in the context */
void wdlock_unlock(struct wdlock_ctx *ctx)
{
	struct wdlock *locks = ctx->locks;

	while (locks != NULL) {
		lock(&locks->wd_lock_guard);
		lock(&locks->lock);
		locks->owner = NULL;
		notify_all(&locks->cv);
		unlock(&locks->lock);
		unlock(&locks->wd_lock_guard);

		locks = locks->next;
	}

	ctx->locks = NULL;
}
