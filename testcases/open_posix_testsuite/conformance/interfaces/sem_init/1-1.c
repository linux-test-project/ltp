/*
    Copyright (c) 2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */

/*
The following test case initializes an unnamed semaphore with a value of 1,
and then check the value of the semaphore.
*/

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "sem_init"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t mysemp;
	int sts;
	int val;

	sts = sem_init(&mysemp, 0, 1);

	if (sem_getvalue(&mysemp, &val) == -1) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;
	}

	if ((sts == -1) && (val != 1)) {
		puts("TEST FAILED");
		return PTS_FAIL;
	} else {
		puts("TEST PASSED");
		sem_destroy(&mysemp);
		return PTS_PASS;
	}
}
