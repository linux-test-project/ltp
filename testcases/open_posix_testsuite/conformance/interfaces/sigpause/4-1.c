/*   
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 This program verifies that sigpause() returns -1 and sets errno to EINVAL
 if passed an invalid signal number.

 Steps:
 1. From the main() function, create a new thread, and using semaphores don't 
    do anything else in the main function until the new thread sets sem to
    INMAIN.
 2. Have the new thread call sigsuspend with an invalid signal number (-1).
 3. Verify the above assertion.
 */

#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define SIGTOTEST SIGABRT
#define INTHREAD 0
#define INMAIN 1

int result = 2;
int sem = INMAIN;

void *a_thread_func()
{
	int return_value = 0;

	return_value = sigpause(-1);
	if (return_value == -1) {
		if (errno == EINVAL) {
			printf ("Test PASSED: sigpause returned -1 and set errno to EINVAL\n");
			result = 0;
		} else {
			printf ("Test FAILED: sigpause did not set errno to EINVAL\n");
			result = 1;
		}
	} else {
		printf ("Test FAILED: sigpause did not return -1\n");
		if (errno == EINVAL) {
			printf ("Test FAILED: sigpause did not set errno to EINVAL\n");
		}
		result = 1;
	}

	sem = INMAIN;

	return NULL;
}

int main()
{
	pthread_t new_th;
	
	sem = INTHREAD;
	if(pthread_create(&new_th, NULL, a_thread_func, NULL) != 0)
	{
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}


	while (sem == INTHREAD)
		sleep(1);

	if(result == 2) {
		return PTS_UNRESOLVED;
	}
	if(result == 1) {
		return PTS_FAIL;
	}

	printf("Test PASSED\n");

	return PTS_PASS;	
}


