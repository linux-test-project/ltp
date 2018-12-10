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
*  sem_unlink will return -1 and set errno to EACCESS if the process has not
* priviledge to unlink the sem.

* The steps are:
* -> open a semaphore with 0744 mask
* -> fork
* -> change child uid
* -> child attempts to unlink the semaphore. It should fail.
* -> join the child
* -> sem_unlink (should be OK)

* The test fails if the child process is able to unlink the semaphore.

*/


/* Some of the routines are XSI extensions */

/******************************************************************************/
/*************************** standard includes ********************************/
/******************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pwd.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

/******************************************************************************/
/***************************   Test framework   *******************************/
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
/**************************** Configuration ***********************************/
/******************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

#define SEM_NAME  "/sem_unlink_3_1"

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

/* Set the euid of this process to a non-root uid */
/* (from ../sem_open/3-1.c)  */
int set_nonroot()
{

	struct passwd *pw;
	setpwent();
	/* search for the first user which is non root */

	while ((pw = getpwent()) != NULL)
		if (strcmp(pw->pw_name, "root"))
			break;

	endpwent();

	if (pw == NULL) {
		output("There is no other user than current and root.\n");
		return 1;
	}

	if (seteuid(pw->pw_uid) != 0) {
		if (errno == EPERM) {
			output
			    ("You don't have permission to change your UID.\n");
			return 1;
		}

		perror("An error occurs when calling seteuid()");
		return 1;
	}

	output("Testing with user '%s' (uid: %d)\n",
	       pw->pw_name, (int)geteuid());
	return 0;
}

/* The main test function. */
int main(void)
{
	int ret, status;
	pid_t ch, ctl;
	sem_t *sem;

	/* Initialize output */
	output_init();

	/* Create the semaphore */
	sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0744, 1);

	if ((sem == SEM_FAILED) && (errno == EEXIST)) {
		sem_unlink(SEM_NAME);
		sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0744, 1);
	}

	if (sem == SEM_FAILED) {
		UNRESOLVED(errno, "Failed to create the semaphore");
	}

	/* fork */
	ch = fork();

	if (ch == -1) {
		UNRESOLVED(errno, "Failed to fork");
	}

	if (ch == 0) {		/* child */
		/* connect to the semaphore */
		sem = sem_open(SEM_NAME, 0);

		if (sem == SEM_FAILED) {
			output
			    ("Failed to connect to the semaphore, error %d: %s\n",
			     errno, strerror(errno));
			exit(1);
		}

		/* change euid */
		ret = set_nonroot();

		if (ret) {
			output("Changing euid failed\n");
			exit(1);
		}

		/* try and unlink, it should fail */
		ret = sem_unlink(SEM_NAME);

		if (ret == 0) {
			output("sem_unlink did not fail in child");
			exit(2);
		}

		if (errno != EACCES) {
			output
			    ("sem_unlink failed with unexpected error %d: %s\n",
			     errno, strerror(errno));
			exit(2);
		}

		/* Ok, child is done. */
		exit(0);
	}

	/* Parent waits for the child to finish */
	ctl = waitpid(ch, &status, 0);

	if (ctl != ch) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status)) {
		FAILED("Child exited abnormally");
	}

	if (WEXITSTATUS(status) == 1) {
		UNRESOLVED(0, "An error occured in child");
	}

	if (WEXITSTATUS(status) == 2) {
		FAILED("Test failed in child");
	}

	if (WEXITSTATUS(status) != 0) {
		UNRESOLVED(0, "Unexpected return value from child");
	}

	/* Unlink */
	ret = sem_unlink(SEM_NAME);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to unlink the semaphore");
	}

	/* Test passed */
#if VERBOSE > 0
	output("Test passed\n");

#endif
	PASSED;
}
