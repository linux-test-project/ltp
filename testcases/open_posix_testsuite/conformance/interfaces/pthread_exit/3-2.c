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
 * The TLD destructors (if any) are executed after the cancelation cleanup handlers,
 * in unspecified order.

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

static int global = 0;
static int tab[4];
static pthread_key_t tld[3];

#define CLEANUP(n) static void clnp##n(void * arg)\
{\
	tab[global]=n; \
	global++; \
}

/* Cancelation cleanup handlers */
CLEANUP(1)
    CLEANUP(2)
    CLEANUP(3)

/* TLD destructor */
static void destructor(void *arg)
{
	*(int *)arg += global;
}

/* Thread routine */
static void *threaded(void *arg PTS_ATTRIBUTE_UNUSED)
{
	int ret = 0;

	ret = pthread_setspecific(tld[0], (void *)&tab[3]);
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to set TLD data");
	}

	pthread_cleanup_push(clnp3, NULL);
	pthread_cleanup_push(clnp2, NULL);
	ret = pthread_setspecific(tld[1], (void *)&tab[3]);
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to set TLD data");
	}

	pthread_cleanup_push(clnp1, NULL);
	ret = pthread_setspecific(tld[2], (void *)&tab[3]);
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to set TLD data");
	}

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
	unsigned int i;
	int j;

	output_init();

	scenar_init();

	for (j = 0; j < 3; j++) {
		ret = pthread_key_create(&tld[j], destructor);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to create a TLD key");
		}
	}

	for (i = 0; i < NSCENAR; i++) {
		if (scenarii[i].detached == 0) {
#if VERBOSE > 0
			output("-----\n");
			output("Starting test with scenario (%i): %s\n", i,
			       scenarii[i].descr);
#endif

			for (j = 0; j < 4; j++)
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

				for (j = 0; j < 3; j++) {
					if ((tab[j] != j + 1) || (tab[3] != 9)) {
						output
						    ("dump:\ntab[0]=%i\ntab[1]=%i\ntab[2]=%i\ntab[3]=%i\n",
						     tab[0], tab[1], tab[2],
						     tab[3]);
						FAILED
						    ("The cleanup handlers were not called as expected");
					}
				}
			}
		}
	}

	for (j = 0; j < 3; j++) {
		ret = pthread_key_delete(tld[j]);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to delete a TLD key");
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
