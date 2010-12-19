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
* The new process is a copy of the original one -- with few exceptions.

* The steps are:
* -> set up some data in process memory space
* -> create a new process
* -> check in this new process that the memory space was copied.
*
* We check that:
* -> a structure object is copied.
* -> a malloc'ed memory block is copied and can be freed in child.
* -> the environment is copied
* -> signal handlers are copied

* The test fails if a difference is detected.

*/

/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

/* We can use XSI features in case it is allowed */
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
/***********************************    Test case   *****************************************/
/********************************************************************************************/

void handler(int sig)
{
	/* this won't be used */
	output("Sig %d received\n", sig);

	return ;
}

/* The main test function. */
int main(int argc, char * argv[])
{
	int ret, status;
	pid_t child, ctl;

	/* structure for structure test */

	struct _thestruct
	{
		char one ;
		short two;
		int three;
		void * four;
	}

	mystruct = {1, 2, 3, (void *) 4};

	/* pointer for malloc'ed memory */
	void * malloced;

	/* Structure for signal handler test */

	struct sigaction sa_ori, sa_child;

	/* Initialize output */
	output_init();

	/* Initialize the memory pointer */
	malloced = (void *) malloc(sysconf(_SC_PAGESIZE));

	if (malloced == NULL)
	{
		UNRESOLVED(errno, "Unable to alloc memory");
	}

	*(double *) malloced = 2.3;

	/* Initialize an environment variable */
	ret = setenv("OPTS_FORK_TC", "2-1.c", 1);

	if (ret != 0)
	{
		UNRESOLVED(errno, "Failed to set the environment variable");
	}

	/* Initialize the signal handler */
	sa_ori.sa_handler = handler;

	ret = sigemptyset(&sa_ori.sa_mask);

	if (ret != 0)
	{
		UNRESOLVED(errno, "sigemptyset failed");
	}

	ret = sigaddset(&sa_ori.sa_mask, SIGUSR2);

	if (ret != 0)
	{
		UNRESOLVED(errno, "sigaddset failed");
	}

	sa_ori.sa_flags = SA_NOCLDSTOP;
	ret = sigaction(SIGUSR1, &sa_ori, NULL);

	if (ret != 0)
	{
		UNRESOLVED(errno, "Failed to set the signal handler");
	}

	/* Create the child */
	child = fork();

	if (child == -1)
	{
		UNRESOLVED(errno, "Failed to fork");
	}

	/* child */
	if (child == 0)
	{
		/* Check the struct was copied */

		if ((mystruct.one != 1) || (mystruct.two != 2) ||
		    (mystruct.three != 3) || (mystruct.four != (void *) 4))
		{
			FAILED("The struct data was not copied to the child process");
		}

		/* Check the malloc'ed memory is copied */
		if (*(double *) malloced != 2.3)
		{
			FAILED("Malloc'd block not copied in child process");
		}

		/* Free the memory -- this should suceed */
		free(malloced);

		/* Check the env variable */
		if (strncmp("2-1.c", getenv("OPTS_FORK_TC"), 6) != 0)
		{
			FAILED("The environment is not copied to the child");
		}

		/* Check the signal handler stuff */
		ret = sigaction(SIGUSR1, NULL, &sa_child);

		if (ret != 0)
		{
			UNRESOLVED(errno, "Failed to read sigaction information in child");
		}

		if (sa_child.sa_handler != handler)
		{
			FAILED("The child signal handler function is different from the parent's");
		}

		ret = sigismember(&sa_child.sa_mask, SIGUSR2);

		if (ret == 0)
		{
			FAILED("The child signal handler mask is different from the parent's");
		}

		if (ret != 1)
		{
			UNRESOLVED(errno, "Unexpected return code from sigismember");
		}

		if (((sa_child.sa_flags & SA_NOCLDSTOP) != SA_NOCLDSTOP)
#ifndef WITHOUT_XOPEN
		        || ((sa_child.sa_flags & SA_ONSTACK) != 0)
		        || ((sa_child.sa_flags & SA_RESETHAND) != 0)
		        || ((sa_child.sa_flags & SA_RESTART) != 0)
		        || ((sa_child.sa_flags & SA_SIGINFO) != 0)
		        || ((sa_child.sa_flags & SA_NOCLDWAIT) != 0)
		        || ((sa_child.sa_flags & SA_NODEFER) != 0)
#endif
		   )
		{
			FAILED("The sigaction flags are different in the child");
		}

		/* The child stops here */
		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child)
	{
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status))
	{
		UNRESOLVED(status, "Child exited abnormally");
	}

	if (WEXITSTATUS(status) == PTS_PASS)
	{

		/* Test passed */
#if VERBOSE > 0
		output("Test passed\n");
#endif

		PASSED;
	}

	if (WEXITSTATUS(status) == PTS_FAIL)
	{

		/* Test failed */
		FAILED("Test failed in child\n");
	}

	/* Otherwise we've an unexpected return code */
	UNRESOLVED(WEXITSTATUS(status), "Child returned an unexpected error code");
}