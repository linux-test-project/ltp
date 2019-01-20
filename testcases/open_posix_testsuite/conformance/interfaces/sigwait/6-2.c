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
* If several threads are waiting for a signal and this signal is generated
* for a specific thread, only this thread is unblocked.

* The steps are:
* -> mask SIGUSR1
* -> create several threads which sigwait for SIGUSR1
* -> pthread_kill one of the threads
* -> Check than this thread has been awaken.

* The test fails if the thread is not awaken.

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
pthread_t last_awaken;
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

	last_awaken = pthread_self();

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
	ret = pthread_kill(ch[0], SIGUSR1);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to raise the signal");
	}

	sleep(1);

	if (n_awaken != 1) {
		output("%d threads were awaken\n", n_awaken);
		FAILED("Unexpected number of threads awaken");
	}

	if (!pthread_equal(last_awaken, ch[0])) {
		FAILED("The awaken thread is not the signal target one.");
	}

	/* Wake other threads */
	for (i = 1; i < NTHREADS; i++) {
		ret = pthread_kill(ch[i], SIGUSR1);

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
