/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  salwan.searty REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 This program verifies that sigpause() returns -1 and sets errno to EINTR
 when it returns.

 Steps:
 1. From the main() function, create a new thread. Give the new thread a
    a second to set up for receiving a signal, and to suspend itself using
    sigpause().
 2. From the main() thread, send signal to new thread to make sigpause return.
 3. Verify that sigpause returns -1 and sets errno to EINTR.
 */


#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define SIGTOTEST SIGABRT
#define INTHREAD 0
#define INMAIN 1

static int result = 2;
static int sem = INTHREAD;

static void handler()
{
	printf("signal was called\n");
	return;
}

static void *a_thread_func()
{
	int return_value = 0;
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTOTEST, &act, 0);
	return_value = sigpause(SIGTOTEST);
	if (return_value == -1) {
		if (errno == EINTR) {
			printf("Test PASSED: sigpause returned -1 "
			       "and set errno to EINTR\n");
			result = 0;
		} else {
			printf("Test FAILED: sigpause did not "
			       "set errno to EINTR\n");
			result = 1;
		}
	} else {
		if (errno == EINTR)
			printf("Test FAILED: sigpause did not return -1\n");

		printf("Test FAILED: sigpause did not set errno to EINTR\n");
		printf("Test FAILED: sigpause did not return -1\n");
		result = 1;

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

	while (sem == INTHREAD)
		sleep(1);

	if (result == 2)
		return PTS_UNRESOLVED;

	if (result == 1)
		return PTS_FAIL;

	printf("Test PASSED\n");
	return PTS_PASS;
}
