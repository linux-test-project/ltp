/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_create() creates a new thread with attributes specified
 * by 'attr', within a process.
 *
 * Steps:
 * 1.  Create a new thread that will go into a never-ending while loop.
 * 2.  If the thread is truly asynchronous, then the main function will
 *     continue instead of waiting for the thread to return (which it never
 *     does in this test case).
 * 3.  An alarm is set to go off (i.e. send the SIGARLM signal) after 3
 *     seconds. This is done for 'timeing-out' reasons, in case main DOES
 *     wait for the thread to return.  This would also mean that the test
 *     failed.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "posixtest.h"

static void *a_thread_function();
static void alarm_handler();

static pthread_t a;

int main(void)
{
	int ret;

	/* Set the action for SIGALRM to generate an error if it is
	 * reached. This is because if SIGALRM was sent, then the
	 * test timed out. */
	if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
		printf("Error in signal()\n");
		return PTS_UNRESOLVED;
	}

	/* SIGALRM will be sent in 5 seconds. */
	alarm(5);

	ret = pthread_create(&a, NULL, a_thread_function, NULL);
	if (ret) {
		fprintf(stderr, "pthread_create(): %s\n", strerror(ret));
		return PTS_UNRESOLVED;
	}

	pthread_cancel(a);

	pthread_join(a, NULL);

	printf("Test PASSED\n");
	return PTS_PASS;
}

/* A never-ending thread function */
static void *a_thread_function()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	while (1)
		sleep(1);

	return NULL;
}

/* If this handler is called, that means that the test has failed. */
static void alarm_handler()
{
	PTS_WRITE_MSG("Test FAILED: Alarm fired while waiting for cancelation\n");
	_exit(PTS_FAIL);
}
