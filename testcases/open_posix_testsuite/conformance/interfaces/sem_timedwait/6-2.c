/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * The process would be blocked, and the timeout parameter is
 * secified in nanoseconds field value greater than or equal to
 * 1000 million.  Should return ERROR (EINVAL).
*/


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "posixtest.h"

#define TEST "5-1"
#define FUNCTION "sem_timedwait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NANOSEC 1000000000

int main(void)
{
	sem_t mysemp;
	struct timespec ts;
	int sts;

	if (sem_init(&mysemp, 0, 0) == -1) {
		perror(ERROR_PREFIX "sem_init");
		return PTS_UNRESOLVED;
	}
	ts.tv_sec = time(NULL);
	ts.tv_nsec = NANOSEC;

	sts = sem_timedwait(&mysemp, &ts);

	if (errno == EINVAL && sts == -1) {
		puts("TEST PASSED");
		sem_destroy(&mysemp);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
