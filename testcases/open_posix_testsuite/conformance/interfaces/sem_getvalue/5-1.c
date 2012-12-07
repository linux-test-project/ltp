/*
    Copyright (c) 2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */

/*
 * This test case verifies that calling sem_getvalue doesn't change the
 * state of the semaphore.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include "posixtest.h"

#define TEST "5-1"
#define FUNCTION "sem_getvalue"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	char semname[NAME_MAX - 4];
	sem_t *mysemp;
	int val;

	snprintf(semname, sizeof(semname), "/" FUNCTION "_" TEST "_%d",
		 getpid());

	mysemp = sem_open(semname, O_CREAT, 0777, 4);

	if (mysemp == SEM_FAILED || mysemp == NULL) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	if (sem_getvalue(mysemp, &val) == -1) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;
	}

	if (sem_trywait(mysemp) == -1) {
		perror(ERROR_PREFIX "sem_trywait");
		return PTS_UNRESOLVED;
	}

	if (sem_getvalue(mysemp, &val) == -1) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;
	}

	/*
	   printf("Current value is: %d\n", val);
	 */

	if (val == 3) {
		puts("TEST PASSED");
		sem_close(mysemp);
		sem_unlink(semname);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
