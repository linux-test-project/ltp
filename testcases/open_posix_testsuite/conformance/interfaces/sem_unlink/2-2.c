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
*  Destruction of the semaphore is postponed until all processes which were using
* the semaphore have called sem_close, _exit or exec.

* The steps are:
* -> Create a named semaphore with value = 0.
* -> create 3 processes. Each call sem_wait, then sem_post, then sem_close/_exit/exec
* -> the main process unlinks the semaphore, the posts it and close it.
* -> Check all child processes have returned successfully.

* The test fails if a semaphore operation returns an error in one of the children.

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

#define SEM_NAME  "/sem_unlink_9_1"

/******************************************************************************/
/***************************    Test case   ***********************************/
/******************************************************************************/

/* Operations common to all processes on the semaphore*/
sem_t *common()
{
	int ret;
	sem_t *sem;

	/* Reconnect to the semaphore */
	sem = sem_open(SEM_NAME, 0);

	if (sem == SEM_FAILED) {
		UNRESOLVED(errno, "Failed to reconnect the semaphore");
	}

	/* block until the semaphore is posted */

	do {
		ret = sem_wait(sem);
	}
	while (ret != 0 && errno == EINTR);

	if (ret != 0) {
		FAILED("Waiting for the semaphore failed");
	}

	/* spend some time... */
	sched_yield();

	sched_yield();

	sched_yield();

	/* Post the semaphore back */
	ret = sem_post(sem);

	if (ret != 0) {
		FAILED("Failed to post the semaphore");
	}

	return sem;
}

/* The main test function. */
int main(void)
{
	int ret, status;
	pid_t p1, p2, p3, ctl;

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

	/* fork 3 times */
	p1 = fork();

	if (p1 == -1) {
		UNRESOLVED(errno, "Failed to fork");
	}

	if (p1 == 0) {		/* child */
		sem = common();
		ret = sem_close(sem);

		if (ret != 0) {
			FAILED("Failed to sem_close in child");
		}

		exit(0);
	}

	p2 = fork();

	if (p2 == -1) {
		UNRESOLVED(errno, "Failed to fork");
	}

	if (p2 == 0) {		/* child */
		sem = common();
		_exit(0);
	}

	p3 = fork();

	if (p3 == -1) {
		UNRESOLVED(errno, "Failed to fork");
	}

	if (p3 == 0) {		/* child */
		sem = common();
		ret = execl("/bin/ls", "ls", NULL);
		UNRESOLVED(errno, "Failed to exec");
	}

	/* Let all processes start and wait for the semaphore */
	sleep(1);

	/* Unlink */
	ret = sem_unlink(SEM_NAME);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to unlink the semaphore");
	}

	/* Post the semaphore */
	ret = sem_post(sem);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to post the semaphore");
	}

	/* and close it in this process */
	ret = sem_close(sem);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to close the semaphore");
	}

	/* Wait all processes */
	ctl = waitpid(p1, &status, 0);

	if (ctl != p1) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
		FAILED("Child 'sem_close' exited abnormally");
	}

	ctl = waitpid(p2, &status, 0);

	if (ctl != p2) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
		FAILED("Child '_exit' exited abnormally");
	}

	ctl = waitpid(p3, &status, 0);

	if (ctl != p3) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
		FAILED("Child 'exec' exited abnormally");
	}

	/* Test passed */
#if VERBOSE > 0
	output("Test passed\n");

#endif
	PASSED;
}
