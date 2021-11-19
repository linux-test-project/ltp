/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
* This sample test aims to check the following assertion:
*
* If the thread calling pthread_join is canceled, the joined thread
* remains joinable.
*
* The steps are:
* -> create a thread blocked on a mutex.
* -> create another thread which tries and join the first thread.
* -> cancel the 2nd thread.
* -> unblock the semaphore then join the 1st thread
* The test fails if the main thread is unable to join the 1st thread.
*/


#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"
#ifndef VERBOSE
#define VERBOSE 1
#endif

#include "../testfrmw/threads_scenarii.c"

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static unsigned int sc;

/* 1st thread function */
static void *threaded(void *arg PTS_ATTRIBUTE_UNUSED)
{
	int ret = 0;

	/* Try and lock the mutex, then exit */
	ret = pthread_mutex_lock(&mtx);
	if (ret != 0)
		UNRESOLVED(ret, "Failed to lock mutex");

	ret = pthread_mutex_unlock(&mtx);
	if (ret != 0)
		UNRESOLVED(ret, "Failed to unlock mutex");

	return NULL;
}

/* Canceled thread */
static void *joiner_func(void *arg)
{
	(void)pthread_join(*(pthread_t *) arg, NULL);

	FAILED("The joiner thread was not canceled");

	/* please the compiler */
	return NULL;
}

/* The main test function. */
int main(void)
{
	int ret = 0;
	pthread_t child;
	pthread_t joiner;

	/* Initialize output routine */
	output_init();

	/* Initialize thread attribute objects */
	scenar_init();

	for (sc = 0; sc < NSCENAR; sc++) {
		if (scenarii[sc].detached == 1)
			continue;

#if VERBOSE > 0
		output("-----\n");
		output("Starting test with scenario (%i): %s\n",
		       sc, scenarii[sc].descr);
#endif

		/* Lock the mutex */
		ret = pthread_mutex_lock(&mtx);
		if (ret != 0)
			UNRESOLVED(ret, "failed to lock the mutex");

		ret = pthread_create(&child, &scenarii[sc].ta, threaded, NULL);

		switch (scenarii[sc].result) {
			/* Operation was expected to succeed */
		case 0:

			if (ret != 0)
				UNRESOLVED(ret, "Failed to create this thread");

			break;
			/* Operation was expected to fail */
		case 1:

			if (ret == 0)
				UNRESOLVED(-1, "An error was expected "
					   "but the thread creation succeeded");

			break;
			/* We did not know the expected result */
		case 2:
		default:
#if VERBOSE > 0

			if (ret == 0)
				output("Thread has been created "
				       "successfully for this scenario\n");
			else
				output("Thread creation failed with the error: "
				       "%s\n", strerror(ret));

#endif

		}
		/* The new thread is running */
		if (ret == 0) {
			/* Now create the joiner thread */
			ret = pthread_create(&joiner, NULL,
					     joiner_func, &child);

			if (ret != 0)
				UNRESOLVED(ret, "Failed to create the "
					   "joiner thread");

			/* Let it enter pthread_join */
			sched_yield();

			/* Cancel the joiner thread */
			ret = pthread_cancel(joiner);
			if (ret != 0)
				UNRESOLVED(ret, "Failed to cancel the thread");

			/* Join the canceled thread */
			ret = pthread_join(joiner, NULL);
			if (ret != 0)
				UNRESOLVED(ret, "Failed to join the "
					   "canceled thread");

			/* Unblock the child thread */
			ret = pthread_mutex_unlock(&mtx);
			if (ret != 0)
				UNRESOLVED(ret, "Failed to unlock the mutex");

			/* Check the first thread is still joinable */
			ret = pthread_join(child, NULL);
			if (ret != 0) {
				output("Error returned: %d\n");
				FAILED("The thread is no more joinable");
			}

		} else {
			ret = pthread_mutex_unlock(&mtx);
			if (ret != 0)
				UNRESOLVED(ret, "Failed to unlock the mutex");
		}
	}

	scenar_fini();
#if VERBOSE > 0
	output("-----\n");
	output("All test data destroyed\n");
	output("Test PASSED\n");
#endif

	PASSED;
}
