/*
    Copyright (c) 2002, Intel Corporation. All rights reserved.
    Created by:  majid.awad REMOVE-THIS AT intel DOT com
    This file is licensed under the GPL license.  For the full content 
    of this license, see the COPYING file at the top level of this 
    source tree.
 */
/*
   sem_close test case that attempts to close a semaphore that doesn't exist.
   ERROR EINVAL: not a valid semaphore descriptor.
 */

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "posixtest.h"

#define TESTNAME "4-1"
#define FUNCTION "sem_close"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TESTNAME ": "


int main()
{
	sem_t   *mysemp;
	char semname[20];


	mysemp = NULL;
 	sprintf(semname, "/tmp/" FUNCTION "_" TESTNAME "_%d", getpid());	

	sem_close(mysemp);

	if ( errno == EINVAL )  {
		puts("TEST PASSED");
		sem_unlink(semname);
		return PTS_PASS;
	} else {
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
