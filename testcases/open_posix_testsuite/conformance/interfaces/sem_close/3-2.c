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
*  If sem_unlink has not been called after sem_open for this semaphore,
* the function has no effect on the semaphore.

* The steps are:
* -> open a semaphore with a value of 2.
* -> wait the semaphore (now value = 1)
* -> close the semaphore.
* -> open the semaphore again (with dummy value = 3)
* -> check the value is 1.
* -> close the semaphore.
* -> unlink the semaphore.

* The test fails if the semaphore value is not 1 after semaphore is reopened.

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

#define SEM_NAME  "/sem_close_3_2"

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

/* The main test function. */
int main(void)
{
	int ret, value;

	sem_t *sem;

	/* Initialize output */
	output_init();

	/* Create the semaphore */
	sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0777, 2);

	if (sem == SEM_FAILED && errno == EEXIST) {
		sem_unlink(SEM_NAME);
		sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0777, 2);
	}

	if (sem == SEM_FAILED) {
		UNRESOLVED(errno, "Failed to create the semaphore");
	}

	/* Use the semaphore to change its value. */
	do {
		ret = sem_wait(sem);
	} while (ret != 0 && errno == EINTR);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to wait for the semaphore");
	}

	/* Here, count is 1. Now, close the semaphore */
	ret = sem_close(sem);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to close the semaphore");
	}

	/* Open the semaphore again */
	sem = sem_open(SEM_NAME, O_CREAT, 0777, 3);

	if (sem == SEM_FAILED) {
		UNRESOLVED(errno, "Failed to re-open the semaphore");
	}

	/* Check current semaphore count */
	ret = sem_getvalue(sem, &value);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to get semaphore value");
	}

	if (value != 1) {
		output("Got value: %d\n", value);
		FAILED("The semaphore count has changed after sem_close");
	}

	/* Now, we can destroy all */
	ret = sem_close(sem);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to close the semaphore");
	}

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
