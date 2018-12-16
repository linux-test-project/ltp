/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
 * Created by: Sebastien Decugis
 *
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
 *
 * This sample test aims to check the following assertion:
 *
 * fork() creates a new process.
 *
 * The steps are:
 * -> create a new process
 * -> the parent and the child sleep 1 sec (check concurrent execution)
 * -> the child posts a semaphore, the parents waits for thsi semaphore
 *    (check the child really executes)
 * -> join and check that total execution time is < 2 sec.
 *
 * The test fails if the duration is > 2 seconds or if semaphore is not posted.
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
#include <time.h>
#include <unistd.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

#define SEM_NAME "/semfork1_1"

int main(void)
{
	int ret, status;
	pid_t child, ctl;
	sem_t *sem;
	struct timespec tsini, tsfin;

	output_init();

	/* read current time */
	ret = clock_gettime(CLOCK_REALTIME, &tsini);
	if (ret == -1)
		UNRESOLVED(errno, "Unable to read CLOCK_REALTIME clock");

	/* Set temporary value in tsfin for semaphore timeout */
	tsfin.tv_sec = tsini.tv_sec + 3;
	tsfin.tv_nsec = tsini.tv_nsec;

	/* Create the child */
	child = fork();
	if (child == -1)
		UNRESOLVED(errno, "Failed to fork");

	/* Open the semaphore */
	sem = sem_open(SEM_NAME, O_CREAT, O_RDWR, 0);
	if (sem == SEM_FAILED)
		UNRESOLVED(errno, "Failed to open the semaphore (try executing "
			   "as root)");

	/* sleep 1 second */
	sleep(1);

	/* child posts the semaphore and terminates */
	if (child == 0) {
		do {
			ret = sem_post(sem);
		} while (ret == -1 && errno == EINTR);

		if (ret == -1)
			UNRESOLVED(errno, "Failed to post the semaphore");

		ret = sem_close(sem);
		if (ret == -1)
			UNRESOLVED(errno, "Failed to close the semaphore");

		/* The child stops here */
		exit(0);
	}

	/* Parent waits for the semaphore */
	do {
		ret = sem_timedwait(sem, &tsfin);
	} while (ret == -1 && errno == EINTR);

	if (ret == -1) {
		if (errno == ETIMEDOUT)
			FAILED("The new process does not execute");
		UNRESOLVED(errno, "Failed to wait for the semaphore");
	}

	/* We don't need the semaphore anymore */
	ret = sem_close(sem);
	if (ret == -1)
		UNRESOLVED(errno, "Failed to close the semaphore");

	ret = sem_unlink(SEM_NAME);
	if (ret == -1)
		UNRESOLVED(errno, "Unable to unlink the semaphore");

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);
	if (ctl != child)
		UNRESOLVED(errno, "Waitpid returned the wrong PID");

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		UNRESOLVED(status, "Child exited abnormally");

	/* Check the global duration */
	ret = clock_gettime(CLOCK_REALTIME, &tsfin);
	if (ret == -1)
		UNRESOLVED(errno, "Unable to read CLOCK_REALTIME clock");

	if (tsfin.tv_nsec < tsini.tv_nsec)
		tsfin.tv_sec -= 1;

	status = tsfin.tv_sec - tsini.tv_sec;
	if (status >= 2) {
		/* the operation was more than 2 secs long */
		FAILED("the processes did not execute concurrently");
	}

	output("Test passed\n");

	PASSED;
}
