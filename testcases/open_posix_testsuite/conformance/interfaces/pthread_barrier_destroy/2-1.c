/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2017, Richard Palethorpe <rpalethorpe@suse.com>
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * pthread_barrier_destroy()
 *
 * Attempt to destroy a barrier which has a waiter. The standard recommends
 * that EBUSY is returned, but the required behaviour is undefined. At the
 * time of writing, glibc blocks until there are no threads waiting on the
 * barrier.
 *
 * Hypothetically, on some implementations, the child could segfault or
 * 'barrier_wait() could return success or EINVAL when 'barrier_destroy() is
 * called. It is difficult to determine whether 'barrier_wait() has returned
 * due to 'barrier_destroy() or some other reason. So we just report what has
 * happened. Obviously a segfault will cause the test to crash, which is
 * highly undesirable behaviour regardless of whether it is POSIX compliant.
 *
 * Also see pthread_cond_destroy 4-1.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include "posixtest.h"

static pthread_barrier_t barrier;
static volatile int thread_state;
#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3
#define WAIT_LOOP 0xFFFFFF

static void *fn_chld(void *arg)
{
	int rc = 0;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	printf("child: barrier wait\n");
	thread_state = ENTERED_THREAD;
	rc = pthread_barrier_wait(&barrier);
	if (rc == PTHREAD_BARRIER_SERIAL_THREAD)
		printf("child: got PTHREAD_BARRIER_SERIAL_THREAD\n");
	else if (rc != 0)
		perror("child: pthread_barrier_wait");
	else
		printf("child: pthread_barrier_wait returned success");

	thread_state = EXITING_THREAD;
	return arg;
}

static void *watchdog(void *arg)
{
	int rc = 0;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	sleep(1);
	printf("watchdog: It appears pthread_barrier_destroy() is blocking\n");
	rc = pthread_barrier_wait(&barrier);
	if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
		perror("watchdog: pthread_barrier_wait");
		exit(PTS_UNRESOLVED);
	}

	return arg;
}

int main(void)
{
	int cnt = 0;
	int rc;
	pthread_t child_thread, watchdog_thread;

	printf("main: Initialize barrier with count = 2\n");
	if (pthread_barrier_init(&barrier, NULL, 2) != 0) {
		perror("main: pthread_barrier_init");
		return PTS_UNRESOLVED;
	}

	printf("main: create child thread\n");
	thread_state = NOT_CREATED_THREAD;
	if (pthread_create(&child_thread, NULL, fn_chld, NULL) != 0) {
		perror("main: pthread_create");
		return PTS_UNRESOLVED;
	}

	printf("main: create watchdog thread\n");
	if (pthread_create(&watchdog_thread, NULL, watchdog, NULL) != 0) {
		perror("main: Error at pthread_create");
		return PTS_UNRESOLVED;
	}

	cnt = 0;
	for (cnt = 0; thread_state < ENTERED_THREAD && cnt < WAIT_LOOP; cnt++)
		sched_yield();
	/* Yield once more to increase the probability that the child thread
	 * will call pthread_barrier_wait() before this thread reaches
	 * pthread_barrier_destroy().
	 */
	sched_yield();

	if (thread_state == EXITING_THREAD) {
		printf("Test FAILED: child thread did not block on pthread_barrier_wait()\n");
		exit(PTS_FAIL);
	} else if (thread_state == NOT_CREATED_THREAD) {
		printf("Child thread did not start (quick enough)\n");
		exit(PTS_UNRESOLVED);
	}

	printf("main: destroy barrier while child is waiting\n");
	rc = pthread_barrier_destroy(&barrier);

	if (rc != EBUSY) {
		printf("UNSUPPORTED: POSIX recommends returning EBUSY, but got: %d, %s\n",
		       rc, strerror(rc));
		rc = PTS_UNSUPPORTED;
	} else {
		printf("Test PASSED\n");
		rc = PTS_PASS;
	}

	pthread_cancel(watchdog_thread);
	pthread_cancel(child_thread);

	return rc;
}
