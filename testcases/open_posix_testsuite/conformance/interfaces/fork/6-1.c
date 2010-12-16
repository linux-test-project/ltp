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
* The opened directory streams are copied to the child process.
* Positionning information may be shared between both processes.

* The steps are:
* -> Open the current directory,
* -> Count the directory entries, then rewind.
* -> create a child
* -> The child counts the directory entries.

* The test fails if the directory is read in the parent and not in the child.

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

#include <dirent.h>

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

int count(DIR * thedir)
{
	int counter = 0;

	struct dirent *dp;

	rewinddir(thedir);

	/* Count the directory entries */

	do
	{
		dp = readdir(thedir);

		if (dp != NULL)
			counter++;
	}
	while (dp != NULL);

	return counter;
}

/* The main test function. */
int main(int argc, char * argv[])
{
	int ret, status;
	pid_t child, ctl;

	int counted;

	/* the '.' directory pointers */
	DIR *dotdir;

	/* Initialize output */
	output_init();

	/* Open the directory */
	dotdir = opendir(".");

	if (dotdir == NULL)
	{
		UNRESOLVED(errno, "opendir failed");
	}

	/* Count entries */
	counted = count(dotdir);

#if VERBOSE > 0

	output("Found %d entries in current dir\n", counted);

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
		/* Count in child process */
		counted = count(dotdir);

#if VERBOSE > 0

		output("Found %d entries in current dir from child\n", counted);
#endif

		ret = closedir(dotdir);

		if (ret != 0)
		{
			UNRESOLVED(errno, "Failed to close dir in child");
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
		FAILED("Child exited abnormally -- dir stream not copied?");
	}

	/* close the directory stream */
	ret = closedir(dotdir);

	if (ret != 0)
	{
		UNRESOLVED(errno, "Failed to closedir in parent");
	}

	/* Test passed */
#if VERBOSE > 0
	output("Test passed\n");

#endif

	PASSED;
}