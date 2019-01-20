/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 * Test pthread_rwlock_rdlock(pthread_rwlock_t * rwlock)
 *
 * If a signal is delivered to a thread waiting for a read-write lock for reading, upon
 * return from the signal handler the thread resumes waiting for the read-write lock for
 * reading as if it was not interrupted.
 *
 * Steps:
 * 1. main thread  create read-write lock 'rwlock', and lock it for writing
 * 2. main thread create a thread sig_thread, the thread is set to handle SIGUSR1
 * 3. sig_thread try to lock 'rwlock' for reading but will block
 * 4. main thread sends SIGUSR1 to sig_thread via pthread_kill
 * 5. test that thread handler is called
 * 6. check that when thread handler returns, sig_thread resumes blocking for rwlock
 * 7. main thread unlock 'rwlock', sig_thread should get the lock
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

static pthread_t sig_thread;
static pthread_rwlock_t rwlock;

static int thread_state;
static int handler_called;

/* thread_state indicates child thread state:
	1: not in child thread yet;
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void sig_handler()
{
	if (pthread_equal(pthread_self(), sig_thread)) {
		printf("sig_handler: handled signal SIGUSR1\n");
		handler_called = 1;
	} else {
		printf("signal is not handled by sig_thread\n");
		exit(PTS_UNRESOLVED);
	}
}

static void *th_fn(void *arg LTP_ATTRIBUTE_UNUSED)
{
	struct sigaction act;
	int rc = 0;

	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	/* Try to block all signals when handling SIGUSR1 */
	sigfillset(&act.sa_mask);
	sigaction(SIGUSR1, &act, 0);

	thread_state = ENTERED_THREAD;
	printf("sig_thread: attemp read lock\n");
	rc = pthread_rwlock_rdlock(&rwlock);
	if (rc != 0) {
		printf
		    ("Test FAILED: sig_thread: Error at pthread_rwlock_rdlock(), Error code=%d\n",
		     rc);
		exit(PTS_FAIL);
	} else
		printf("sig_thread: acquired read lock\n");

	printf("sig_thread: unlock read lock\n");
	if (pthread_rwlock_unlock(&rwlock) != 0) {
		printf("sig_thread: Error release readlock\n");
		exit(PTS_UNRESOLVED);
	}
	thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	int cnt;
	handler_called = 0;

	if (pthread_rwlock_init(&rwlock, NULL) != 0) {
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt write lock\n");
	if (pthread_rwlock_wrlock(&rwlock) != 0) {
		printf("main: Error at pthread_rwlock_wrlock()\n");
		return PTS_UNRESOLVED;
	} else
		printf("main: acquired write lock\n");

	thread_state = NOT_CREATED_THREAD;
	if (pthread_create(&sig_thread, NULL, th_fn, NULL) != 0) {
		printf("main: Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}

	/* wait at most 3 seconds for sig_thread to block */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 3);

	if (thread_state == EXITING_THREAD) {
		printf
		    ("Test FAILED: thread did not block on read lock when a writer holds the lock\n");
		exit(PTS_FAIL);
	} else if (thread_state != ENTERED_THREAD) {
		printf("Unexpected thread state: %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	/* sig_thread is blocking */
	printf("main: fire SIGUSR1 to sig_thread\n");
	if (pthread_kill(sig_thread, SIGUSR1) != 0) {
		printf("main: failed to send SIGUSER to sig_thread\n");
		exit(PTS_UNRESOLVED);
	}

	/* wait at most 3 seconds for the signal to be handled */
	cnt = 0;
	do {
		sleep(1);
	} while (handler_called == 0 && cnt++ < 3);

	if (handler_called != 1) {
		printf("SIGUSR1 was not caught by sig_thread\n");
		exit(PTS_UNRESOLVED);
	}

	/* sig_thread resume to block? */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 3);

	if (thread_state == EXITING_THREAD) {
		printf
		    ("Test FAILED: upon return from signal handler, sig_thread does not resume to block\n");
		exit(PTS_FAIL);
	} else if (thread_state != ENTERED_THREAD) {
		printf("Unexpected thread state: %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	printf
	    ("sig_thread: correctly still blocking after signal handler returns\n");
	printf("main: unlock write lock\n");
	if (pthread_rwlock_unlock(&rwlock) != 0) {
		printf("main: Failed to release write lock\n");
		exit(PTS_UNRESOLVED);
	}

	/* sig_thread got the read lock? */
	cnt = 0;
	do {
		sleep(1);
	} while (thread_state != EXITING_THREAD && cnt++ < 3);

	if (thread_state == ENTERED_THREAD) {
		printf
		    ("Test FAILED: sig_thread blocked on read lock when writer release the lock\n");
		exit(PTS_FAIL);
	} else if (thread_state != EXITING_THREAD) {
		printf("Unexpected thread state: %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}

	if (pthread_join(sig_thread, NULL) != 0) {
		printf("main: failed at pthread_join()\n");
		exit(PTS_UNRESOLVED);
	}

	if (pthread_rwlock_destroy(&rwlock) != 0) {
		printf("main: failed at pthread_rwlock_destroy()\n");
		exit(PTS_UNRESOLVED);
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
