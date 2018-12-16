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

 * This file is a stress test for the function pthread_cond_timedwait.
 *
 * It aims to check the following assertion:
 *  When a cancel request unblocks the thread,
 *  it must not consume any pending condition signal request.

 * The steps are:
 *  -> Create a bunch of threads waiting on a condvar.
 *  -> At the same time (using a barrier) one thread is canceled and the condition is signaled.
 *  -> Test checks that the cond signaling was not lost (at least one thread must have woken cleanly).
 *  -> Then everything is cleaned up and started again.

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
#include <string.h>
#include <time.h>

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

/* Size of the "bunch" of threads -- the real number will be 2 more threads per scenarii */
#define NCHILDREN (20)

#define TIMEOUT  (60)

#ifndef WITHOUT_ALTCLK
#define USE_ALTCLK		/* make tests with MONOTONIC CLOCK if supported */
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

struct _scenar {
	int m_type;		/* Mutex type to use */
	int mc_pshared;		/* 0: mutex and cond are process-private (default) ~ !0: Both are process-shared, if supported */
	int c_clock;		/* 0: cond uses the default clock. ~ !0: Cond uses monotonic clock, if supported. */
	int fork;		/* 0: Test between threads. ~ !0: Test across processes, if supported (mmap) */
	char *descr;		/* Case description */
} scenarii[] = {
	{
	PTHREAD_MUTEX_DEFAULT, 0, 0, 0, "Default mutex"}
	, {
	PTHREAD_MUTEX_NORMAL, 0, 0, 0, "Normal mutex"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 0, 0, 0, "Errorcheck mutex"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 0, 0, 0, "Recursive mutex"}

	, {
	PTHREAD_MUTEX_DEFAULT, 1, 0, 0, "PShared default mutex"}
	, {
	PTHREAD_MUTEX_NORMAL, 1, 0, 0, "Pshared normal mutex"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 1, 0, 0, "Pshared errorcheck mutex"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 1, 0, 0, "Pshared recursive mutex"}

	, {
	PTHREAD_MUTEX_DEFAULT, 1, 0, 1,
		    "Pshared default mutex across processes"}
	, {
	PTHREAD_MUTEX_NORMAL, 1, 0, 1,
		    "Pshared normal mutex across processes"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 1, 0, 1,
		    "Pshared errorcheck mutex across processes"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 1, 0, 1,
		    "Pshared recursive mutex across processes"}

#ifdef USE_ALTCLK
	, {
	PTHREAD_MUTEX_DEFAULT, 1, 1, 1,
		    "Pshared default mutex and alt clock condvar across processes"}
	, {
	PTHREAD_MUTEX_NORMAL, 1, 1, 1,
		    "Pshared normal mutex and alt clock condvar across processes"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 1, 1, 1,
		    "Pshared errorcheck mutex and alt clock condvar across processes"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 1, 1, 1,
		    "Pshared recursive mutex and alt clock condvar across processes"}

	, {
	PTHREAD_MUTEX_DEFAULT, 0, 1, 0,
		    "Default mutex and alt clock condvar"}
	, {
	PTHREAD_MUTEX_NORMAL, 0, 1, 0,
		    "Normal mutex and alt clock condvar"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 0, 1, 0,
		    "Errorcheck mutex and alt clock condvar"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 0, 1, 0,
		    "Recursive mutex and alt clock condvar"}

	, {
	PTHREAD_MUTEX_DEFAULT, 1, 1, 0,
		    "PShared default mutex and alt clock condvar"}
	, {
	PTHREAD_MUTEX_NORMAL, 1, 1, 0,
		    "Pshared normal mutex and alt clock condvar"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 1, 1, 0,
		    "Pshared errorcheck mutex and alt clock condvar"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 1, 1, 0,
		    "Pshared recursive mutex and alt clock condvar"}
#endif
};

#define NSCENAR (sizeof(scenarii)/sizeof(scenarii[0]))

/* This is the shared structure for all threads related to the same condvar */
struct celldata {
	pthread_t workers[NCHILDREN * SCALABILITY_FACTOR + 2];
	pthread_t signaler;

	pthread_barrier_t bar;
	pthread_mutex_t mtx;
	pthread_cond_t cnd;
	clockid_t cid;

	int boolean;
	int count;

	long canceled;
	long cancelfailed;
	long cnttotal;
} cells[NSCENAR * SCALABILITY_FACTOR];

char do_it = 1;
pthread_attr_t ta;

void cleanup(void *arg)
{
	int ret;
	struct celldata *cd = (struct celldata *)arg;

	/* Unlock the mutex */
	ret = pthread_mutex_unlock(&(cd->mtx));
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to unlock mutex in cancel handler");
	}

}

void *worker(void *arg)
{
	int ret;
	struct celldata *cd = (struct celldata *)arg;
	struct timespec ts;

	/* lock the mutex */
	ret = pthread_mutex_lock(&(cd->mtx));
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to lock mutex in worker");
	}

	/* Tell the cellmaster we are ready (count++) */
	cd->count += 1;

	/* Timeout = now + TIMEOUT */
	ret = clock_gettime(cd->cid, &ts);
	if (ret != 0) {
		UNRESOLVED(errno, "Gettime failed");
	}
	ts.tv_sec += TIMEOUT * SCALABILITY_FACTOR;

	/* register cleanup handler */
	pthread_cleanup_push(cleanup, arg);

	do {
		/* cond timedwait (while boolean == false) */
		ret = pthread_cond_timedwait(&(cd->cnd), &(cd->mtx), &ts);

		/* if timeout => failed (lost signal) */
		if (ret == ETIMEDOUT) {
			FAILED
			    ("Timeout occured. A condition signal was probably lost.");
		}

		if (ret != 0) {
			UNRESOLVED(ret, "Cond timedwait failed");
		}

	} while (cd->boolean == 0);

	/* broadcast the condition */
	ret = pthread_cond_broadcast(&(cd->cnd));
	if (ret != 0) {
		UNRESOLVED(ret, "Broadcasting the condition failed");
	}

	/* unregister the cleanup */
	pthread_cleanup_pop(0);

	/* unlock the mutex */
	ret = pthread_mutex_unlock(&(cd->mtx));
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to unlock the mutex");
	}

	return NULL;
}

void *signaler(void *arg)
{
	int ret;
	struct celldata *cd = (struct celldata *)arg;

	/* Lock the mutex if required */
	if (cd->boolean == -1) {
		ret = pthread_mutex_lock(&(cd->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "mutex lock failed in signaler");
		}
	}

	/* wait the barrier */
	ret = pthread_barrier_wait(&(cd->bar));
	if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		UNRESOLVED(ret, "Barrier wait failed");
	}

	/* signal the cond */
	ret = pthread_cond_signal(&(cd->cnd));
	if (ret != 0) {
		UNRESOLVED(ret, "Signaling the cond failed");
	}

	/* Unlock the mutex if required */
	if (cd->boolean == -1) {
		ret = pthread_mutex_unlock(&(cd->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "mutex unlock failed in signaler");
		}
	}

	return NULL;
}

void *cellmanager(void *arg)
{
	int ret, i;
	struct celldata *cd = (struct celldata *)arg;
	struct timespec ts;
	int randval;
	void *w_ret;

	cd->canceled = 0;
	cd->cancelfailed = 0;
	cd->cnttotal = 0;

	/* while do_it */
	while (do_it) {
		/* Initialize some stuff */
		cd->boolean = 0;
		cd->count = 0;
		cd->cnttotal += 1;

		/* create the workers */
		for (i = 0; i < NCHILDREN * SCALABILITY_FACTOR + 2; i++) {
			ret =
			    pthread_create(&(cd->workers[i]), &ta, worker, arg);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to create enough threads");
			}
		}

		/* choose a (pseudo) random thread to cancel */
		ret = clock_gettime(cd->cid, &ts);
		if (ret != 0) {
			UNRESOLVED(errno, "Failed to read clock");
		}
		randval =
		    (ts.tv_sec +
		     (ts.tv_nsec >> 10)) % (NCHILDREN * SCALABILITY_FACTOR + 2);

		/* wait for the workers to be ready */
		do {
			ret = pthread_mutex_lock(&(cd->mtx));
			if (ret != 0) {
				UNRESOLVED(ret, "Mutex lock failed");
			}

			i = cd->count;

			ret = pthread_mutex_unlock(&(cd->mtx));
			if (ret != 0) {
				UNRESOLVED(ret, "Mutex unlock failed");
			}
		} while (i < NCHILDREN * SCALABILITY_FACTOR + 2);

		/* Set the boolean (1 => no lock in signaler; -1 => lock) */
		cd->boolean = (ts.tv_sec & 1) ? -1 : 1;

		/* create the signaler */
		ret = pthread_create(&(cd->signaler), &ta, signaler, arg);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to create signaler thread");
		}

		/* wait the barrier */
		ret = pthread_barrier_wait(&(cd->bar));
		if ((ret != 0) && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
			UNRESOLVED(ret, "Failed to wait for the barrier");
		}

		/* cancel the chosen thread */
		ret = pthread_cancel(cd->workers[randval]);

		/* it is possible the thread is already terminated -- so we don't stop on error */
		if (ret != 0) {
#if VERBOSE > 2
			output("%d\n", randval);
#endif
			cd->cancelfailed += 1;
		}

		/* join every threads */
		ret = pthread_join(cd->signaler, NULL);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to join the signaler thread");
		}

		for (i = 0; i < NCHILDREN * SCALABILITY_FACTOR + 2; i++) {
			ret = pthread_join(cd->workers[i], &w_ret);
			if (ret != 0) {
				UNRESOLVED(ret, "Unable to join a worker");
			}
			if (w_ret == PTHREAD_CANCELED)
				cd->canceled += 1;
		}
	}

	return NULL;
}

void sighdl(int sig)
{
	/* do_it = 0 */
	do {
		do_it = 0;
	}
	while (do_it);
}

int main(int argc, char *argv[])
{
	int ret, i, j;
	struct sigaction sa;

	pthread_mutexattr_t ma;
	pthread_condattr_t ca;
	clockid_t cid = CLOCK_REALTIME;
	long canceled = 0;
	long cancelfailed = 0;
	long cnttotal = 0;

	long pshared, monotonic, cs;

	pthread_t mngrs[NSCENAR * SCALABILITY_FACTOR];

	output_init();

	/* check the system abilities */
	pshared = sysconf(_SC_THREAD_PROCESS_SHARED);
	cs = sysconf(_SC_CLOCK_SELECTION);
	monotonic = sysconf(_SC_MONOTONIC_CLOCK);

#if VERBOSE > 0
	output("Test starting\n");
	output("System abilities:\n");
	output(" TPS : %li\n", pshared);
	output(" CS  : %li\n", cs);
	output(" MON : %li\n", monotonic);
	if ((cs < 0) || (monotonic < 0))
		output("Alternative clock won't be tested\n");
#endif

	if (monotonic < 0)
		cs = -1;

#ifndef USE_ALTCLK
	if (cs > 0)
		output
		    ("Implementation supports the MONOTONIC CLOCK but option is disabled in test.\n");
#endif

	/* Initialize the celldatas according to scenarii */
	for (i = 0; i < NSCENAR; i++) {
#if VERBOSE > 1
		output("[parent] Preparing attributes for: %s\n",
		       scenarii[i].descr);
#ifdef WITHOUT_XOPEN
		output("[parent] Mutex attributes DISABLED -> not used\n");
#endif
#endif

		/* set / reset everything */
		ret = pthread_mutexattr_init(&ma);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "[parent] Unable to initialize the mutex attribute object");
		}
		ret = pthread_condattr_init(&ca);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "[parent] Unable to initialize the cond attribute object");
		}
#ifndef WITHOUT_XOPEN
		/* Set the mutex type */
		ret = pthread_mutexattr_settype(&ma, scenarii[i].m_type);
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Unable to set mutex type");
		}
#if VERBOSE > 1
		output("[parent] Mutex type : %i\n", scenarii[i].m_type);
#endif
#endif

		/* Set the pshared attributes, if supported */
		if ((pshared > 0) && (scenarii[i].mc_pshared != 0)) {
			ret =
			    pthread_mutexattr_setpshared(&ma,
							 PTHREAD_PROCESS_SHARED);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to set the mutex process-shared");
			}
			ret =
			    pthread_condattr_setpshared(&ca,
							PTHREAD_PROCESS_SHARED);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to set the cond var process-shared");
			}
#if VERBOSE > 1
			output("[parent] Mutex & cond are process-shared\n");
#endif
		}
#if VERBOSE > 1
		else {
			output("[parent] Mutex & cond are process-private\n");
		}
#endif

		/* Set the alternative clock, if supported */
#ifdef USE_ALTCLK
		if ((cs > 0) && (scenarii[i].c_clock != 0)) {
			ret = pthread_condattr_setclock(&ca, CLOCK_MONOTONIC);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to set the monotonic clock for the cond");
			}
#if VERBOSE > 1
			output("[parent] Cond uses the Monotonic clock\n");
#endif
		}
#if VERBOSE > 1
		else {
			output("[parent] Cond uses the default clock\n");
		}
#endif
		ret = pthread_condattr_getclock(&ca, &cid);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to get clock from cond attr");
		}
#endif

		/* Initialize all the mutex and condvars which uses those attributes */
		for (j = 0; j < SCALABILITY_FACTOR; j++) {
			cells[i + j * NSCENAR].cid = cid;

			/* initialize the condvar */
			ret =
			    pthread_cond_init(&(cells[i + j * NSCENAR].cnd),
					      &ca);
			if (ret != 0) {
				UNRESOLVED(ret, "Cond init failed");
			}

			/* initialize the mutex */
			ret =
			    pthread_mutex_init(&(cells[i + j * NSCENAR].mtx),
					       &ma);
			if (ret != 0) {
				UNRESOLVED(ret, "Mutex init failed");
			}

			/* initialize the barrier */
			ret =
			    pthread_barrier_init(&(cells[i + j * NSCENAR].bar),
						 NULL, 2);
			if (ret != 0) {
				UNRESOLVED(ret, "Failed to init barrier");
			}
		}

		ret = pthread_condattr_destroy(&ca);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Failed to destroy the cond var attribute object");
		}

		ret = pthread_mutexattr_destroy(&ma);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Failed to destroy the mutex attribute object");
		}
	}
#if VERBOSE > 1
	output("[parent] All condvars & mutex are ready\n");
#endif

	/* register the signal handler */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sighdl;
	if ((ret = sigaction(SIGUSR1, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler");
	}
#if VERBOSE > 1
	output("[parent] Signal handler registered\n");
#endif

	/* Initialize the thread attribute object */
	ret = pthread_attr_init(&ta);
	if (ret != 0) {
		UNRESOLVED(ret,
			   "[parent] Failed to initialize a thread attribute object");
	}
	ret = pthread_attr_setstacksize(&ta, sysconf(_SC_THREAD_STACK_MIN));
	if (ret != 0) {
		UNRESOLVED(ret, "[parent] Failed to set thread stack size");
	}

	/* create the NSCENAR * SCALABILITY_FACTOR manager threads */
	for (i = 0; i < NSCENAR * SCALABILITY_FACTOR; i++) {
		ret = pthread_create(&mngrs[i], &ta, cellmanager, &(cells[i]));
		/* In case of failure we can exit; the child process will die after a while */
		if (ret != 0) {
			UNRESOLVED(ret, "[Parent] Failed to create a thread");
		}
#if VERBOSE > 1
		if ((i % 4) == 0)
			output("[parent] %i manager threads created...\n",
			       i + 1);
#endif
	}

#if VERBOSE > 1
	output("[parent] All %i manager threads are running...\n",
	       NSCENAR * SCALABILITY_FACTOR);
#endif

	/* join the manager threads and destroy the cells */
	for (i = 0; i < NSCENAR * SCALABILITY_FACTOR; i++) {
		ret = pthread_join(mngrs[i], NULL);
		if (ret != 0) {
			UNRESOLVED(ret, "[Parent] Failed to join a thread");
		}

		canceled += cells[i].canceled;
		cancelfailed += cells[i].cancelfailed;
		cnttotal += cells[i].cnttotal;

		ret = pthread_barrier_destroy(&(cells[i].bar));
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to destroy a barrier");
		}

		ret = pthread_cond_destroy(&(cells[i].cnd));
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to destroy a cond");
		}

		ret = pthread_mutex_destroy(&(cells[i].mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to destroy a mutex");
		}
	}

	/* exit */
#if VERBOSE > 0
	output("Test passed\n");
	output("  Total loops          : %8li\n", cnttotal);
#endif
#if VERBOSE > 1
	output("  Failed cancel request: %8li\n", cancelfailed);
	output("  Canceled threads     : %8li\n", canceled);
#endif

	PASSED;
}
