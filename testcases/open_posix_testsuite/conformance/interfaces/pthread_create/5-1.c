/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_create()
 * The thread is created executing 'start_routine' with 'arg' as its only
 * argument.
 *
 * Steps:
 * 1.  Create 5 separete threads using pthread_create() passing to it a single int 'arg'.
 * 2.  Use that passed int argument in the thread function start routine  and make sure no
 *     errors occur.
 */

#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "posixtest.h"

#define NUM_THREADS 5

static void *a_thread_func(void *num)
{
	printf("arg = %li\n", (long)num);
	return num;
}

int main(void)
{
	pthread_t new_th;
	long i;
	void *res;
	int ret;

	for (i = 1; i < NUM_THREADS + 1; i++) {
		ret = pthread_create(&new_th, NULL, a_thread_func, (void *)i);

		if (ret) {
			fprintf(stderr, "pthread_create(): %s\n",
			        strerror(ret));
			return PTS_FAIL;
		}

		pthread_join(new_th, &res);

		if ((long)res != i) {
			printf("Test FAILED: Returned value did not match %li != %li",
			       (long)res, i);
			return PTS_FAIL;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
