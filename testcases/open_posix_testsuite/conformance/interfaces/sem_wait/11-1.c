/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * sem_trywait shall try to lock the unlocked semaphore and decrement
 * the semaphore value by one.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "11-1"
#define FUNCTION "sem_trywait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t *mysemp;
	char semname[28];
	int val;

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	/* Initial value of Semaphore is 1 */
	mysemp = sem_open(semname, O_CREAT, 0777, 1);
	if (mysemp == SEM_FAILED || mysemp == NULL) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	/* Lock Semaphore by sem_trywait */
	if (sem_trywait(mysemp) == -1) {
		perror(ERROR_PREFIX "sem_wait");
		return PTS_UNRESOLVED;
	}

	/* Value of Semaphore */
	if (sem_getvalue(mysemp, &val) == -1) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;

		/* Checking if the value of the Semaphore decremented by one */
	} else if (val == 0) {
		puts("TEST PASSED");
		sem_unlink(semname);
		sem_close(mysemp);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
