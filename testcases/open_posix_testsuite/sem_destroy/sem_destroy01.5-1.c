/*
    Copyright (c) 2003, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content 
    of this license, see the COPYING file at the top level of this 
    source tree.
 */
/*
 * sem_destory test case that attempts to destroy a semaphore that doesn't exist.
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TEST "5-1"
#define FUNCTION "sem_destroy"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "


int main()
{
	sem_t   mysemp;


	if ((sem_destroy(&mysemp)  == -1) && ( errno == EINVAL)) {
		puts("TEST PASSED");
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
