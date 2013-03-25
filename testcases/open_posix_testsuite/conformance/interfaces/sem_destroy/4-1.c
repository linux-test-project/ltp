/*
    Copyright (c) 2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */

/*
 * This test case verify the unamed semaphore is destroyed by calling
 * sem_destroy to return 0 on successful call.
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
#define FUNCTION "sem_destroy"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t mysemp;

	if (sem_init(&mysemp, 0, 0) == -1) {
		perror(ERROR_PREFIX "sem_init");
		return PTS_UNRESOLVED;
	}

	if (sem_destroy(&mysemp) == 0) {
		puts("TEST PASSED");
		return PTS_PASS;
	} else {
		puts("TEST FAILED: couldn't destroy sempahore.");
		return PTS_FAIL;
	}
}
