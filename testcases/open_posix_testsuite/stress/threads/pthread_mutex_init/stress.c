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

 * This file is a stress test for the pthread_mutex_init function.

 * The steps are:
 * -> Create some threads
 * -> each thread loops on initializing and destroying a mutex
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
#define VERBOSE 2
#endif
#define N 20

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/
char do_it = 1;
#ifndef WITHOUT_XOPEN
int types[] = { PTHREAD_MUTEX_NORMAL,
	PTHREAD_MUTEX_ERRORCHECK,
	PTHREAD_MUTEX_RECURSIVE,
	PTHREAD_MUTEX_DEFAULT
};
#endif

/******** Threads function *********/
void *threaded(void *arg)
{
	int me = (int)arg;
	pthread_mutex_t mtx;
	pthread_mutex_t *pmtx;
	pthread_mutexattr_t ma;
	pthread_mutexattr_t *pma;
	int ret;
#ifndef WITHOUT_XOPEN
	int sz = 2 + (sizeof(types) / sizeof(int));
#else
	int sz = 2;
#endif
	/* We will use mutex from the stack or from malloc'ed memory */
	char loc = ((me % 5) % 2);

	if (loc) {
		pmtx = &mtx;
	} else {
		pmtx = malloc(sizeof(pthread_mutex_t));
		if (pmtx == NULL) {
			UNRESOLVED(errno, "Memory allocation for mutex failed");
		}
	}

	me %= sz;

	switch (me) {
	case 0:		/* We will initialize the mutex with NULL pointer */
		pma = NULL;
		break;

	default:		/* We will initialize the mutex with an attribute object */
		if ((ret = pthread_mutexattr_init(&ma))) {
			UNRESOLVED(ret, "Mutex attribute init failed");
		}
		pma = &ma;

		if (me == 1)
			break;

		if ((ret = pthread_mutexattr_settype(&ma, types[me - 2]))) {
			UNRESOLVED(ret, "Mutex attribute settype failed");
		}
	}

	while (do_it) {
		ret = pthread_mutex_init(pmtx, pma);
		if (ret != 0) {
			FAILED("Mutex init failed");
		}
		/* We use the mutex to check everything is OK */
		ret = pthread_mutex_lock(pmtx);
		if (ret != 0) {
			FAILED("Mutex lock failed");
		}
		ret = pthread_mutex_unlock(pmtx);
		if (ret != 0) {
			FAILED("Mutex unlock failed");
		}
		ret = pthread_mutex_destroy(pmtx);
		if (ret != 0) {
			FAILED("Mutex destroy failed");
		}
	}

	if (!loc)		/* mutex was malloc'ed */
		free(pmtx);

	if (me)
		if ((ret = pthread_mutexattr_destroy(pma))) {
			FAILED("Mutex attribute destroy failed at the end");
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
	output("pthread_mutex_init stress test passed\n");
#endif

	/* Everything went OK */
	PASSED;
}
