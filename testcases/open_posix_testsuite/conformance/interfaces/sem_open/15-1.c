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
*  If a process calls sem_open several times with the same name,
* the same adress must be returned as long as the semaphore
* has not been unlinked or closed as many times as opened.

* The steps are:
* -> Create a semaphore with sem_open
* -> call sem_open several times with the same name.
* -> Check that the same address is returned for the semaphore.

* The test fails if a different address is returned.

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

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

/* The main test function. */
int main(void)
{
	int ret, i;
	char *name = "/sem_open_15_1";

	sem_t *sems[4];

	/* Initialize output */
	output_init();

	/* Initialize all semaphores */

	for (i = 0; i < 4; i++) {
		sems[i] = sem_open(name, O_CREAT, 0777, 1);

		if (sems[i] == SEM_FAILED) {
			UNRESOLVED(errno, "Failed to sem_open");
		}

	}

	/* Check all calls returned the same @ */
	for (i = 0; i < 3; i++) {
		if (sems[i] != sems[i + 1]) {
			FAILED("sem_open returned a different address");
		}

		/* Close some semaphores */
		ret = sem_close(sems[i]);

		if (ret != 0) {
			UNRESOLVED(errno, "Failed to sem_close");
		}
	}

	/* Now, reopen, we should still get the same address */
	for (i = 0; i < 3; i++) {
		sems[i] = sem_open(name, O_CREAT, 0777, 1);

		if (sems[i] == SEM_FAILED) {
			UNRESOLVED(errno, "Failed to sem_open");
		}

	}

	/* Check all calls returned the same @ */
	for (i = 0; i < 3; i++) {
		if (sems[i] != sems[i + 1]) {
			FAILED("sem_open returned a different address");
		}
	}

	/* Close all semaphores */
	for (i = 0; i < 4; i++) {
		ret = sem_close(sems[i]);

		if (ret != 0) {
			UNRESOLVED(errno, "Failed to sem_close");
		}
	}

	sem_unlink(name);

	/* Test passed */
#if VERBOSE > 0

	output("Test passed\n");

#endif

	PASSED;
}
