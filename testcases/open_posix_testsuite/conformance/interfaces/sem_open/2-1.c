/*
    Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */

/*
   sem_open test case that attempts to open a new semaphore,
   close the semaphore and then open and existing semaphore, which
   should fail when both O_CREAT and O_EXCL name exist during the opening
   of a Semaphore.  Fail to open is a Pass for this test.
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
#define FUNCTION "sem_open"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t *mysemp;
	char semname[50];

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	mysemp = sem_open(semname, O_CREAT, 0777, 0);
	if (mysemp == SEM_FAILED) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	if (sem_close(mysemp) == -1) {
		perror(ERROR_PREFIX "sem_close");
		return PTS_UNRESOLVED;
	}

	mysemp = sem_open(semname, O_CREAT | O_EXCL, 0777, 1);
	if ((mysemp == SEM_FAILED) && (errno == EEXIST)) {
		puts("TEST PASSED");
		sem_unlink(semname);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
