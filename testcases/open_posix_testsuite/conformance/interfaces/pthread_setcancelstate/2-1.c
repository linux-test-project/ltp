/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test pthread_cancelstate
 * The cancelability state of a newly created thread is PTHREAD_CANCEL_ENABLE.
 *
 * STEPS:
 * 1. Create a thread.
 * 2. In the thread function, set cancel_flag to 1.
 * 3. Send out a thread cancel request to the new thread
 * 4. If the cancel request was honored, the cancel_flag will remain 1.
 * 5. If not, the thread will continue until the end of execution, cancel_flag will be set to -1
 *    and,therefore failing the test.
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define INTHREAD 0		/* Control going to or is already for Thread */
#define INMAIN 1		/* Control going to or is already for Main */

int sem1;			/* Manual semaphore */
int cancel_flag;

/* Function that the thread executes upon its creation */
void *a_thread_func()
{
	/* Set default cancel state should be ENABLE, meaning it should honor all
	 * cancel requests. */
	/* Set cancel type to ASYNCHRONOUS so that it honors cancel requests immediately. */
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	cancel_flag = 1;

	/* Indicate to main() that the thread has been created. */
	sem1 = INMAIN;

	/* Wait until main() has sent out a cancel request, meaning until it
	 * sets sem1==INTHREAD. */
	while (sem1 == INMAIN)
		sleep(1);

	/* If the thread correctly honors the cancel request, then the cancel_flag will
	 * remain 1.  If it contiues on with the thread execution, then the cancel_flag
	 * will be -1, and therefore failing this test. */
	pthread_testcancel();

	/* Should not reach here if the thread correctly honors the cancel request. */
	cancel_flag = -1;
	pthread_exit(0);
	return NULL;
}

int main(void)
{
	pthread_t new_th;

	/* Initializing values */
	sem1 = INTHREAD;
	cancel_flag = 0;

	/* Create a new thread. */
	if (pthread_create(&new_th, NULL, a_thread_func, NULL) != 0) {
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}

	/* Make sure thread is created before we cancel it. (wait for
	 * a_thread_func() to set sem1=INMAIN.) */
	while (sem1 == INTHREAD)
		sleep(1);

	if (pthread_cancel(new_th) != 0) {
		perror("Error sending cancel request\n");
		return PTS_UNRESOLVED;
	}

	/* Indicate to the thread function that the thread cancel request
	 * has been sent to it. */
	sem1 = INTHREAD;

	/* Wait for thread to end execution. */
	if (pthread_join(new_th, NULL) != 0) {
		perror("Error in pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	/* This means that the cancel request was ignored rather than honored, and
	 * the test fails. */
	if (cancel_flag < 0) {
		printf
		    ("Test FAILED: Thread default cancel type is not PTHREAD_CANCEL_ENABLE, it did not honor cancel request\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
