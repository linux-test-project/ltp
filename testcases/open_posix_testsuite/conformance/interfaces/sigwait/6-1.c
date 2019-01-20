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

* This sample test aims to check the following assertion:
*
* If several threads are blocked in a call to sigwait,
* only one is unblocked when the signal becomes pending.

* The steps are:
* -> mask SIGUSR1
* -> create several threads which sigwait for SIGUSR1
* -> raise SIGUSR1
* -> Check than only 1 thread has been awaken.

* The test fails if less or more than 1 thread returns from sigwait when the
* signal is raised.

*/


/******************************************************************************/
/*************************** standard includes ********************************/
/******************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <signal.h>
#include <errno.h>

/******************************************************************************/
/***************************   Test framework   *******************************/
/******************************************************************************/
#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"
/* This header is responsible for defining the following macros:
 * UNRESOLVED(ret, descr);
 *    where descr is a description of the error and ret is an int
 *   (error code for example)
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

/******************************************************************************/
/**************************** Configuration ***********************************/
/******************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

#define NTHREADS 5

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

int n_awaken = 0;
sigset_t setusr;

/* Thread function */
void *threaded(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int ret;
	int sig;

	/* The signal is already masked, because inherited from the parent */

	/* wait for the signal */
	ret = sigwait(&setusr, &sig);

	if (ret != 0) {
		UNRESOLVED(ret, "failed to wait for signal in thread");
	}

	n_awaken++;

	/* quit */
	return NULL;
}

/* The main test function. */
int main(void)
{
	int ret, i;
	pthread_t ch[NTHREADS];

	/* Initialize output */
	output_init();

	/* Set the signal mask */
	ret = sigemptyset(&setusr);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to empty signal set");
	}

	ret = sigaddset(&setusr, SIGUSR1);

	if (ret != 0) {
		UNRESOLVED(ret, "failed to add SIGUSR1 to signal set");
	}

	ret = pthread_sigmask(SIG_BLOCK, &setusr, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to block SIGUSR1");
	}

	/* Create the children */

	for (i = 0; i < NTHREADS; i++) {
		ret = pthread_create(&ch[i], NULL, threaded, NULL);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to create a thread");
		}
	}

	/* raise the signal */
	ret = kill(getpid(), SIGUSR1);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to raise the signal");
	}

	sleep(1);

	if (n_awaken != 1) {
		output("%d threads were awaken\n", n_awaken);
		FAILED("Unexpected number of threads awaken");
	}

	/* Wake other threads */
	for (; n_awaken < NTHREADS; sched_yield()) {
		ret = kill(getpid(), SIGUSR1);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to raise the signal");
		}
	}

	/* Wait for child thread termination */
	for (i = 0; i < NTHREADS; i++) {
		ret = pthread_join(ch[i], NULL);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to join the thread");
		}
	}

	/* Test passed */
#if VERBOSE > 0

	output("Test passed\n");

#endif

	PASSED;
}
