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
* If the function fails, the policy and parameter of the target thread
* shall not be modified.

* The steps are:
* -> Create a new thread
* -> Change its priority to a known valid value.
* -> Change its priority to an invalid value.

* The test fails if the priority is changed and an error returned.
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

/* thread function */
void *threaded(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int ret = 0;

	struct sched_param sp;

	/* Set priority to a known value */
	sp.sched_priority = sched_get_priority_max(SCHED_RR);

	ret = pthread_setschedparam(pthread_self(), SCHED_RR, &sp);

	if (ret != 0) {
		UNRESOLVED(ret,
			   "Failed to set thread policy -- need to be root?");
	}

	/* check the thread attributes have been applied
	   (we only check what is reported, not the real behavior)
	 */
	check_param(pthread_self(), SCHED_RR, sp.sched_priority);

	/* Now set the priority to an invalid value. */
	sp.sched_priority++;

	ret = pthread_setschedparam(pthread_self(), SCHED_RR, &sp);

	if (ret != 0) {
		/* check the thread attributes have been applied
		   (we only check what is reported, not the real behavior)
		 */
		check_param(pthread_self(), SCHED_RR, sp.sched_priority - 1);
#if VERBOSE > 0
		output
		    ("Setting to a wrong priority failed with error %d (%s).\n",
		     ret, strerror(ret));
	} else {
		output("UNTESTED: setting to max prio + 1 did not fail.\n");
#endif

	}

	return NULL;
}

/* The main test function. */
int main(void)
{
	int ret = 0;
	pthread_t child;

	/* Initialize output routine */
	output_init();

	/* Create the controler thread */
	ret = pthread_create(&child, NULL, threaded, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "thread creation failed");
	}

	ret = pthread_join(child, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join the thread");
	}
#if VERBOSE > 0
	output("Test PASSED.\n");

#endif

	PASSED;
}
