/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_getcpuclockid()
 *
 * Steps:
 * 	1. Create a thread
 *	2. pthread_join the created thread
 *	3. Call the API to get the clockid of this thread, it may fail.
 */

#include <pthread.h>
#include <time.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "3-1"
#define FUNCTION "pthread_getcpuclockid"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

void *thread_func()
{
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	int rc;
	clockid_t cid;
	pthread_t new_th;

	rc = pthread_create(&new_th, NULL, thread_func, NULL);
	if (rc != 0) {
		perror(ERROR_PREFIX "failed to create a thread");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_join(new_th, NULL);
	if (rc != 0) {
		perror(ERROR_PREFIX "pthread_join");
		exit(PTS_UNRESOLVED);
	}

	rc = pthread_getcpuclockid(new_th, &cid);
	if (rc == ESRCH) {
		printf("pthread_getcpuclockid returns ESRCH "
		       "when thread_id doesn't exist\n");
	} else {
		printf("pthread_getcpuclockid doesn't return "
		       "ESRCH when thread_id doesn't exist\n");
	}
	return PTS_PASS;
}
