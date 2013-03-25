/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/* This test case verifies that the semaphore shall be locked until the
 * sem_post is executed and returns successfully.
 * Lines: 39056-39057
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "posixtest.h"

#define TEST "3-1"
#define FUNCTION "sem_wait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t *mysemp;
	char semname[28];
	int val;

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	mysemp = sem_open(semname, O_CREAT, 0, 1);
	if (mysemp == SEM_FAILED || mysemp == NULL) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	if (sem_wait(mysemp) == -1) {
		perror(ERROR_PREFIX "sem_wait");
		return PTS_UNRESOLVED;
	}

	if (sem_post(mysemp) == -1) {
		perror(ERROR_PREFIX "sem_post");
		return PTS_UNRESOLVED;
	}

	if (sem_getvalue(mysemp, &val) < 0) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;
	}

	if (val == 1) {
		puts("TEST PASSED");
		sem_unlink(semname);
		sem_close(mysemp);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
