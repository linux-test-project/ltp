/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * A single attributes object can be used in multiple simultaneous calls to
 * pthread_create().
 * NOTE: Results are undefined if pthread_attr_init() is called specifying an
 * already initialized 'attr' attributes object.
 *
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Create many threads using the same attribute object.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include "posixtest.h"

#define NUM_THREADS	5

void *a_thread_func(void *attr LTP_ATTRIBUTE_UNUSED)
{
	pthread_exit(NULL);
	return NULL;
}

int main(void)
{
	pthread_t new_threads[NUM_THREADS];
	pthread_attr_t new_attr;
	int i, ret;

	/* Initialize attribute */
	if (pthread_attr_init(&new_attr) != 0) {
		perror("Cannot initialize attribute object\n");
		return PTS_UNRESOLVED;
	}

	/* Create [NUM_THREADS] number of threads with the same attribute
	 * object. */
	for (i = 0; i < NUM_THREADS; i++) {
		ret =
		    pthread_create(&new_threads[i], &new_attr, a_thread_func,
				   NULL);
		if ((ret != 0) && (ret == EINVAL)) {
			printf("Test FAILED\n");
			return PTS_FAIL;
		} else if (ret != 0) {
			perror("Error creating thread\n");
			return PTS_UNRESOLVED;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;

}
