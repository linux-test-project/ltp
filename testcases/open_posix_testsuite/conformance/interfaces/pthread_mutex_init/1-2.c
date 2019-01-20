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
 * If the mutex attribute pointer passed to pthread_mutex_init is NULL,
 * the effects on the mutex are the same as if a default mutex attribute object had been passed.
 *
 * The steps are:
 *  * create two mutexes. One is initialized with NULL attribute, the other with a default attribute object.
 *  * Compare the following features between the two mutexes:
 *      -> Can it cause / detect a deadlock? (attempt to lock a mutex the thread already owns).
 *            If detected, do both mutexes cause the same error code?
 *      -> Is an error returned when unlocking the mutex in unlocked state?
 *            When unlocking the mutex owned by another thread?
 *
 * The test will pass if the results of each feature are the same for the two mutexes
 * (making no assumption on what is the default behavior).
 * The test will be unresolved if any initialization fails.
 * The test will fail if a feature differs between the two mutex objects.
 */

 /*
  * - adam.li@intel.com 2004-05-09
  *   Add to PTS. Please refer to http://nptl.bullopensource.org/phpBB/
  *   for general information
  */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>

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
/***********************************    Test case   *****************************************/
/********************************************************************************************/

/**** global variables ****/
pthread_mutex_t *p_mtx;
int retval = 0;
int returned = 0;
int canceled = 0;
sem_t semA, semB;

/***** Cancelation handlers  *****/
void cleanup_deadlk(void *arg LTP_ATTRIBUTE_UNUSED)
{
	canceled = 1;
	pthread_mutex_unlock(p_mtx);
}

/***** Threads functions *****/
void *deadlk_issue(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int ret, tmp;

	if ((ret = pthread_mutex_lock(p_mtx))) {
		UNRESOLVED(ret, "First mutex lock in deadlk_issue");
	}
	pthread_cleanup_push(cleanup_deadlk, NULL);
	if ((ret = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &tmp))) {
		UNRESOLVED(ret, "Set cancel type in deadlk_issue");
	}
	if ((ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &tmp))) {
		UNRESOLVED(ret, "Set cancel state in deadlk_issue");
	}
#if VERBOSE >1
	output("Thread releases the semaphore...\n");
#endif
	if ((ret = sem_post(&semA))) {
		UNRESOLVED(errno, "Sem_post in deadlk_issue");
	}

	returned = 0;
	retval = pthread_mutex_lock(p_mtx);
	returned = 1;

	if ((ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &tmp))) {
		UNRESOLVED(ret, "Set cancel state in deadlk_issue");
	}
	pthread_cleanup_pop(0);
	return NULL;
}

void *unlock_issue(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int ret;

#if VERBOSE >1
	output("Locking in child...\n");
#endif
	if ((ret = pthread_mutex_lock(p_mtx))) {
		UNRESOLVED(ret, "First mutex lock in unlock_issue");
	}

	if ((ret = sem_post(&semA))) {
		UNRESOLVED(errno, "Sem_post in unlock_issue");
	}

	if ((ret = sem_wait(&semB))) {
		UNRESOLVED(errno, "Sem_wait in unlock_issue");
	}

	if (retval != 0) {	/* parent thread failed to unlock the mutex) */
#if VERBOSE >1
		output("Unlocking in child...\n");
#endif
		if ((ret = pthread_mutex_unlock(p_mtx))) {
			FAILED
			    ("Mutex unlock returned an error but mutex is unlocked.");
		}
	}

	return NULL;
}

/***** main program *****/
int main(void)
{
	pthread_mutex_t mtx_null, mtx_def;
	pthread_mutexattr_t mattr;
	pthread_t thr;

	pthread_mutex_t *tab_mutex[2] = { &mtx_null, &mtx_def };
	int tab_res[2][3] = { {0, 0, 0}, {0, 0, 0} };

	int ret;
	void *th_ret;

	int i;

	output_init();

#if VERBOSE >1
	output("Test starting...\n");
#endif

	/* We first initialize the two mutexes. */
	if ((ret = pthread_mutex_init(&mtx_null, NULL))) {
		UNRESOLVED(ret, "NULL mutex init");
	}

	if ((ret = pthread_mutexattr_init(&mattr))) {
		UNRESOLVED(ret, "Mutex attribute init");
	}
	if ((ret = pthread_mutex_init(&mtx_def, &mattr))) {
		UNRESOLVED(ret, "Default attribute mutex init");
	}

	if ((ret = pthread_mutexattr_destroy(&mattr))) {
		UNRESOLVED(ret, "Mutex attribute destroy");
	}

	if ((ret = sem_init(&semA, 0, 0))) {
		UNRESOLVED(errno, "Sem A init");
	}
	if ((ret = sem_init(&semB, 0, 0))) {
		UNRESOLVED(errno, "Sem B init");
	}
#if VERBOSE >1
	output("Data initialized...\n");
#endif

	/* OK let's go for the first part of the test : abnormals unlocking */

	/* We first check if unlocking an unlocked mutex returns an error. */
	retval = pthread_mutex_unlock(tab_mutex[0]);
	ret = pthread_mutex_unlock(tab_mutex[1]);
#if VERBOSE >0
	output
	    ("Results for unlock issue #1:\n mutex 1 unlocking returned %i\n mutex 2 unlocking returned %i\n",
	     retval, ret);
#endif
	if (ret != retval) {
		FAILED("Unlocking an unlocked mutex behaves differently.");
	}

	/* Now we focus on unlocking a mutex lock by another thread */
	for (i = 0; i < 2; i++) {
		p_mtx = tab_mutex[i];
		tab_res[i][0] = 0;
		tab_res[i][1] = 0;
		tab_res[i][2] = 0;

#if VERBOSE >1
		output("Creating thread (unlock)...\n");
#endif

		if ((ret = pthread_create(&thr, NULL, unlock_issue, NULL))) {
			UNRESOLVED(ret, "Unlock issue thread create");
		}

		if ((ret = sem_wait(&semA))) {
			UNRESOLVED(errno,
				   "Sem A wait failed for unlock issue.");
		}
#if VERBOSE >1
		output("Unlocking in parent...\n");
#endif
		retval = pthread_mutex_unlock(p_mtx);

		if ((ret = sem_post(&semB))) {
			UNRESOLVED(errno,
				   "Sem B post failed for unlock issue.");
		}

		if ((ret = pthread_join(thr, &th_ret))) {
			UNRESOLVED(ret, "Join thread");
		}
#if VERBOSE >1
		output("Thread joined successfully...\n");
#endif

		tab_res[i][0] = retval;
	}
#if VERBOSE >0
	output
	    ("Results for unlock issue #2:\n mutex 1 returned %i\n mutex 2 returned %i\n",
	     tab_res[0][0], tab_res[1][0]);
#endif

	if (tab_res[0][0] != tab_res[1][0]) {
		FAILED("Unlocking an unowned mutex behaves differently.");
	}

	/* We now are going to test the deadlock issue
	 */

	/* We start with testing the NULL mutex features */
	for (i = 0; i < 2; i++) {
		p_mtx = tab_mutex[i];
		tab_res[i][0] = 0;
		tab_res[i][1] = 0;
		tab_res[i][2] = 0;

#if VERBOSE >1
		output("Creating thread (deadlk)...\n");
#endif

		if ((ret = pthread_create(&thr, NULL, deadlk_issue, NULL))) {
			UNRESOLVED(ret, "Deadlk_issue thread create");
		}

		/* Now we are waiting the thread is ready to relock the mutex. */
		if ((ret = sem_wait(&semA))) {
			UNRESOLVED(errno, "Sem wait");
		}

		/* To ensure thread runs until second lock, we yield here */
		sched_yield();

		/* OK, now we cancel the thread */
		canceled = 0;
#if VERBOSE >1
		output("Cancel thread...\n");
#endif
		if (returned == 0)
			if ((ret = pthread_cancel(thr))) {
				UNRESOLVED(ret, "Cancel thread (deadlk_issue)");
			}
#if VERBOSE >1
		output("Thread canceled...\n");
#endif

		if ((ret = pthread_join(thr, &th_ret))) {
			UNRESOLVED(ret, "Join thread");
		}
#if VERBOSE >1
		output("Thread joined successfully...\n");
#endif

		tab_res[i][2] = retval;
		tab_res[i][1] = returned;
		tab_res[i][0] = canceled;
	}

	/* Now we parse the results */
#if VERBOSE >0
	output
	    ("Results for deadlock issue:\n mutex 1 \t%s\t%s%i\n mutex 2 \t%s\t%s%i\n",
	     tab_res[0][0] ? "deadlock" : "no deadlock",
	     tab_res[0][1] ? "returned " : "did not return ", tab_res[0][2],
	     tab_res[1][0] ? "deadlock" : "no deadlock",
	     tab_res[1][1] ? "returned " : "did not return ", tab_res[1][2]);
#endif

	if (tab_res[0][0] != tab_res[1][0]) {
		FAILED("One mutex deadlocks, not the other");
	}

	if (tab_res[0][1] != tab_res[1][1]) {
		UNRESOLVED(tab_res[0][1], "Abnormal situation!");
	}

	if ((tab_res[0][1] == 1) && (tab_res[0][2] != tab_res[1][2])) {
		FAILED("The locks returned different error codes.");
	}

	PASSED;
}
