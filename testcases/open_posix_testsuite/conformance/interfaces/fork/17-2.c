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
* For the SCHED_FIFO and SCHED_RR scheduling policies,
* the child process inherits the policy and priority
* settings of the parent process during a fork() function.

* The steps are:
* -> Change the parent's scheduling policy and parameter
* -> fork
* -> check the child inherited the same policy.

* The test fails if the child does not inherit the parent's values.

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

#include <sys/wait.h>
#include <errno.h>

#include <sched.h>

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

#define POLICY SCHED_FIFO

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/
/* The main test function. */
int main(void)
{
	int ret, param, status;
	pid_t child, ctl;

	struct sched_param sp;

	/* Initialize output */
	output_init();

	/* Change process policy and parameters */
	sp.sched_priority = param = sched_get_priority_max(POLICY);

	if (sp.sched_priority == -1) {
		UNRESOLVED(errno, "Failed to get max priority value");
	}

	ret = sched_setscheduler(0, POLICY, &sp);

	if (ret == -1) {
		UNRESOLVED(errno, "Failed to change process scheduling policy");
	}

	/* Create the child */
	child = fork();

	if (child == -1) {
		UNRESOLVED(errno, "Failed to fork");
	}

	/* child */
	if (child == 0) {

		/* Check the scheduling policy */
		ret = sched_getscheduler(0);

		if (ret == -1) {
			UNRESOLVED(errno,
				   "Failed to read scheduling policy in child");
		}

		if (ret != POLICY) {
			FAILED("The scheduling policy was not inherited");
		}

		ret = sched_getparam(0, &sp);

		if (ret != 0) {
			UNRESOLVED(errno,
				   "Failed to read scheduling parameter in child");
		}

		if (sp.sched_priority != param) {
			FAILED("The scheduling parameter was not inherited");
		}

		/* We're done */
		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status) || (WEXITSTATUS(status) != PTS_PASS)) {
		FAILED("Child exited abnormally");
	}

	/* Test passed */
#if VERBOSE > 0
	output("Test passed\n");

#endif
	PASSED;
}
