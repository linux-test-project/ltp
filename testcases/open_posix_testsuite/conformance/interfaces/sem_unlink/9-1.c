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
*  When the semaphore is referenced by other processes, the function has
* no effect on the state of the semaphore.

* The steps are:
* -> Create a named semaphore
* -> create a thread which waits on the semaphore
* -> unlink the semaphore
* -> check the thread did not return
* -> post the semaphore
* -> check the thread returned
* -> close the semaphore

* The test fails if the semaphore state is changed by sem_unlink.

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

#define SEM_NAME  "/sem_unlink_9_1"

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/
int thread_state = 0;

void *threaded(void *arg)
{
	int ret;
	thread_state = 1;

	do {
		ret = sem_wait(arg);
	}
	while (ret != 0 && errno == EINTR);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to wait for the semaphore");
	}

	thread_state = 2;
	return NULL;
}

/* The main test function. */
int main(void)
{
	int ret;
	pthread_t child;
	sem_t *sem;

	/* Initialize output */
	output_init();

	/* Create the semaphore */
	sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0777, 0);

	if ((sem == SEM_FAILED) && (errno == EEXIST)) {
		sem_unlink(SEM_NAME);
		sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0777, 0);
	}

	if (sem == SEM_FAILED) {
		UNRESOLVED(errno, "Failed to create the semaphore");
	}

	/* Create the child */
	ret = pthread_create(&child, NULL, threaded, sem);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to create the thread");
	}

	/* Wait for the child to be waiting. */
	while (thread_state != 1) {
		sched_yield();
	}

	/* Unlink */
	ret = sem_unlink(SEM_NAME);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to unlink the semaphore");
	}

	/* Check the semaphore state did not change. */
	sleep(1);

	if (thread_state != 1) {
		FAILED("sem_unlink made sem_wait thread return");
	}

	/* Post the semaphore */
	ret = sem_post(sem);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to post the semaphore");
	}

	/* Join the thread */
	ret = pthread_join(child, NULL);

	if (ret != 0) {
		UNRESOLVED(ret, "Failed to join the thread");
	}

	/* Now, we can destroy all */
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
