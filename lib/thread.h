#ifndef THREAD_H
#define THREAD_H

#include <stdbool.h>
#include <stdlib.h>
#include <ucontext.h>

#define MAX_THREADS_COUNT 1000
typedef enum thread_state { BLOCKED, READY, TERMINATED } thread_state;

typedef struct thread_struct {
	int thread_id;
	ucontext_t context; // The context (counter, register, stack pointers, etc) of the thread
	thread_state state;
	void* (*start_routine)();
	void* retval;
} thread_struct;


// Initialize the library.
int init(long timeInMicroSec);

// Create a thread (stored in the reference given by the first parameter), that will run the start_routine function, with args parameters (https://man7.org/linux/man-pages/man3/pthread_create.3.html)
int my_pthread_create(int* thread_id, void* (*start_routine)());

// Wait for the given thread to terminate. (https://man7.org/linux/man-pages/man3/pthread_join.3.html)
int my_pthread_join(int thread_id, void** retval);

// Get the ID of the calling thread (https://man7.org/linux/man-pages/man3/pthread_self.3.html)
int my_pthread_self();

// Terminate the calling thread and returns a value via retval (https://man7.org/linux/man-pages/man3/pthread_exit.3.html)
void my_pthread_exit(void* retval);

// Causes the calling thread to relinquish the CPU (https://man7.org/linux/man-pages/man3/pthread_yield.3.html)
void my_pthread_yield(void);

#endif