/*   
 * Copyright (c) 2002-3, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * Test that the pthread_kill() function shall return ESRCH when no 
 * thread could be found corresponding to that specified by the given 
 * thread ID.
 *
 * NOTE: Cannot find 6-1.c in PTS cvs. So write this one.
 */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

void * thread_function(void *arg)
{
	/* Does nothing */
	pthread_exit((void*)0);
	
	/* To please some compilers */
	return NULL;
}

int main()
{
	pthread_t child_thread;
	pthread_t invalid_tid;
	
	int rc;

	rc = pthread_create(&child_thread, NULL, 
		thread_function, NULL);
	if (rc != 0)
	{
		printf("Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}
	
	rc = pthread_join(child_thread, NULL);
	if (rc != 0)
	{
		printf("Error at pthread_join()\n");
		return PTS_UNRESOLVED;
	}
		
	/* Now the child_thread exited, it is an invalid tid */
	memcpy(&invalid_tid, &child_thread, 
			sizeof(pthread_t)); 
	
 	if (pthread_kill(invalid_tid, 0) == ESRCH) {
		printf("pthread_kill() returns ESRCH.\n");
		return PTS_PASS;
	}

	printf("Test Fail\n");
	return PTS_FAIL;
}

