/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content 
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 */

/*
 * errno return EINVAL: the semaphore argument doesn't not refer to a valid
 * semaphore.
 */


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"


#define TESTNAME "6-1"
#define FUNCTION "sem_wait"
#define ERROR_PREFIX "unexpected errno: " FUNCTION " " TESTNAME ": "



int main() {
	sem_t *mysemp;

	mysemp = NULL;
	if ( (sem_wait(mysemp) == -1) && ( errno == EINVAL)) { 
		puts("TEST PASSED");
		return PTS_PASS;
	} else {
		puts("TEST FAILED: ERROR CODE is not EINVAL");
		return PTS_FAIL;
	}
}
