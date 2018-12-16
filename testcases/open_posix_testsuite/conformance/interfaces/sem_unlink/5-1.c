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
*  sem_unlink will return -1 and set errno to ENAMETOOLONG when the semaphore
* name length is greater than PATH_MAX or a component length is greater than
* NAME_MAX.

* The steps are:
* -> If PATH_MAX is positive,
*    -> create a semaphore with a name bigger than PATH_MAX.
*    -> if the creation succeeds, try to unlink. It should fail.
* -> If NAME_MAX is positive, do similar test.

* The test fails if the ENAMETOOLONG is not returned.
* It also FAILS if this error is returned, as it means we can create a semaphore
* which cannot be removed.
* So actually, if the creation succeeds, the test fails :))

*/


/******************************************************************************/
/*************************** standard includes ********************************/
/******************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>

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

#define SEM_NAME  "/sem_unlink_5_1"

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

/* The main test function. */
int main(void)
{
	int ret, error;
	sem_t *sem;
	long PATH_MAX, NAME_MAX;
	char *sem_name;

	/* Initialize output */
	output_init();

	/* Get PATH_MAX value */
	PATH_MAX = pathconf("/", _PC_PATH_MAX);

#if VERBOSE > 0
	output("PATH_MAX: %ld\n", PATH_MAX);
#endif

	if (PATH_MAX > 0) {
		/* create a semaphore with a name longer than PATH_MAX */
		sem_name = calloc(PATH_MAX + 1, sizeof(char));

		if (sem_name == NULL) {
			UNRESOLVED(errno,
				   "Failed to allocate space for the semaphore name");
		}

		/* the space was allocated */
		sem_name[0] = '/';

		sem_name[PATH_MAX] = '\0';

		memset(sem_name + 1, 'P', PATH_MAX - 1);

		/* Create the semaphore */
		sem = sem_open(sem_name, O_CREAT, 0777, 1);

		if (sem != SEM_FAILED) {
			ret = sem_unlink(sem_name);
			error = errno;
			free(sem_name);

			if (ret == 0) {
				FAILED
				    ("The function did not return ENAMETOOLONG as expected");
			} else {
				output("Error was %d: %s\n", error,
				       strerror(error));
				FAILED
				    ("Unable to unlink a semaphore which we just created");
			}
		}
#if VERBOSE > 0
		else {
			output
			    ("Creation of the semaphore failed with error %d: %s\n",
			     errno, strerror(errno));
		}

#endif

	}

	/* Get NAME_MAX value */
	NAME_MAX = pathconf("/", _PC_NAME_MAX);

#if VERBOSE > 0
	output("NAME_MAX: %ld\n", NAME_MAX);

#endif

	if (NAME_MAX > 0) {
		/* create a semaphore with a name longer than NAME_MAX */
		sem_name = calloc(NAME_MAX + 2, sizeof(char));

		if (sem_name == NULL) {
			UNRESOLVED(errno,
				   "Failed to allocate space for the semaphore name");
		}

		/* the space was allocated */
		sem_name[0] = '/';

		sem_name[NAME_MAX + 1] = '\0';

		memset(sem_name + 1, 'N', NAME_MAX);

		/* Create the semaphore */
		sem = sem_open(sem_name, O_CREAT, 0777, 1);

		if (sem != SEM_FAILED) {
			ret = sem_unlink(sem_name);
			error = errno;
			free(sem_name);

			if (ret == 0) {
				FAILED
				    ("The function did not return ENAMETOOLONG as expected");
			} else {
				output("Error was %d: %s\n", error,
				       strerror(error));
				FAILED
				    ("Unable to unlink a semaphore which we just created");
			}
		}
#if VERBOSE > 0
		else {
			output
			    ("Creation of the semaphore failed with error %d: %s\n",
			     errno, strerror(errno));
		}

#endif

	}

	/* Test passed */
#if VERBOSE > 0
	output("Test passed\n");

#endif
	PASSED;
}
