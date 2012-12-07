/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
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

* This stress test aims to test the following assertion:
*  sem_getvalue always returns the value of the semaphore at a given time
* during the call into the sval argument.

* The steps are:
* -> Create a named semaphore and an unnamed semaphore, initialized to 0.
* -> create two threads which continuously post and wait these semaphores.
* -> main loops on sem_getvalue on these two semaphores.

* The test fails if sem_getvalue gets a value different from 0 or 1.

*/

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

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

#ifndef VERBOSE
#define VERBOSE 1
#endif

#define SEM_NAME "/set_getval_stress"

#define INIT_VAL 0

char do_it = 1;
long long iterations = 0;

/* Handler for user request to terminate */
void sighdl(int sig)
{
	/* do_it = 0 */

	do {
		do_it = 0;
	} while (do_it);
}

/* Thread function */
void *threaded(void *arg)
{
	int ret = 0;

	do {
		/* sem_post */
		ret = sem_post(arg);

		if (ret != 0) {
			UNRESOLVED(errno, "Failed to post the semaphore");
		}

		/* sem wait */
		do {
			ret = sem_wait(arg);
		} while ((ret != 0) && (errno == EINTR));

		if (ret != 0) {
			UNRESOLVED(errno, "Failed to wait for the semaphore");
		}

	} while (do_it);

	return NULL;
}

/* Main function */
int main(int argc, char *argv[])
{
	int ret = 0, value;

	struct sigaction sa;

	pthread_t child1, child2;

	sem_t unnamed, *named;

	/* Initialize output routine */
	output_init();

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

	/* Initialize the semaphores */
	named = sem_open(SEM_NAME, O_CREAT, 0777, INIT_VAL);

	if (named == SEM_FAILED) {
		UNRESOLVED(errno, "Failed to sem_open");
	}

	ret = sem_unlink(SEM_NAME);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to sem_unlink");
	}

	ret = sem_init(&unnamed, 0, INIT_VAL);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to sem_init");
	}

	/* Create the threads */
	ret = pthread_create(&child1, NULL, threaded, named);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to create a thread");
	}

	ret = pthread_create(&child2, NULL, threaded, &unnamed);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to create a thread");
	}

	/* loop */
	while (do_it) {
		ret = sem_getvalue(named, &value);

		if (ret != 0) {
			UNRESOLVED(errno, "Failed to get sem value");
		}

		if ((value != INIT_VAL) && (value != INIT_VAL + 1)) {
			output("Got value %d, expected %d or %d only\n",
			       value, INIT_VAL, INIT_VAL + 1);
			FAILED
			    ("sem_getvalue returned an invalid value for the named semaphore");
		}

		ret = sem_getvalue(&unnamed, &value);

		if (ret != 0) {
			UNRESOLVED(errno, "Failed to get sem value");
		}

		if ((value != INIT_VAL) && (value != INIT_VAL + 1)) {
			output("Got value %d, expected %d or %d only\n",
			       value, INIT_VAL, INIT_VAL + 1);
			FAILED
			    ("sem_getvalue returned an invalid value for the unnamed semaphore");
		}

		iterations++;
	}

	/* Join the threads */
	ret = pthread_join(child1, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join a thread");
	}

	ret = pthread_join(child2, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join a thread");
	}

	/* Destroy the semaphores */
	ret = sem_close(named);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to close the semaphore");
	}

	ret = sem_destroy(&unnamed);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to destroy the semaphore");
	}

	/* Passed */
	output("pthread_exit stress test PASSED -- %llu iterations\n",
	       iterations);

	PASSED;
}
