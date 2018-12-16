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
* If pshared is non-zero, any process that can access the semaphore can use it.

* The steps are:
* -> Create a shared memory segment and mmap it.
* -> sem_init a semaphore placed in this segment, with pshared != 0 and val=0
* -> fork.
* -> child process post the semaphore then exit
* -> parent process waits for the semaphore.

* The test fails if it hangs

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
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

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
	int ret, status;
	pid_t child, ctl;
	int fd;
	void *buf;
	sem_t *sem;

	/* Initialize output */
	output_init();

	/* Create the shared memory segment */
	fd = shm_open("/sem_init_3-2", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

	if (fd == -1) {
		UNRESOLVED(errno, "Failed to open shared memory segment");
	}

	/* Size the memory segment to 1 page size. */
	ret = ftruncate(fd, sysconf(_SC_PAGESIZE));

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to size the shared memory segment");
	}

	/* Map these sengments in the process memory space */
	buf =
	    mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE,
		 MAP_SHARED, fd, 0);

	if (buf == MAP_FAILED) {
		UNRESOLVED(errno, "Failed to mmap the shared memory segment");
	}

	sem = (sem_t *) buf;

	/* Initialize the semaphore */
	ret = sem_init(sem, 1, 0);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to init the semaphore");
	}

	/* Create the child */
	child = fork();

	if (child == -1) {
		UNRESOLVED(errno, "Failed to fork");
	}

	/* child */
	if (child == 0) {
		/* Post the sempahore */
		ret = sem_post(sem);

		if (ret != 0) {
			UNRESOLVED(errno, "Failed to post the semaphore");
		}

		/* We're done */
		exit(PTS_PASS);
	}

	/* Wait the sempahore */
	do {
		ret = sem_wait(sem);
	} while (ret != 0 && errno == EINTR);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to wait for the semaphore");
	}

	/* Parent joins the child */
	ctl = waitpid(child, &status, 0);

	if (ctl != child) {
		UNRESOLVED(errno, "Waitpid returned the wrong PID");
	}

	if (!WIFEXITED(status) || (WEXITSTATUS(status) != PTS_PASS)) {
		FAILED("Child exited abnormally");
	}

	/* Clean things */
	ret = sem_destroy(sem);

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to destroy the semaphore");
	}

	ret = shm_unlink("/sem_init_3-2");

	if (ret != 0) {
		UNRESOLVED(errno, "Failed to unlink shared memory");
	}

	/* Test passed */
#if VERBOSE > 0

	output("Test passed\n");

#endif

	PASSED;
}
