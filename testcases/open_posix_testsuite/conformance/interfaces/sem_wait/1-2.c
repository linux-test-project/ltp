/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Keep calling sem_wait until the sempahore is locked.  That is
 * decrementing the semaphore value by one until its zero or locked.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "posixtest.h"

#define TEST "2-1"
#define FUNCTION "sem_wait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t *mysemp;
	char semname[28];
	int value = 10;
	int val;

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	/* Initial value of Semaphore is 10 */
	mysemp = sem_open(semname, O_CREAT, 0777, value);
	if (mysemp == SEM_FAILED || mysemp == NULL) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	while (value) {		// checking the value if zero yet.
		if (sem_wait(mysemp) == -1) {
			perror(ERROR_PREFIX "sem_getvalue");
			return PTS_UNRESOLVED;
		} else {
			value--;
		}
	}

	if (sem_getvalue(mysemp, &val) == -1) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;
	} else if (val == 0) {
		puts("TEST PASSED");
		sem_unlink(semname);
		sem_close(mysemp);
		return PTS_PASS;
	} else {
		puts("TEST FAILED: Semaphore is not locked");
		return PTS_FAIL;
	}
}
