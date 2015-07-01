/*
    Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */
/*
   open_sem test case that attempts to open a new semaphore,
   with the maximum VALUE allowed.
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include "posixtest.h"

#define TEST "5-1"
#define FUNCTION "sem_open"

int main(void)
{
	sem_t *mysemp;
	char semname[50];
	int counter = SEM_VALUE_MAX;

	if (counter >= INT_MAX) {
		return PTS_PASS;
	}

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	++counter;
	mysemp = sem_open(semname, O_CREAT, 0444, counter);
	if ((mysemp == SEM_FAILED) && (errno == EINVAL)) {
		puts("TEST PASSED");
		sem_unlink(semname);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
