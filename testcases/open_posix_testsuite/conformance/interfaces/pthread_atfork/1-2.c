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
* pthread_atfork registers the 'prepare' handler to be called before fork()
* processing in the context of the fork() calling thread.
*
* pthread_atfork registers the 'parent' handler to be called after fork()
* processing in the context of the fork() calling thread in the parent process.
*
* pthread_atfork registers the 'child' handler to be called after fork()
* processing in the context of the fork() calling thread in the child process.

* The steps are:
* -> Create a new thread
* -> Call pthread_atfork from the main thread.
* -> Child thread forks.
* -> Check that the handlers have been called, and in the context of the
*    calling thread.

* The test fails if the thread executing the handlers is not the one expected,
* or if the handlers are not executed.

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

pthread_t threads[3];
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_t ch;

/* at fork handlers */
void prepare(void)
{
	threads[0] = pthread_self();
}

void parent(void)
{
	threads[1] = pthread_self();
}

void child(void)
{
	threads[2] = pthread_self();
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
		if (!pthread_equal(ch, threads[0])) {
			FAILED
			    ("prepare handler was not called in the thread s context");
		}

		if (!pthread_equal(pthread_self(), threads[2])) {
			FAILED
			    ("child handler was not called in the thread s context");
		}

		/* We're done */
		exit(PTS_PASS);
	}

	if (!pthread_equal(ch, threads[0])) {
		FAILED
		    ("prepare handler was not called in the thread s context");
	}

	if (!pthread_equal(pthread_self(), threads[1])) {
		FAILED("parent handler was not called in the thread s context");
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
	int ret;

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
	ret = pthread_atfork(prepare, parent, child);

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
