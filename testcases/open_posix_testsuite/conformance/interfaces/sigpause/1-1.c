/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program verifies that sigpause() removes sig from the signal mask.

 Steps:
 1. From the main() function, create a new thread. Give the new thread a
    a second to set up for receiving a signal, and to suspend itself using
    sigpause().
 2. Have main() send the signal indicated by SIGTOTEST to the new thread,
    using pthread_kill(). After doing this, give the new thread a second
    to get to the signal handler.
 3. In the main() thread, if the handler_called variable wasn't set to 1,
    then the test has failed, else it passed.
 */


#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define SIGTOTEST SIGABRT

static int handler_called;

static void handler()
{
	printf("signal was called\n");
	handler_called = 1;
	return;
}

static void *a_thread_func()
{
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTOTEST, &act, 0);
	sighold(SIGTOTEST);
	sigpause(SIGTOTEST);
	return NULL;
}

int main(void)
{
	pthread_t new_th;

	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	sleep(1);

	if (pthread_kill(new_th, SIGTOTEST) != 0) {
		printf("Test UNRESOLVED: Couldn't send signal to thread\n");
		return PTS_UNRESOLVED;
	}

	sleep(1);

	if (handler_called != 1) {
		printf("Test FAILED: signal wasn't removed from signal mask\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
