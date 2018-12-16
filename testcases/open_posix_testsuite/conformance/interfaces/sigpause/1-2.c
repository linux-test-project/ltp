/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program verifies that sigpause() suspends the calling process
 until it receives a signal.

 Steps:
 1. From the main() function, create a new thread. Give the new thread a
    a second to set up for receiving a signal, and to suspend itself using
    sigpause().
 2. For about ten seconds, keep checking from main() that the "returned"
    variable hasn't been set yet. If it has, that means that sigpause
    returned even before a signal was sent to it, thus FAIL the test.
 3. After the ten seconds, send the new thread a signal using pthread_kill,
    and verify that "returned" has now been set to 1, meaning that the
    sigpause returned from suspension.
 */


#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define SIGTOTEST SIGABRT

static int returned;

static void handler()
{
	printf("signal was called\n");
	return;
}

static void *a_thread_func()
{
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTOTEST, &act, 0);
	sigpause(SIGTOTEST);
	returned = 1;
	return NULL;
}

int main(void)
{
	pthread_t new_th;
	int i;

	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < 10; i++) {
		sleep(1);
		if (returned == 1) {
			printf("Test FAILED: sigpause returned before "
			       "it received a signal\n");
			return PTS_FAIL;
		}
	}

	if (pthread_kill(new_th, SIGTOTEST) != 0) {
		printf("Test UNRESOLVED: Couldn't send signal to thread\n");
		return PTS_UNRESOLVED;
	}

	sleep(1);

	if (returned != 1) {
		printf("Test FAILED: signal was sent, but sigpause "
		       "never returned.\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
