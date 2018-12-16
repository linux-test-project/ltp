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
*  sem_unlink will return -1 and set errno to ENOENT when the named semaphore
* does not exist.

* The steps are:
* -> Unlink once to make sure this name does not exist. Ignore the error.
* -> Unlink again and check that ENOENT is returned

* The test fails if the error is not ENOENT, or there is no error.

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

#define SEM_NAME  "/sem_unlink_4_2"

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

/* The main test function. */
int main(void)
{
	int ret;

	/* Initialize output */
	output_init();

	(void)sem_unlink(SEM_NAME);

	ret = sem_unlink(SEM_NAME);

	if (ret != -1) {
		FAILED("sem_unlink did not return -1");
	}

	if (errno != ENOENT) {
		output("Error %d: %s\n", errno, strerror(errno));
		FAILED("The error was not ENOENT");
	}

	/* Test passed */
#if VERBOSE > 0
	output("Test passed\n");

#endif
	PASSED;
}
