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
* pthread_kill() sends the specified signal to the specified thread.

* The steps are:
* -> set a signal handler
* -> Create a new thread
* -> pthread_kill this thread from the main thread
* -> Wait that the signal handler is called
* -> Check that the correct signal is received in the correct thread

* The test fails if a bad signal is received or bad thread receives it.

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

static volatile int handler_called;
pthread_t ch;

/* Signal handler */
void handler(int sig)
{
	handler_called = sig;

	if (!pthread_equal(pthread_self(), ch)) {
		FAILED
		    ("The signal handler was not trigged in the killed thread");
	}
}

/* Thread function */
void *threaded(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int rebours = 3;

	/* sleep up to 3 seconds */

	while ((!handler_called) && (rebours--))
		sleep(1);

	/* quit */
	return NULL;
}

/* The main test function. */
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

	ret = sigaction(SIGUSR2, &sa, 0);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to set signal handler");
	}

	/* Create the child */
	ret = pthread_create(&ch, NULL, threaded, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to create a thread");
	}

	/* kill the child thread */
	ret = pthread_kill(ch, SIGUSR2);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to kill child thread");
	}

	/* Wait for child thread termination */
	ret = pthread_join(ch, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join the thread");
	}

	/* Check if handler has been trigged inside the child */
	if (handler_called != SIGUSR2) {
		FAILED("Wrong signal received in thread");
	}

	/* Test passed */
#if VERBOSE > 0

	output("Test passed\n");

#endif

	PASSED;
}
