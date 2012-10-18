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
* with this program; if not, write the Free Software Foundation, Inc., 59
* Temple Place - Suite 330, Boston MA 02111-1307, USA.

* This sample test aims to check the following assertion:
*
* Interval timers are reset in the child process.

* The steps are:
* -> create an interval timer in the parent process
* -> fork
* -> check the timer has been cleared in child.

* The test fails if the timer is running in the child.

*/

/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

/* The interval timers are an XSI feature */
#ifndef WITHOUT_XOPEN
#define _XOPEN_SOURCE 600
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

#include <sys/wait.h>
 #include <errno.h>

#include <sys/time.h>

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
#ifndef WITHOUT_XOPEN
/* The main test function. */
int main(int argc, char * argv[])
{
	int ret, status;
	pid_t child, ctl;

	struct itimerval it;

	/* Initialize output */
	output_init();

	/* Create the interval timer */
	it.it_interval.tv_sec = 15;
	it.it_interval.tv_usec = 0;
	it.it_value.tv_sec = 10;
	it.it_value.tv_usec = 0;

	ret = setitimer(ITIMER_REAL, &it, NULL);

	if (ret != 0)
	{
		UNRESOLVED(errno, "Failed to set interval timer for ITIMER_REAL");
	}

	ret = setitimer(ITIMER_VIRTUAL, &it, NULL);

	if (ret != 0)
	{
		UNRESOLVED(errno, "Failed to set interval timer for ITIMER_VIRTUAL");
	}

	ret = setitimer(ITIMER_PROF, &it, NULL);

	if (ret != 0)
	{
		UNRESOLVED(errno, "Failed to set interval timer for ITIMER_PROF");
	}

#if VERBOSE > 0
	output("All interval timers are set.\n");

#endif

	/* Create the child */
	child = fork();

	if (child == -1)
	{
		UNRESOLVED(errno, "Failed to fork");
	}

	/* child */
	if (child == 0)
	{
		/* Check we get the correct information: timer is reset */
		ret = getitimer(ITIMER_REAL, &it);

		if (ret != 0)
		{
			UNRESOLVED(errno, "Failed to read ITIMER_REAL in child");
		}

		if (it.it_value.tv_sec != 0)
		{
			FAILED("Timer ITIMER_REAL was not reset in child");
		}

		ret = getitimer(ITIMER_VIRTUAL, &it);

		if (ret != 0)
		{
			UNRESOLVED(errno, "Failed to read ITIMER_VIRTUAL in child");
		}

		if (it.it_value.tv_sec != 0)
		{
			FAILED("Timer ITIMER_VIRTUAL was not reset in child");
		}

		ret = getitimer(ITIMER_PROF, &it);

		if (ret != 0)
		{
			UNRESOLVED(errno, "Failed to read ITIMER_PROF in child");
		}

		if (it.it_value.tv_sec != 0)
		{
			FAILED("Timer ITIMER_PROF was not reset in child");
		}

		/* We're done */
		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child)
	{
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status) || (WEXITSTATUS(status) != PTS_PASS))
	{
		FAILED("Child exited abnormally");
	}

	/* Test passed */
#if VERBOSE > 0

	output("Test passed\n");

#endif

	PASSED;
}

#else /* WITHOUT_XOPEN */
int main(int argc, char * argv[])
{
	output_init();
	UNTESTED("This testcase requires XSI features");
}
#endif
