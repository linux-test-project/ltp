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
* When pthread_atfork is called several times, the prepare handlers are executed
* in reversed order as they were registered, and child and parent handlers are
* executed in the same order as they were registered.

* The steps are:
* -> Register some handlers for which call order is traceable.

* The test fails if the registered handlers are not executed as expected.

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

int control = 0;
int nerrors = 0;

/* pthread_atfork handlers */
void pre3(void)
{
	control++;

	if (control != 1)
		nerrors++;
}

void pre2(void)
{
	control++;

	if (control != 2)
		nerrors++;
}

void pre1(void)
{
	control++;

	if (control != 3)
		nerrors++;
}

void par1(void)
{
	control++;

	if (control != 4)
		nerrors++;
}

void par2(void)
{
	control++;

	if (control != 5)
		nerrors++;
}

void par3(void)
{
	control++;

	if (control != 6)
		nerrors++;
}

void chi1(void)
{
	control += 2;

	if (control != 5)
		nerrors++;
}

void chi2(void)
{
	control += 2;

	if (control != 7)
		nerrors++;
}

void chi3(void)
{
	control += 2;

	if (control != 9)
		nerrors++;
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
		if (nerrors) {
			FAILED("Errors occured in the child");
		}

		/* We're done */
		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status) || (WEXITSTATUS(status) != PTS_PASS)) {
		FAILED("Child exited abnormally");
	}

	if (nerrors) {
		FAILED("Errors occured in the parent (only)");
	}

	/* quit */
	return NULL;
}

/* The main test function. */
int main(void)
{
	int ret;
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
	ret = pthread_atfork(pre1, par1, chi1);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to register the atfork handlers");
	}

	ret = pthread_atfork(pre2, par2, chi2);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to register the atfork handlers");
	}

	ret = pthread_atfork(pre3, par3, chi3);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to register the atfork handlers");
	}

	/* Let the child go on */
	ret = pthread_mutex_unlock(&mtx);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to unlock mutex");
	}

	ret = pthread_join(ch, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join the thread");
	}

	/* Test passed */
#if VERBOSE > 0

	output("Test passed\n");

#endif

	PASSED;
}
