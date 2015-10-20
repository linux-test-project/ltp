/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_create()
 *
 * The signal state of the new thread will be initialized as so:
 *
 * - The signal mask shall be inherited from the created thread
 * - The set of signals pending for the new thread shall be empty.
 *
 * Steps:
 * 1.  In main(), create a signal mask with a few signals in the set (SIGUSR1 and SIGUSR2).
 * 2.  Raise those signals in main.  These signals now should be pending.
 * 3.  Create a thread using pthread_create().
 * 4.  The thread should have the same signal mask, but no signals should be pending.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "posixtest.h"

static sigset_t th_pendingset, th_sigmask;

static void *a_thread_func()
{
	pthread_sigmask(SIG_SETMASK, NULL, &th_sigmask);

	if (sigpending(&th_pendingset) != 0) {
		printf("Error calling sigpending()\n");
		exit(PTS_UNRESOLVED);
	}

	return NULL;
}

int main(void)
{
	pthread_t new_th;
	sigset_t main_sigmask, main_pendingset;
	int ret;

	if ((sigemptyset(&main_sigmask) != 0) ||
	    (sigemptyset(&main_pendingset) != 0)) {
		perror("sigemptyset()");
		return PTS_UNRESOLVED;
	}

	if (sigaddset(&main_sigmask, SIGUSR1) != 0) {
		perror("sigaddset(SIGUSR1)");
		return PTS_UNRESOLVED;
	}

	if (sigaddset(&main_sigmask, SIGUSR2) != 0) {
		perror("sigaddset(SIGUSR2)");
		return PTS_UNRESOLVED;
	}

	ret = pthread_sigmask(SIG_SETMASK, &main_sigmask, NULL);
	if (ret) {
		fprintf(stderr, "pthread_sigmask(): %s\n", strerror(ret));
		return PTS_UNRESOLVED;
	}

	if (raise(SIGUSR1) != 0) {
		printf("Could not raise SIGALRM\n");
		return PTS_UNRESOLVED;
	}
	if (raise(SIGUSR2) != 0) {
		printf("Could not raise SIGALRM\n");
		return PTS_UNRESOLVED;
	}

	ret = pthread_create(&new_th, NULL, a_thread_func, NULL);
	if (ret) {
		fprintf(stderr, "pthread_create(): %s\n", strerror(ret));
		return PTS_UNRESOLVED;
	}

	ret = pthread_join(new_th, NULL);
	if (ret) {
		fprintf(stderr, "pthread_join(): %s\n", strerror(ret));
		return PTS_UNRESOLVED;
	}

	ret = sigismember(&th_sigmask, SIGUSR1);
	if (ret == 0) {
		printf("FAIL: SIGUSR1 not a member of new thread sigmask.\n");
		return PTS_FAIL;
	}
	if (ret != 1) {
		perror("sigismember(sigmask, SIGUSR1)");
		return PTS_UNRESOLVED;
	}

	ret = sigismember(&th_sigmask, SIGUSR2);
	if (ret == 0) {
		printf("FAIL: SIGUSR2 not a member of new thread sigmask.\n");
		return PTS_FAIL;
	}
	if (ret != 1) {
		perror("sigismember(sigmask, SIGUSR2)");
		return PTS_UNRESOLVED;
	}

	ret = sigismember(&th_pendingset, SIGUSR1);
	if (ret == 1) {
		printf("FAIL: SIGUSR1 is member of new thread pendingset.\n");
		return PTS_FAIL;
	}
	if (ret != 0) {
		perror("sigismember(pendingset, SIGUSR1)");
		return PTS_UNRESOLVED;
	}

	ret = sigismember(&th_pendingset, SIGUSR2);
	if (ret == 1) {
		printf("FAIL: SIGUSR1 is member of new thread pendingset.\n");
		return PTS_FAIL;
	}
	if (ret != 0) {
		perror("sigismember(pendingset, SIGUSR2)");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
