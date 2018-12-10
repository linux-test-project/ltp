/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * This test case will verify that sem_timedwait will wait one second to
 * unlock the locked semaphore, and then when the semaphore fails to
 * unlock should return -1, and leave the semaphore status unchanged
 * by doing sem_post on the sempahore and check the current value of
 * the semaphore.
 */


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "posixtest.h"

#define TEST "2-2"
#define FUNCTION "sem_timedwait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t mysemp;
	struct timespec ts;
	int sts, val;

	if (sem_init(&mysemp, 0, 0) == -1) {
		perror(ERROR_PREFIX "sem_init");
		return PTS_UNRESOLVED;
	}

	ts.tv_sec = time(NULL) + 1;
	ts.tv_nsec = 0;

	/* Try to lock Semaphore */
	sts = sem_timedwait(&mysemp, &ts);

	if (sem_post(&mysemp) == -1) {
		perror(ERROR_PREFIX "sem_post");
		return PTS_UNRESOLVED;
	}

	if (sem_getvalue(&mysemp, &val) == -1) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;
	}

	if ((val == 1) && (sts == -1)) {
		puts("TEST PASSED");
		sem_destroy(&mysemp);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
