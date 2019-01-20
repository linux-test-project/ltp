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
 * Pending cancelation cleanup handlers are executed in FILO order.

 * The steps are:
 *
 *  -> Create threads with different attributes (but all must be joinable)
 *  -> inside the thread,
 *     -> register three cancelation handlers
 *     -> call pthread_exit.
 *  -> In the main thread, we join the thread and check the cleanups were executed in order.

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

int global = 0;
int tab[3];

#define CLEANUP(n) void clnp##n(void * arg)\
{\
	tab[global]=n; \
	global++; \
}

CLEANUP(1)
    CLEANUP(2)
    CLEANUP(3)

void *threaded(void *arg LTP_ATTRIBUTE_UNUSED)
{
	pthread_cleanup_push(clnp3, NULL);
	pthread_cleanup_push(clnp2, NULL);
	pthread_cleanup_push(clnp1, NULL);

	pthread_exit(NULL + 1);

	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);

	FAILED("pthread_exit() did not terminate the thread");
	return NULL;
}

int main(void)
{
	int ret = 0;
	void *rval;
	pthread_t child;
	int i, j;

	output_init();

	scenar_init();

	for (i = 0; i < NSCENAR; i++) {
		if (scenarii[i].detached == 0) {
#if VERBOSE > 0
			output("-----\n");
			output("Starting test with scenario (%i): %s\n", i,
			       scenarii[i].descr);
#endif

			for (j = 0; j < 3; j++)
				tab[j] = 0;
			global = 0;

			ret =
			    pthread_create(&child, &scenarii[i].ta, threaded,
					   NULL);
			switch (scenarii[i].result) {
			case 0:	/* Operation was expected to succeed */
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Failed to create this thread");
				}
				break;

			case 1:	/* Operation was expected to fail */
				if (ret == 0) {
					UNRESOLVED(-1,
						   "An error was expected but the thread creation succeeded");
				}
				break;

			case 2:	/* We did not know the expected result */
			default:
#if VERBOSE > 0
				if (ret == 0) {
					output
					    ("Thread has been created successfully for this scenario\n");
				} else {
					output
					    ("Thread creation failed with the error: %s\n",
					     strerror(ret));
				}
#endif
			}
			if (ret == 0) {	/* The new thread is running */
				ret = pthread_join(child, &rval);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Unable to join a thread");
				}

				if (rval != (NULL + 1)) {
					FAILED
					    ("pthread_join() did not retrieve the pthread_exit() param");
				}

				for (j = 0; j < 3; j++)
					if (tab[j] != j + 1) {
						output
						    ("dump:\ntab[0]=%i\ntab[1]=%i\ntab[2]=%i\n",
						     tab[0], tab[1], tab[2]);
						FAILED
						    ("The cleanup handlers were not called as expected");
					}
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
