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
 * If the calling thread is the last thread in the process,
 * the effect are as if an implicit call to exit(0) had been made.

 * The steps are:
 *
 * -> main creates a thread.
 * -> this thread forks(). The new process contains only 1 thread.
 * -> the thread in the new process calls pthread_exit(non-0 value).
 * -> main process joins the child process and checks the behavior
 *     is as if exit(0) had been called.
 *     The checked items are:
 *       -> the return value.
 *       -> the atexit() routines have been called.
  */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sched.h>
#include <semaphore.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/mman.h>

/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"
 /* This header is responsible for defining the following macros:
  * UNRESOLVED(ret, descr);
  *    where descr is a description of the error and ret is an int (error code for example)
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

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

#include "../testfrmw/threads_scenarii.c"

/* This file will define the following objects:
 * scenarii: array of struct __scenario type.
 * NSCENAR : macro giving the total # of scenarii
 * scenar_init(): function to call before use the scenarii array.
 * scenar_fini(): function to call after end of use of the scenarii array.
 */

/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

/* This will be used to control that atexit() has been called */
static int *ctl;
static long mf;
static unsigned int sc;

static void clnp(void)
{
	*ctl = 1;
}

/* Thread routine */
static void *threaded(void *arg PTS_ATTRIBUTE_UNUSED)
{
	int ret = 0;

	pid_t pid, chk;
	int status;

	if (mf > 0)
		*ctl = 0;

#if VERBOSE > 0
	fflush(stdout);
#endif

	pid = fork();
	if (pid == (pid_t) - 1) {
		UNRESOLVED(errno, "Failed to fork()");
	}
	if (pid == 0) {
		/* children */
		if (mf > 0) {
			ret = atexit(clnp);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Failed to register atexit function");
			}
		}

		/* exit the last (and only) thread */
		pthread_exit(&ret);

		FAILED
		    ("pthread_exit() did not terminate the process when there was only 1 thread");
	}

	/* Only the parent process goes this far */
	chk = waitpid(pid, &status, 0);
	if (chk != pid) {
		output("Expected pid: %i. Got %i\n", (int)pid, (int)chk);
		UNRESOLVED(errno, "Waitpid failed");
	}

	if (WIFSIGNALED(status)) {
		output("Child process killed with signal %d\n",
		       WTERMSIG(status));
		UNRESOLVED(-1, "Child process was killed");
	}

	if (WIFEXITED(status)) {
		ret = WEXITSTATUS(status);
	} else {
		UNRESOLVED(-1, "Child process was neither killed nor exited");
	}

	if (ret != 0) {
		output("Exit status was: %i\n", ret);
		FAILED("The child process did not exit with 0 status.");
	}

	if (mf > 0)
		if (*ctl != 1)
			FAILED
			    ("pthread_exit() in the last thread did not execute atexit() routines");

	/* Signal we're done (especially in case of a detached thread) */
	do {
		ret = sem_post(&scenarii[sc].sem);
	}
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1) {
		UNRESOLVED(errno, "Failed to wait for the semaphore");
	}

	return NULL;
}

/* Main routine */
int main(void)
{
	int ret = 0;
	pthread_t child;

	mf = sysconf(_SC_MAPPED_FILES);

	output_init();

	scenar_init();

	/* We want to share some memory with the child process */
	if (mf > 0) {
		/* We will place the test data in a mmaped file */
		char filename[] = "/tmp/pthread_exit_6-1-XXXXXX";
		size_t sz;
		void *mmaped;
		int fd;
		char *tmp;

		/* We now create the temp files */
		fd = mkstemp(filename);
		if (fd == -1) {
			UNRESOLVED(errno,
				   "Temporary file could not be created");
		}

		/* and make sure the file will be deleted when closed */
		unlink(filename);

#if VERBOSE > 1
		output("Temp file created (%s).\n", filename);
#endif

		sz = (size_t) sysconf(_SC_PAGESIZE);

		tmp = calloc(1, sz);
		if (tmp == NULL) {
			UNRESOLVED(errno, "Memory allocation failed");
		}

		/* Write the data to the file.  */
		if (write(fd, tmp, sz) != (ssize_t) sz) {
			UNRESOLVED(sz, "Writting to the file failed");
		}

		free(tmp);

		/* Now we can map the file in memory */
		mmaped =
		    mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (mmaped == MAP_FAILED) {
			UNRESOLVED(errno, "mmap failed");
		}

		ctl = (int *)mmaped;

		/* Our datatest structure is now in shared memory */
#if VERBOSE > 1
		output("Testdata allocated in shared memory.\n");
#endif
	}

	for (sc = 0; sc < NSCENAR; sc++) {
#if VERBOSE > 0
		output("-----\n");
		output("Starting test with scenario (%i): %s\n", sc,
		       scenarii[sc].descr);
#endif

		ret = pthread_create(&child, &scenarii[sc].ta, threaded, &ctl);
		switch (scenarii[sc].result) {
		case 0:	/* Operation was expected to succeed */
			if (ret != 0) {
				UNRESOLVED(ret, "Failed to create this thread");
			}
			break;

		case 1:	/* Operation was expected to fail */
			if (ret == 0) {
				UNRESOLVED(-1,
					   "An error was expected but the thread creation succeeded");
			}
			break;

		case 2:	/* We did not know the expected result */
		default:
#if VERBOSE > 0
			if (ret == 0) {
				output
				    ("Thread has been created successfully for this scenario\n");
			} else {
				output
				    ("Thread creation failed with the error: %s\n",
				     strerror(ret));
			}
#endif
		}
		if (ret == 0) {	/* The new thread is running */
			if (scenarii[sc].detached == 0) {
				ret = pthread_join(child, NULL);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Unable to join a thread");
				}
			} else {
				/* Just wait for the thread to terminate */
				do {
					ret = sem_wait(&scenarii[sc].sem);
				}
				while ((ret == -1) && (errno == EINTR));
				if (ret == -1) {
					UNRESOLVED(errno,
						   "Failed to wait for the semaphore");
				}
			}
		}
	}

	scenar_fini();
#if VERBOSE > 0
	output("-----\n");
	output("All test data destroyed\n");
	output("Test PASSED\n");
#endif

	PASSED;
}
