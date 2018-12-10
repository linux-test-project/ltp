/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 * This sample test aims to check the following assertion:
 *
 * pthread_detach() will indicate that the thread resources
 * can be reclaimed as soon as the thread terminates.
 * This means that pthread_join() will fail on such a thread.

 * The steps are:
 *
 * -> create a thread with a joinable state
 * -> detach the thread, either from inside the thread or from outside
 * -> try and join the thread, and check an error is returned.

 * The test fails if pthread_join succeeds.

 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sched.h>
#include <semaphore.h>
#include <errno.h>
#include <assert.h>
/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"
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
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

#include "../testfrmw/threads_scenarii.c"

/* This file will define the following objects:
 * scenarii: array of struct __scenario type.
 * NSCENAR : macro giving the total # of scenarii
 * scenar_init(): function to call before use the scenarii array.
 * scenar_fini(): function to call after end of use of the scenarii array.
 */

/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

void *threaded(void *arg)
{
	int ret = 0;

	if (arg != NULL) {
		ret = pthread_detach(pthread_self());
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to detach the thread");
		}
	}

	/* Post the semaphore to unlock the main thread in case of a detached thread */
	do {
		ret = sem_post(&(scenarii[sc].sem));
	}
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1) {
		UNRESOLVED(errno, "Failed to post the semaphore");
	}

	return arg;
}

int main(void)
{
	int ret = 0;
	pthread_t child;

	output_init();

	scenar_init();

	for (sc = 0; sc < NSCENAR; sc++) {
#if VERBOSE > 0
		output("-----\n");
		output("Starting test with scenario (%i): %s\n", sc,
		       scenarii[sc].descr);
#endif

		if (scenarii[sc].detached != 0) {	/* only joinable threads can be detached */
			ret =
			    pthread_attr_setdetachstate(&scenarii[sc].ta,
							PTHREAD_CREATE_JOINABLE);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to set detachstate back to joinable");
			}
		}

		/* for detached scenarii, we will call pthread_detach from inside the thread.
		   for joinable scenarii, we'll call pthread_detach from this thread. */

		ret =
		    pthread_create(&child, &scenarii[sc].ta, threaded,
				   (scenarii[sc].detached != 0) ? &ret : NULL);
		switch (scenarii[sc].result) {
		case 0:	/* Operation was expected to succeed */
			if (ret != 0) {
				UNRESOLVED(ret, "Failed to create this thread");
			}
			break;

		case 1:	/* Operation was expected to fail */
			if (ret == 0) {
				UNRESOLVED(-1,
					   "An error was expected but the thread creation succeeded");
			}
#if VERBOSE > 0
			break;

		case 2:	/* We did not know the expected result */
		default:
			if (ret == 0) {
				output
				    ("Thread has been created successfully for this scenario\n");
			} else {
				output
				    ("Thread creation failed with the error: %s\n",
				     strerror(ret));
			}
#endif
		}
		if (ret == 0) {	/* The new thread is running */
			/* Just wait for the thread to terminate */
			do {
				ret = sem_wait(&(scenarii[sc].sem));
			}
			while ((ret == -1) && (errno == EINTR));
			if (ret == -1) {
				UNRESOLVED(errno,
					   "Failed to post the semaphore");
			}

			/* If we must detach from here, we do it now. */
			if (scenarii[sc].detached == 0) {
				ret = pthread_detach(child);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Failed to detach the child thread.");
				}
			}

			/* now check that the thread resources are freed. */
			ret = pthread_join(child, NULL);
			if (ret == 0) {
				FAILED
				    ("We were able to join a detached thread.");
			}

			/* Let the thread an additionnal row to cleanup */
			sched_yield();
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
