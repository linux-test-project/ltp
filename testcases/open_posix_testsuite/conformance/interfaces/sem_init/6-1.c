/*
    Copyright (c) 2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */
/* sem_init shall fail if the valueargument exceeds SEM_VALUE_MAX.
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"
#include <limits.h>

#define TEST "6-1"
#define FUNCTION "sem_init"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	sem_t mysemp;
	int counter = SEM_VALUE_MAX;

	if (SEM_VALUE_MAX >= INT_MAX) {
		puts("Test skipped");
		return PTS_PASS;
	}

	++counter;
	sem_init(&mysemp, 0, counter);

	if (errno == EINVAL) {
		puts("TEST PASSED");
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
