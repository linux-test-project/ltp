/*   
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_getcpuclockid()
 * 
 * Steps:
 * 	1. Create a thread
 *	2. Call the API to get the clockid in the created thread
 */

#include <pthread.h>
#include <time.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "pthread_getcpuclockid"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

void *thread_func()
{
	int rc;
	clockid_t cid;

	rc = pthread_getcpuclockid(pthread_self(), &cid); 
        if (rc !=0 ) {
                perror(ERROR_PREFIX "pthread_getcpuclockid");
                exit(PTS_FAIL);
        }
	printf("clock id of new thread is %d\n", cid);

	pthread_exit(0);
	return NULL;
}

int main()
{
	int rc;
	pthread_t new_th;

	rc = pthread_create(&new_th, NULL, thread_func, NULL);
        if (rc !=0 ) {
                perror(ERROR_PREFIX "failed to create a thread");
                exit(PTS_UNRESOLVED);
        }

	rc = pthread_join(new_th, NULL);
        if(rc != 0)
        {
                perror(ERROR_PREFIX "pthread_join");
                exit(PTS_UNRESOLVED);
        }

	return PTS_PASS;
}


