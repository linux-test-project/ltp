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
* NULL can be passed as any of these handlers when no treatment is required.

* The steps are:
* -> Create a new thread
* -> Try all NULL / non NULL combinations (7) of pthread_atfork parameters.

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

int iPrepare = 0, iParent = 0, iChild = 0;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

/* pthread_atfork handlers */
/* 0: NULL NULL NULL  (1)
   1:  p1  NULL NULL  (2)
   2: NULL pa2  NULL  (4)
   3: NULL NULL  c3   (8)
   4:  p4  pa4  NULL  (16)
   5:  p5  NULL  c5   (32)
   6: NULL pa6   c6   (64)
 The ultimate combination is already tested in other testcase.
 tot:  50   84  104
 */
void p1(void)
{
	iPrepare |= 1 << 1;
}

void p4(void)
{
	iPrepare |= 1 << 4;
}

void p5(void)
{
	iPrepare |= 1 << 5;
}

void pa2(void)
{
	iParent |= 1 << 2;
}

void pa4(void)
{
	iParent |= 1 << 4;
}

void pa6(void)
{
	iParent |= 1 << 6;
}

void c3(void)
{
	iChild |= 1 << 3;
}

void c5(void)
{
	iChild |= 1 << 5;
}

void c6(void)
{
	iChild |= 1 << 6;
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
		if (iPrepare != 50) {
			FAILED("prepare handler were not called as expected");
		}

		if (iChild != 104) {
			FAILED("prepare handler were not called as expected");
		}

		/* We're done */
		exit(PTS_PASS);
	}

	if (iPrepare != 50) {
		FAILED("prepare handler were not called as expected");
	}

	if (iParent != 84) {
		FAILED("prepare handler were not called as expected");
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
	ret = pthread_atfork(NULL, NULL, NULL);

	if (ret != 0) {
		UNRESOLVED(ret,
			   "Failed to register the atfork handlers(N,N,N)");
	}

	ret = pthread_atfork(p1, NULL, NULL);

	if (ret != 0) {
		UNRESOLVED(ret,
			   "Failed to register the atfork handlers(h,N,N)");
	}

	ret = pthread_atfork(NULL, pa2, NULL);

	if (ret != 0) {
		UNRESOLVED(ret,
			   "Failed to register the atfork handlers(N,h,N)");
	}

	ret = pthread_atfork(NULL, NULL, c3);

	if (ret != 0) {
		UNRESOLVED(ret,
			   "Failed to register the atfork handlers(N,N,h)");
	}

	ret = pthread_atfork(p4, pa4, NULL);

	if (ret != 0) {
		UNRESOLVED(ret,
			   "Failed to register the atfork handlers(h,h,N)");
	}

	ret = pthread_atfork(p5, NULL, c5);

	if (ret != 0) {
		UNRESOLVED(ret,
			   "Failed to register the atfork handlers(h,N,h)");
	}

	ret = pthread_atfork(NULL, pa6, c6);

	if (ret != 0) {
		UNRESOLVED(ret,
			   "Failed to register the atfork handlers(N,h,h)");
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
