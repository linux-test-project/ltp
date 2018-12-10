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
* If Real-Time Signals extension is supported, and several signals in the
* range SIGRTMIN-SIGRTMAX are selected, the lower numbered is returned first.

* The steps are:
* -> mask SIGRTMIN-SIGRTMAX
* -> raise the signals in the range SIGRTMIN-SIGRTMAX
* -> sigwait a signal and check we get always teh lowest-ordered.

* The test fails if we select an unexpected signal.

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

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

/* The main test function. */
int main(void)
{
	int ret, i, sig;
	long rts;
	sigset_t set;

	/* Initialize output */
	output_init();

	/* Test the RTS extension */
	rts = sysconf(_SC_REALTIME_SIGNALS);

	if (rts < 0L) {
		UNTESTED("This test needs the RTS extension");
	}

	/* Set the signal mask */
	ret = sigemptyset(&set);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to empty signal set");
	}

	/* Add all SIGRT signals */
	for (i = SIGRTMIN; i <= SIGRTMAX; i++) {

		ret = sigaddset(&set, i);

		if (ret != 0) {
			UNRESOLVED(ret, "failed to add signal to signal set");
		}
	}

	/* Block all RT signals */
	ret = pthread_sigmask(SIG_BLOCK, &set, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to block RT signals");
	}

	/* raise the signals in no particular order */
	for (i = SIGRTMIN + 1; i <= SIGRTMAX; i += 3) {
		ret = raise(i);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to raise the signal");
		}
	}

	for (i = SIGRTMIN; i <= SIGRTMAX; i += 3) {
		ret = raise(i);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to raise the signal");
		}
	}

	for (i = SIGRTMIN + 2; i <= SIGRTMAX; i += 3) {
		ret = raise(i);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to raise the signal");
		}
	}

	/* All RT signals are pending */

	/* Check the signals are delivered in order */
	for (i = SIGRTMIN; i <= SIGRTMAX; i++) {
		ret = sigwait(&set, &sig);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to sigwait for RT signal");
		}

		if (sig != i) {
			output("SIGRTMIN: %d, SIGRTMAX: %d, i: %d, sig:%d\n",
			       SIGRTMIN, SIGRTMAX, i, sig);
			FAILED("Got wrong signal");
		}
	}

	/* Test passed */
#if VERBOSE > 0
	output("Test passed\n");

#endif
	PASSED;
}
