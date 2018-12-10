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
 *  While a thread is blocked on a conditionnal variable,
 * a dynamic binding exists between this conditionnal variable
 * and the mutex which was the second argument.
 *  This dynamic binding stops existing when the last thread is unblocked.
 *  Even if the conditionnal variable can then be reused with another mutex,
 * the threads which have been unblocked must still acquire
 * the mutex they had associated with the conditionnal variable at call time.
 *

 * The steps are:
 * -> Create two mutexes m1 and m2 (errorcheck or recursive)
 * -> Create a condition variable c depending on a bootlean b
 * -> create N threads which will
 *     -> lock m1
 *     -> wait or timedwait c, m1 (while b is false)
 *     -> check it owns m1 (check depends on the mutex type)
 *     -> lock m2
 *     -> wait or timedwait c, m2 (while b is false)
 *     -> check it owns m2
 *     -> mark this thread as terminate
 * -> Once all threads are waiting on (c,m1),
 *     mark b as true then broadcast c until all threads are terminated.
 */

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

#define NTHREADS (100)

#ifndef WITHOUT_ALTCLK
#define USE_ALTCLK		/* make tests with MONOTONIC CLOCK if supported */
#endif

#ifndef WITHOUT_XOPEN

struct _td {
	pthread_mutex_t mtx1, mtx2;	/* The two mutex m1 and m2 */
	pthread_cond_t cnd;	/* The cond var c */
	char boolcnd;		/* The boolean predicate b associated with c */
	int type;		/* Type of mutex */
	clockid_t cid;		/* Clock used by cond c */
	int started;		/* # of threads which are already waiting */
	int stopped;		/* # of threads which are terminated */
} data;

void *threaded(void *arg)
{
	int ret;

	struct timespec ts;

	/* Prepare the timeout parameter */
	ret = clock_gettime(data.cid, &ts);
	if (ret != 0)
		UNRESOLVED(ret, "Unable to get time from clock");
	ts.tv_sec += 30;

	/* Lock m1 */
	ret = pthread_mutex_lock(&(data.mtx1));
	if (ret != 0)
		UNRESOLVED(ret, "Unable tu lock m1 in thread");

	/* Tell the parent this thread is started */
	data.started++;

	/* wait for the cond - bind the cond to the mutex m1 */
	do {
		if (arg == NULL)
			ret = pthread_cond_wait(&(data.cnd), &(data.mtx1));
		else
			ret = pthread_cond_timedwait(&(data.cnd), &(data.mtx1),
						     &ts);
	} while ((ret == 0) && (data.boolcnd == 0));
	if (ret != 0)
		UNRESOLVED(ret, "First wait failed in thread");

	/* Test ownership and unlock m1 */
	if (data.type == PTHREAD_MUTEX_RECURSIVE) {
		ret = pthread_mutex_trylock(&(data.mtx1));
		if (ret != 0)
			FAILED
			    ("Unable to re-lock recursive mutex after cond wait");
		ret = pthread_mutex_unlock(&(data.mtx1));
		if (ret != 0)
			UNRESOLVED(ret, "Mutex unlock failed");
	}

	ret = pthread_mutex_unlock(&(data.mtx1));
	if (ret != 0)
		FAILED("Unable to unlock m1 in thread - not owner?");

	ret = pthread_mutex_unlock(&(data.mtx1));
	if (ret == 0)
		FAILED("Unlocking an unlocked mutex succeeded");

	/* Lock m2 */
	ret = pthread_mutex_lock(&(data.mtx2));
	if (ret != 0)
		UNRESOLVED(ret, "Unable tu lock m2 in thread");

	/* wait for the cond - bind the cond to the mutex m2 */
	do {
		if (arg == NULL)
			ret = pthread_cond_wait(&(data.cnd), &(data.mtx2));
		else
			ret = pthread_cond_timedwait(&(data.cnd), &(data.mtx2),
						     &ts);
	} while ((ret == 0) && (data.boolcnd == 0));
	if (ret != 0)
		UNRESOLVED(ret, "Second wait failed in thread");

	/* Mark the thread as terminated while we are protected by m2 */
	data.stopped++;

	/* Test ownership and unlock m2 */
	if (data.type == PTHREAD_MUTEX_RECURSIVE) {
		ret = pthread_mutex_trylock(&(data.mtx2));
		if (ret != 0)
			FAILED
			    ("Unable to re-lock recursive mutex after cond wait");
		ret = pthread_mutex_unlock(&(data.mtx2));
		if (ret != 0)
			UNRESOLVED(ret, "Mutex unlock failed");
	}
	ret = pthread_mutex_unlock(&(data.mtx2));
	if (ret != 0)
		FAILED("Unable to unlock m2 in thread - not owner?");
	ret = pthread_mutex_unlock(&(data.mtx2));
	if (ret == 0)
		FAILED("Unlocking an unlocked mutex succeeded");

	return NULL;
}

int main(void)
{
	int ret, j;
	unsigned int i;
	pthread_mutexattr_t ma;
	pthread_condattr_t ca;
	pthread_t th[NTHREADS];
	int loc_started, loc_stopped;

	long altclk_ok, pshared_ok;

	struct {
		char altclk;	/* Want to use alternative clock */
		char pshared;	/* Want to use process-shared primitives */
		int type;	/* mutex type */
		char *descr;	/* Description of the case */

	} scenar[] = {
		{
		0, 0, PTHREAD_MUTEX_RECURSIVE, "Recursive mutex"}, {
		0, 0, PTHREAD_MUTEX_ERRORCHECK, "Errorcheck mutex"},
#ifdef USE_ALTCLK
		{
		1, 0, PTHREAD_MUTEX_RECURSIVE,
			    "Recursive mutex + altclock cond"}, {
		1, 0, PTHREAD_MUTEX_ERRORCHECK,
			    "Errorcheck mutex + altclock cond"}, {
		1, 1, PTHREAD_MUTEX_RECURSIVE,
			    "Recursive pshared mutex + altclock cond"}, {
		1, 1, PTHREAD_MUTEX_ERRORCHECK,
			    "Errorcheck pshared mutex + altclock cond"},
#endif
		{
		0, 1, PTHREAD_MUTEX_RECURSIVE, "Recursive pshared mutex"}, {
	0, 1, PTHREAD_MUTEX_ERRORCHECK,
			    "Errorcheck pshared mutex"},};

	output_init();

	/* Initialize the constants */
	altclk_ok = sysconf(_SC_CLOCK_SELECTION);
	if (altclk_ok > 0)
		altclk_ok = sysconf(_SC_MONOTONIC_CLOCK);

#ifndef USE_ALTCLK
	if (altclk_ok > 0)
		output("Implementation supports the MONOTONIC CLOCK "
		       "but option is disabled in test.\n");
#endif

	pshared_ok = sysconf(_SC_THREAD_PROCESS_SHARED);

#if VERBOSE > 0
	output("Test starting\n");
	output(" Process-shared primitive %s be tested\n",
	       (pshared_ok > 0) ? "will" : "won't");
	output(" Alternative clock for cond %s be tested\n",
	       (altclk_ok > 0) ? "will" : "won't");
#endif

	for (i = 0; i < (sizeof(scenar) / sizeof(scenar[0])); i++) {
#if VERBOSE > 1
		output("Starting test for %s\n", scenar[i].descr);
#endif

		/* Initialize the data structure */
		ret = pthread_mutexattr_init(&ma);
		if (ret != 0)
			UNRESOLVED(ret, "Mutex attribute object init failed");

		ret = pthread_mutexattr_settype(&ma, scenar[i].type);
		if (ret != 0)
			UNRESOLVED(ret, "Unable to set mutex type");

		if ((pshared_ok > 0) && (scenar[i].pshared != 0)) {
			ret =
			    pthread_mutexattr_setpshared(&ma,
							 PTHREAD_PROCESS_SHARED);
			if (ret != 0)
				UNRESOLVED(ret,
					   "Unable to set mutex process-shared");
		}

		ret = pthread_condattr_init(&ca);
		if (ret != 0)
			UNRESOLVED(ret, "Cond attribute object init failed");

		if ((pshared_ok > 0) && (scenar[i].pshared != 0)) {
			ret =
			    pthread_condattr_setpshared(&ca,
							PTHREAD_PROCESS_SHARED);
			if (ret != 0)
				UNRESOLVED(ret,
					   "Unable to set cond process-shared");
		}
#ifdef USE_ALTCLK
		if ((altclk_ok > 0) && (scenar[i].altclk != 0)) {
			ret = pthread_condattr_setclock(&ca, CLOCK_MONOTONIC);
			if (ret != 0)
				UNRESOLVED(ret,
					   "Unable to set alternative (monotonic) clock for cond");
		}
#endif

		ret = pthread_mutex_init(&(data.mtx1), &ma);
		if (ret != 0)
			UNRESOLVED(ret, "Unable to init mutex 1");

		ret = pthread_mutex_init(&(data.mtx2), &ma);
		if (ret != 0)
			UNRESOLVED(ret, "Unable to init mutex 2");

		ret = pthread_cond_init(&(data.cnd), &ca);
		if (ret != 0)
			UNRESOLVED(ret, "Unable to initialize condvar");

		data.boolcnd = 0;

		ret = pthread_mutexattr_gettype(&ma, &(data.type));
		if (ret != 0)
			UNRESOLVED(ret, "Unable to get type from mutex attr");

#ifdef USE_ALTCLK
		ret = pthread_condattr_getclock(&ca, &(data.cid));
		if (ret != 0)
			UNRESOLVED(ret,
				   "Unable to get clock ID from cond attr");
#else
		data.cid = CLOCK_REALTIME;
#endif

		data.started = 0;
		data.stopped = 0;

		/* Start the threads */
#if VERBOSE > 1
		output("Initialization OK, starting threads\n");
#endif
		for (j = 0; j < NTHREADS; j++) {
			ret =
			    pthread_create(&th[j], NULL, threaded,
					   (void *)(long)(j & 1));
			if (ret != 0)
				UNRESOLVED(ret, "Thread creation failed");
		}

		/* Wait for the threads to be started */
		do {
			ret = pthread_mutex_lock(&(data.mtx1));
			if (ret != 0)
				UNRESOLVED(ret, "Unable to lock m1 in parent");
			loc_started = data.started;
			ret = pthread_mutex_unlock(&(data.mtx1));
			if (ret != 0)
				UNRESOLVED(ret,
					   "Unable to unlock m1 in parent");
		} while (loc_started < NTHREADS);

		/* Broadcast the condition until all threads are terminated */
		data.boolcnd = 1;
		do {
			ret = pthread_cond_broadcast(&(data.cnd));
			if (ret != 0)
				UNRESOLVED(ret, "Unable to broadcast cnd");
			sched_yield();
			ret = pthread_mutex_lock(&(data.mtx2));
			if (ret != 0)
				UNRESOLVED(ret, "Unable to lock m2 in parent");
			loc_stopped = data.stopped;
			ret = pthread_mutex_unlock(&(data.mtx2));
			if (ret != 0)
				UNRESOLVED(ret,
					   "Unable to unlock m2 in parent");
		} while (loc_stopped < NTHREADS);

		/* Join the threads */
		for (j = 0; j < NTHREADS; j++) {
			ret = pthread_join(th[j], NULL);
			if (ret != 0)
				UNRESOLVED(ret, "Thread join failed");
		}

#if VERBOSE > 1
		output("Test passed for %s\n", scenar[i].descr);
#endif

		/* Destroy data */
		ret = pthread_cond_destroy(&(data.cnd));
		if (ret != 0)
			UNRESOLVED(ret, "Cond destroy failed");

		ret = pthread_mutex_destroy(&(data.mtx1));
		if (ret != 0)
			UNRESOLVED(ret, "Mutex 1 destroy failed");

		ret = pthread_mutex_destroy(&(data.mtx2));
		if (ret != 0)
			UNRESOLVED(ret, "Mutex 2 destroy failed");

		ret = pthread_condattr_destroy(&ca);
		if (ret != 0)
			UNRESOLVED(ret, "Cond attribute destroy failed");

		ret = pthread_mutexattr_destroy(&ma);
		if (ret != 0)
			UNRESOLVED(ret, "Mutex attr destroy failed");
	}			/* Proceed to next case */

	PASSED;
}

#else /* WITHOUT_XOPEN */
int main(void)
{
	output_init();
	UNTESTED("This test requires XSI features");
}
#endif
