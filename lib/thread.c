#include "thread.h"
#include "mutex.h"
#include "utility_methods.h"

#include <signal.h>
#include <stdio.h>
#include <sys/time.h>


#define SIGNAL SIGALRM
struct itimerval signal_triggering_timer;
struct sigaction signal_handler;

static ucontext_t* main_context;
static thread_struct thread_list[MAX_THREADS_COUNT];

static thread_struct* running_thread = NULL;
static size_t thread_count = 0;


static thread_struct* get_next_thread_in_ready_state() { // using preemptive Round-Robin
	bool found = false;
	int next_thread_id;

	for (size_t i = 1; i < thread_count && !found; i++) {
		size_t current_thread_id = (running_thread->thread_id + i) % thread_count;
		if (thread_list[current_thread_id].state == READY) {
			found = true;
			next_thread_id = current_thread_id;
		}
	}

	return &thread_list[next_thread_id];
}

void schedule_threads() {
	if (thread_count == 0) {
		exit(0);
	}

	block_signals();
	thread_struct* current_thread = running_thread;
	running_thread = get_next_thread_in_ready_state();
	unblock_signals();
	swapcontext(&current_thread->context, &running_thread->context);
}

// setting up a timer that periodically triggers a signal
static int setup_triggering_signal_timer(long timeInMicroSec) {

	signal_handler.sa_handler = &schedule_threads;
	sigemptyset(&signal_handler.sa_mask);
	sigaddset(&signal_handler.sa_mask, SIGNAL);

	sigaddset(&signal_handler.sa_mask, SA_RESTART);

	struct sigaction old_signal_handler;
	if (sigaction(SIGNAL, &signal_handler, &old_signal_handler) == -1) {
		perror("sigaction");
		return EXIT_FAILURE;
	}

	signal_triggering_timer.it_interval.tv_sec = 0;
	signal_triggering_timer.it_interval.tv_usec = timeInMicroSec;

	signal_triggering_timer.it_value.tv_sec = 0;
	signal_triggering_timer.it_value.tv_usec = timeInMicroSec;


	if (setitimer(ITIMER_REAL, &signal_triggering_timer, NULL) == -1) {
		if (sigaction(SIGNAL, &old_signal_handler, NULL) == -1) {
			perror("sigaction");
		}

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/////////////////////
void display_deadlock_method() {
	block_signals();
	display_deadlocks();
	unblock_signals();
}

static int display_deadlock_handler() {
	struct sigaction signal_action;

	signal_action.sa_handler = display_deadlock_method;
	sigemptyset(&signal_action.sa_mask);
	signal_action.sa_flags = SA_RESTART;  // Restart functions if interrupted by handler

	return EXIT_SUCCESS;
}


static void store_current_thread_return_value() {
	block_signals();
	thread_struct* thread = running_thread;
	unblock_signals();

	void* result = thread->start_routine();
	my_pthread_exit(result);
}

thread_struct* init_thread(thread_state state) {
	thread_struct* thread = &thread_list[thread_count];
	thread->thread_id = thread_count;
	thread->state = state;

	if (getcontext(&thread->context) == -1) {
		exit(EXIT_FAILURE);
	}

	thread_count++;
	return thread;
}

thread_struct* create_thread(thread_state state, void* (*start_routine)(void*)) {
	thread_struct* thread = init_thread(state);

	thread->context.uc_stack.ss_sp = malloc(SIGSTKSZ);
	thread->context.uc_stack.ss_size = SIGSTKSZ;
	thread->context.uc_stack.ss_flags = 0;
	thread->context.uc_link = main_context;
	makecontext(&thread->context, (void (*)(void))store_current_thread_return_value, 0);

	thread->start_routine = start_routine;

	return thread;
}

int init(long timeInMicroSec) {
	// Register main as an ULT thread
	running_thread = init_thread(READY);
	main_context = &running_thread->context;

	int status = setup_triggering_signal_timer(timeInMicroSec);
	if (status != EXIT_SUCCESS) {
		return status;
	}

	status = display_deadlock_handler();
	if (EXIT_SUCCESS != status) {
		return status;
	}
	return EXIT_SUCCESS;
}

////////////////////////////

int my_pthread_create(int* thread_id, void* (*start_routine)()) {
	if (thread_count == MAX_THREADS_COUNT - 1) {
		return EXIT_FAILURE;
	}

	block_signals();
	*thread_id = create_thread(READY, start_routine)->thread_id;
	unblock_signals();

	return 0;
}

void my_pthread_yield() { 
	raise(SIGNAL); 
}

int my_pthread_join(int thread_id, void** retval) {
	thread_struct* thread = &thread_list[thread_id];
	while (thread->state != TERMINATED) {
		my_pthread_yield();
	}

	if (retval != NULL) {
		*retval = thread->retval;
	}

	return 0;
}

int my_pthread_self() {
	return running_thread->thread_id; 
}

void my_pthread_exit(void* retval) {
	block_signals();
	running_thread->retval = retval;
	running_thread->state = TERMINATED;
	unblock_signals();

	my_pthread_yield();
}
