/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/* sem_timedwait shall return zero if the calling process successfully
 * performed the semaphore lock operation on the semaphore designated
 * by sem.
 */


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "posixtest.h"

#define TEST "4-1"
#define FUNCTION "sem_timedwait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t mysemp;
	struct timespec ts;
	int unresolved = 0, sts;

	if (sem_init(&mysemp, 0, 1) == -1) {
		perror(ERROR_PREFIX "sem_init");
		unresolved = 1;
	}

	ts.tv_sec = time(NULL) + 1;
	ts.tv_nsec = 0;

	/* Lock Semaphore */
	sts = sem_timedwait(&mysemp, &ts);
	if (sts == -1) {
		perror(ERROR_PREFIX "sem_timedwait");
		unresolved = 1;
	}

	/* unlock Semaphore */
	if (sem_post(&mysemp) == -1) {
		perror(ERROR_PREFIX "sem_post");
		unresolved = 1;
	}

	if ((sts == 0) && (unresolved == 0)) {
		puts("TEST PASSED");
		sem_destroy(&mysemp);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
