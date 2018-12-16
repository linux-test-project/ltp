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

* This sample test aims to check the following assertion:
*
* The function sets the scheduling policy and parameter of the specified thread.

* The steps are:
* -> Create a new thread
* -> create another thread which changes the scheduling policy of the 1st thread

* The test fails if the 1st thread policy is not changed.
*/

/******************************************************************************/
/*********************** standard includes ************************************/
/******************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sched.h>
#include <errno.h>

/******************************************************************************/
/***********************   Test framework   ***********************************/
/******************************************************************************/
#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"
/* This header is responsible for defining the following macros:
 * UNRESOLVED(ret, descr);
 *    where descr is a description of the error and ret is an int
 *   (error code for example)
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

/******************************************************************************/
/***************************** Configuration **********************************/
/******************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/******************************************************************************/
/*****************************    Test case   *********************************/
/******************************************************************************/

/* This function checks the thread policy & priority */
void check_param(pthread_t thread, int policy, int priority)
{
	int ret = 0;

	int t_pol;

	struct sched_param t_parm;

	/* Check the priority is valid */

	if (priority == -1) {
		UNRESOLVED(errno, "Wrong priority value");
	}

	/* Get the thread's parameters */
	ret = pthread_getschedparam(thread, &t_pol, &t_parm);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to get thread's parameters");
	}

	if (t_pol != policy) {
		FAILED("The thread's policy is not as expected");
	}

	if (t_parm.sched_priority != priority) {
		FAILED("The thread's priority is not as expected");
	}
}

/* thread function 1 */
void *controler(void *arg)
{
	int ret = 0;

	/* Wait until the policy has been changed. */
	ret = pthread_barrier_wait(arg);

	if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD) {
		UNRESOLVED(ret, "barrier wait failed");
	}

	/* check the thread attributes have been applied
	   (we only check what is reported, not the real behavior)
	 */
	check_param(pthread_self(), SCHED_RR, sched_get_priority_min(SCHED_RR));

	return NULL;
}

/* thread function 2 */
void *changer(void *arg)
{
	int ret = 0;

	struct sched_param sp;
	sp.sched_priority = sched_get_priority_min(SCHED_RR);

	if (sp.sched_priority < 0) {
		UNTESTED("Failed to get min SCHED_RR range");
	}

	/* set the other thread's policy */
	ret = pthread_setschedparam(*(pthread_t *) arg, SCHED_RR, &sp);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to set other's thread policy");
	}

	return NULL;
}

/* The main test function. */
int main(void)
{
	int ret = 0;
	pthread_t tcontrol, tchange;
	pthread_barrier_t bar;

	/* Initialize output routine */
	output_init();

	ret = pthread_barrier_init(&bar, NULL, 2);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to init barrier");
	}

	/* Create the controler thread */
	ret = pthread_create(&tcontrol, NULL, controler, &bar);

	if (ret != 0) {
		UNRESOLVED(ret, "thread creation failed");
	}

	/* Now create the changer thread */
	ret = pthread_create(&tchange, NULL, changer, &tcontrol);

	if (ret != 0) {
		UNRESOLVED(ret, "thread creation failed");
	}

	/* wait until the changer finishes */
	ret = pthread_join(tchange, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join the thread");
	}

	/* let the controler control */
	ret = pthread_barrier_wait(&bar);

	if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD) {
		UNRESOLVED(ret, "barrier wait failed");
	}

	ret = pthread_join(tcontrol, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join the thread");
	}

	ret = pthread_barrier_destroy(&bar);

	if (ret != 0) {
		UNRESOLVED(ret, "barrier destroy failed");
	}

	PASSED;
}
