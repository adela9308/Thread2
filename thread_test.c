#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lib/thread.h"
#include "lib/utility_methods.h"

#define THREAD_COUNT 10

void* print_from_thread() {
	int self = my_pthread_self();
	for (size_t i = 0; i < 50; i++) {
		ms_sleep(100);
		printf("Hello from thread %d (%ld)\n", self, i);
	}

	return (void*)(self * 11);
}

int main() {
	int threads[THREAD_COUNT];
	int retvals[THREAD_COUNT];

	init(100000);

	for (size_t i = 0; i < THREAD_COUNT; i++) {
		my_pthread_create(&threads[i], &print_from_thread);
	}

	for (size_t i = 0; i < THREAD_COUNT; i++) {
		my_pthread_join(threads[i], (void**)&retvals[i]);
		printf("Joined thread %d with return value %d\n", threads[i], retvals[i]);
	}

	return 0;
}