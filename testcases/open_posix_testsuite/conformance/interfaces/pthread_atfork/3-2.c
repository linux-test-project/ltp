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
* If there is not enough memory to store handler adresses,
* the function returns ENOMEM.

* The steps are:
* -> Register up to 10000 handlers.
* -> In case of failure, check the error code is ENOMEM.
* -> Otherwise, check the handlers are all executed..

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

#include <sys/wait.h>
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

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

int controls[3] = { 0, 0, 0 };

/* pthread_atfork handlers */
void prepare(void)
{
	controls[0]++;
}

void parent(void)
{
	controls[1]++;
}

void child(void)
{
	controls[2]++;
}

/* Thread function */
void *threaded(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int ret, status;
	pid_t child, ctl;

	/* Wait main thread has registered the handler */
	ret = pthread_mutex_lock(&mtx);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to lock mutex");
	}

	ret = pthread_mutex_unlock(&mtx);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to unlock mutex");
	}

	/* fork */
	child = fork();

	if (child == -1) {
		UNRESOLVED(errno, "Failed to fork");
	}

	/* child */
	if (child == 0) {
		if (controls[0] != 10000) {
			FAILED("prepare handler skipped some rounds");
		}

		if (controls[2] != 10000) {
			FAILED("child handler skipped some rounds");
		}

		/* We're done */
		exit(PTS_PASS);
	}

	if (controls[0] != 10000) {
		FAILED("prepare handler skipped some rounds");
	}

	if (controls[1] != 10000) {
		FAILED("parent handler skipped some rounds");
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status) || (WEXITSTATUS(status) != PTS_PASS)) {
		FAILED("Child exited abnormally");
	}

	/* quit */
	return NULL;
}

/* The main test function. */
int main(void)
{
	int ret, i;
	pthread_t ch;

	/* Initialize output */
	output_init();

	ret = pthread_mutex_lock(&mtx);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to lock mutex");
	}

	ret = pthread_create(&ch, NULL, threaded, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to create a thread");
	}

	/* Register the handlers */
	for (i = 0; i < 10000; i++) {
		ret = pthread_atfork(prepare, parent, child);

		if (ret == ENOMEM) {
			output("ENOMEM returned after %i iterations\n", i);
			break;
		}

		if (ret != 0) {
			UNRESOLVED(ret,
				   "Failed to register the atfork handlers");
		}
	}

	if (ret == 0) {

		/* Let the child go on */
		ret = pthread_mutex_unlock(&mtx);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to unlock mutex");
		}

		ret = pthread_join(ch, NULL);

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
