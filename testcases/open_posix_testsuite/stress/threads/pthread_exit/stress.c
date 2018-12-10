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

 * This file is a stress test for the function pthread_exit.
 *
 * It aims to check that:
 *  -> when the threads are joinable, pthread_join always retrieve the
 *     correct value.
 *  -> pthread_exit() frees all the resources used by the threads.
 *
 * The second assertion is implicitly checked by monitoring the system
 * while the stress test is running.
 *
 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <signal.h>
#include <semaphore.h>

/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
#include "testfrmw.h"
#include "testfrmw.c"
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
#ifndef SCALABILITY_FACTOR
#define SCALABILITY_FACTOR 1
#endif
#ifndef VERBOSE
#define VERBOSE 1
#endif

#define FACTOR 5

/* This testcase needs the XSI features */
#ifndef WITHOUT_XOPEN
/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

#include "threads_scenarii.c"

/* This file will define the following objects:
 * scenarii: array of struct __scenario type.
 * NSCENAR : macro giving the total # of scenarii
 * scenar_init(): function to call before use the scenarii array.
 * scenar_fini(): function to call after end of use of the scenarii array.
 */

/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

char do_it = 1;
long long iterations = 0;

/* Handler for user request to terminate */
void sighdl(int sig)
{
	/* do_it = 0 */
	do {
		do_it = 0;
	}
	while (do_it);
}

/* Cleanup handler to make sure the thread is exiting */
void cleanup(void *arg)
{
	int ret = 0;
	sem_t *sem = (sem_t *) arg;

	/* Signal we're done (especially in case of a detached thread) */
	do {
		ret = sem_post(sem);
	}
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1) {
		UNRESOLVED(errno, "Failed to wait for the semaphore");
	}
}

/* Thread routine */
void *threaded(void *arg)
{
	pthread_cleanup_push(cleanup, &scenarii[sc].sem);

	pthread_exit(arg);
	FAILED("the pthread_exit routine returned");

	pthread_cleanup_pop(1);

	return NULL;		/* For the sake of compiler */
}

/* main routine */
int main(int argc, char *argv[])
{
	int ret, i;
	void *rval;
	struct sigaction sa;

	pthread_t threads[NSCENAR * SCALABILITY_FACTOR * FACTOR];
	int rets[NSCENAR * SCALABILITY_FACTOR * FACTOR];

	/* Initialize output */
	output_init();

	/* Initialize scenarii table */
	scenar_init();

	/* Register the signal handler for SIGUSR1 */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sighdl;
	if ((ret = sigaction(SIGUSR1, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler");
	}
	if ((ret = sigaction(SIGALRM, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler");
	}
#if VERBOSE > 1
	output("[parent] Signal handler registered\n");
#endif

	while (do_it) {
		/* Create all the threads */
		for (i = 0; i < SCALABILITY_FACTOR * FACTOR; i++) {
			for (sc = 0; sc < NSCENAR; sc++) {
				/* Skip the alternative stack threads */
				if (scenarii[sc].altstack != 0)
					continue;

				rets[i * NSCENAR + sc] =
				    pthread_create(&threads[i * NSCENAR + sc],
						   &scenarii[sc].ta, threaded,
						   &threads[i * NSCENAR + sc]);
				switch (scenarii[sc].result) {
				case 0:	/* Operation was expected to succeed */
					if (rets[i * NSCENAR + sc] != 0) {
						UNRESOLVED(rets
							   [i * NSCENAR + sc],
							   "Failed to create this thread");
					}
					break;

				case 1:	/* Operation was expected to fail */
					if (rets[i * NSCENAR + sc] == 0) {
						UNRESOLVED(-1,
							   "An error was expected but the thread creation succeeded");
					}
					break;

				case 2:	/* We did not know the expected result */
				default:
#if VERBOSE > 5
					if (rets[i * NSCENAR + sc] == 0) {
						output
						    ("Thread has been created successfully for this scenario\n");
					} else {
						output
						    ("Thread creation failed with the error: %s\n",
						     strerror(rets
							      [i * NSCENAR +
							       sc]));
					}
#endif
					;
				}
				if (rets[i * NSCENAR + sc] == 0) {
					/* Just wait for the thread to terminate */
					do {
						ret =
						    sem_wait(&scenarii[sc].sem);
					}
					while ((ret == -1) && (errno == EINTR));
					if (ret == -1) {
						UNRESOLVED(errno,
							   "Failed to wait for the semaphore");
					}
				}
			}
		}

		/* Join all the joinable threads and check the value */
		for (i = 0; i < SCALABILITY_FACTOR * FACTOR; i++) {
			for (sc = 0; sc < NSCENAR; sc++) {
				if ((scenarii[sc].altstack == 0)
				    && (scenarii[sc].detached == 0)
				    && (rets[i * NSCENAR + sc] == 0)) {
					ret =
					    pthread_join(threads
							 [i * NSCENAR + sc],
							 &rval);
					if (ret != 0) {
						UNRESOLVED(ret,
							   "Unable to join a thread");
					}

					if (rval !=
					    (void *)&threads[i * NSCENAR +
							     sc]) {
						output
						    ("arg: %p -- got %p -- NULL=%p\n",
						     &threads[i * NSCENAR + sc],
						     rval, NULL);
						FAILED
						    ("The retrieved error value is corrupted");
					}
				}
			}
		}

		iterations++;
	}

	/* Destroy scenarii attributes */
	scenar_fini();

	/* Test passed */
	output("pthread_exit stress test PASSED -- %llu iterations\n",
	       iterations);
	PASSED;
}

#else /* WITHOUT_XOPEN */
int main(int argc, char *argv[])
{
	output_init();
	UNTESTED("This test requires XSI features");
}
#endif
