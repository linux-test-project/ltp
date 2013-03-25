/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_setspecific()
 *
 * shall acssociate a thread-specific value with a key obtained via a previouse call to
 * pthread_key_create.  Different threads may bind different values to the same key.
 * Calling pthread_setspecific with a key value not obtiained from pthread_key_create of after
 * the key has been deleted with pthread_key_delete is undefined.
 *
 * Steps:
 * 1.  Create a key
 * 2.  Bind a value from the main thread to this key
 * 3.  Create a thread and bind another value to this key
 * 4.  Compare the values bound to the key between the main thread and the newly created thread,
 *     they should be different and pertaining to what each thread set as the value.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define KEY_VALUE_1 100
#define KEY_VALUE_2 200

pthread_key_t key;
void *rc1;
void *rc2;

void *a_thread_func()
{
	/* Bind a value to key for this thread (this will be different from the value
	 * that we bind for the main thread) */
	if (pthread_setspecific(key, (void *)(KEY_VALUE_2)) != 0) {
		printf
		    ("Test FAILED: Could not set the value of the key to %d\n",
		     (KEY_VALUE_2));
		pthread_exit((void *)PTS_FAIL);
		return NULL;
	}

	/* Get the bound value of the key that we just set. */
	rc2 = pthread_getspecific(key);

	pthread_exit(0);
	return NULL;

}

int main(void)
{
	pthread_t new_th;

	/* Create the key */
	if (pthread_key_create(&key, NULL) != 0) {
		printf("Error: pthread_key_create() failed\n");
		return PTS_UNRESOLVED;
	}

	/* Bind a value for this main thread */
	if (pthread_setspecific(key, (void *)(KEY_VALUE_1)) != 0) {
		printf
		    ("Test FAILED: Could not set the value of the key to %d\n",
		     (KEY_VALUE_1));
		return PTS_FAIL;
	}

	/* Create another thread.  This thread will also bind a value to the key */
	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0) {
		printf("Error: in pthread_create()\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to end execution */
	pthread_join(new_th, NULL);

	/* Get the value associated for the key in this main thread */
	rc1 = pthread_getspecific(key);

	/* Compare this value with the value associated for the key in the newly created
	 * thread, they should be different. */
	if (rc1 != (void *)(KEY_VALUE_1)) {
		printf
		    ("Test FAILED: Incorrect value bound to key, expected %d, got %ld\n",
		     KEY_VALUE_1, (long)rc1);
		return PTS_FAIL;
	}

	if (rc2 != (void *)(KEY_VALUE_2)) {
		printf
		    ("Test FAILED: Incorrect value bound to key, expected %d, got %ld\n",
		     KEY_VALUE_2, (long)rc2);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
