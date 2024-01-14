#ifndef MUTEX_H
#define MUTEX_H
#include "thread.h"

typedef struct mutex_struct {
	int mutex_id;
	int thread_holder_id;
	bool waiting_threads[MAX_THREADS_COUNT];
	int waiting_threads_count;
} mutex_struct;

int my_mutex_init(int* mutex_id);
int my_mutex_lock(int mutex_id);
int my_mutex_unlock(int mutex_id);
int my_mutex_destroy(int mutex_id);

void display_deadlocks();

#endif