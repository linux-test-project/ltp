/*
* Copyright (c) 2004, Bull S.A..  All rights reserved.
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
* The new process has only one thread.

* The steps are:
* -> create a thread.
* -> fork
* -> Check that the thread is not running in the new process image.

* The test fails if the thread is executing in the child process.

*/


#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

static sem_t *sem;

static void *threaded(void *arg)
{
	int ret = 0;

	do {
		ret = sem_wait(sem);
	} while (ret != 0 && errno == EINTR);

	if (ret != 0)
		UNRESOLVED(errno, "failed to wait for the semaphore in child");

	if (*((pid_t *) arg) != getpid())
		FAILED("The thread is executing in the child process");

	return NULL;
}

int main(void)
{
	int ret, status;
	pid_t child, ctl;
	pthread_t th;

	output_init();

	ctl = getpid();

	/* Initialize the semaphore */
	sem = sem_open("/fork_21_1", O_CREAT, O_RDWR, 0);

	if (sem == SEM_FAILED)
		UNRESOLVED(errno, "Failed to open the semaphore");

	sem_unlink("/fork_21_1");

	/* Create thread */
	ret = pthread_create(&th, NULL, threaded, &ctl);

	if (ret != 0)
		UNRESOLVED(ret, "Failed to create the thread");

	/* Create the child */
	child = fork();

	if (child == -1)
		UNRESOLVED(errno, "Failed to fork");

	/* We post the semaphore twice */
	do {
		ret = sem_post(sem);
	} while (ret != 0 && errno == EINTR);

	if (ret != 0)
		UNRESOLVED(errno, "Failed to post the semaphore");

	/* child */
	if (child == 0) {
		/* sleep a little while to let the thread execute in case it exists */
		sleep(1);

		/* We're done */
		exit(PTS_PASS);
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child)
		UNRESOLVED(errno, "Waitpid returned the wrong PID");

	if (!WIFEXITED(status) || WEXITSTATUS(status) != PTS_PASS)
		FAILED("Child exited abnormally");

	/* Destroy everything */
	ret = sem_close(sem);

	if (ret != 0)
		UNRESOLVED(errno, "Failed to close the semaphore");

	ret = pthread_join(th, NULL);

	if (ret != 0)
		UNRESOLVED(ret, "Failed to join the thread in parent");

#if VERBOSE > 0
	output("Test passed\n");
#endif
	PASSED;
}
