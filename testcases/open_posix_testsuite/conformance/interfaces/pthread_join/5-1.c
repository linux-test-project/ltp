/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that pthread_join()
 *
 * Upon successful completion, it returns 0;
 *
 * Steps:
 * 1.  Create a new thread.
 * 2.  Join that thread to main.  If the return code is not 0,
 *     or the other valid error codes of EINVAL, ESRCH or EDEADLK,
 *     then the test fails.
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

static void *a_thread_func()
{
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;
	int ret;

	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	ret = pthread_join(new_th, NULL);
	if (ret != 0) {
		if ((ret != EINVAL) && (ret != ESRCH) && (ret != EDEADLK)) {
			printf("Test FAILED: Invalid return code %d.\n", ret);
			return PTS_FAIL;
		}
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
