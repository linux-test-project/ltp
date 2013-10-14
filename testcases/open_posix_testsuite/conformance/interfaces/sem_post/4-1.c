/*
    Copyright (c) 2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */

/*
 * This test case verifies the sem_post returns 0 on successful call.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "4-1"
#define FUNCTION "sem_post"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t *mysemp;
	char semname[28];

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	/* Initial value of Semaphore is 0 */
	mysemp = sem_open(semname, O_CREAT, 0777, 0);
	if (mysemp == SEM_FAILED || mysemp == NULL) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	if (sem_post(mysemp) == 0) {
		puts("TEST PASSED");
		sem_close(mysemp);
		sem_unlink(semname);
		return PTS_PASS;
	}

	puts("TEST FAILED: value of sem_post is not returning zero");
	return PTS_FAIL;
}
