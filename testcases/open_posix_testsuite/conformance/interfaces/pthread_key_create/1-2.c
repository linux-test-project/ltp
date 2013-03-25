/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_key_create()
 *
 *  shall create a thread-specific data key visible to all threaads in the process.  Key values
 *  provided by pthread_key_create() are opaque objects used to locate thread-specific data.
 *  Although the same key value may be used by different threads, the values bound to the key
 *  by pthread_setspecific() are maintained on a per-thread basis and persist for the life of
 *  the calling thread.
 *
 * Steps:
 * 1. Define and create a key
 * 2. Verify that you can create many threads with the same key value
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define NUM_OF_THREADS 10
#define KEY_VALUE 1000
pthread_key_t keys[NUM_OF_THREADS];
int i;

/* Thread function that sets the key to KEY_VALUE */
void *a_thread_func()
{
	/* Set the key to KEY_VALUE */
	if (pthread_setspecific(keys[i], (void *)(KEY_VALUE)) != 0) {
		printf("Error: pthread_setspecific() failed\n");
		pthread_exit((void *)PTS_FAIL);
	}

	pthread_exit(0);
}

int main(void)
{
	pthread_t new_th;
	void *value_ptr;

	/* Create a key */
	for (i = 0; i < NUM_OF_THREADS; i++) {
		if (pthread_key_create(&keys[i], NULL) != 0) {
			printf("Error: pthread_key_create() failed\n");
			return PTS_UNRESOLVED;
		}
	}

	/* Create NUM_OF_THREADS threads and in the thread_func, it will
	 * use pthread_setspecific with the same KEY_VALUE */
	for (i = 0; i < NUM_OF_THREADS; i++) {
		/* Create a thread */
		if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0) {
			perror("Error creating thread\n");
			return PTS_UNRESOLVED;
		}

		/* Wait for thread to end */
		if (pthread_join(new_th, &value_ptr) != 0) {
			perror("Error in pthread_join\n");
			return PTS_UNRESOLVED;
		}

		if (value_ptr == (void *)PTS_FAIL) {
			printf
			    ("Test FAILED: Could not use a certain key value to set for many keys\n");
			return PTS_FAIL;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
