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
 * pthread_create creates a thread with attributes as specified in the attr parameter.

 * The steps are:
 *
 * -> Create a new thread with known parameters.
 * -> Check the thread behavior conforms to these parameters.
 *     This checking consists in:
 *    -> If an alternative stack has been specified, check that the new thread stack is within this specified area.
 *    -> If stack size and guard size are known, check that accessing the guard size fails. (new process)
 *    -> If we are able to run threads with high priority and known sched policy, check that a high priority thread executes before a low priority thread.
              (This will be done in another test has it fails with Linux kernel (2.6.8 at least)
 *    -> The previous test could be extended to cross-process threads to check the scope attribute behavior (postponned for now).
 *    (*) The detachstate attribute is not tested cause this would mean a speculative test. Moreover, it is already tested elsewhere.

 * The test fails if one of those tests fails.

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
#define STD_MAIN		/* This allows main() to be defined in the included file */
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

/* The overflow function is used to test the stack overflow */
void *overflow(void *arg)
{
	void *current;
	void *pad[50];		/* We want to consume the stack quickly */
	long stacksize = sysconf(_SC_THREAD_STACK_MIN);	/* make sure we touch the current stack memory */
	int ret = 0;

	pad[1] = NULL;		/* so compiler stops complaining about unused variables */

	if (arg == NULL) {
		/* first call */
		current = overflow(&current);

		/* Terminate the overflow thread */
		/* Post the semaphore to unlock the main thread in case of a detached thread */
		do {
			ret = sem_post(&scenarii[sc].sem);
		}
		while ((ret == -1) && (errno == EINTR));
		if (ret == -1) {
			UNRESOLVED(errno, "Failed to post the semaphore");
		}
		return NULL;
	}

	/* we cast the pointers into long, which might be a problem on some architectures... */
	if (((long)arg) < ((long)&current)) {
		/* the stack is growing up */
		if (((long)&current) - ((long)arg) >= stacksize) {
			output
			    ("Growing up stack started below %p and we are currently up to %p\n",
			     arg, &current);
			return NULL;
		}
	} else {
		/* the stack is growing down */
		if (((long)arg) - ((long)&current) >= stacksize) {
			output
			    ("Growing down stack started upon %p and we are currently down to %p\n",
			     arg, &current);
			return NULL;
		}
	}

	/* We are not yet overflowing, so we loop */
	{
		return overflow(arg);
	}
}

void *threaded(void *arg)
{
	int ret = 0;

#if VERBOSE > 4
	output("Thread %i starting...\n", sc);
#endif

	/* Alternate stack test */
	if (scenarii[sc].bottom != NULL) {
#ifdef WITHOUT_XOPEN
		output
		    ("Unable to test the alternate stack feature; need an integer pointer cast\n");
#else
		intptr_t stack_start, stack_end, current_pos;

		stack_start = (intptr_t) scenarii[sc].bottom;
		stack_end =
		    stack_start + (intptr_t) sysconf(_SC_THREAD_STACK_MIN);
		current_pos = (intptr_t) & ret;

#if VERBOSE > 2
		output("Stack bottom: %p\n", scenarii[sc].bottom);
		output("Stack end   : %p (stack is 0x%lx bytes)\n",
		       (void *)stack_end, stack_end - stack_start);
		output("Current pos : %p\n", &ret);
#endif
		if ((stack_start > current_pos) || (current_pos > stack_end)) {
			FAILED("The specified stack was not used.\n");
		}
#endif // WITHOUT_XOPEN
	}

	/* Guard size test */
	if ((scenarii[sc].bottom == NULL)	/* no alternative stack was specified */
	    &&(scenarii[sc].guard == 2)	/* guard area size is 1 memory page */
	    &&(scenarii[sc].altsize == 1)) {	/* We know the stack size */
		pid_t child, ctrl;
		int status;

		child = fork();	/* We'll test the feature in another process as this test may segfault */

		if (child == -1) {
			UNRESOLVED(errno, "Failed to fork()");
		}

		if (child != 0) {	/* father */
			/* Just wait for the child and check its return value */
			ctrl = waitpid(child, &status, 0);
			if (ctrl != child) {
				UNRESOLVED(errno,
					   "Failed to wait for process termination");
			}

			if (WIFEXITED(status)) {	/* The process exited */
				if (WEXITSTATUS(status) == 0) {
					FAILED
					    ("Overflow into the guard area did not fail");
				}
				if (WEXITSTATUS(status) == PTS_UNRESOLVED) {
					UNRESOLVED(-1,
						   "The child process returned unresolved status");
				}
#if VERBOSE > 4
				else {
					output
					    ("The child process returned: %i\n",
					     WEXITSTATUS(status));
				}
			} else {
				output("The child process did not returned\n");
				if (WIFSIGNALED(status))
					output("It was killed with signal %i\n",
					       WTERMSIG(status));
				else
					output
					    ("neither was it killed. (status = %i)\n",
					     status);
#endif
			}
		}

		if (child == 0) {	/* this is the new process */
			pthread_t th;

			ret = pthread_create(&th, &scenarii[sc].ta, overflow, NULL);	/* Create a new thread with the same attributes */
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to create another thread with the same attributes in the new process");
			}

			if (scenarii[sc].detached == 0) {
				ret = pthread_join(th, NULL);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Unable to join a thread");
				}
			} else {
				/* Just wait for the thread to terminate */
				do {
					ret = sem_wait(&scenarii[sc].sem);
				}
				while ((ret == -1) && (errno == EINTR));
				if (ret == -1) {
					UNRESOLVED(errno,
						   "Failed to wait for the semaphore");
				}
			}

			/* Terminate the child process here */
			exit(0);
		}
	}

	/* Post the semaphore to unlock the main thread in case of a detached thread */
	do {
		ret = sem_post(&scenarii[sc].sem);
	}
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1) {
		UNRESOLVED(errno, "Failed to post the semaphore");
	}

	return arg;
}
