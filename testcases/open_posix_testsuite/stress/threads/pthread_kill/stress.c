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

* This stress test aims to test the following assertions:
*  -> pthread_kill() does not make the system unstable
*  -> no signal get lost when they are not already pending.

* The steps are:
* -> create 2 threads which send signals heavily to a 3rd thread.
* -> Create another thread which sends a signal synchronously to another one.

* The test fails if a signal is lost. The other assertion is tested implicitely

*/

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <semaphore.h>
#include <errno.h>
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

/********************************************************************************************/
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

char do_it = 1;
long long iterations = 0;

/* Handler for user request to terminate */
void sighdl(int sig)
{
	/* do_it = 0 */

	do {
		do_it = 0;
	}
	while (do_it);
}

void floodsighdl(int sig)
{
	/* Nothing to do */
	return;
}

/* Signals flood receiver thread */
void *flood_receiver(void *arg)
{
	int ret = 0;
	/* register the signal handler for this one thread */

	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = floodsighdl;

	if ((ret = sigaction(SIGABRT, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler");
	}

	if ((ret = sigaction(SIGBUS, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler");
	}

	/* Wait for the other threads to terminate */

	do {
		sched_yield();
	} while (*(int *)arg);

	return NULL;
}

/* Signal flood threads */
void *flooder_1(void *arg)
{
	int ret = 0;

	while (do_it) {
		iterations++;
		ret = pthread_kill(*(pthread_t *) arg, SIGABRT);

		if (ret != 0) {
			UNRESOLVED(ret, "Flooder 1 thread got an error");
		}
	}

	return NULL;
}

void *flooder_2(void *arg)
{
	int ret = 0;

	while (do_it) {
		iterations++;
		ret = pthread_kill(*(pthread_t *) arg, SIGBUS);

		if (ret != 0) {
			UNRESOLVED(ret, "Flooder 1 thread got an error");
		}
	}

	return NULL;
}

/* Synchronized threads */
int sync;
void syncsighdl(int sig)
{
	/* signal we have been called */
	sync = 1;
	return;
}

void *sync_rec(void *arg)
{
	int ret = 0;

	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = syncsighdl;

	if ((ret = sigaction(SIGILL, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler");
	}

	/* wait until termination */
	do {
		sched_yield();
	} while (*(int *)arg);

	return NULL;
}

void *sync_send(void *arg)
{
	int ret = 0;

	while (do_it) {
		/* Disarm the flag */
		sync = 0;
		/* Send the signal */
		ret = pthread_kill(*(pthread_t *) arg, SIGILL);

		if (ret != 0) {
			UNRESOLVED(ret, "Failed to send signal");
		}

		/* Sleep up to 5 sec */
		for (ret = 0; (ret < 5) && (sync == 0); ret++)
			sleep(1);

		/* Test if signal was received */
		if (sync == 0) {
			FAILED
			    ("Signal SIGILL was not delivered within 5 second -- lost?");
		}
	}

	return NULL;

}

/* Main function */
int main(int argc, char *argv[])
{
	int ret = 0;

	int flooding = 1;
	pthread_t fl_rec;
	pthread_t fl_snd1, fl_snd2;

	int synchro = 1;
	pthread_t sy_rec;
	pthread_t sy_snd;

	struct sigaction sa;

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

	ret = pthread_create(&fl_rec, NULL, flood_receiver, &flooding);

	if (ret != 0) {
		UNRESOLVED(ret, "Unable to create a thread");
	}

	ret = pthread_create(&fl_snd1, NULL, flooder_1, &fl_rec);

	if (ret != 0) {
		UNRESOLVED(ret, "Unable to create a thread");
	}

	ret = pthread_create(&fl_snd2, NULL, flooder_2, &fl_rec);

	if (ret != 0) {
		UNRESOLVED(ret, "Unable to create a thread");
	}

	ret = pthread_create(&sy_rec, NULL, sync_rec, &synchro);

	if (ret != 0) {
		UNRESOLVED(ret, "Unable to create a thread");
	}

	ret = pthread_create(&sy_snd, NULL, sync_send, &sy_rec);

	if (ret != 0) {
		UNRESOLVED(ret, "Unable to create a thread");
	}

	/* Wait the user stops the test */
	ret = pthread_join(fl_snd1, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join a thread");
	}

	ret = pthread_join(fl_snd2, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join a thread");
	}

	flooding = 0;
	ret = pthread_join(fl_rec, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join a thread");
	}

	ret = pthread_join(sy_snd, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join a thread");
	}

	synchro = 0;

	ret = pthread_join(sy_rec, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join a thread");
	}

	/* We've been asked to stop */
	output("pthread_kill stress test PASSED -- %llu iterations\n",
	       iterations);

	PASSED;
}
