/*
    Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */

/*
   sem_open test case that attempts to open a new semaphoree, and the
   return value is not NULL
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "1-2"
#define FUNCTION "sem_open"

int main(void)
{
	sem_t *mysemp;
	char semname[50];

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	mysemp = sem_open(semname, O_CREAT, 0777, 1);

	/* Checking if mysemp has a value returned. From sem_open */

	if (mysemp != NULL) {
		puts("TEST PASSED");
		sem_close(mysemp);
		sem_unlink(semname);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
