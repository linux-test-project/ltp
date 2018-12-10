/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program verifies that sigpause() restores sig to the signal mask before
 returning.

 Steps:
 1. From the main() function, create a new thread. Give the new thread a
    a second to set up for receiving a signal, add SIGTOTEST to its signal
    mask and to suspend itself using sigpause(SIGTOTEST).
 2. Have main() send the signal indicated by SIGTOTEST to the new thread,
    using pthread_kill(), and using the concept of semaphores, have the main()
 3. Once the new thread returns from sigpause, have the new thread raise
    SIGTOTEST. At this point, SIGTOTEST should be restored to the signal mask,
    so the signal handler should not be called yet, and the signal should be
    pending.
    If it is not, set the variable return_value to 1, indicating a test failure.
 4. Now, from the new thread, set sem back to INMAIN to allow main to continue
    running.
 5. The PTS exit code that main() will return with will depend on the value of
    return_value:
	PTS_UNRESOLVED if return value is 2
	PTS_PASS if return value is 0
	PTS_FAIL if return value is 1
 */


#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define SIGTOTEST SIGABRT
#define INMAIN 0
#define INTHREAD 1

static int handler_called;
static int return_value = 2;
static int sem = INMAIN;

static void handler()
{
	printf("signal was called\n");
	handler_called = 1;
	return;
}

static void *a_thread_func()
{
	struct sigaction act;
	sigset_t pendingset;

	act.sa_flags = 0;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTOTEST, &act, 0);
	sighold(SIGTOTEST);

	if ((sigpause(SIGTOTEST) != -1) || (errno != EINTR)) {
		printf("Test UNRESOLVED: sigpause didn't return -1 "
		       "and/or didn't set errno correctly.");
		return_value = 2;
		return NULL;
	}

	sleep(1);

	raise(SIGTOTEST);
	sigpending(&pendingset);
	if (sigismember(&pendingset, SIGTOTEST) == 1) {
		printf("Test PASSED: signal mask was restored when "
		       "sigpause returned.");
		return_value = 0;
	}

	sem = INMAIN;
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

	sem = INTHREAD;
	while (sem == INTHREAD)
		sleep(1);

	if (handler_called != 1) {
		printf("Test UNRESOLVED: signal wasn't removed from "
		       "signal mask\n");
		return PTS_UNRESOLVED;
	}

	if (return_value != 0) {
		if (return_value == 1)
			return PTS_FAIL;
		if (return_value == 2)
			return PTS_UNRESOLVED;
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
