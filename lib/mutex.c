#include <errno.h>
#include <stdio.h>

#include "mutex.h"
#include "utility_methods.h"


static mutex_struct mutex_list[MAX_THREADS_COUNT];
static size_t mutex_count = 0;

int my_mutex_init(int* mutex_id) {
	if (mutex_count == MAX_THREADS_COUNT - 1) {
		return EXIT_FAILURE;
	}

	mutex_struct* mutex = &mutex_list[mutex_count];
	mutex->mutex_id = mutex_count;
	*mutex_id = mutex_count;
	mutex->thread_holder_id = -1;
	mutex->waiting_threads_count = 0;

	mutex_count++;
	return EXIT_SUCCESS;
}

int my_mutex_lock(int mutex_id) {
	block_signals();
	int calling_thread_id = my_pthread_self();
	mutex_struct* mutex = &mutex_list[mutex_id];
	display_deadlocks();
	unblock_signals();

	if (mutex->thread_holder_id == calling_thread_id) {
		return EXIT_SUCCESS;
	}

	if (mutex->thread_holder_id != -1) {
		block_signals();
		mutex->waiting_threads_count++;
		mutex->waiting_threads[calling_thread_id] = true;
		unblock_signals();

		while (mutex->thread_holder_id != -1) {
			my_pthread_yield();
		}
	}

	block_signals();
	mutex->thread_holder_id = calling_thread_id;
	unblock_signals();

	return EXIT_SUCCESS;
}

int my_mutex_unlock(int mutex_id) {
	block_signals();
	int calling_thread_id = my_pthread_self();
	mutex_struct* mutex = &mutex_list[mutex_id];

	if (mutex->thread_holder_id != calling_thread_id) { // The current thread does not own the mutex.
		unblock_signals();
		errno = EPERM;
		return EXIT_FAILURE;
	}

	mutex->thread_holder_id = -1;
	for (size_t i = 0; i < mutex->waiting_threads_count; i++) {
		mutex->waiting_threads[i] = false;
	}
	mutex->waiting_threads_count = 0;

	unblock_signals();
	return EXIT_SUCCESS;
}

int my_mutex_destroy(int mutex_id) {
	mutex_struct* mutex = &mutex_list[mutex_id];

	if (mutex->thread_holder_id != -1) {
		errno = EBUSY;
		return EXIT_FAILURE;
	}

	mutex->mutex_id = -1;
	return EXIT_SUCCESS;
}

void display_deadlocks() {
	bool displayed_deadlock = false;

	for (size_t i = 0; i < mutex_count; i++) {
		mutex_struct* mutex1 = &mutex_list[i];
		if (mutex1->thread_holder_id == -1) {
			continue;
		}
		for (size_t j = i; j < mutex_count; j++) {
			mutex_struct* mutex2 = &mutex_list[j];
			if (mutex2->thread_holder_id == -1 || mutex1->thread_holder_id == mutex2->thread_holder_id) {
				continue;
			}

			if (mutex1->waiting_threads[mutex2->thread_holder_id] && mutex2->waiting_threads[mutex1->thread_holder_id]) {
				// Two different threads waiting for the same lock
				printf("**Deadlock Spotted** Thread %d -> mutex %d held by thread %d and thread %d -> mutex %d held by %d\n",
					mutex2->thread_holder_id, mutex1->mutex_id, mutex1->thread_holder_id, mutex1->thread_holder_id, mutex2->mutex_id, mutex2->thread_holder_id);
				if (!displayed_deadlock) {
					displayed_deadlock = true;
				}
			}
		}
	}

	if (!displayed_deadlock) {
		printf("---- There were no deadlocks detected\n");
	}
}
