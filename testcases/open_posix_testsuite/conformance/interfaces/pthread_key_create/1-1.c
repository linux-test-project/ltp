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
 * 1. Define an array of keys
 * 2. Use pthread_key_create() and create those keys
 * 3. Verify that you can set and get specific values for those keys without errors.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define NUM_OF_KEYS 10
#define KEY_VALUE 0

pthread_key_t keys[NUM_OF_KEYS];

int main(void)
{
	int i;
	void *rc;

	for (i = 0; i < NUM_OF_KEYS; i++) {
		if (pthread_key_create(&keys[i], NULL) != 0) {
			printf("Error: pthread_key_create() failed\n");
			return PTS_UNRESOLVED;
		} else {
			if (pthread_setspecific
			    (keys[i], (void *)(long)(i + KEY_VALUE)) != 0) {
				printf("Error: pthread_setspecific() failed\n");
				return PTS_UNRESOLVED;
			}

		}
	}

	for (i = 0; i < NUM_OF_KEYS; ++i) {
		rc = pthread_getspecific(keys[i]);
		if (rc != (void *)(long)(i + KEY_VALUE)) {
			printf
			    ("Test FAILED: Did not return correct value of thread-specific key, expected %ld, but got %ld\n",
			     (long)(i + KEY_VALUE), (long)rc);
			return PTS_FAIL;
		} else {
			if (pthread_key_delete(keys[i]) != 0) {
				printf("Error: pthread_key_delete() failed\n");
				return PTS_UNRESOLVED;
			}
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
