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
 * This function is a cancelation point: when cancelability
 * is PTHREAD_CANCEL_DEFERRED and a cancel request falls, the thread
 * must relock the mutex before the first (if any) clean up handler is called.

 * The steps are:
 * -> Create a thread
 *   ->  this thread locks a mutex then waits for a condition
 * -> cancel the thread
 *   -> the cancelation handler will test if the thread owns the mutex.
 */

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <time.h>
#include <semaphore.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

#ifndef WITHOUT_ALTCLK
#define USE_ALTCLK		/* make tests with MONOTONIC CLOCK if supported */
#endif

struct {
	pthread_mutex_t mtx;
	pthread_cond_t cnd;
	int type;
	clockid_t cid;
	sem_t semA;
	sem_t semB;
	int bool;
} data;

/****  First handler that will be poped
 *  This one works only with recursive mutexes
 */
void clnp1(void *arg)
{
	int ret;

	(void) arg;

	if (data.type == PTHREAD_MUTEX_RECURSIVE) {
		ret = pthread_mutex_trylock(&(data.mtx));
		if (ret != 0) {
			FAILED
			    ("Unable to double-lock a recursive mutex in clean-up handler 1");
		}
		ret = pthread_mutex_unlock(&(data.mtx));
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to unlock double-locked recursive mutex in clean-up handler 1");
		}
	}
	return;
}

/**** Second handler
 *  This one will trigger an action in main thread, while we are owning the mutex
 */
void clnp2(void *arg)
{
	int ret;

	(void) arg;

	do {
		ret = sem_post(&(data.semA));
	} while ((ret != 0) && (errno == EINTR));
	if (ret != 0) {
		UNRESOLVED(errno, "Sem post failed in cleanup handler 2");
	}

	do {
		ret = sem_wait(&(data.semB));
	} while ((ret != 0) && (errno == EINTR));
	if (ret != 0) {
		UNRESOLVED(errno, "Sem wait failed in cleanup handler 2");
	}

	return;
}

/**** Third handler
 *  Will actually unlock the mutex, then try to unlock second time to check an error is returned
 */
void clnp3(void *arg)
{
	int ret;

	(void) arg;

	ret = pthread_mutex_unlock(&(data.mtx));
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to unlock mutex in clean-up handler 3");
	}

	if ((data.type == PTHREAD_MUTEX_ERRORCHECK)
	    || (data.type == PTHREAD_MUTEX_RECURSIVE)) {
		ret = pthread_mutex_unlock(&(data.mtx));
		if (ret == 0) {
			UNRESOLVED(ret,
				   "Was able to unlock unlocked mutex in clean-up handler 3");
		}
	}

	return;
}

/**** Thread function
 * This function will lock the mutex, then install the cleanup handlers
 * and wait for the cond. At this point it will be canceled.
 */
void *threaded(void *arg)
{
	int ret;

	(void) arg;

	struct timespec ts;

	ret = clock_gettime(data.cid, &ts);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to get time from clock");
	}

	ts.tv_sec += 30;

	ret = pthread_mutex_lock(&(data.mtx));
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to lock the mutex in thread");
	}

	do {
		ret = sem_post(&(data.semA));
	} while ((ret != 0) && (errno == EINTR));
	if (ret != 0) {
		UNRESOLVED(errno, "Sem post failed in thread");
	}

	pthread_cleanup_push(clnp3, NULL);
	pthread_cleanup_push(clnp2, NULL);
	pthread_cleanup_push(clnp1, NULL);

	do {
		ret = pthread_cond_timedwait(&(data.cnd), &(data.mtx), &ts);
	} while ((ret == 0) && (data.bool == 0));

	if (ret != 0) {
		UNRESOLVED(ret, "Timedwait failed");
	}

	/* We will exit even if the error is timedwait */
	/* If we are here, the thread was not canceled */
	FAILED("The thread has not been canceled");

	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(1);

	return NULL;
}

int main(void)
{
	int ret;
	unsigned int i;
	void *rc;

	pthread_mutexattr_t ma;
	pthread_condattr_t ca;
	pthread_t th;

	long altclk_ok, pshared_ok;
	struct timespec processing_completion_ts = {0, 100000};

	struct {
		char altclk;	/* Want to use alternative clock */
		char pshared;	/* Want to use process-shared primitives */
		int type;	/* mutex type */
		char *descr;	/* Description of the case */

	} scenar[] = { {
	0, 0, PTHREAD_MUTEX_NORMAL, "Normal mutex"}
#ifdef USE_ALTCLK
	, {
	1, 0, PTHREAD_MUTEX_NORMAL, "Normal mutex + altclock cond"}
	, {
	1, 1, PTHREAD_MUTEX_NORMAL, "PShared mutex + altclock cond"}
#endif
	, {
	0, 1, PTHREAD_MUTEX_NORMAL, "Pshared mutex"}
#ifndef WITHOUT_XOPEN
	, {
	0, 0, PTHREAD_MUTEX_ERRORCHECK, "Errorcheck mutex"}
	, {
	0, 0, PTHREAD_MUTEX_RECURSIVE, "Recursive mutex"}
#ifdef USE_ALTCLK
	, {
	1, 0, PTHREAD_MUTEX_RECURSIVE, "Recursive mutex + altclock cond"}
	, {
	1, 0, PTHREAD_MUTEX_ERRORCHECK,
		    "Errorcheck mutex + altclock cond"}
	, {
	1, 1, PTHREAD_MUTEX_RECURSIVE,
		    "Recursive pshared mutex + altclock cond"}
	, {
	1, 1, PTHREAD_MUTEX_ERRORCHECK,
		    "Errorcheck pshared mutex + altclock cond"}
#endif
	, {
	0, 1, PTHREAD_MUTEX_RECURSIVE, "Recursive pshared mutex"}
	, {
	0, 1, PTHREAD_MUTEX_ERRORCHECK, "Errorcheck pshared mutex"}
#endif
	};

	output_init();

	/* Initialize the constants */
	altclk_ok = sysconf(_SC_CLOCK_SELECTION);
	if (altclk_ok > 0)
		altclk_ok = sysconf(_SC_MONOTONIC_CLOCK);
#ifndef USE_ALTCLK
	if (altclk_ok > 0)
		output
		    ("Implementation supports the MONOTONIC CLOCK but option is disabled in test.\n");
#endif

	pshared_ok = sysconf(_SC_THREAD_PROCESS_SHARED);

#if VERBOSE > 0
	output("Test starting\n");
	output(" Process-shared primitive %s be tested\n",
	       (pshared_ok > 0) ? "will" : "won't");
	output(" Alternative clock for cond %s be tested\n",
	       (altclk_ok > 0) ? "will" : "won't");
#endif

	ret = sem_init(&(data.semA), 0, 0);
	if (ret != 0) {
		UNRESOLVED(errno, "Unable to init sem A");
	}

	ret = sem_init(&(data.semB), 0, 0);
	if (ret != 0) {
		UNRESOLVED(errno, "Unable to init sem B");
	}

	for (i = 0; i < (sizeof(scenar) / sizeof(scenar[0])); i++) {
#if VERBOSE > 1
		output("Starting test for %s\n", scenar[i].descr);
#endif

		/* Initialize the data structure */
		ret = pthread_mutexattr_init(&ma);
		if (ret != 0) {
			UNRESOLVED(ret, "Mutex attribute object init failed");
		}

		ret = pthread_mutexattr_settype(&ma, scenar[i].type);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to set mutex type");
		}

		if ((pshared_ok > 0) && (scenar[i].pshared != 0)) {
			ret =
			    pthread_mutexattr_setpshared(&ma,
							 PTHREAD_PROCESS_SHARED);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to set mutex process-shared");
			}
		}

		ret = pthread_condattr_init(&ca);
		if (ret != 0) {
			UNRESOLVED(ret, "Cond attribute object init failed");
		}

		if ((pshared_ok > 0) && (scenar[i].pshared != 0)) {
			ret =
			    pthread_condattr_setpshared(&ca,
							PTHREAD_PROCESS_SHARED);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to set cond process-shared");
			}
		}
#ifdef USE_ALTCLK
		if ((altclk_ok > 0) && (scenar[i].altclk != 0)) {
			ret = pthread_condattr_setclock(&ca, CLOCK_MONOTONIC);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to set alternative (monotonic) clock for cond");
			}
		}
#endif

		ret = pthread_mutex_init(&(data.mtx), &ma);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to init mutex");
		}

		ret = pthread_cond_init(&(data.cnd), &ca);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to initialize condvar");
		}

		ret = pthread_mutexattr_gettype(&ma, &(data.type));
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to get type from mutex attr");
		}
#ifdef USE_ALTCLK
		ret = pthread_condattr_getclock(&ca, &(data.cid));
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to get clock ID from cond attr");
		}
#else
		data.cid = CLOCK_REALTIME;
#endif

		data.bool = 0;

		/** Data is ready, create the thread */
#if VERBOSE > 1
		output("Initialization OK, starting thread\n");
#endif

		ret = pthread_create(&th, NULL, threaded, NULL);
		if (ret != 0) {
			UNRESOLVED(ret, "Thread creation failed");
		}

		/** Wait for the thread to be waiting */
		do {
			ret = sem_wait(&(data.semA));
		} while ((ret != 0) && (errno == EINTR));
		if (ret != 0) {
			UNRESOLVED(errno, "Sem wait failed in main");
		}

		ret = pthread_mutex_lock(&(data.mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to lock mutex in main");
		}

		data.bool = 1;

		/** Cancel the thread */
		ret = pthread_cancel(th);
		if (ret != 0) {
			UNRESOLVED(ret, "Thread cancelation failed");
		}

		sched_yield();
		nanosleep(&processing_completion_ts, NULL);

		ret = pthread_mutex_unlock(&(data.mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to unlock mutex in main");
		}

		/** Wait for the thread to be executing second cleanup handler */
		do {
			ret = sem_wait(&(data.semA));
		} while ((ret != 0) && (errno == EINTR));
		if (ret != 0) {
			UNRESOLVED(errno, "Sem wait failed in main");
		}

		/** Here the child should own the mutex, we check this */
		ret = pthread_mutex_trylock(&(data.mtx));
		if (ret == 0) {
			FAILED
			    ("The child did not own the mutex inside the cleanup handler");
		}

		/** Let the cleanups go on */
		do {
			ret = sem_post(&(data.semB));
		} while ((ret != 0) && (errno == EINTR));
		if (ret != 0) {
			UNRESOLVED(errno, "Sem post failed in main");
		}

		/** Join the thread */
		ret = pthread_join(th, &rc);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to join the thread");
		}
		if (rc != PTHREAD_CANCELED) {
			FAILED("thread was not canceled");
		}
#if VERBOSE > 1
		output("Test passed for %s\n", scenar[i].descr);
#endif

		/* Destroy datas */
		ret = pthread_cond_destroy(&(data.cnd));
		if (ret != 0) {
			UNRESOLVED(ret, "Cond destroy failed");
		}

		ret = pthread_mutex_destroy(&(data.mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Mutex destroy failed");
		}

		ret = pthread_condattr_destroy(&ca);
		if (ret != 0) {
			UNRESOLVED(ret, "Cond attribute destroy failed");
		}

		ret = pthread_mutexattr_destroy(&ma);
		if (ret != 0) {
			UNRESOLVED(ret, "Mutex attr destroy failed");
		}
	}			/* Proceed to next case */

	ret = sem_destroy(&(data.semA));
	if (ret != 0) {
		UNRESOLVED(errno, "Sem destroy failed");
	}

	ret = sem_destroy(&(data.semB));
	if (ret != 0) {
		UNRESOLVED(errno, "Sem destroy failed");
	}

	PASSED;
}
