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

*  Heavy cancelation does not break the system or the user application.

* The steps are:
* Create some threads which:
*  Create a thread.
*  Cancel this thread, as it terminates.
*  Check the return value.

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

#define NTHREADS 30

/********************************************************************************************/
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

char do_it = 1;

/* Handler for user request to terminate */
void sighdl(int sig)
{
	do {
		do_it = 0;
	}
	while (do_it);
}

long long canceled, ended;

/* The canceled thread */
void *th(void *arg)
{
	int ret = 0;
	ret = pthread_barrier_wait(arg);

	if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		UNRESOLVED(ret, "Failed to wait for the barrier");
	}

	return NULL;
}

/* Thread function */
void *threaded(void *arg)
{
	int ret = 0;
	pthread_t child;

	/* Initialize the barrier */
	ret = pthread_barrier_init(arg, NULL, 2);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to initialize a barrier");
	}

	while (do_it) {
		/* Create the thread */
		ret = pthread_create(&child, NULL, th, arg);

		if (ret != 0) {
			UNRESOLVED(ret, "Thread creation failed");
		}

		/* Synchronize */
		ret = pthread_barrier_wait(arg);

		if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
			UNRESOLVED(ret, "Failed to wait for the barrier");
		}

		/* Cancel the thread */
		ret = pthread_cancel(child);

		if (ret == 0)
			canceled++;
		else
			ended++;

		/* Join the thread */
		ret = pthread_join(child, NULL);

		if (ret != 0) {
			UNRESOLVED(ret, "Unable to join the child");
		}

	}

	/* Destroy the barrier */
	ret = pthread_barrier_destroy(arg);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to destroy a barrier");
	}

	return NULL;
}

/* Main function */
int main(int argc, char *argv[])
{
	int ret = 0, i;

	struct sigaction sa;

	pthread_t th[NTHREADS];
	pthread_barrier_t b[NTHREADS];

	/* Initialize output routine */
	output_init();

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

	for (i = 0; i < NTHREADS; i++) {
		ret = pthread_create(&th[i], NULL, threaded, &b[i]);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to create a thread");
		}
	}

#if VERBOSE > 1
	output("[parent] All threads are running\n");

#endif

	/* Then join */
	for (i = 0; i < NTHREADS; i++) {
		ret = pthread_join(th[i], NULL);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to join a thread");
		}
	}

	/* We've been asked to stop */

	output("pthread_cancel stress test PASSED\n");

	output(" - %llu threads canceled\n", canceled);

	output(" - %llu threads ended\n", ended);

	PASSED;
}
