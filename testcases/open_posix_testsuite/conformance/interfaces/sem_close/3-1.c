/*
    Copyright (c) 2002, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */

/*
    sem_close will have no effect on the state of the semaphore if
    sem_unlink has been unsuccessful.
*/

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "3-1"
#define FUNCTION "sem_close"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t *mysemp;
	char semname[28];

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	mysemp = sem_open(semname, O_CREAT, 0444, 1);
	if (mysemp == SEM_FAILED) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	if ((sem_unlink(semname)) == 0) {
		if (sem_close(mysemp) == -1) {
			perror(ERROR_PREFIX "sem_close");
			return PTS_UNRESOLVED;
		}
		puts("TEST PASSED");
		return PTS_PASS;
	} else {

		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
