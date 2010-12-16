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
* with this program; if not, write the Free Software Foundation, Inc., 59
* Temple Place - Suite 330, Boston MA 02111-1307, USA.

* This sample test aims to check the following assertions:
*
* If the signal action was set with the signal() function, getting it into oact
then reinstalling it with act must be valid.

* The steps are:
* -> register a signal handler for SIGTERM with signal().
* -> check this signal handler works.
* -> change the signal handler with sigaction, saving old handler in oact.
* -> check the new signal handler works.
* -> set the old signal handler back
* -> check the old signal handler still works.

* The test fails if a signal handler does not work as expected.
*/

/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

/******************************************************************************/
/*************************** standard includes ********************************/
/******************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <signal.h>
#include <errno.h>

/******************************************************************************/
/***************************   Test framework   *******************************/
/******************************************************************************/
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
#include "posixtest.h"
#include <time.h>
#include <sys/types.h>

#ifdef __GNUC__ /* We are using GCC */

#define UNRESOLVED(x, s) \
 { output("Test %s unresolved: got %i (%s) on line %i (%s)\n", __FILE__, x, strerror(x), __LINE__, s); \
 	output_fini(); \
 	exit(PTS_UNRESOLVED); }

#define FAILED(s) \
 { output("Test %s FAILED: %s\n", __FILE__, s); \
 	output_fini(); \
 	exit(PTS_FAIL); }

#define PASSED \
  output_fini(); \
  exit(PTS_PASS);

#define UNTESTED(s) \
{	output("File %s cannot test: %s\n", __FILE__, s); \
	  output_fini(); \
  exit(PTS_UNTESTED); \
}

#else /* not using GCC */

#define UNRESOLVED(x, s) \
 { output("Test unresolved: got %i (%s) on line %i (%s)\n", x, strerror(x), __LINE__, s); \
  output_fini(); \
 	exit(PTS_UNRESOLVED); }

#define FAILED(s) \
 { output("Test FAILED: %s\n", s); \
  output_fini(); \
 	exit(PTS_FAIL); }

#define PASSED \
  output_fini(); \
  exit(PTS_PASS);

#define UNTESTED(s) \
{	output("Unable to test: %s\n", s); \
	  output_fini(); \
  exit(PTS_UNTESTED); \
}

#endif
void output_init()
{
	/* do nothing */
	return ;
}

void output(char * string, ...)
{
	va_list ap;
#ifndef PLOT_OUTPUT
	char *ts = "[??:??:??]";

	struct tm * now;
	time_t nw;
#endif

#ifndef PLOT_OUTPUT
	nw = time(NULL);
	now = localtime(&nw);

	if (now == NULL)
		printf(ts);
	else
		printf("[%2.2d:%2.2d:%2.2d]", now->tm_hour, now->tm_min, now->tm_sec);

#endif
	va_start(ap, string);

	vprintf(string, ap);

	va_end(ap);

}

void output_fini()
{
	/*do nothing */
	return ;
}

/******************************************************************************/
/**************************** Configuration ***********************************/
/******************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

#define SIGNAL SIGTERM

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

sig_atomic_t called = 1;

void handler_1(int sig)
{
	called++;
}

void handler_2(int sig)
{
	called--;
}

/* main function */
int main()
{
	int ret;

	struct sigaction sa, save;

	/* Initialize output */
	output_init();

	/* Register the signal handler with signal */

	if (SIG_ERR == signal(SIGNAL, handler_1))
	{
		UNRESOLVED(errno, "Failed to register signal handler with signal()");
	}

	/* As whether signal handler is restored to default when executed
	is implementation defined, we cannot check it was registered here. */

	/* Set the new signal handler with sigaction*/
	sa.sa_flags = 0;

	sa.sa_handler = handler_2;

	ret = sigemptyset(&sa.sa_mask);

	if (ret != 0)
	{
		UNRESOLVED(ret, "Failed to empty signal set");
	}

	/* Install the signal handler for SIGTERM */
	ret = sigaction(SIGNAL, &sa, &save);

	if (ret != 0)
	{
		UNRESOLVED(ret, "Failed to set signal handler");
	}

	/* Check the signal handler has been set up */
	ret = raise(SIGNAL);

	if (ret != 0)
	{
		UNRESOLVED(ret , "Failed to raise the signal");
	}

	if (called != 0)
	{
		FAILED("handler not executed");
	}

	/* Restore the first signal handler */
	ret = sigaction(SIGNAL, &save, 0);

	if (ret != 0)
	{
		UNRESOLVED(ret, "Failed to set signal handler");
	}

	/* Check the signal handler has been set up */
	ret = raise(SIGNAL);

	if (ret != 0)
	{
		UNRESOLVED(ret , "Failed to raise the signal");
	}

	if (called != 1)
	{
		FAILED("handler not executed");
	}

	/* Test passed */
#if VERBOSE > 0

	output("Test passed\n");

#endif

	PASSED;
}