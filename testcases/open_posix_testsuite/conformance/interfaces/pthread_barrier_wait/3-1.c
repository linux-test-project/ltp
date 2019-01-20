/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * pthread_barrier_wait()
 *
 * If a signal is delivered to a thread blocked on a barrier, upon return
 * from the signal handler the thread shall resume waiting at the barrier
 * if the barrier wait has not completed (that is, if the
 * required number of threads have not arrived at the barrier
 * during the execution of the signal handler);
 *
 * Steps:
 * 1. Main initialize barrier with count 2
 * 2. Main create a child thread
 * 3. Child thread call pthread_barrier_wait(), should block
 * 4. While child thread is blocking, send SIGUSR1 to child
 * 5. The signal handler did nothing just print a message
 * 6. After return from the signal handler, child should resume blocking
 * 7. Main call pthread_barrier_wait(), child and main should all return
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
static int sig_rcvd;

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

void sig_handler()
{
	sig_rcvd = 1;
	printf("thread: interrupted by SIGUSR1\n");
}

static void *fn_chld(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int rc = 0;
	struct sigaction act;

	thread_state = ENTERED_THREAD;

	/* Set up thread to handle SIGUSR1 */
	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	sigfillset(&act.sa_mask);
	sigaction(SIGUSR1, &act, 0);

	printf("thread: call barrier wait\n");
	rc = pthread_barrier_wait(&barrier);
	if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
		printf
		    ("Test FAILED: child: pthread_barrier_wait() got unexpected "
		     "return code : %d\n", rc);
		exit(PTS_FAIL);
	} else if (rc == PTHREAD_BARRIER_SERIAL_THREAD)
		printf("thread: get PTHREAD_BARRIER_SERIAL_THREAD\n");

	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	int cnt = 0;
	int rc;
	pthread_t child_thread;
	sig_rcvd = 0;

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

	printf("main: send SIGUSR1 to child thread\n");
	if (pthread_kill(child_thread, SIGUSR1) != 0) {
		printf("main: Error at pthread_kill()\n");
		exit(PTS_UNRESOLVED);
	}

	/* Expect the child to continue blocking */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 2);

	if (sig_rcvd != 1) {
		printf("child did not handle SIGUSR1\n");
		exit(PTS_UNRESOLVED);
	}

	if (thread_state == EXITING_THREAD) {
		/* child thread did not block */
		printf("Test FAILED: child thread should still block on "
		       "pthread_barrier_wait() when interrupted by signal\n");
		exit(PTS_FAIL);
	} else if (thread_state != ENTERED_THREAD) {
		printf("Unexpected thread state: %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}
	printf("main: thread continued blocking after handling SIGUSR1\n");

	printf("main: call barrier wait\n");
	rc = pthread_barrier_wait(&barrier);

	if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
		printf
		    ("Test FAILED: main: pthread_barrier_wait() got unexpected "
		     "return code : %d\n", rc);
		exit(PTS_FAIL);
	} else if (rc == PTHREAD_BARRIER_SERIAL_THREAD)
		printf("main: get PTHREAD_BARRIER_SERIAL_THREAD\n");

	/* We expected the child returned from barrier wait */
	cnt = 0;
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
