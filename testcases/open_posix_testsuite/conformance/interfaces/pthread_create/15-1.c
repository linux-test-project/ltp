/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
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
 * If the current thread has an alternate stack, the new thread does not inherit
 * this stack

 * The steps are:
 * -> Create a thread with an alternate stack.
 * -> From this thread, create another thread.
 * -> Check that the new thread does not use the same stack.

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

#include <sched.h>
#include <semaphore.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>

/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"
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

/********************************************************************************************/
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

/* main is defined in the next file */
#define STD_MAIN
#include "../testfrmw/threads_scenarii.c"

/* This file will define the following objects:
 * scenarii: array of struct __scenario type.
 * NSCENAR : macro giving the total # of scenarii
 * scenar_init(): function to call before use the scenarii array.
 * scenar_fini(): function to call after end of use of the scenarii array.
 */

/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

static void *teststack(void *arg)
{
	int ret = 0;
	*(int **)arg = &ret;
	return NULL;
}

/* Thread function */
static void *threaded(void *arg)
{
	int ret;
	int *child_stack;
	pthread_t gchild;

	int sz = sysconf(_SC_THREAD_STACK_MIN);

	if (scenarii[sc].bottom != NULL) {
#if VERBOSE > 1
		output("Processing test\n");
#endif

		/* Create a new thread and get a location inside its stack */
		ret = pthread_create(&gchild, NULL, teststack, &child_stack);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Failed to create a thread with default attribute");
		}

		ret = pthread_join(gchild, NULL);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to join the test thread");
		}

		/* Check the new thread stack location was outside of the current thread location */
		/* We convert all the @ to longs */
#if VERBOSE > 4
		output("Current stack : %p -> %p\n", scenarii[sc].bottom,
		       sz + (long)scenarii[sc].bottom);
		output("Child location: %p\n", child_stack);
#endif

		if ((((long)scenarii[sc].bottom) < ((long)child_stack))
		    && (((long)child_stack) <
			(((long)scenarii[sc].bottom) + sz))) {
			FAILED
			    ("The new thread inherited th alternate stack from its parent");
		}
	}

	/* Signal we're done (especially in case of a detached thread) */
	do {
		ret = sem_post(&scenarii[sc].sem);
	}
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1) {
		UNRESOLVED(errno, "Failed to wait for the semaphore");
	}

	/* return */
	return arg;
}
