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
*  A call to sem_open with the same name refers to a new semaphore, once
* sem_unlink has been called.

* The steps are:
* -> open a semaphore
* -> unlink
* -> try to reconnect (should fail)
* -> open with O_CREATE
* -> check the semaphore are different

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

#define SEM_NAME  "/sem_unlink_6_1"

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

/* The main test function. */
int main(void)
{
	int ret, value;

	sem_t *sem1, *sem2;

	/* Initialize output */
	output_init();

	/* Create the semaphore */
	sem1 = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0777, 1);

	if ((sem1 == SEM_FAILED) && (errno == EEXIST)) {
		sem_unlink(SEM_NAME);
		sem1 = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0777, 1);
	}

	if (sem1 == SEM_FAILED) {
		UNRESOLVED(errno, "Failed to create the semaphore");
	}

	/* Unlink */
	ret = sem_unlink(SEM_NAME);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to unlink the semaphore");
	}

	/* Try reconnect */
	sem2 = sem_open(SEM_NAME, 0);

	if (sem2 != SEM_FAILED) {
		FAILED("Reconnecting the unlinked semaphore did not failed");
	}

	if (errno != ENOENT) {
		output("Error %d: %s\n", errno, strerror(errno));
		FAILED
		    ("Reconnecting the unlinked semaphore failed with a wrong error");
	}

	/* Reopen the semaphore */
	sem2 = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0777, 3);

	if (sem2 == SEM_FAILED) {
		output("Gor error %d: %s\n", errno, strerror(errno));
		FAILED("Failed to recreate the semaphore");
	}

	/* Check the semaphore have different values */
	ret = sem_getvalue(sem1, &value);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to read sem1 value");
	}

	if (value != 1) {
		output("Read: %d\n", value);
		FAILED("Semaphore value is not as expected");
	}

	ret = sem_getvalue(sem2, &value);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to read sem1 value");
	}

	if (value != 3) {
		output("Read: %d\n", value);
		FAILED("Semaphore value is not as expected");
	}

	/* Unlink */
	ret = sem_unlink(SEM_NAME);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to unlink the semaphore");
	}

	/* close both */
	ret = sem_close(sem1);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to close the semaphore");
	}

	ret = sem_close(sem2);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to close the semaphore");
	}

	/* Test passed */
#if VERBOSE > 0
	output("Test passed\n");

#endif
	PASSED;
}
