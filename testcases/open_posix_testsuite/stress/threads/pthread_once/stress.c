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

*  The init_routine from pthread_once never execute
* more or less than once.

* The steps are:
* -> Create several threads
* -> All threads call pthread_once at the same time
* -> Check the init_routine executed once.

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
long long iterations = 0;

/* Handler for user request to terminate */
void sighdl(int sig)
{
	do {
		do_it = 0;
	}
	while (do_it);
}

pthread_once_t once_ctl;
int once_chk;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void init_routine(void)
{
	int ret = 0;
	ret = pthread_mutex_lock(&mtx);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to lock mutex in initializer");
	}

	once_chk++;

	ret = pthread_mutex_unlock(&mtx);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to unlock mutex in initializer");
	}

	return;
}

/* Thread function */
void *threaded(void *arg)
{
	int ret = 0;

	/* Wait for all threads being created */
	ret = pthread_barrier_wait(arg);

	if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		UNRESOLVED(ret, "Barrier wait failed");
	}

	/* Call init routine */
	ret = pthread_once(&once_ctl, init_routine);

	if (ret != 0) {
		UNRESOLVED(ret, "pthread_once failed");
	}

	return NULL;
}

/* Main function */
int main(int argc, char *argv[])
{
	int ret = 0, i;

	struct sigaction sa;

	pthread_barrier_t bar;

	pthread_t th[NTHREADS];

	/* Initialize output routine */
	output_init();

	/* Initialize barrier */
	ret = pthread_barrier_init(&bar, NULL, NTHREADS);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to init barrier");
	}

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
		/* Reinitialize once handler & check value */
		once_ctl = PTHREAD_ONCE_INIT;
		once_chk = 0;

		/* create the threads */

		for (i = 0; i < NTHREADS; i++) {
			ret = pthread_create(&th[i], NULL, threaded, &bar);

			if (ret != 0) {
				UNRESOLVED(ret, "Failed to create a thread");
			}
		}

		/* Then join */
		for (i = 0; i < NTHREADS; i++) {
			ret = pthread_join(th[i], NULL);

			if (ret != 0) {
				UNRESOLVED(ret, "Failed to join a thread");
			}
		}

		/* check the value */
		ret = pthread_mutex_lock(&mtx);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to lock mutex in initializer");
		}

		if (once_chk != 1) {
			output("Control: %d\n", once_chk);
			FAILED("The initializer function did not execute once");
		}

		ret = pthread_mutex_unlock(&mtx);

		if (ret != 0) {
			UNRESOLVED(ret,
				   "Failed to unlock mutex in initializer");
		}

		iterations++;
	}

	/* We've been asked to stop */

	output("pthread_once stress test PASSED -- %llu iterations\n",
	       iterations);

	ret = pthread_barrier_destroy(&bar);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to destroy the barrier");
	}

	PASSED;
}
