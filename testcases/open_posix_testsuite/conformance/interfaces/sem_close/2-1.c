/* Copyright (c) 2002, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */

/*
   This case verifies that sem_close deallocate resources, and then the
   semaphore is available for reuse.
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "2-1"
#define FUNCTION "sem_close"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t *mysemp;
	char semname[28];

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	mysemp = sem_open(semname, O_CREAT, 0777, 1);

	if (mysemp == SEM_FAILED) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	/* Deallocate mysemp */
	if ((sem_close(mysemp)) == -1) {
		return PTS_UNRESOLVED;
	}

	/* Make mysemp available for reuse */
	mysemp = sem_open(semname, O_CREAT, 0777, 1);

	if (mysemp == SEM_FAILED) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	if ((sem_close(mysemp)) == 0) {
		puts("TEST PASSED");
		sem_unlink(semname);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
