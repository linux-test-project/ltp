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
 *

 * This file is a scalability test for the pthread_mutex_lock function.
 * The goal is to test if there is a limit on the number
 *  of threads waiting on the same mutex.

 * The steps are:
 * -> Create 5 mutex with different attributes.
 * -> lock the 5 mutex in the main thread
 * -> Create the maximum amount of threads allowed on the system.
 * -> each thread, for each mutex:
 *       - locks the mutex
 *       - increments a counter
 *       - unlocks the mutex
 *       - if the counter equals the amount of threads,
 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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
#define SCALABILITY_FACTOR 1	/* This is not used in this testcase */
#endif
#ifndef VERBOSE
#define VERBOSE 2
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

#ifndef WITHOUT_XOPEN
int types[] = {
	PTHREAD_MUTEX_NORMAL,
	PTHREAD_MUTEX_ERRORCHECK,
	PTHREAD_MUTEX_RECURSIVE,
	PTHREAD_MUTEX_DEFAULT
};
#endif

/* The mutex the threads will block on */
pthread_mutex_t mtx[5];

/* The condition used to signal the main thread to go to the next step */
pthread_cond_t cnd;
pthread_mutex_t m;

/* The shared data used to control the results of the test */
unsigned long nbthOK[5];
unsigned long nbthNOK[5];
unsigned long nbthTOT;

/*****
 *
 */
void *threaded(void *arg)
{
	int ret;
	int i;
	int bool;

	for (i = 0; i < 5; i++) {
		ret = pthread_mutex_lock(&mtx[i]);
		if (ret == 0) {	/* The thread was blocked successfuly */
			/* We increment nbth[i] */
			ret = pthread_mutex_lock(&m);
			if (ret != 0) {
				UNRESOLVED(ret, "Unable to lock 'm'");
			}
			nbthOK[i]++;
			bool = ((nbthOK[i] + nbthNOK[i]) >= nbthTOT);
			ret = pthread_mutex_unlock(&m);
			if (ret != 0) {
				UNRESOLVED(ret, "Unable to unlock 'm'");
			}

			/* We can unlock the test mutex */
			ret = pthread_mutex_unlock(&mtx[i]);
			if (ret != 0) {
				FAILED("Unlocking a test mutex failed");
			}
		} else {	/* Locking the test mutex failed */

			/* We increment nbth[i] */
			ret = pthread_mutex_lock(&m);
			if (ret != 0) {
				UNRESOLVED(ret, "Unable to lock 'm'");
			}
			nbthNOK[i]++;
			bool = ((nbthOK[i] + nbthNOK[i]) >= nbthTOT);
			ret = pthread_mutex_unlock(&m);
			if (ret != 0) {
				UNRESOLVED(ret, "Unable to unlock 'm'");
			}
		}

		/* When every thread has passed the lock call, bool is true.
		   we signal the main thread to release the next mutex. */

		if (bool) {
			ret = pthread_cond_signal(&cnd);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Signaling the condition failed");
			}
		}
	}

	/* The test is terminated, the thread can die */
	ret = pthread_detach(pthread_self());
	if (ret != 0) {
		UNRESOLVED(ret, "Thread detach failed");
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t th;
	pthread_attr_t tha;
	pthread_mutexattr_t ma;
	int ret;
	int i;

	output_init();

#if VERBOSE > 1
	output("Test starting, initializing data\n");
#endif

	/* Init the shared data */
	for (i = 0; i < 4; i++) {
		nbthOK[i] = 0;
		nbthNOK[i] = 0;
	}
	nbthTOT = 0;

	/* Init the cnd */
	ret = pthread_mutex_init(&m, NULL);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to initialize 'm'");
	}
	ret = pthread_cond_init(&cnd, NULL);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to initialize 'cnd'");
	}

	/* Init the 5 mutexes */
	ret = pthread_mutexattr_init(&ma);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to initialize 'ma'");
	}

	for (i = 0; i < 5; i++) {
#ifndef WITHOUT_XOPEN
		if (i > 0) {
			ret = pthread_mutexattr_settype(&ma, types[i - 1]);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to set mutex attribute type");
			}
		}
#endif
		ret = pthread_mutex_init(&mtx[i], &ma);
		if (ret != 0) {
			UNRESOLVED(ret, "A mutex init failed");
		}
	}

	ret = pthread_mutexattr_destroy(&ma);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to destroy the mutex attribute object");
	}

	/* Lock the mutexes */
	for (i = 0; i < 5; i++) {
		ret = pthread_mutex_lock(&mtx[i]);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to lock a mutex for the first time");
		}
	}

	/* Init the threads attribute */
	ret = pthread_attr_init(&tha);
	if (ret != 0) {
		UNRESOLVED(ret, "Thread attribute init failed");
	}

	ret = pthread_attr_setstacksize(&tha, sysconf(_SC_THREAD_STACK_MIN));
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to set stack size to minimum value");
	}

	/* Create as many threads as possible */
#if VERBOSE > 1
	output("Creating threads...\n");
#endif
	do {
		ret = pthread_create(&th, &tha, threaded, NULL);
		if (ret == 0)
			nbthTOT++;
	} while (ret == 0);

#if VERBOSE > 1
	output("Created %d threads.\n", nbthTOT);
#endif

	/* lock m */
	ret = pthread_mutex_lock(&m);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to lock 'm' in main thread");
	}

	/* For each mutex */
	for (i = 0; i < 5; i++) {
		/* Yield to let other threads enter the lock function */
		sched_yield();

		/* unlock the test mutex */
		ret = pthread_mutex_unlock(&mtx[i]);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to unlock a test mutex in main thread");
		}

		/* wait for cnd */
		do {
			ret = pthread_cond_wait(&cnd, &m);
		}
		while ((ret == 0) && ((nbthOK[i] + nbthNOK[i]) < nbthTOT));
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to wait for 'cnd'");
		}
	}

	/* unlock m */
	ret = pthread_mutex_unlock(&m);
	if (ret != 0) {
		UNRESOLVED(ret, "Final 'm' unlock failed");
	}

	/* Destroy everything */
	ret = pthread_attr_destroy(&tha);
	if (ret != 0) {
		UNRESOLVED(ret, "Final thread attribute destroy failed");
	}

	for (i = 0; i < 5; i++) {
		ret = pthread_mutex_destroy(&mtx[i]);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to destroy a test mutex");
		}
	}

	ret = pthread_cond_destroy(&cnd);
	if (ret != 0) {
		UNRESOLVED(ret, "Final cond destroy failed");
	}

	ret = pthread_mutex_destroy(&m);
	if (ret != 0) {
		UNRESOLVED(ret, "Final mutex destroy failed");
	}

	/* Output the results */
	output("Sample results:\n");
	output(" %lu threads were created\n", nbthTOT);
	for (i = 0; i < 5; i++) {
		output(" %lu threads have waited on mutex %i\n", nbthOK[i],
		       i + 1);
		output("  (and %lu threads could not wait)\n", nbthNOK[i]);
		ret += nbthNOK[i];
	}

	/* Exit */
	if (ret == 0) {
		PASSED;
	} else {
		FAILED("There may be an issue in scalability");
	}
}
