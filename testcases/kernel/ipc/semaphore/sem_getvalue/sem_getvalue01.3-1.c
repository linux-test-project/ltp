/*
    Copyright (c) 2002, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content 
    of this license, see the COPYING file at the top level of this 
    source tree.
 */

/*
 * This test case will call sem_getvalue on invalid semaphore.
*/


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TESTNAME "3-1"
#define FUNCTION "sem_getvalue"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TESTNAME ": "


int main() {

	char semname[20];
	sem_t *mysemp;
	unsigned int val;

	sprintf(semname, "/tmp/" FUNCTION "_" TESTNAME "_%d", getpid());

	mysemp = sem_open(semname, O_CREAT, 0777, 0);

	if( sem_close(mysemp) == -1 ) {
		perror(ERROR_PREFIX "sem_close");
		return PTS_UNRESOLVED;
	}

	if( sem_unlink(semname) == -1 ) {
		perror(ERROR_PREFIX "sem_unlink");
		return PTS_UNRESOLVED;
	}

	if( sem_getvalue(mysemp, &val) > 0) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;
	}

	if (errno == EINVAL) {
		puts("TEST PASSED");
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}

}
