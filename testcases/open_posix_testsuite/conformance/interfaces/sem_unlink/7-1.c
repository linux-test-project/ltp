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
*  sem_unlink does not block.

* The steps are:
* -> open a semaphore
* -> create a thread which blocks on the semaphore
* -> call sem_unlink
* -> clean up

* The test fails when it hangs on sem_unlink.

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

#define SEM_NAME  "/sem_unlink_7_1"

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

void *threaded(void *arg)
{
	int ret = 0;

	do {
		ret = sem_wait(arg);
	}
	while ((ret != 0) && (errno == EINTR));

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to wait for the semaphore");
	}

	return NULL;
}

/* The main test function. */
int main(void)
{
	int ret;
	pthread_t thread;
	sem_t *sem;

	/* Initialize output */
	output_init();

	/* Create the semaphore */
	sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0777, 1);

	if ((sem == SEM_FAILED) && (errno == EEXIST)) {
		sem_unlink(SEM_NAME);
		sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0777, 1);
	}

	if (sem == SEM_FAILED) {
		UNRESOLVED(errno, "Failed to create the semaphore");
	}

	/* Create the child thread */
	ret = pthread_create(&thread, NULL, threaded, sem);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to create the thread");
	}

	/* Let some time for the thread to block */
	sleep(1);

	/* Unlink */
	ret = sem_unlink(SEM_NAME);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to unlink the semaphore");
	}

	/* Now, we're success */
	ret = sem_post(sem);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to post the semaphore");
	}

	/* Join the thread */
	ret = pthread_join(thread, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join the thread");
	}

	/* close  */
	ret = sem_close(sem);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to close the semaphore");
	}

	/* Test passed */
#if VERBOSE > 0
	output("Test passed\n");

#endif
	PASSED;
}
