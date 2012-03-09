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
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.

 * This sample test aims to check the following assertion:
 *
 * pthread_create creates a new thread within the process

 * The steps are:
 *
 *  -> get the thread ID and the process ID of the main thread.
 *  -> create a new thread, get the thread & process ID.
 *  -> check that the thread IDs are different but process IDs are the same

 * The test fails if the thread IDs are the same or the proces IDs are different.

 */

 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L

 /* Some routines are part of the XSI Extensions */
#ifndef WITHOUT_XOPEN
 #define _XOPEN_SOURCE	600
#endif
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

struct testdata
{
	pthread_t tid;
	pid_t	  pid;
	sem_t	* sem;
};

int global; /* This value is used to check both threads share the same process memory (and not a copy) */

void * threaded (void * arg)
{
	struct testdata * td=(struct testdata *) arg;
	pthread_t mytid;
	pid_t mypid;
	int ret = 0;

	/* Compare the process IDs */
	mypid=getpid();
	#if VERBOSE > 0
	output("  Main pid: %i  thread pid: %i\n", td->pid, mypid);
	#endif

	if (mypid != td->pid)
	{
		FAILED("New thread does not belong to the same process as its parent thread");
	}

	/* Compare the threads IDs */
	mytid = pthread_self();
	#if VERBOSE > 0
	/* pthread_t is a pointer with Linux/nptl. This output can be erroneous for other arcs */
	output("  Main tid: %p  thread tid: %p\n", td->tid, mytid);
	#endif

	if (pthread_equal(mytid, td->tid) != 0)
	{
		FAILED("The created thread has the same thread ID as its parent");
	}

	/* Change the global value */
	global++;

	/* Post the semaphore to unlock the main thread in case of a detached thread */
	do { ret = sem_post(td->sem); }
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1)  {  UNRESOLVED(errno, "Failed to post the semaphore");  }

	return arg;
}

int main (int argc, char *argv[])
{
	int ret=0;
	struct testdata td;
	void * rval;
	pthread_t child;
	int i;

	output_init();

	td.tid=pthread_self();
	td.pid=getpid();

	scenar_init();

	for (i=0; i < NSCENAR; i++)
	{
		#if VERBOSE > 0
		output("-----\n");
		output("Starting test with scenario (%i): %s\n", i, scenarii[i].descr);
		#endif

		td.sem = &scenarii[i].sem;

		global = 2*i;

		ret = pthread_create(&child, &scenarii[i].ta, threaded, &td);
		switch (scenarii[i].result)
		{
			case 0: /* Operation was expected to succeed */
				if (ret != 0)  {  UNRESOLVED(ret, "Failed to create this thread");  }
				break;

			case 1: /* Operation was expected to fail */
				if (ret == 0)  {  UNRESOLVED(-1, "An error was expected but the thread creation succeeded");  }
				break;

			case 2: /* We did not know the expected result */
			default:
				#if VERBOSE > 0
				if (ret == 0)
					{ output("Thread has been created successfully for this scenario\n"); }
				else
					{ output("Thread creation failed with the error: %s\n", strerror(ret)); }
				#endif
		}
		if (ret == 0) /* The new thread is running */
		{
			if (scenarii[i].detached == 0)
			{
				ret = pthread_join(child, &rval);
				if (ret != 0)  {  UNRESOLVED(ret, "Unable to join a thread");  }

				if (rval != &td)
				{
					FAILED("Could not get the thread return value. Did it execute?");
				}
			}
			else
			{
				/* Just wait for the thread terminate */
				do { ret = sem_wait(td.sem); }
				while ((ret == -1) && (errno == EINTR));
				if (ret == -1)  {  UNRESOLVED(errno, "Failed to post the semaphore");  }
			}

			if (global != (2*i + 1))
			{
				/* Maybe a possible issue with CPU memory-caching here? */
				FAILED("The threads do not share the same process memory.");
			}
		}
	}

	scenar_fini();
	#if VERBOSE > 0
	output("-----\n");
	output("All test data destroyed\n");
	output("Test PASSED\n");
	#endif

	PASSED;
}