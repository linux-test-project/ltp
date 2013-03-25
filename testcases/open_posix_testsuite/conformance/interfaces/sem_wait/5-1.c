/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * errno return EAGAIN: The semaphore can't be immediately locked by
 * sem_trywait when its already locked.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "5-1"
#define FUNCTION "sem_trywait"
#define ERROR_PREFIX "unexpected errno: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t *mysemp;
	char semname[28];

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	/* Initial value of Semaphore is 0 Locked */
	mysemp = sem_open(semname, O_CREAT, 0777, 0);
	if (mysemp == SEM_FAILED || mysemp == NULL) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}

	/* Lock Semaphore by sem_trywait */
	if ((sem_trywait(mysemp) == -1) && (errno == EAGAIN)) {
		puts("TEST PASSED");
		sem_close(mysemp);
		sem_unlink(semname);
		return PTS_PASS;
	} else {
		puts("TEST FAILED: ERROR IS NOT EAGAIN");
		return PTS_FAIL;
	}
}
