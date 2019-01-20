/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * pthread_barrier_wait()
 *
 * The pthread_barrier_wait() function shall synchronize participating threads
 * at the barrier referenced by barrier. The calling thread shall block
 * until the required number of threads have called pthread_barrier_wait()
 * specifying the barrier.
 *
 * Steps:
 * 1. Main initialize barrier with count 2
 * 2. Main create a child thread
 * 3. Child thread call pthread_barrier_wait(), should block
 * 4. Main call pthread_barrier_wait(), child and main should all return
 *    from pthread_barrier_wait()
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "posixtest.h"

static pthread_barrier_t barrier;
static int thread_state;
#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void *fn_chld(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int rc = 0;
	thread_state = ENTERED_THREAD;

	printf("child: barrier wait\n");
	rc = pthread_barrier_wait(&barrier);
	if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
		printf
		    ("Test FAILED: child: pthread_barrier_wait() got unexpected "
		     "return code : %d\n", rc);
		exit(PTS_FAIL);
	} else if (rc == PTHREAD_BARRIER_SERIAL_THREAD) {
		printf("child: get PTHREAD_BARRIER_SERIAL_THREAD\n");
	}

	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

void sig_handler()
{
	printf("Interrupted by SIGALRM\n");
	printf("Test FAILED: main blocked on barrier wait\n");
	exit(PTS_FAIL);
}

int main(void)
{
	int cnt = 0;
	int rc;
	pthread_t child_thread;
	struct sigaction act;

	/* Set up main thread to handle SIGALRM */
	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	sigfillset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);

	printf("Initialize barrier with count = 2\n");
	if (pthread_barrier_init(&barrier, NULL, 2) != 0) {
		printf("main: Error at pthread_barrier_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: create child thread\n");
	thread_state = NOT_CREATED_THREAD;
	if (pthread_create(&child_thread, NULL, fn_chld, NULL) != 0) {
		printf("main: Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}

	/* Expect the child to block */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 2);

	if (thread_state == EXITING_THREAD) {
		/* child thread did not block */
		printf("Test FAILED: child thread did not block on "
		       "pthread_barrier_wait()\n");
		exit(PTS_FAIL);
	} else if (thread_state != ENTERED_THREAD) {
		printf("Unexpected thread state: %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	printf("main: call barrier wait\n");

	/* we should not block here, but just in case we do */
	alarm(2);

	rc = pthread_barrier_wait(&barrier);

	if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
		printf
		    ("Test FAILED: main: pthread_barrier_wait() get unexpected "
		     "return code : %d\n", rc);
		exit(PTS_FAIL);
	} else if (rc == PTHREAD_BARRIER_SERIAL_THREAD) {
		printf("main: got PTHREAD_BARRIER_SERIAL_THREAD\n");
	}

	/* We expected the child returned from barrier wait */
	cnt = 1;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 3);

	if (thread_state == ENTERED_THREAD) {
		printf("Test FAILED: child thread still blocked on "
		       "barrier wait\n");
		return PTS_FAIL;
	} else if (thread_state != EXITING_THREAD) {
		printf("main: Unexpected thread state: %d\n", thread_state);
		return PTS_UNRESOLVED;
	}

	if (pthread_join(child_thread, NULL) != 0) {
		printf("main: Error at pthread_join()\n");
		exit(PTS_UNRESOLVED);
	}

	if (pthread_barrier_destroy(&barrier) != 0) {
		printf("Error at pthread_barrier_destroy()");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
