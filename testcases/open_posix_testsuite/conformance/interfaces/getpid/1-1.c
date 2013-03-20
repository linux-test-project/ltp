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
 * getpid() always returns the process ID of the calling thread/process.

 * The steps are:
 *
 * -> create two threads and check they get the same getpid() return value.
 * -> create two processes and check they get different getpid() return value.
 * -> check that the child process getpid() return value matchs the fork() return
      value in the parent process.

 * The test fails if any of the previous checks is not verified.

 */

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

#ifndef WITHOUT_XOPEN

static pid_t *sharedpid;

/* This will be executed by the child process */
static void child(void)
{
	*sharedpid = getpid();
	exit(0);
}

/* This will be executed by the child thread */
static void *threaded(void *arg)
{
	*(pid_t *) arg = getpid();
	return NULL;
}

int main(void)
{
	int ret, status;
	long mf;		/* Is memory mapping supported? */
	pid_t mypid, hispid, ctlpid;
	pthread_t child_thread;

	output_init();

	mypid = getpid();
#if VERBOSE > 1
	output("Main pid: %d\n", mypid);
#endif

	ret = pthread_create(&child_thread, NULL, threaded, &hispid);
	if (ret != 0) {
		UNRESOLVED(ret, "Thread creation failed");
	}
	ret = pthread_join(child_thread, NULL);
	if (ret != 0) {
		UNRESOLVED(ret, "Thread join failed");
	}
#if VERBOSE > 1
	output("Thread pid: %d\n", hispid);
#endif

	/* Compare threads PIDs */
	if (mypid != hispid) {
		FAILED
		    ("Child thread got a different return value from getpid()\n");
	}

	/* Test system abilities */
	mf = sysconf(_SC_MAPPED_FILES);

#if VERBOSE > 0
	output("Test starting\n");
	output("System abilities:\n");
	output(" MF  : %li\n", mf);
	if (mf <= 0)
		output("Unable to test without shared data\n");
#endif

	/* We need MF support for the process-cross testing */
	if (mf > 0) {
		/* We will place the child pid in a mmaped file */
		char filename[] = "/tmp/getpid-1-XXXXXX";
		void *mmaped;
		int fd;

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

		/* Fill the file up to 1 pagesize */
		ret = ftruncate(fd, sysconf(_SC_PAGESIZE));
		if (ret != 0) {
			UNRESOLVED(errno, "ftruncate operation failed");
		}

		/* Now we can map the file in memory */
		mmaped =
		    mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ | PROT_WRITE,
			 MAP_SHARED, fd, 0);
		if (mmaped == MAP_FAILED) {
			UNRESOLVED(errno, "mmap failed");
		}

		/* Set the sharedpid pointer to this mmaped area */
		sharedpid = (pid_t *) mmaped;

		/* Our data is now in shared memory */
#if VERBOSE > 1
		output("Shared memory is ready.\n");
#endif

		/* Okay, let's create the child process */
		hispid = fork();
		if (hispid == (pid_t) - 1) {
			UNRESOLVED(errno, "Fork failed");
		}

		/* Child process : */
		if (hispid == (pid_t) 0)
			child();

		/* Otherwise, we're the parent */
		ctlpid = waitpid(hispid, &status, 0);
		if (ctlpid != hispid) {
			UNRESOLVED(errno,
				   "waitpid waited for the wrong process");
		}
		if (!WIFEXITED(status) || WEXITSTATUS(status)) {
			UNRESOLVED(status,
				   "The child process did not terminate as expected");
		}
#if VERBOSE > 1
		output("Child process pid: %d\n", hispid);
#endif

		/* Check the child pid is the same as fork returned */
		if (hispid != *sharedpid) {
			FAILED
			    ("getpid() in the child returned a different value than fork() in the parent");
		}

		/* Check the child pid is different than the parent pid */
		if (hispid == mypid) {
			FAILED
			    ("Both child and parent getpid() return values are equal");
		}
	}
#if VERBOSE > 0
	output("Test passed\n");
#endif

	PASSED;
}

#else /* WITHOUT_XOPEN */
int main(void)
{
	output_init();
	UNTESTED("This test requires XSI features");
}
#endif
