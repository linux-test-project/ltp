/*
 * Copyright (c) 2003, 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Under no circumstance shall the function fail with a timeout if
 * the semaphore can be locked immediately. The validity of the
 * abs_timeout need not be checked if the semaphore can be locked
 * immediately.
 */


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "posixtest.h"

#define TIMEOUT 2
#define INVALIDTIMEOUT -2
#define TEST "11-1"
#define FUNCTION "sem_timedwait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t mysemp[2];
	struct timespec ts[2];
	int val[2], sts[2];
	int i;

	for (i = 0; i < 2; i++) {
		if (sem_init(&mysemp[i], 0, 1) == -1) {
			perror(ERROR_PREFIX "sem_init");
			return PTS_UNRESOLVED;
		}
		if (i == 0) {
			ts[i].tv_sec = time(NULL) + TIMEOUT;
			ts[i].tv_nsec = 0;
		} else if (i == 1) {
			ts[i].tv_sec = time(NULL) + INVALIDTIMEOUT;
			ts[i].tv_nsec = 0;
		}
		/* Lock Semaphore */
		sts[i] = sem_timedwait(&mysemp[i], &ts[i]);
		if (sts[i] == -1) {
			perror(ERROR_PREFIX "sem_timedwait");
			return PTS_UNRESOLVED;
		}

		/* Value of Semaphore */
		if (sem_getvalue(&mysemp[i], &val[i]) == -1) {
			perror(ERROR_PREFIX "sem_getvalue");
			return PTS_UNRESOLVED;
		}

		/* Checking if the value of the Semaphore decremented by one */
		if ((val[i] == 0) && (sts[i] == 0)) {
			puts("TEST PASSED");
			sem_destroy(&mysemp[i]);
			return PTS_PASS;
		} else {
			puts("TEST FAILED");
			sem_destroy(&mysemp[i]);
			return PTS_FAIL;
		}
	}
}
