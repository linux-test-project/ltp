/*   
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_attr_setstacksize()
 * 
 * Steps:
 * 1.  Initialize pthread_attr_t object (attr) 
 * 2.  set stacksize to attr
 * 3.  create a thread with the attr
 * 4.  In the created thread, read stackaddr
 *
 * NOTE: pthread_getattr_np is not a POSIX compliant API. It is 
 *       provided by nptl, which is developed by Ulrich Drepper.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif 
#include <pthread.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "2-1"
#define FUNCTION "pthread_attr_setstacksize"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

size_t stack_size;

void *thread_func()
{
	pthread_attr_t attr;
	size_t ssize;
	int rc;

        /* pthread_getattr_np is not POSIX Compliant API*/
	rc = pthread_getattr_np(pthread_self(), &attr);
	if (rc != 0)
    	{
      		perror(ERROR_PREFIX "pthread_getattr_np");
      		exit(PTS_UNRESOLVED);
	}

	pthread_attr_getstacksize(&attr, &ssize);
	if (ssize != stack_size)
	{	
		perror(ERROR_PREFIX "got the wrong stacksize or stackaddr");
		exit(PTS_FAIL);
	}	

	pthread_exit(0);
	return NULL;
}

int main()
{
	pthread_t new_th;
	pthread_attr_t attr;
	size_t ssize;
	void *saddr;
	int rc;

	/* Initialize attr */
	rc = pthread_attr_init(&attr);
	if( rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_init");
		exit(PTS_UNRESOLVED);
	}
	
	stack_size = PTHREAD_STACK_MIN;

	if (posix_memalign (&saddr, sysconf(_SC_PAGE_SIZE), 
            stack_size) != 0)
    	{
      		perror (ERROR_PREFIX "out of memory while "
                        "allocating the stack memory");
      		exit(PTS_UNRESOLVED);
    	}

	rc = pthread_attr_setstacksize(&attr, stack_size);
        if (rc != 0 ) {
                perror(ERROR_PREFIX "pthread_attr_setstacksize");
                exit(PTS_UNRESOLVED);
        }

	rc = pthread_attr_getstacksize(&attr, &ssize);
        if (rc != 0 ) {
                perror(ERROR_PREFIX "pthread_attr_getstacksize");
                exit(PTS_UNRESOLVED);
        }

	rc = pthread_create(&new_th, &attr, thread_func, NULL);
	if (rc !=0 ) {
		perror(ERROR_PREFIX "failed to create a thread");
                exit(PTS_FAIL);
        }

	rc = pthread_join(new_th, NULL);
	if(rc != 0)
        {
                perror(ERROR_PREFIX "pthread_join");
		exit(PTS_UNRESOLVED);
        }

	rc = pthread_attr_destroy(&attr);
	if(rc != 0)
        {
                perror(ERROR_PREFIX "pthread_attr_destroy");
		exit(PTS_UNRESOLVED);
        }
	
	printf("Test PASSED\n");
	return PTS_PASS;
}


