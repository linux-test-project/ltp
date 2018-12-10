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

 * This file is a stress test for the pthread_mutex_trylock function.
 *
 * It aims to check that when EBUSY is returned, the mutex has not been locked.

 * The steps are:
 * -> do
 *   -> Set up a timeout. The test fails if this timeout expires.
 *   -> lock the mutex
 *   -> create N threads.
 *     -> each thread loops on pthread_mutex_trylock while ret == EBUSY and go_on is true..
 *     -> if ret == 0 && go_on is true, wait on a barrier then go_on = 0.
 *     -> if ret == 0 unlock the mutex
 *   -> do
 *     -> unlock the mutex
 *     -> yield
 *     -> trylock the mutex
 *   -> while we don't get EBUSY.
 *   -> wait on the barrier to unblock the thread which got the 0 error code.
 *       If no thread got this code but one got the mutex,
 *       the main thread will hang here and the timeout will expire (test FAILS).
 *   -> join all the threads
 * -> while we don't receive SIGUSR1
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
#include <semaphore.h>
#include <signal.h>

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
#ifndef VERBOSE
#define VERBOSE 1
#endif

#ifndef SCALABILITY_FACTOR
#define SCALABILITY_FACTOR 1
#endif

#define NCHILDREN  (20 * SCALABILITY_FACTOR)

#define TIMEOUT  (120 * SCALABILITY_FACTOR)

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

char do_it = 1;

/* Signal handler which will stop the stress run */
void sighdl(int sig)
{
	do {
		do_it = 0;
	}
	while (do_it);
}

/* Timeout thread */
void *timer(void *arg)
{
	unsigned int to = TIMEOUT;
	do {
		to = sleep(to);
	}
	while (to > 0);
	FAILED
	    ("Operation timed out. EBUSY was returned while a thread acquired the mutex?.");
	return NULL;		/* For compiler */
}

/* Test specific data */
char go_on = 0;

typedef struct {
	pthread_mutex_t *mtx;
	pthread_barrier_t *bar;
} testdata_t;

struct _scenar {
	int m_type;		/* Mutex type to use */
	int m_pshared;		/* 0: mutex is process-private (default) ~ !0: mutex is process-shared, if supported */
	char *descr;		/* Case description */
} scenarii[] = {
	{
	PTHREAD_MUTEX_DEFAULT, 0, "Default mutex"}
#ifndef WITHOUT_XOPEN
	, {
	PTHREAD_MUTEX_NORMAL, 0, "Normal mutex"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 0, "Errorcheck mutex"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 0, "Recursive mutex"}
#endif

	, {
	PTHREAD_MUTEX_DEFAULT, 1, "Pshared mutex"}
#ifndef WITHOUT_XOPEN
	, {
	PTHREAD_MUTEX_NORMAL, 1, "Pshared Normal mutex"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 1, "Pshared Errorcheck mutex"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 1, "Pshared Recursive mutex"}
#endif
};

#define NSCENAR (sizeof(scenarii)/sizeof(scenarii[0]))

void *threaded(void *arg)
{
	int ret = 0, ret2 = 0;
	testdata_t *td = (testdata_t *) arg;

	/* do */
	do {
		/* trylock the mutex */
		ret = pthread_mutex_trylock(td->mtx);

		/* yield */
		sched_yield();

	}
	/* while trylock returns EBUSY and go_on == 1 */
	while ((ret == EBUSY) && (go_on == 1));

	/* if go_on==1 and ret == 0 */
	if ((go_on == 1) && (ret == 0)) {
#if VERBOSE > 6
		output("[child %p] I got the mutex\n", pthread_self());
#endif

		/* barrier */
		ret2 = pthread_barrier_wait(td->bar);
		if ((ret2 != 0) && (ret2 != PTHREAD_BARRIER_SERIAL_THREAD)) {
			UNRESOLVED(ret2, "Pthread_barrier_wait failed");
		}

		/* go_on = 0 */
		go_on = 0;
	}

	/* if ret == 0 */
	if (ret == 0) {
		/* Unlock the mutex */
		ret = pthread_mutex_unlock(td->mtx);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to unlock the mutex");
		}
	}

	/* end of thread */
	return NULL;
}

int main(int argc, char *argv[])
{
	int ret;
	struct sigaction sa;

	pthread_t t_child[NCHILDREN];

	pthread_t t_timer;

	testdata_t td;

	int i, ch;
	long pshared;
	pthread_mutex_t mtx[NSCENAR + 2];
	pthread_mutexattr_t ma;
	pthread_barrier_t bar;

	/* Initialize output */
	output_init();

	/* System abilities */
	pshared = sysconf(_SC_THREAD_PROCESS_SHARED);

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

	/* Initialize the barrier */
	ret = pthread_barrier_init(&bar, NULL, 2);
	if (ret != 0) {
		UNRESOLVED(ret, "Barrier init failed");
	}
	td.bar = &bar;

	/* Initialize every mutexattr & mutex objects */
	for (i = 0; i < NSCENAR; i++) {
		ret = pthread_mutexattr_init(&ma);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "[parent] Unable to initialize the mutex attribute object");
		}
#ifndef WITHOUT_XOPEN
		/* Set the mutex type */
		ret = pthread_mutexattr_settype(&ma, scenarii[i].m_type);
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Unable to set mutex type");
		}
#endif
		/* Set the pshared attributes, if supported */
		if ((pshared > 0) && (scenarii[i].m_pshared != 0)) {
			ret =
			    pthread_mutexattr_setpshared(&ma,
							 PTHREAD_PROCESS_SHARED);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to set the mutex process-shared");
			}
		}

		/* Initialize the mutex */
		ret = pthread_mutex_init(&mtx[i], &ma);
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Mutex init failed");
		}

		/* Destroy the ma object */
		ret = pthread_mutexattr_destroy(&ma);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Failed to destroy the mutexattr object");
		}
	}
	/* Default mutexattr object */
	ret = pthread_mutexattr_init(&ma);
	if (ret != 0) {
		UNRESOLVED(ret,
			   "[parent] Unable to initialize the mutex attribute object");
	}
	ret = pthread_mutex_init(&mtx[i], &ma);
	if (ret != 0) {
		UNRESOLVED(ret, "[parent] Mutex init failed");
	}
	ret = pthread_mutexattr_destroy(&ma);
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to destroy the mutexattr object");
	}
	/* Default mutex */
	ret = pthread_mutex_init(&mtx[i + 1], NULL);
	if (ret != 0) {
		UNRESOLVED(ret, "[parent] Mutex init failed");
	}

	i = 0;
	/* While we are not asked to stop */
	while (do_it) {
		/* Start the timeout thread */
		ret = pthread_create(&t_timer, NULL, timer, NULL);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to create timer thread");
		}

		/* Set the td pointer to the next mutex */
		td.mtx = &mtx[i];

		/* lock this mutex */
		ret = pthread_mutex_lock(td.mtx);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to lock a mutex");
		}

		/* go_on = 1 */
		go_on = 1;

		/* Start the children */
		for (ch = 0; ch < NCHILDREN; ch++) {
			ret = pthread_create(&t_child[ch], NULL, threaded, &td);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Failed to create enough threads");
			}
		}
#if VERBOSE > 5
		output("[parent] The children are running...\n");
#endif

		/* do */
		do {
			/* unlock the mutex */
			ret = pthread_mutex_unlock(td.mtx);
			if (ret != 0) {
				UNRESOLVED(ret, "Failed to unlcok the mutex");
			}

			/* yield */
			sched_yield();

			/* trylock the mutex again */
			ret = pthread_mutex_trylock(td.mtx);
		}
		/* while trylock succeeds */
		while (ret == 0);
		if (ret != EBUSY) {
			UNRESOLVED(ret, "An unexpected error occured");
		}
#if VERBOSE > 6
		output
		    ("[parent] Mutex is busy, a child shall be waiting on the barrier\n");
#endif

		/* barrier */
		ret = pthread_barrier_wait(&bar);
		if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
			UNRESOLVED(ret, "Pthread_barrier_wait failed");
		}

		/* cancel the timeout thread */
		ret = pthread_cancel(t_timer);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to cancel the timeout thread");
		}

		/* join every threads (incl. the timeout) */
		ret = pthread_join(t_timer, NULL);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to join timeout thread");
		}
		for (ch = 0; ch < NCHILDREN; ch++) {
			ret = pthread_join(t_child[ch], NULL);
			if (ret != 0) {
				UNRESOLVED(ret, "Failed to join a child");
			}
		}

		/* next mutex */
		i++;
		i %= NSCENAR + 2;
	}

	/* destroy the barrier & mutexes objects */
	ret = pthread_barrier_destroy(&bar);
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to destroy the barrier");
	}

	for (i = 0; i < NSCENAR + 2; i++) {
		ret = pthread_mutex_destroy(&mtx[i]);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to destroy a mutex");
		}
	}

#if VERBOSE > 0
	output("pthread_mutex_trylock stress test passed\n");
#endif

	/* test passed */
	PASSED;
}
