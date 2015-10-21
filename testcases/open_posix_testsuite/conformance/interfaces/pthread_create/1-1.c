/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_create() creates a new thread with attributes specified
 * by 'attr', within a process.
 *
 * Steps:
 * 1.  Create a thread using pthread_create()
 * 2.  Compare the thread ID of 'main' to the thread ID of the newly created
 *     thread. They should be different but process IDs are the same.
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

static int pid;

static void *a_thread_func()
{
	pid = getpid();

	return NULL;
}

int main(void)
{
	pthread_t main_th, new_th;
	int ret, ppid;

	ret = pthread_create(&new_th, NULL, a_thread_func, NULL);
	if (ret) {
		fprintf(stderr, "pthread_create(): %s\n", strerror(ret));
		return PTS_UNRESOLVED;
	}

	main_th = pthread_self();

	if (pthread_equal(new_th, main_th) != 0) {
		printf("Test FAILED: A new thread wasn't created\n");
		return PTS_FAIL;
	}

	ret = pthread_join(new_th, NULL);
	if (ret) {
		fprintf(stderr, "pthread_join(): %s\n", strerror(ret));
		return PTS_UNRESOLVED;
	}

	ppid = getpid();

	if (pid != ppid) {
		printf("Test FAILED: Pids are different %i != %i\n", pid, ppid);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
