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

* This sample test aims to check the following assertions:
*
* sigaction returns -1 and errno is set to EINVAL if signal number is invalid
* or an attempt to do an operation which is not allowed is made.

* The steps are:
* -> Try setting a signal handler for signal SIGRTMAX + 1
* -> Try setting a signal handler for SIGKILL
* -> Try setting a signal handler for SIGSTOP
* -> Try ignoring SIGSTOP
* -> Try ignoring SIGKILL

* The test fails if the signals are not delivered in FIFO order.
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

#define SIG_INVALID SIGRTMAX+10

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	return;
}

/* main function */
int main(void)
{
	int ret;

	struct sigaction sa;

	/* Initialize output */
	output_init();

	/* Set the signal handler */
	sa.sa_flags = 0;

	sa.sa_handler = handler;

	ret = sigemptyset(&sa.sa_mask);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to empty signal set");
	}

	/* Install the signal handler for SIGRTMAX */
#if VERBOSE > 0
	output("Trying to catch invalid signal %d\n", SIG_INVALID);

#endif
	ret = sigaction(SIG_INVALID, &sa, 0);

	if (ret == 0) {
		output("Is signal %d valid on this implementation?\n",
		       SIG_INVALID);
		FAILED("Setting handler for invalid signal did not fail");
	}

	if (errno != EINVAL) {
		output("Got error %d (%s) instead of %d (%s)\n",
		       errno, strerror(errno), EINVAL, strerror(EINVAL));
		FAILED("Wrong error code returned");
	}

	/* Install the signal handler for SIGKILL */
#if VERBOSE > 0
	output("Trying to catch unauthorized signal SIGKILL (%d)\n", SIGKILL);

#endif
	ret = sigaction(SIGKILL, &sa, 0);

	if (ret == 0) {
		FAILED("Setting handler for SIGKILL did not fail");
	}

	if (errno != EINVAL) {
		output("Got error %d (%s) instead of %d (%s)\n",
		       errno, strerror(errno), EINVAL, strerror(EINVAL));
		FAILED("Wrong error code returned");
	}

	/* Install the signal handler for SIGSTOP */
#if VERBOSE > 0
	output("Trying to catch unauthorized signal SIGSTOP (%d)\n", SIGSTOP);

#endif
	ret = sigaction(SIGSTOP, &sa, 0);

	if (ret == 0) {
		FAILED("Setting handler for SIGSTOP did not fail");
	}

	if (errno != EINVAL) {
		output("Got error %d (%s) instead of %d (%s)\n",
		       errno, strerror(errno), EINVAL, strerror(EINVAL));
		FAILED("Wrong error code returned");
	}

	sa.sa_handler = SIG_IGN;

	/* Ingrore SIGKILL */
#if VERBOSE > 0
	output("Trying to ignore unauthorized signal SIGKILL (%d)\n", SIGKILL);

#endif
	ret = sigaction(SIGKILL, &sa, 0);

	if (ret == 0) {
		FAILED("Ignoring SIGKILL did not fail");
	}

	if (errno != EINVAL) {
		output("Got error %d (%s) instead of %d (%s)\n",
		       errno, strerror(errno), EINVAL, strerror(EINVAL));
		FAILED("Wrong error code returned");
	}

	/* Ignore SIGSTOP */
#if VERBOSE > 0
	output("Trying to ignore unauthorized signal SIGSTOP (%d)\n", SIGSTOP);

#endif
	ret = sigaction(SIGSTOP, &sa, 0);

	if (ret == 0) {
		FAILED("Ignoring SIGSTOP did not fail");
	}

	if (errno != EINVAL) {
		output("Got error %d (%s) instead of %d (%s)\n",
		       errno, strerror(errno), EINVAL, strerror(EINVAL));
		FAILED("Wrong error code returned");
	}

	/* Test passed */
#if VERBOSE > 0

	output("Test passed\n");

#endif

	PASSED;
}
