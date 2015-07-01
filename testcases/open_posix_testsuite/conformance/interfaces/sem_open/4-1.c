/*
    Copyright (c) 2002-2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */
/*
   open_sem test case that attempts to open a new semaphore,
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

#define TEST "4-1"
#define FUNCTION "sem_open"

int main(void)
{
	sem_t *mysemp;
	char semname[50];

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	/*Trying to open the first Sem */
	mysemp = sem_open(semname, O_CREAT, 0444, 1);
	sem_close(mysemp);

	/* Opening the same existance SEM */
	mysemp = sem_open(semname, O_CREAT | O_EXCL, 0444, 1);

	if ((mysemp == SEM_FAILED) && (errno == EEXIST)) {
		puts("TEST PASSED");
		sem_unlink(semname);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
