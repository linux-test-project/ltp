/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * adam.li@intel.com
 * If the Timers option is supported, the timeout shall be based on
 * the CLOCK_REALTIME clock. If the Timers option is not supported,
 * the timeout shall be based on the system clock as returned by
 * the time() function.
 *
 */


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "posixtest.h"

#define TEST "10-1"
#define FUNCTION "sem_timedwait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define SLEEP_SEC 1

int main(void)
{
	sem_t mysemp;
	struct timespec ts, ts_2;
	int rc;

	/* Init the value to 0 */
	if (sem_init(&mysemp, 0, 0) == -1) {
		perror(ERROR_PREFIX "sem_init");
		return PTS_UNRESOLVED;
	}

	/* Set the abs timeout */
#ifdef CLOCK_REALTIME
	printf("Test CLOCK_REALTIME\n");
	if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
		perror("clock_gettime()");
		return PTS_UNRESOLVED;
	}
	ts.tv_sec += SLEEP_SEC;
	ts.tv_nsec = 0;
#else
	ts.tv_sec = time(NULL);
	ts.tv_sec += SLEEP_SEC;
	ts.tv_nsec = 0;
#endif
	/* Lock Semaphore */
	rc = sem_timedwait(&mysemp, &ts);
	if (rc != -1 || (rc == -1 && errno != ETIMEDOUT)) {
		perror(ERROR_PREFIX "sem_timedwait");
		printf("Expect timedout\n");
		return PTS_UNRESOLVED;
	}

	/* Check the time */
#ifdef CLOCK_REALTIME
	if (clock_gettime(CLOCK_REALTIME, &ts_2) != 0) {
		perror("clock_gettime()");
		return PTS_UNRESOLVED;
	}
#else
	ts_2.tv_sec = time(NULL);
#endif
	if (ts_2.tv_sec == ts.tv_sec) {
		puts("TEST PASSED");
		sem_destroy(&mysemp);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
