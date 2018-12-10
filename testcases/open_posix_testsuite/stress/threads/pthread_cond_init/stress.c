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

 * This file is a stress test for the pthread_cond_init function.

 * The steps are:
 * -> Create some threads
 * -> each thread loops on initializing and destroying a condition variable
 * -> the whole process stop when receiving signal SIGUSR1
 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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
#define N 20

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/
char do_it = 1;
pthread_mutex_t cnt_mtx = PTHREAD_MUTEX_INITIALIZER;
unsigned long long cnt = 0;

/******** Threads function *********/
void *threaded(void *arg)
{
	int me = (int)arg;
	pthread_cond_t cnd;
	pthread_cond_t *pcnd;
	pthread_condattr_t ca;
	pthread_condattr_t *pca;
	struct timespec now;
	pthread_mutex_t mtx, *pmtx;
	int ret;
	int sz = 4;
	/* We will use mutex from the stack or from malloc'ed memory */
	char loc = ((me % 5) % 2);

	pmtx = &mtx;

	if (loc) {
		pcnd = &cnd;
	} else {
		pcnd = malloc(sizeof(pthread_cond_t));
		if (pcnd == NULL) {
			UNRESOLVED(errno,
				   "Memory allocation for condvar failed");
		}
	}

	me %= sz;

	ret = clock_gettime(CLOCK_REALTIME, &now);
	if (ret != 0) {
		UNRESOLVED(errno, "Clock get time (realtime) failed.");
	}

	switch (me) {
	case 0:		/* We will initialize the cond with NULL pointer attribute */
		pca = NULL;
		break;

	default:		/* We will initialize the cond with an attribute object */
		if ((ret = pthread_condattr_init(&ca))) {
			UNRESOLVED(ret, "Cond attribute init failed");
		}
		pca = &ca;

		if ((me == 1) || (me == 3)) {
			ret =
			    pthread_condattr_setpshared(&ca,
							PTHREAD_PROCESS_SHARED);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to set PShared condvar");
			}
		}

		if ((sysconf(_SC_MONOTONIC_CLOCK) > 0)
		    && ((me == 1) || (me == 2))) {
			ret = pthread_condattr_setclock(&ca, CLOCK_MONOTONIC);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to attribute monotonic clock to condvar");
			}

			ret = clock_gettime(CLOCK_MONOTONIC, &now);
			if (ret != 0) {
				UNRESOLVED(errno,
					   "Clock get time (realtime) failed.");
			}
		}

	}

	ret = pthread_mutex_init(pmtx, NULL);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to initialize a mutex");
	}
	ret = pthread_mutex_lock(pmtx);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to lock a mutex");
	}

	while (do_it) {
		ret = pthread_cond_init(pcnd, pca);
		if (ret != 0) {
			FAILED("Cond init failed");
		}
		/* We use the mutex to check everything is OK */
		ret = pthread_cond_timedwait(pcnd, pmtx, &now);
		if (ret != ETIMEDOUT) {
			if (ret == 0) {
				FAILED("");
			} else {
				UNRESOLVED(ret,
					   "Cond timedwait returned unexpected error code");
			}
		}
		ret = pthread_cond_signal(pcnd);
		if (ret != 0) {
			FAILED("Cond signal failed");
		}
		ret = pthread_cond_broadcast(pcnd);
		if (ret != 0) {
			FAILED("Cond broadcast failed");
		}
		ret = pthread_cond_destroy(pcnd);
		if (ret != 0) {
			FAILED("Cond destroy failed");
		}
		ret = pthread_mutex_lock(&cnt_mtx);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to lock counter mutex");
		}
		cnt++;
		ret = pthread_mutex_unlock(&cnt_mtx);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to unlock counter mutex");
		}
	}

	if (!loc)		/* cond was malloc'ed */
		free(pcnd);

	if (me)
		if ((ret = pthread_condattr_destroy(pca))) {
			FAILED("Cond attribute destroy failed at the end");
		}

	return NULL;
}

/******** Signal handler ************/
void sighdl(int sig)
{
	do {
		do_it = 0;
	}
	while (do_it);
}

/******** Parent thread *************/
int main(int argc, char *argv[])
{
	struct sigaction sa;
	pthread_t threads[N * SCALABILITY_FACTOR];
	int i;
	int ret;

	output_init();

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sighdl;
	if ((ret = sigaction(SIGUSR1, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler");
	}

	for (i = 0; (i < (N * SCALABILITY_FACTOR) && (ret == 0)); i++) {
		ret = pthread_create(&threads[i], NULL, threaded, (void *)i);
	}
	if (ret != 0) {		/* A thread creation failed */
		/* Stop the started threads */
		do {
			do_it = 0;
		}
		while (do_it);
		for (; i > 0; i--)
			pthread_join(threads[i - 1], NULL);

		UNRESOLVED(ret, "Unable to create enough threads");
	}

	/* Every threads were created; we now just wait */
	for (i = 0; i < (N * SCALABILITY_FACTOR); i++) {
		if ((ret = pthread_join(threads[i], NULL))) {
			FAILED("Unable to join a thread");
		}
	}
#if VERBOSE > 0
	output("pthread_cond_init stress test passed (%llu iterations)\n", cnt);
#endif

	/* Everything went OK */
	PASSED;
}
