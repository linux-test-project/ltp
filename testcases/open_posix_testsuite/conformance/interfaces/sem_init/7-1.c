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
*  sem_init returns -1 and sets errno to ENOSPC if the system lacks a resource
* or SEM_NSEMS_MAX has been reached.

* The steps are:
* -> Try and sem_init SEM_NSEMS_MAX semaphores.
* -> Try and sem_init an additional semaphore.

* The test fails if the last creation does not return an error.

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
	sem_t *sems;
	sem_t sem_last;

	long max;

	/* Initialize output */
	output_init();

	max = sysconf(_SC_SEM_NSEMS_MAX);

	if (max <= 0) {
		output("sysconf(_SC_SEM_NSEMS_MAX) = %ld\n", max);
		UNTESTED("There is no constraint on SEM_NSEMS_MAX");
	}

	sems = (sem_t *) calloc(max, sizeof(sem_t));

	if (sems == NULL) {
		UNRESOLVED(errno, "Failed to alloc space");
	}

	for (i = 0; i < max; i++) {
		ret = sem_init(&sems[i], 0, 0);

		if (ret != 0) {
			output
			    ("sem_init failed to initialize the %d nth semaphore.\n",
			     i);
			output("Tryed to initialize %ld.\n", max);
			output("Error is %d: %s\n", errno, strerror(errno));

			for (; i > 0; i--)
				sem_destroy(&sems[i - 1]);

			free(sems);

			PASSED;
		}
	}

	ret = sem_init(&sem_last, 0, 1);

	if (ret == 0) {
		FAILED
		    ("We were able to sem_init more than SEM_NSEMS_MAX semaphores");
	}

	if (errno != ENOSPC) {
		output("Error is %d: %s\n", errno, strerror(errno));
	}

	for (i = 0; i < max; i++)
		sem_destroy(&sems[i]);

	free(sems);

	/* Test passed */
#if VERBOSE > 0

	output("Test passed\n");

#endif

	PASSED;
}
