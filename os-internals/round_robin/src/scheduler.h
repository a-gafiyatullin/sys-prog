#ifndef ROUND_ROBIN_SCHEDULER_H
#define ROUND_ROBIN_SCHEDULER_H

#include "stdlib.h"
#include <stdio.h>
#include <errno.h>

extern int max_time_on_cpu;

enum states { RUNNING, READY, BLOCKED }; // thread states

struct thread {
	int thread_id;
	int time_quantums;
	enum states state;

	struct thread *next;
	struct thread *prev;
};

extern struct thread *threads_queue;
extern struct thread *tail_threads_queue; // pointer to tail of threads_queue
extern struct thread *running_thread; // pointer to current thread

/* timeslice - time that thread use CPU. */
void scheduler_setup(int timeslice);

/* create new thread. */
void new_thread(int thread_id);

/* end the current running thread. */
void exit_thread();

/* block the current running thread. */
void block_thread();

/* wake up the previously blocked thread. */
void wake_thread(int thread_id);

/* increase time_quantums of the current thread */
void timer_tick();

/* return the current running thread id */
int current_thread();

#endif //ROUND_ROBIN_SCHEDULER_H
