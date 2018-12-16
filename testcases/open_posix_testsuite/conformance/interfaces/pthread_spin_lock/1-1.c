/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_spin_lock(pthread_spinlock_t *lock)
 *
 * The function shall lock the spin lock referenced by lock. The calling thread
 * shall acquire the lock if it is not held by another thread. Otherwise, the
 * thread shall spin (that is, shall not return from the pthread_spin_lock())
 * until the lock becomes available.
 *
 * Steps:
 * 1.  Initialize a pthread_spinlock_t object 'spinlock' with
 *     pthread_spin_init()
 * 2.  Main thread lock 'spinlock', should get the lock
 * 3.  Create a child thread. The thread lock 'spinlock', should spin.
 * 4.  After child thread spin for 2 seconds, send SIGALRM to it.
 * 5.  Child thread check its status in the signal handler.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "posixtest.h"

static pthread_spinlock_t spinlock;
static volatile int thread_state;

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3
#define GET_SPIN_LOCK 4

static void sig_handler()
{
	/* Just return */
	pthread_exit(0);
	return;
}

static void *fn_chld(void *arg)
{
	int rc = 0;

	struct sigaction act;
	struct timespec ts;
	thread_state = ENTERED_THREAD;
	int cnt = 0;

	(void) arg;

	/* Unblock the SIGALRM signal for the thread */
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGALRM);
	if (pthread_sigmask(SIG_UNBLOCK, &act.sa_mask, NULL)) {
		perror("thread: could not unblock SIGALRM\n");
		return (void *)PTS_UNRESOLVED;
	}

	/* Set up child thread to handle SIGALRM */
	act.sa_flags = 0;
	act.sa_handler = sig_handler;
	sigfillset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);

	printf("thread: send SIGALRM to me after 2 secs\n");
	alarm(2);

	printf("thread: attempt spin lock\n");
	rc = pthread_spin_lock(&spinlock);
	if (rc != 0) {
		printf
		    ("Test FAILED: thread failed to get spin lock,error code:%d\n",
		     rc);
		pthread_exit((void *)PTS_FAIL);
	}

	printf("thread: acquired spin lock\n");

	thread_state = GET_SPIN_LOCK;
	/* Wait 10 seconds for SIGALRM to be sent */
	while (cnt++ < 10) {
		ts.tv_sec = 1;
		ts.tv_nsec = 0;
		nanosleep(&ts, NULL);
	}

	/* Shouldn't get here.  If we do, it means that SIGALRM wasn't sent/received */
	printf
	    ("Error in thread: SIGALRM was not received/sent correctly, timedout after 10 secs of waiting.\n");
	pthread_exit((void *)PTS_UNRESOLVED);
	return NULL;
}

int main(void)
{
	pthread_t child_thread;
	void *value_ptr;
	struct sigaction sa;

	/* Block the SIGALRM signal for main thread */
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGALRM);
	if (pthread_sigmask(SIG_BLOCK, &sa.sa_mask, NULL)) {
		perror("main: could not block SIGALRM\n");
		return PTS_UNRESOLVED;
	}

	if (pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE) != 0) {
		perror("main: Error at pthread_spin_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt spin lock\n");

	/* We should get the lock */
	if (pthread_spin_lock(&spinlock) != 0) {
		printf
		    ("Test FAILED: main cannot get spin lock when no one owns the lock\n");
		return PTS_FAIL;
	}

	printf("main: acquired spin lock\n");

	thread_state = NOT_CREATED_THREAD;

	printf("main: create thread\n");
	if (pthread_create(&child_thread, NULL, fn_chld, NULL) != 0) {
		printf("main: Error creating child thread\n");
		return PTS_UNRESOLVED;
	}

	/* Wait for thread to end execution */
	if (pthread_join(child_thread, &value_ptr) != 0) {
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* Check the return value of the thread */
	if (thread_state == GET_SPIN_LOCK) {
		printf
		    ("Test FAILED: Child thread did not spin on spin lock when other thread holds the lock\n");
		exit(PTS_FAIL);
	} else if (thread_state == ENTERED_THREAD) {
		printf("thread: spins on spin lock\n");
		printf("Test PASSED\n");
		exit(PTS_PASS);
	} else {
		printf("Unexpected child thread state: %d\n", thread_state);
		exit(PTS_UNRESOLVED);
	}
}
