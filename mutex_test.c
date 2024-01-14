#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lib/mutex.h"
#include "lib/thread.h"
#include "lib/utility_methods.h"


#define THREAD_COUNT 10

int count = 0;
int count_mutex;
int last_thread;

void* print_from_thread() {
	int thread_id = my_pthread_self();

	if (thread_id == 5) {
		ms_sleep(3000);
	}

	my_mutex_lock(count_mutex);
	count++;
	last_thread = thread_id;
	my_mutex_unlock(count_mutex);

	return NULL;
}

int main() {
	int threads[THREAD_COUNT];

	init(1000);
	my_mutex_init(&count_mutex);
	
	for (size_t i = 0; i < THREAD_COUNT; i++) {
		my_pthread_create(&threads[i], &print_from_thread);
	}

	for (size_t i = 0; i < THREAD_COUNT; i++) {
		my_pthread_join(threads[i], NULL);
	}

	printf("Final count value: %d\n", count);
	printf("The last thread that wrote was %lu\n", last_thread);

	return 0;
}