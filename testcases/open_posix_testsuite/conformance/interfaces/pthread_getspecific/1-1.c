/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_getspecific()
 *
 *  shall return the value currently bound to the specified key on behalf of the calling thread.
 *  Calling pthread_getspecific() with a key value not obtained from pthread_key_create() or
 *  after key has been deleted with pthread_key_delete() is undefined.
 *
 * Steps:
 * 1.  Create NUM_OF_KEYS pthread_key_t objects
 * 2.  Set each key to a thread-specific value with pthread_setspecific()
 * 3.  Call pthread_getspecific() on each key and check that the value returned is correct.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define NUM_OF_KEYS 10
#define KEY_VALUE 0

int main(void)
{
	pthread_key_t keys[NUM_OF_KEYS];
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
			    ("Test FAILED: Did not return correct value of thread-specific key, expected %d, but got %ld\n",
			     (i + KEY_VALUE), (long)rc);
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
