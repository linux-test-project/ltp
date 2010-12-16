/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis

* This program is free software; you can redistribute it and/or modify it
* under the terms of version 2 of the GNU General Public License as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it would be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write the Free Software Foundation, Inc., 59
* Temple Place - Suite 330, Boston MA 02111-1307, USA.

* This sample test aims to check the following assertion:
*
* If the thread calling pthread_join is canceled, the joined thread remains joinable.

* The steps are:
* -> create a thread blocked on a mutex.
* -> create another thread which tries and join the first thread.
* -> cancel the 2nd thread.
* -> unblock the semaphore then join the 1st thread

* The test fails if the main thread is unable to join the 1st thread.

*/

/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>

/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
#include "testfrmw.h"
 #include "testfrmw.c"
/* This header is responsible for defining the following macros:
 * UNRESOLVED(ret, descr);
 *    where descr is a description of the error and ret is an int (error code for example)
 * FAILED(descr);
 *    where descr is a short text saying why the test has failed.
 * PASSED();
 *    No parameter.
 *
 * Both three macros shall terminate the calling process.
 * The testcase shall not terminate in any other maneer.
 *
 * The other file defines the functions
 * void output_init()
 * void output(char * string, ...)
 *
 * Those may be used to output information.
 */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************     Helper     *****************************************/
/********************************************************************************************/
#include "threads_scenarii.c"
/* this file defines:
* scenarii: array of struct __scenario type.
* NSCENAR : macro giving the total # of scenarii
* scenar_init(): function to call before use the scenarii array.
* scenar_fini(): function to call after end of use of the scenarii array.
*/

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

/* 1st thread function */
void * threaded (void * arg)
{
	int ret = 0;

	/* Try and lock the mutex, then exit */

	ret = pthread_mutex_lock(&mtx);

	if (ret != 0)
	{
		UNRESOLVED(ret, "Failed to lock mutex");
	}

	ret = pthread_mutex_unlock(&mtx);

	if (ret != 0)
	{
		UNRESOLVED(ret, "Failed to unlock mutex");
	}

	return NULL;
}

/* Canceled thread */
void * joiner_func(void * arg)
{
	(void) pthread_join(*(pthread_t *) arg, NULL);

	FAILED("The joiner thread was not canceled");

	/* please the compiler */
	return NULL;
}

/* The main test function. */
int main(int argc, char *argv[])
{
	int ret = 0;
	pthread_t child;
	pthread_t joiner;

	/* Initialize output routine */
	output_init();

	/* Initialize thread attribute objects */
	scenar_init();

	for (sc = 0; sc < NSCENAR; sc++)
	{
#if VERBOSE > 0
		output("-----\n");
		output("Starting test with scenario (%i): %s\n", sc, scenarii[ sc ].descr);
#endif

		/* Lock the mutex */
		ret = pthread_mutex_lock(&mtx);

		if (ret != 0)
		{
			UNRESOLVED(ret, "failed to lock the mutex");
		}

		ret = pthread_create(&child, &scenarii[ sc ].ta, threaded, NULL);

		switch (scenarii[ sc ].result)
		{
				case 0:                                       /* Operation was expected to succeed */

				if (ret != 0)
				{
					UNRESOLVED(ret, "Failed to create this thread");
				}

				break;

				case 1:                                       /* Operation was expected to fail */

				if (ret == 0)
				{
					UNRESOLVED(-1, "An error was expected but the thread creation succeeded");
				}

				break;

				case 2:                                       /* We did not know the expected result */
				default:
#if VERBOSE > 0

				if (ret == 0)
				{
					output("Thread has been created successfully for this scenario\n");
				}
				else
				{
					output("Thread creation failed with the error: %s\n", strerror(ret));
				}

#endif

		}

		if (ret == 0)                                       /* The new thread is running */
		{

			/* Now create the joiner thread */
			ret = pthread_create(&joiner, NULL, joiner_func, &child);

			if (ret != 0)
			{
				UNRESOLVED(ret, "Failed to create the joiner thread");
			}

			/* Let it enter pthread_join */
			sched_yield();

			/* Cancel the joiner thread */
			ret = pthread_cancel(joiner);

			if (ret != 0)
			{
				UNRESOLVED(ret, "Failed to cancel the thread");
			}

			/* Join the canceled thread */
			ret = pthread_join(joiner, NULL);

			if (ret != 0)
			{
				UNRESOLVED(ret, "Failed to join the canceled thread");
			}

			/* Unblock the child thread */
			ret = pthread_mutex_unlock(&mtx);

			if (ret != 0)
			{
				UNRESOLVED(ret, "Failed to unlock the mutex");
			}

			/* Check the first thread is still joinable */
			ret = pthread_join(child, NULL);

			if (ret != 0)
			{
				output("Error returned: %d\n");
				FAILED("The thread is no more joinable");
			}

		}
		else
		{
			ret = pthread_mutex_unlock(&mtx);

			if (ret != 0)
			{
				UNRESOLVED(ret, "Failed to unlock the mutex");
			}
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