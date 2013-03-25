/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test that pthread_join()
 *
 * When pthread_join() returns successfully, the target thread has been
 * terminated.
 *
 * Steps:
 * 1.  Create a new thread.
 * 2.  Send a cancel request to it from main, then use pthread_join to
 *     wait for it to end.
 * 3.  The thread will sleep for 3 seconds, then call test_cancel() to
 *     cancel execution.
 * 4.  When this happens, the cleanup handler should be called.
 * 5.  Main will test that when pthread_join allows main to continue
 *     with the process that the thread has ended execution.  If the
 *     cleanup_handler was not called, then the test fails.
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define TIMEOUT 10
static int cleanup_flag;

/*
 * Cleanup function that the thread executes when it is canceled.  So if
 * cleanup_flag is 1, it means that the thread was canceled.
 */
static void a_cleanup_func()
{
	cleanup_flag = 1;
}

static void *a_thread_func()
{
	int err;

	err = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	if (err != 0) {
		fprintf(stderr, "pthread_setcancelstate: %s", strerror(err));
		goto thread_exit_failure;
	}

	err = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	if (err != 0) {
		fprintf(stderr, "pthread_setcanceltype: %s", strerror(err));
		goto thread_exit_failure;
	}

	/* Set up the cleanup handler */
	pthread_cleanup_push(a_cleanup_func, NULL);

	/* Wait for a timeout period for the cancel request to be sent. */
	sleep(TIMEOUT);

	/* Should not get here, but just in case pthread_testcancel() didn't
	 * work -- the cancel request timed out. */
	pthread_cleanup_pop(0);

thread_exit_failure:
	cleanup_flag = -1;

	return NULL;
}

int main(void)
{
	pthread_t new_th;

	int err;

	err = pthread_create(&new_th, NULL, a_thread_func, NULL);
	if (err != 0) {
		fprintf(stderr, "pthread_create: %s\n", strerror(err));
		return PTS_UNRESOLVED;
	}

	/* Remove potential for race */
	sleep(TIMEOUT / 2);

	err = pthread_cancel(new_th);
	if (err != 0) {
		fprintf(stderr, "pthread_cancel: %s\n", strerror(err));
		return PTS_UNRESOLVED;
	}

	err = pthread_join(new_th, NULL);
	if (err != 0) {
		fprintf(stderr, "pthread_join: %s\n", strerror(err));
		return PTS_UNRESOLVED;
	}

	if (cleanup_flag == 0) {
		printf("Test FAILED: Thread did not finish execution when "
		       "pthread_join returned.\n");
		return PTS_FAIL;
	}

	if (cleanup_flag == -1) {
		printf("Error in pthread_testcancel.  Cancel request timed "
		       "out.\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
