/*
    Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */
/*
   sem_open test case attempts to create a named SEM that doesn't exist.
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "6-1"
#define FUNCTION "sem_open"

int main(void)
{
	sem_t *mysemp;
	char semname[50];

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	mysemp = sem_open(semname, 0);

	if ((mysemp == SEM_FAILED) && (errno == ENOENT)) {
		puts("TEST PASSED");
		sem_unlink(semname);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
