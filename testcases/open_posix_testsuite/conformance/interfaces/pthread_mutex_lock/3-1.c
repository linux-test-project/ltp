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

 * This sample test aims to check the following assertion:
 * The function does not return an error code of EINTR

 * The steps are:
 *
 * -> Create a thread which loops on pthread_mutex_lock and pthread_mutex_unlock
 *      operations.
 * -> Create another thread which loops on sending a signal to the first thread.
 *
 *
 */

 /*
  * - adam.li@intel.com 2004-05-13
  *   Add to PTS. Please refer to http://nptl.bullopensource.org/phpBB/
  *   for general information
  */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

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
#define WITH_SYNCHRO
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/
char do_it = 1;
unsigned long count_ope = 0;
pthread_mutex_t count_protect = PTHREAD_MUTEX_INITIALIZER;
#ifdef WITH_SYNCHRO
sem_t semsig1;
sem_t semsig2;
unsigned long count_sig = 0;
#endif
sem_t semsync;
sem_t semsync2;

typedef struct {
	pthread_t *thr;
	int sig;
#ifdef WITH_SYNCHRO
	sem_t *sem;
#endif
} thestruct;

/* the following function keeps on sending the signal to the thread pointed by arg
 *  If WITH_SYNCHRO is defined, the target thread has a handler for the signal */
void *sendsig(void *arg)
{
	thestruct *thearg = (thestruct *) arg;
	int ret;
	while (do_it) {
#ifdef WITH_SYNCHRO
		if ((ret = sem_wait(thearg->sem))) {
			UNRESOLVED(errno, "Sem_wait in sendsig");
		}
		count_sig++;
#endif

		if ((ret = pthread_kill(*(thearg->thr), thearg->sig))) {
			UNRESOLVED(ret, "Pthread_kill in sendsig");
		}

	}

	return NULL;
}

/* Next are the signal handlers. */
void sighdl1(int sig LTP_ATTRIBUTE_UNUSED)
{
#ifdef WITH_SYNCHRO
	if ((sem_post(&semsig1))) {
		UNRESOLVED(errno, "Sem_post in signal handler 1");
	}
#endif
}

void sighdl2(int sig LTP_ATTRIBUTE_UNUSED)
{
#ifdef WITH_SYNCHRO
	if ((sem_post(&semsig2))) {
		UNRESOLVED(errno, "Sem_post in signal handler 2");
	}
#endif
}

/* The following function loops on init/destroy some mutex (with different attributes)
 * it does check that no error code of EINTR is returned */

void *threaded(void *arg LTP_ATTRIBUTE_UNUSED)
{
	pthread_mutexattr_t ma[4], *pma[5];
	pthread_mutex_t m[5];
	int i;
	int ret;

	/* We need to register the signal handlers */
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sighdl1;
	if ((ret = sigaction(SIGUSR1, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler1");
	}
	sa.sa_handler = sighdl2;
	if ((ret = sigaction(SIGUSR2, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler2");
	}

	if ((sem_post(&semsync2))) {
		UNRESOLVED(errno, "could not post semsync2");
	}

	/* Initialize the different mutex */
	pma[4] = NULL;

	for (i = 0; i < 4; i++) {
		pma[i] = &ma[i];
		if ((ret = pthread_mutexattr_init(pma[i]))) {
			UNRESOLVED(ret, "pthread_mutexattr_init");
		}
	}

#ifndef WITHOUT_XOPEN
	if ((ret = pthread_mutexattr_settype(pma[0], PTHREAD_MUTEX_NORMAL))) {
		UNRESOLVED(ret, "pthread_mutexattr_settype (normal)");
	}
	if ((ret = pthread_mutexattr_settype(pma[1], PTHREAD_MUTEX_ERRORCHECK))) {
		UNRESOLVED(ret, "pthread_mutexattr_settype (errorcheck)");
	}
	if ((ret = pthread_mutexattr_settype(pma[2], PTHREAD_MUTEX_RECURSIVE))) {
		UNRESOLVED(ret, "pthread_mutexattr_settype (recursive)");
	}
	if ((ret = pthread_mutexattr_settype(pma[3], PTHREAD_MUTEX_DEFAULT))) {
		UNRESOLVED(ret, "pthread_mutexattr_settype (default)");
	}
#if VERBOSE >1
	output
	    ("Mutex attributes NORMAL,ERRORCHECK,RECURSIVE,DEFAULT initialized\n");
#endif
#else
#if VERBOSE > 0
	output
	    ("Mutex attributes NORMAL,ERRORCHECK,RECURSIVE,DEFAULT unavailable\n");
#endif
#endif

	for (i = 0; i < 5; i++) {
		ret = pthread_mutex_init(&m[i], pma[i]);
		if (ret == EINTR) {
			FAILED("pthread_mutex_init returned EINTR");
		}
		if (ret != 0) {
			UNRESOLVED(ret, "pthread_mutex_init failed");
		}
	}
	/* The mutex are ready, we will loop on lock/unlock now */

	while (do_it) {
		for (i = 0; i < 5; i++) {
			ret = pthread_mutex_lock(&m[i]);
			if (ret == EINTR) {
				FAILED("pthread_mutex_lock returned EINTR");
			}
			if (ret != 0) {
				UNRESOLVED(ret, "pthread_mutex_lock failed");
			}
			ret = pthread_mutex_unlock(&m[i]);
			if (ret == EINTR) {
				FAILED("pthread_mutex_unlock returned EINTR");
			}
			if (ret != 0) {
				UNRESOLVED(ret, "pthread_mutex_unlock failed");
			}
		}
		ret = pthread_mutex_lock(&count_protect);
		if (ret == EINTR) {
			FAILED("pthread_mutex_lock returned EINTR");
		}
		if (ret != 0) {
			UNRESOLVED(ret, "pthread_mutex_lock failed");
		}
		count_ope++;
		pthread_mutex_unlock(&count_protect);
		if (ret == EINTR) {
			FAILED("pthread_mutex_unlock returned EINTR");
		}
		if (ret != 0) {
			UNRESOLVED(ret, "pthread_mutex_unlock failed");
		}
	}

	/* Now we can destroy the mutex objects */
	for (i = 0; i < 4; i++) {
		if ((ret = pthread_mutexattr_destroy(pma[i]))) {
			UNRESOLVED(ret, "pthread_mutexattr_init");
		}
	}
	for (i = 0; i < 5; i++) {
		ret = pthread_mutex_destroy(&m[i]);
		if (ret == EINTR) {
			FAILED("pthread_mutex_destroy returned EINTR");
		}
		if (ret != 0) {
			UNRESOLVED(ret, "pthread_mutex_destroy failed");
		}
	}

	do {
		ret = sem_wait(&semsync);
	} while (ret && (errno == EINTR));
	if (ret) {
		UNRESOLVED(errno, "Could not wait for sig senders termination");
	}

	return NULL;
}

/* At last (but not least) we need a main */
int main(void)
{
	int ret;
	pthread_t th_work, th_sig1, th_sig2;
	thestruct arg1, arg2;

	output_init();

#ifdef WITH_SYNCHRO
#if VERBOSE >1
	output("Running in synchronized mode\n");
#endif
	if ((sem_init(&semsig1, 0, 1))) {
		UNRESOLVED(errno, "Semsig1  init");
	}
	if ((sem_init(&semsig2, 0, 1))) {
		UNRESOLVED(errno, "Semsig2  init");
	}
#endif

	if ((sem_init(&semsync, 0, 0))) {
		UNRESOLVED(errno, "semsync init");
	}

	if ((sem_init(&semsync2, 0, 0))) {
		UNRESOLVED(errno, "semsync2 init");
	}
#if VERBOSE >1
	output("Starting the worker thread\n");
#endif
	if ((ret = pthread_create(&th_work, NULL, threaded, NULL))) {
		UNRESOLVED(ret, "Worker thread creation failed");
	}

	do {
		ret = sem_wait(&semsync2);
	} while (ret && (errno == EINTR));

	arg1.thr = &th_work;
	arg2.thr = &th_work;
	arg1.sig = SIGUSR1;
	arg2.sig = SIGUSR2;
#ifdef WITH_SYNCHRO
	arg1.sem = &semsig1;
	arg2.sem = &semsig2;
#endif

#if VERBOSE >1
	output("Starting the signal sources\n");
#endif
	if ((ret = pthread_create(&th_sig1, NULL, sendsig, (void *)&arg1))) {
		UNRESOLVED(ret, "Signal 1 sender thread creation failed");
	}
	if ((ret = pthread_create(&th_sig2, NULL, sendsig, (void *)&arg2))) {
		UNRESOLVED(ret, "Signal 2 sender thread creation failed");
	}

	/* Let's wait for a while now */
#if VERBOSE >1
	output("Let the worker be killed for a second\n");
#endif
	sleep(1);

	/* Now stop the threads and join them */
#if VERBOSE >1
	output("Stop everybody\n");
#endif
	do {
		do_it = 0;
	}
	while (do_it);

	if ((ret = pthread_join(th_sig1, NULL))) {
		UNRESOLVED(ret, "Signal 1 sender thread join failed");
	}
	if ((ret = pthread_join(th_sig2, NULL))) {
		UNRESOLVED(ret, "Signal 2 sender thread join failed");
	}
#if VERBOSE >1
	output("Signal sources are stopped, we can stop the worker\n");
#endif
	if ((sem_post(&semsync))) {
		UNRESOLVED(errno, "could not post semsync");
	}

	if ((ret = pthread_join(th_work, NULL))) {
		UNRESOLVED(ret, "Worker thread join failed");
	}
#if VERBOSE > 0
	output("Test executed successfully.\n");
	output("  %d mutex lock and unlock were done.\n", count_ope);
#ifdef WITH_SYNCHRO
	output("  %d signals were sent meanwhile.\n", count_sig);
#endif
#endif
	PASSED;
}
