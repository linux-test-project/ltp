/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * pthread_barrier_wait()
 *
 * The pthread_barrier_wait( ) function may fail if:
 * [EINVAL] The value specified by barrier does not refer to an initialized barrier object.
 *
 * This case will always pass.
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

int rc;

void sig_handler()
{
	printf("main: blocked on barrier wait with an un-initializied barrier object.\n");
	printf("Test PASSED: Note*: Expected EINVAL when calling this funtion with an un-initialized barrier object, but standard says 'may' fail.\n");
	exit(PTS_PASS);
}

int main()
{
	pthread_barrier_t barrier;
	struct sigaction act;	

	/* Set up main thread to handle SIGALRM */
	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	sigfillset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);

	/* Intialize return code */
	rc = 1;	
	
	/* Call pthread_barrier_wait while refering to an un-initialized barrier object */
	
	/* Just in case we are blocked, send a SIGALRM after 2 sec. */
	alarm(2);
	
	rc = pthread_barrier_wait(&barrier);
	
	if(rc == EINVAL)
	{
		printf("Test PASSED\n");
	}
	else
	{
		printf("return code : %d, %s\n" , rc, strerror(rc));
		printf("Test PASSED: Note*: Expected EINVAL when calling this funtion with an un-initialized barrier object, but standard says 'may' fail.\n");
	} 
	
	return PTS_PASS;
}
