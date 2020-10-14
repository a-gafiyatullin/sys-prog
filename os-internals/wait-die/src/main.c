#include <stdio.h>
#include <stdlib.h>
#include "wd_lock.h"

#define THREAD_NUM 8
#define LOCKS_NUM 2 * (1 + THREAD_NUM)

struct wdlock locks[LOCKS_NUM];
struct wdlock_ctx ctxs[THREAD_NUM];
pthread_t threads[THREAD_NUM];

struct data {
	void *obj;
	struct wdlock_ctx *ctx;
};

struct data data_array[THREAD_NUM];

void *test(void *obj)
{
	struct data *data_elem = (struct data *)(obj);

	int locked = 0;
	while (!locked) {
		int i;
		for (i = 0; i < LOCKS_NUM; i++) {
			if (wdlock_lock(&locks[i], data_elem->ctx) == 0) {
				wdlock_unlock(data_elem->ctx);
				break;
			}
		}
		if (i == LOCKS_NUM) {
			locked = 1;
		}
	}

	(*((int *)data_elem->obj))++;
	wdlock_unlock(data_elem->ctx);
}

int main()
{
	for (int i = 0; i < THREAD_NUM; i++) {
		wdlock_ctx_init(&ctxs[i]);
		data_array[i].ctx = &ctxs[i];
	}

	for (int i = 0; i < LOCKS_NUM; i++) {
		wdlock_init(&locks[i]);
		locks[i].num = i;
	}

	int storage = 0;
	for (int i = 0; i < THREAD_NUM; i++) {
		data_array[i].obj = &storage;
	}

	for (int i = 0; i < THREAD_NUM; i++) {
		if (pthread_create(&threads[i], NULL, test, &data_array[i]) !=
		    0) {
			perror("pthread_create");
			exit(-1);
		}
	}

	for (int i = 0; i < THREAD_NUM; i++) {
		pthread_join(threads[i], NULL);
	}

	printf("SUM: %d\n", storage);

	return 0;
}
