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

 * This stress test aims to test the following assertion:
 *  -> pthread_self() returns the thread ID of the calling thread.
 *      Therefore, it never returns the same value for 2 different running threads.

 * The steps are:
 * -> Create some threads with different parameters
 * -> Get the threads IDs
 * -> Compare all the threads IDs to find duplicates.

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

#include <semaphore.h>
#include <errno.h>
#include <signal.h>

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
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

#include "threads_scenarii.c"

/* This file will define the following objects:
 * scenarii: array of struct __scenario type.
 * NSCENAR : macro giving the total # of scenarii
 * scenar_init(): function to call before use the scenarii array.
 * scenar_fini(): function to call after end of use of the scenarii array.
 */

/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

char do_it = 1;
long long iterations = 0;

/* Handler for user request to terminate */
void sighdl(int sig)
{
	/* do_it = 0 */
	do {
		do_it = 0;
	}
	while (do_it);
}

/* Protect concurrent access to the shared data */
pthread_mutex_t m_synchro = PTHREAD_MUTEX_INITIALIZER;

/* Signaled when all threads are running */
pthread_cond_t c_synchro = PTHREAD_COND_INITIALIZER;
int c_boolean;

/* Thread ID returned by pthread_self  */
pthread_t running[NSCENAR];

/* Thread function */
void *threaded(void *arg)
{
	int ret = 0;
	int me = *(int *)arg;

#if VERBOSE > 6
	output("[child%d] starting\n", me);
#endif
	/* Wait for all threads being created */
	ret = pthread_mutex_lock(&m_synchro);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex lock failed");
	}
#if VERBOSE > 6
	output("[child%d] got mutex\n", me);
#endif

	running[me] = pthread_self();

	/* Signal we're running */
	do {
		ret = sem_post(&scenarii[me].sem);
	}
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1) {
		UNRESOLVED(errno, "Failed to post the semaphore");
	}
#if VERBOSE > 6
	output("[child%d] posted semaphore %p\n", me, &scenarii[me].sem);
#endif

	while (!c_boolean) {
		ret = pthread_cond_wait(&c_synchro, &m_synchro);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to wait the condvar");
		}
#if VERBOSE > 6
		output("[child%d] awaken\n", me);
#endif
	}

	ret = pthread_mutex_unlock(&m_synchro);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex unlock failed");
	}
#if VERBOSE > 6
	output("[child%d] exiting\n", me);
#endif

	return arg;
}

/* Main function */
int main(int argc, char *argv[])
{
	int ret = 0;
	struct sigaction sa;

	pthread_t creation[NSCENAR];	/* Thread ID returned in pthread_create */
	int status[NSCENAR];	/* Status of thread creation */
	int ids[NSCENAR];

	int i;

	for (sc = 0; sc < NSCENAR; sc++)
		ids[sc] = sc;

	/* Initialize output routine */
	output_init();

	/* Initialize thread attribute objects */
	scenar_init();

	/* Register the signal handler for SIGUSR1 */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sighdl;
	if ((ret = sigaction(SIGUSR1, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler");
	}
	if ((ret = sigaction(SIGALRM, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler");
	}
#if VERBOSE > 1
	output("[parent] Signal handler registered\n");
#endif

	while (do_it) {
		/* Initialize the shared data */
		c_boolean = 0;

		/* Create all the threads */
		for (sc = 0; sc < NSCENAR; sc++) {
			/* Create the thread */
			status[sc] =
			    pthread_create(&creation[sc], &scenarii[sc].ta,
					   threaded, &ids[sc]);

			/* Check creation status */
			switch (scenarii[sc].result) {
			case 0:	/* Operation was expected to succeed */
				if (status[sc] != 0) {
					UNRESOLVED(ret,
						   "Failed to create this thread");
				}
				break;

			case 1:	/* Operation was expected to fail */
				if (status[sc] == 0) {
					UNRESOLVED(-1,
						   "An error was expected but the thread creation succeeded");
				}
				break;

			case 2:	/* We did not know the expected result */
			default:
				/* Nothing */
				;
			}
		}
#if VERBOSE > 6
		output("[parent] threads created\n");
#endif

		/* Now wait that all threads are running */
		for (sc = 0; sc < NSCENAR; sc++) {
			if (status[sc] == 0) {	/* The new thread is running */
#if VERBOSE > 6
				output("[parent] Waiting for thread %d: %p\n",
				       sc, &scenarii[sc].sem);
#endif
				do {
					ret = sem_wait(&scenarii[sc].sem);
				}
				while ((ret == -1) && (errno == EINTR));
				if (ret == -1) {
					UNRESOLVED(errno,
						   "Failed to wait for the semaphore");
				}
			}
		}

#if VERBOSE > 6
		output("[parent] Locking the mutex\n");
#endif

		ret = pthread_mutex_lock(&m_synchro);
		if (ret != 0) {
			UNRESOLVED(ret, "Mutex lock failed");
		}

		/* Now, we've got all shared data set, so we can seek for duplicates */
		for (sc = 0; sc < NSCENAR; sc++) {
			if (status[sc] != 0)	/* The new thread is running */
				continue;

			if (pthread_equal(creation[sc], running[sc]) == 0) {
				output("pthread_create returned an ID of %p\n",
				       creation[sc]);
				output
				    ("pthread_self in the thread returned %p\n",
				     running[sc]);
				FAILED("Error: Values mismatch");
			}

			for (i = sc + 1; i < NSCENAR; i++) {
				if (status[i] != 0)
					continue;

				if (pthread_equal(creation[sc], creation[i])) {
					FAILED
					    ("Two different running threads have the same ID");
				}
			}
		}
#if VERBOSE > 6
		output("[parent] No duplicate found\n");
#endif

		/* We're done, we can terminate the threads */
		c_boolean = 1;
		ret = pthread_mutex_unlock(&m_synchro);
		if (ret != 0) {
			UNRESOLVED(ret, "Mutex unlock failed");
		}

		ret = pthread_cond_broadcast(&c_synchro);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to broadcast the cond");
		}
#if VERBOSE > 6
		output("[parent] Cond broadcasted\n");
#endif

		/* Join the joinable threads */
		for (sc = 0; sc < NSCENAR; sc++) {
			if (status[sc] != 0)	/* The new thread is running */
				continue;

			if (scenarii[sc].detached == 0) {
#if VERBOSE > 6
				output("[parent] Joining %d\n", sc);
#endif
				ret = pthread_join(creation[sc], NULL);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Unalbe to join a thread");
				}
#if VERBOSE > 6
				output("[parent] Joined %d\n", sc);
#endif
			}

		}
		iterations++;
	}

	/* We've been asked to stop */

	scenar_fini();

	output("pthread_exit stress test PASSED -- %llu iterations\n",
	       iterations);

	PASSED;
}
