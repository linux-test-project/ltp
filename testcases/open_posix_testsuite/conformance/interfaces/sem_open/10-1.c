/*
    Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */

/*
   sem_open test case that attempts to open a new semaphore,
   call sem_wait, then try to re-create the same semaphore, which
   have no effect when you try to open a semaphore that already exist.
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "10-1"
#define FUNCTION "sem_open"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t *mysemp;
	char semname[50];
	int val;

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	mysemp = sem_open(semname, O_CREAT, 0777, 5);
	if (mysemp == SEM_FAILED) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	if (sem_wait(mysemp) == -1) {
		perror(ERROR_PREFIX "sem_close");
		return PTS_UNRESOLVED;
	}

	mysemp = sem_open(semname, O_CREAT, 0777, 1);
	if (mysemp == SEM_FAILED) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	if (sem_getvalue(mysemp, &val) == -1) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;
	}

	if (val != 4) {
		puts("TEST FAILED: second call of sem_open took place");
		sem_unlink(semname);
		return PTS_FAIL;
	} else {
		puts("TEST PASSED");
		sem_close(mysemp);
		sem_unlink(semname);
		return PTS_PASS;
	}
}
