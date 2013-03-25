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
 * The function does not return EINTR

 * The steps are:
 * -> pthread_kill a thread which creates threads
 * -> check that EINTR is never returned

 */

 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

 /* Some routines are part of the XSI Extensions */
#ifndef WITHOUT_XOPEN
#define _XOPEN_SOURCE	600
#endif
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
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

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

#define WITH_SYNCHRO

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

char do_it = 1;
char woken = 0;
unsigned long count_ope = 0;
#ifdef WITH_SYNCHRO
sem_t semsig1;
sem_t semsig2;
unsigned long count_sig = 0;
#endif

sigset_t usersigs;

typedef struct {
	int sig;
#ifdef WITH_SYNCHRO
	sem_t *sem;
#endif
} thestruct;

/* the following function keeps on sending the signal to the process */
void *sendsig(void *arg)
{
	thestruct *thearg = (thestruct *) arg;
	int ret;
	pid_t process;

	process = getpid();

	/* We block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &usersigs, NULL);
	if (ret != 0) {
		UNRESOLVED(ret,
			   "Unable to block SIGUSR1 and SIGUSR2 in signal thread");
	}

	while (do_it) {
#ifdef WITH_SYNCHRO
		if ((ret = sem_wait(thearg->sem))) {
			UNRESOLVED(errno, "Sem_wait in sendsig");
		}
		count_sig++;
#endif

		ret = kill(process, thearg->sig);
		if (ret != 0) {
			UNRESOLVED(errno, "Kill in sendsig");
		}

	}

	return NULL;
}

/* Next are the signal handlers. */
/* This one is registered for signal SIGUSR1 */
void sighdl1(int sig)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&semsig1)) {
		UNRESOLVED(errno, "Sem_post in signal handler 1");
	}
#endif
}

/* This one is registered for signal SIGUSR2 */
void sighdl2(int sig)
{
#ifdef WITH_SYNCHRO
	if (sem_post(&semsig2)) {
		UNRESOLVED(errno, "Sem_post in signal handler 2");
	}
#endif
}

/* Thread function -- almost does nothing */
void *threaded(void *arg)
{
	int ret;

	/* Signal we're done (especially in case of a detached thread) */
	do {
		ret = sem_post(&scenarii[sc].sem);
	}
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1) {
		UNRESOLVED(errno, "Failed to wait for the semaphore");
	}

	/* return */
	return arg;
}

/* Test function -- creates the threads and check that EINTR is never returned. */
void *test(void *arg)
{
	int ret = 0;
	pthread_t child;

	/* We don't block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_UNBLOCK, &usersigs, NULL);
	if (ret != 0) {
		UNRESOLVED(ret,
			   "Unable to unblock SIGUSR1 and SIGUSR2 in worker thread");
	}

	sc = 0;

	while (do_it) {
#if VERBOSE > 5
		output("-----\n");
		output("Starting test with scenario (%i): %s\n", sc,
		       scenarii[sc].descr);
#endif

		count_ope++;

		ret = pthread_create(&child, &scenarii[sc].ta, threaded, NULL);
		if (ret == EINTR) {
			FAILED("pthread_create returned EINTR");
		}
		switch (scenarii[sc].result) {
		case 0:	/* Operation was expected to succeed */
			if (ret != 0) {
				UNRESOLVED(ret, "Failed to create this thread");
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
#if VERBOSE > 5
			if (ret == 0) {
				output
				    ("Thread has been created successfully for this scenario\n");
			} else {
				output
				    ("Thread creation failed with the error: %s\n",
				     strerror(ret));
			}
#endif
			;
		}
		if (ret == 0) {	/* The new thread is running */
			if (scenarii[sc].detached == 0) {
				ret = pthread_join(child, NULL);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Unable to join a thread");
				}
			} else {
				/* Just wait for the thread to terminate */
				do {
					ret = sem_wait(&scenarii[sc].sem);
				}
				while ((ret == -1) && (errno == EINTR));
				if (ret == -1) {
					UNRESOLVED(errno,
						   "Failed to wait for the semaphore");
				}
			}
		}

		/* Change thread attribute for the next loop */
		sc++;
		sc %= NSCENAR;
	}
	return NULL;
}

/* Main function */
int main(void)
{
	int ret;
	pthread_t th_work, th_sig1, th_sig2;
	thestruct arg1, arg2;
	struct sigaction sa;

	/* Initialize output routine */
	output_init();

	/* Initialize thread attribute objects */
	scenar_init();

	/* We need to register the signal handlers for the PROCESS */
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

	/* We prepare a signal set which includes SIGUSR1 and SIGUSR2 */
	sigemptyset(&usersigs);
	ret = sigaddset(&usersigs, SIGUSR1);
	ret |= sigaddset(&usersigs, SIGUSR2);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to add SIGUSR1 or 2 to a signal set");
	}

	/* We now block the signals SIGUSR1 and SIGUSR2 for this THREAD */
	ret = pthread_sigmask(SIG_BLOCK, &usersigs, NULL);
	if (ret != 0) {
		UNRESOLVED(ret,
			   "Unable to block SIGUSR1 and SIGUSR2 in main thread");
	}
#ifdef WITH_SYNCHRO
	if (sem_init(&semsig1, 0, 1)) {
		UNRESOLVED(errno, "Semsig1  init");
	}
	if (sem_init(&semsig2, 0, 1)) {
		UNRESOLVED(errno, "Semsig2  init");
	}
#endif

	if ((ret = pthread_create(&th_work, NULL, test, NULL))) {
		UNRESOLVED(ret, "Worker thread creation failed");
	}

	arg1.sig = SIGUSR1;
	arg2.sig = SIGUSR2;
#ifdef WITH_SYNCHRO
	arg1.sem = &semsig1;
	arg2.sem = &semsig2;
#endif

	if ((ret = pthread_create(&th_sig1, NULL, sendsig, (void *)&arg1))) {
		UNRESOLVED(ret, "Signal 1 sender thread creation failed");
	}
	if ((ret = pthread_create(&th_sig2, NULL, sendsig, (void *)&arg2))) {
		UNRESOLVED(ret, "Signal 2 sender thread creation failed");
	}

	/* Let's wait for a while now */
	sleep(1);

	/* Now stop the threads and join them */
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

	if ((ret = pthread_join(th_work, NULL))) {
		UNRESOLVED(ret, "Worker thread join failed");
	}

	scenar_fini();

#if VERBOSE > 0
	output("Test executed successfully.\n");
	output("  %d thread creations.\n", count_ope);
#ifdef WITH_SYNCHRO
	output("  %d signals were sent meanwhile.\n", count_sig);
#endif
#endif
	PASSED;
}
