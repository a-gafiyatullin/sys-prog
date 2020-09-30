#include "scheduler.h"

int max_time_on_cpu = 0;

struct thread *threads_queue = NULL;
struct thread *tail_threads_queue = NULL; // pointer to tail of threads_queue
struct thread *running_thread = NULL; // pointer to current thread

void scheduler_setup(int timeslice)
{
	max_time_on_cpu = timeslice;
}

void new_thread(int thread_id)
{
	struct thread *new_thread = malloc(sizeof(struct thread));
	if (new_thread == NULL) {
		perror("new_thread");
		exit(errno);
	}

	new_thread->thread_id = thread_id;
	new_thread->state = READY;
	new_thread->time_quantums = 0;
	new_thread->next = NULL;
	new_thread->prev = NULL;

	if (threads_queue == NULL) {
		/* it is the first thread */
		threads_queue = new_thread;
		tail_threads_queue = new_thread;

	} else {
		/* it is not the first thread */
		tail_threads_queue->next = new_thread;
		new_thread->prev = tail_threads_queue;
		tail_threads_queue = new_thread;
	}

	/* start this thread if there are no running threads */
	if (running_thread == NULL) {
		running_thread = new_thread;
		new_thread->state = RUNNING;
	}
}

void choose_ready_thread()
{
	/* choose thread that ready to use CPU */
	running_thread = NULL;
	struct thread *current = threads_queue;
	while (current != NULL) {
		if (current->state == READY) {
			running_thread = current;
			break;
		}
		current = current->next;
	}

	if (running_thread != NULL) {
		running_thread->state = RUNNING;
	}
}

void exit_thread()
{
	if (running_thread == NULL) {
		return;
	}

	struct thread *to_delete = running_thread;

	/* delete this thread */
	struct thread *prev = to_delete->prev;
	struct thread *next = to_delete->next;
	if (prev == NULL) {
		threads_queue = next;
	} else {
		prev->next = next;
	}
	if (next != NULL) {
		next->prev = prev;
	} else {
		tail_threads_queue = prev;
	}

	free(to_delete);

	choose_ready_thread();
}

void push_thread_to_tail(struct thread *th)
{
	if (th == tail_threads_queue) {
		return;
	}
	/* push the thread to end of the queue */
	struct thread *prev = th->prev;
	struct thread *next = th->next;
	if (prev == NULL) {
		threads_queue = next;
	} else {
		prev->next = next;
	}
	if (next != NULL) {
		next->prev = prev;
	}

	tail_threads_queue->next = th;
	th->next = NULL;
	th->prev = tail_threads_queue;
	tail_threads_queue = th;
}

void block_thread()
{
	if (running_thread == NULL) {
		return;
	}

	running_thread->state = BLOCKED;
	running_thread->time_quantums = 0;

	choose_ready_thread();
}

void wake_thread(int thread_id)
{
	struct thread *current = threads_queue;
	while (current != NULL) {
		if (current->state == BLOCKED &&
		    current->thread_id == thread_id) {
			current->state = READY;
			break;
		}
		current = current->next;
	}

	push_thread_to_tail(current);

	if (running_thread == NULL) {
		choose_ready_thread();
	}
}

void timer_tick()
{
	if (running_thread == NULL) {
		return;
	}

	running_thread->time_quantums++;
	if (running_thread->time_quantums >= max_time_on_cpu) {
		running_thread->state = READY;
		running_thread->time_quantums = 0;

		push_thread_to_tail(running_thread);
		choose_ready_thread();
	}
}

int current_thread()
{
	return (running_thread == NULL ? -1 : running_thread->thread_id);
}
