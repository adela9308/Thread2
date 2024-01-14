#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "lib/mutex.h"
#include "lib/thread.h"
#include "lib/utility_methods.h"

#define MAX_SLEEP 5

int mutex1;
int mutex2;

void* function_in_thread_1() {
	int random_time;

	while (1) {
		srand(time(NULL));
		random_time = rand() % MAX_SLEEP;
		printf("[%s]Sleeping for %d seconds\n", __FUNCTION__, random_time);
		ms_sleep(random_time * 1000);
		printf("[%s]Trying to acquire mutex1 (holding none)\n", __FUNCTION__);
		my_mutex_lock(mutex1);
		printf("[%s]Acquired mutex1\n", __FUNCTION__);

		random_time = rand() % MAX_SLEEP;
		printf("[%s]Sleeping for %d seconds\n", __FUNCTION__, random_time);
		sleep(random_time);

		printf("[%s]Trying to acquire mutex2 (holding mutex1) \n", __FUNCTION__);
		my_mutex_lock(mutex2);
		printf("[%s]Acquired mutex2\n\n", __FUNCTION__);

		my_mutex_unlock(mutex2);
		my_mutex_unlock(mutex1);
	}

	return NULL;
}

void* function_in_thread_2() {
	int random_time;

	while (1) {
		printf("LALALLA");
		srand(time(NULL));
		random_time = rand() % MAX_SLEEP;
		printf("[%s]Sleeping for %d seconds\n", __FUNCTION__, random_time);
		ms_sleep(random_time * 1000);
		printf("[%s]Trying to acquire mutex2 (holding none)\n", __FUNCTION__);
		my_mutex_lock(mutex2);
		printf("[%s]Acquired mutex2\n", __FUNCTION__);

		random_time = rand() % MAX_SLEEP;
		printf("[%s]Sleeping for %d seconds\n", __FUNCTION__, random_time);
		sleep(random_time);

		printf("[%s]Trying to acquire mutex1 (holding mutex2) \n", __FUNCTION__);
		my_mutex_lock(mutex1);
		printf("[%s]Acquired mutex1\n\n", __FUNCTION__);

		my_mutex_unlock(mutex1);
		my_mutex_unlock(mutex2);
	}
	return NULL;
}

int main() {
	int thread1;
	int thread2;

	init(100000);
	my_mutex_init(&mutex1);
	my_mutex_init(&mutex2);
	
	my_pthread_create(&thread1, &function_in_thread_1);
	my_pthread_create(&thread2, &function_in_thread_2);
	
	my_pthread_join(thread1, NULL);
	my_pthread_join(thread2, NULL);

	return 0;
}