#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utility_methods.h"

void block_signals() {
	sigset_t signals;
	sigemptyset(&signals);
	sigaddset(&signals, SIGALRM);

	if (sigprocmask(SIG_BLOCK, &signals, NULL) == -1) {
		perror("sigprocmask");
		abort();
	}
}

void unblock_signals() {
	sigset_t signals;
	sigemptyset(&signals);
	sigaddset(&signals, SIGALRM);

	if (sigprocmask(SIG_UNBLOCK, &signals, NULL) == -1) {
		perror("sigprocmask");
		abort();
	}
}

int ms_sleep(unsigned int ms) {
	int result = 0;

	struct timespec ts_remaining = { ms / 1000, (ms % 1000) * 1000000L };
	do {
		struct timespec ts_sleep = ts_remaining;
		result = nanosleep(&ts_sleep, &ts_remaining);
	} while ((EINTR == errno) && (-1 == result)); // sleep operation is retried if it gets interrupted by a signal -> EINTR == errno

	if (result == -1) {
		perror("nanosleep() failed");
	}

	return result;
}