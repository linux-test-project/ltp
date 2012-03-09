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
 * The parent process ID of the child is the process ID of the parent (caller of fork())

 * The steps are:
 * -> create a child
 * -> check its parent process ID is the PID of its parent.

 * The test fails if the IDs differ.

 */

 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L

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

/* The main test function. */
int main(int argc, char * argv[])
{
	int status;
	pid_t child, ctl;

	/* Initialize output */
	output_init();

	/* Get parent process ID */
	ctl = getpid();

	/* Create the child */
	child = fork();
	if (child == -1)  {  UNRESOLVED(errno, "Failed to fork");  }

	/* child */
	if (child == 0)
	{
		/* Check the parent process ID */
		if (ctl != getppid())
		{
			FAILED("The parent process ID is not the PID of the parent");
		}

		/* We're done */
		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);
	if (ctl != child)  {  UNRESOLVED(errno, "Waitpid returned the wrong PID");  }
	if ((!WIFEXITED(status)) || (WEXITSTATUS(status) != PTS_PASS))
	{
		UNRESOLVED(status, "Child exited abnormally");
	}

	/* Test passed */
	#if VERBOSE > 0
	output("Test passed\n");
	#endif

	PASSED;
}