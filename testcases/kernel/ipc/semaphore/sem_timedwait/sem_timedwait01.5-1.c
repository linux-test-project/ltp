/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content 
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 */

/*
 * sem_timedwait shall fail when the sem argument doesn't refer to a 
 * valid semaphore.
 */

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "posixtest.h"


#define TEST "5-1"
#define FUNCTION "sem_timedwait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "



int main() {
	sem_t mysemp;
	struct timespec ts;
	int sts;

        if ( sem_init (&mysemp, 0, 1) == -1 ) {
                perror(ERROR_PREFIX "sem_init");
                return PTS_UNRESOLVED;
        }

        if ( sem_destroy (&mysemp) == -1 ) {
                perror(ERROR_PREFIX "sem_destroy");
                return PTS_UNRESOLVED;
        }

	ts.tv_sec=time(NULL)+1;
        ts.tv_nsec=0;

	sts = sem_timedwait(&mysemp, &ts);

	if(( errno == EINVAL ) && ( sts == -1)) {
		puts("TEST PASSED");
		return PTS_PASS;
	} else { 
		puts("TEST FAILED");
		return PTS_FAIL;
	}
}
