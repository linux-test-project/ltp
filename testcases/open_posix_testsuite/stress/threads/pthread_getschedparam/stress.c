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
* with this program; if not, write the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

* This stress test aims to test the following assertion:

*  pthread_getschedparam() always returns the scheduling parameters of
* the queried thread.

* The steps are:
* -> Create several threads with different scheduling parameters.
* -> create more threads which call continuously the routine, and check
* -> that the correct parameters are always returned.

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
#include <sched.h>

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

typedef struct _tdata {
	int policy;
	int prio;
	pthread_t thread;
} testdata_t;

testdata_t td[4];

/* Thread function */
void *threaded(void *arg)
{
	int ret = 0;
	int i = 0;
	int pol;

	struct sched_param sp;

	while (do_it) {
		for (i = 0; i < 4; i++) {
			ret = pthread_getschedparam(td[i].thread, &pol, &sp);

			if (ret != 0) {
				UNRESOLVED(ret, "Failed to get sched param");
			}

			if (pol != td[i].policy) {
				FAILED("Wrong scheduling policy read");
			}

			if (sp.sched_priority != td[i].prio) {
				FAILED("Wrong scheduling priority read");
			}

		}

		/* We don't really care about concurrent access for this data */
		iterations++;
	}

	return NULL;
}

/* alternative policy threads */
void *rt_thread(void *arg)
{
	int ret = 0;

	/* This thread does almost nothing but wait... */
	ret = pthread_barrier_wait(arg);

	if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		UNRESOLVED(ret, "Failed to wait for barrier");
	}

	return NULL;
}

/* Main function */
int main(int argc, char *argv[])
{
	int ret = 0, i;

	struct sigaction sa;

	pthread_barrier_t bar;

	pthread_attr_t ta[4];

	pthread_t th[NTHREADS];

	struct sched_param sp;

	/* Initialize output routine */
	output_init();

	/* Initialize barrier */
	ret = pthread_barrier_init(&bar, NULL, 5);

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

	td[0].policy = td[1].policy = SCHED_FIFO;

	td[2].policy = td[3].policy = SCHED_RR;

	td[0].prio = sched_get_priority_min(SCHED_FIFO);

	if (td[0].prio == -1) {
		UNRESOLVED(errno, "Failed to get scheduler range value");
	}

	td[1].prio = sched_get_priority_max(SCHED_FIFO);

	if (td[1].prio == -1) {
		UNRESOLVED(errno, "Failed to get scheduler range value");
	}

	td[2].prio = sched_get_priority_min(SCHED_RR);

	if (td[2].prio == -1) {
		UNRESOLVED(errno, "Failed to get scheduler range value");
	}

	td[3].prio = sched_get_priority_max(SCHED_RR);

	if (td[3].prio == -1) {
		UNRESOLVED(errno, "Failed to get scheduler range value");
	}

	/* Initialize the threads attributes and create the RT threads */
	for (i = 0; i < 4; i++) {
		ret = pthread_attr_init(&ta[i]);

		if (ret != 0) {
			UNRESOLVED(ret,
				   "Failed to initialize thread attribute");
		}

		ret =
		    pthread_attr_setinheritsched(&ta[i],
						 PTHREAD_EXPLICIT_SCHED);

		if (ret != 0) {
			UNRESOLVED(ret,
				   "Failed to set explicit scheduling attribute");
		}

		sp.sched_priority = td[i].prio;

		ret = pthread_attr_setschedparam(&ta[i], &sp);

		if (ret != 0) {
			UNRESOLVED(ret,
				   "failed to set thread attribute sched param");
		}

		ret = pthread_attr_setschedpolicy(&ta[i], td[i].policy);

		if (ret != 0) {
			UNRESOLVED(ret,
				   "failed to set thread attribute sched prio");
		}

		ret = pthread_create(&td[i].thread, &ta[i], rt_thread, &bar);

		if (ret != 0) {
			UNRESOLVED(ret,
				   "Failed to create a RT thread -- need more privilege?");
		}

	}

	/* Create the worker threads */
	for (i = 0; i < NTHREADS; i++) {
		ret = pthread_create(&th[i], NULL, threaded, NULL);

		if (ret != 0) {
			UNRESOLVED(ret, "failed to create a worker thread");
		}
	}

	/* Wait for the worker threads to finish */
	for (i = 0; i < NTHREADS; i++) {
		ret = pthread_join(th[i], NULL);

		if (ret != 0) {
			UNRESOLVED(ret, "failed to join a worker thread");
		}
	}

	/* Join the barrier to terminate the RT threads */
	ret = pthread_barrier_wait(&bar);

	if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		UNRESOLVED(ret, "Failed to wait for the barrier");
	}

	/* Join the RT threads */
	for (i = 0; i < 4; i++) {
		ret = pthread_join(td[i].thread, NULL);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to join a thread");
		}
	}

	/* Done! */
	output("pthread_getschedparam stress test PASSED -- %llu iterations\n",
	       iterations);

	ret = pthread_barrier_destroy(&bar);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to destroy the barrier");
	}

	PASSED;
}
