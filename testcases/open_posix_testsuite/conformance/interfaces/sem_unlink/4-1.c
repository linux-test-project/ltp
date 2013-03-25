/*
    Copyright (c) 2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content
    of this license, see the COPYING file at the top level of this
    source tree.
 */

/*
 * Trying to unlink a semaphore which it doesn't exist.  It give an ERROR:
 * ENOENT.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "4-1"
#define FUNCTION "sem_unlink"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{

	char semname[28];

	sem_unlink(semname);

	if (errno == ENOENT) {
		puts("TEST PASSED");
		return PTS_PASS;
	} else {
		puts("TEST FAILED: semaphore does exist");
		return PTS_FAIL;
	}
}
