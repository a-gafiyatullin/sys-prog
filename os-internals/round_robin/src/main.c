#include "scheduler.h"
#include "assert.h"

#define MAX_TIME_ON_CPU 3

int main()
{
	scheduler_setup(MAX_TIME_ON_CPU);

	assert(current_thread() == -1);

	new_thread(0);
	assert(current_thread() == 0);

	timer_tick();
	new_thread(1);
	assert(current_thread() == 0);

	timer_tick();
	new_thread(2);
	assert(current_thread() == 0);

	timer_tick();
	new_thread(3);
	assert(current_thread() == 1);

	timer_tick();
	assert(current_thread() == 1);

	timer_tick();
	assert(current_thread() == 1);

	timer_tick();
	assert(current_thread() == 2);

	timer_tick();
	assert(current_thread() == 2);

	timer_tick();
	assert(current_thread() == 2);

	timer_tick();
	assert(current_thread() == 0);

	timer_tick();
	block_thread();
	assert(current_thread() == 3);

	timer_tick();
	assert(current_thread() == 3);

	timer_tick();
	assert(current_thread() == 3);

	timer_tick();
	assert(current_thread() == 1);

	timer_tick();
	block_thread();
	assert(current_thread() == 2);

	timer_tick();
	assert(current_thread() == 2);

	timer_tick();
	assert(current_thread() == 2);

	timer_tick();
	assert(current_thread() == 3);

	timer_tick();
	assert(current_thread() == 3);

	timer_tick();
	assert(current_thread() == 3);

	timer_tick();
	assert(current_thread() == 2);

	timer_tick();
	wake_thread(1);
	assert(current_thread() == 2);

	timer_tick();
	assert(current_thread() == 2);

	timer_tick();
	assert(current_thread() == 3);

	timer_tick();
	assert(current_thread() == 3);

	timer_tick();
	assert(current_thread() == 3);

	timer_tick();
	assert(current_thread() == 1);

	timer_tick();
	exit_thread();
	assert(current_thread() == 2);

	timer_tick();
	wake_thread(0);
	assert(current_thread() == 2);

	timer_tick();
	block_thread();
	assert(current_thread() == 3);

	timer_tick();
	exit_thread();
	assert(current_thread() == 0);

	timer_tick();
	block_thread();
	assert(current_thread() == -1);

	timer_tick();
	wake_thread(2);
	assert(current_thread() == 2);

	for (int i = 0; i < MAX_TIME_ON_CPU * 2; i++) {
		timer_tick();
		assert(current_thread() == 2);
	}

	timer_tick();
	exit_thread();
	wake_thread(0);
	assert(current_thread() == 0);

	exit_thread();
	assert(current_thread() == -1);

	return 0;
}
