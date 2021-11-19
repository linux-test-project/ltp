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
 * The pthread_mutex_trylock() function locks the mutex object
 * when it is unlocked.

 * The steps are:
 *
 * -> For each kind of mutex,
 *   -> trylock the mutex. It shall suceed.
 *   -> trylock the mutex again. It shall fail (except in case of recursive mutex).
 *   -> create a new child (either thread or process)
 *      -> the new child trylock the mutex. It shall fail.
 *   -> undo everything.
 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>

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
/***********************************    Test case   *****************************************/
/********************************************************************************************/
typedef struct {
	pthread_mutex_t mtx;
	int status;		/* error code */
} testdata_t;

static struct _scenar {
	int m_type;		/* Mutex type to use */
	int m_pshared;		/* 0: mutex is process-private (default) ~ !0: mutex is process-shared, if supported */
	int fork;		/* 0: Test between threads. ~ !0: Test across processes, if supported (mmap) */
	char *descr;		/* Case description */
} scenarii[] = {
	{
	PTHREAD_MUTEX_DEFAULT, 0, 0, "Default mutex"}
#ifndef WITHOUT_XOPEN
	, {
	PTHREAD_MUTEX_NORMAL, 0, 0, "Normal mutex"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 0, 0, "Errorcheck mutex"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 0, 0, "Recursive mutex"}
#endif

	, {
	PTHREAD_MUTEX_DEFAULT, 1, 0, "Pshared mutex"}
#ifndef WITHOUT_XOPEN
	, {
	PTHREAD_MUTEX_NORMAL, 1, 0, "Pshared Normal mutex"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 1, 0, "Pshared Errorcheck mutex"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 1, 0, "Pshared Recursive mutex"}
#endif

	, {
	PTHREAD_MUTEX_DEFAULT, 1, 1, "Pshared mutex across processes"}
#ifndef WITHOUT_XOPEN
	, {
	PTHREAD_MUTEX_NORMAL, 1, 1,
		    "Pshared Normal mutex across processes"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 1, 1,
		    "Pshared Errorcheck mutex across processes"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 1, 1,
		    "Pshared Recursive mutex across processes"}
#endif
};

#define NSCENAR (sizeof(scenarii)/sizeof(scenarii[0]))

/* The test function will only perform a trylock operation then return. */
static void *tf(void *arg)
{
	testdata_t *td = (testdata_t *) arg;

	td->status = pthread_mutex_trylock(&(td->mtx));

	if (td->status == 0) {
		int ret;

		ret = pthread_mutex_unlock(&(td->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to unlock a locked semaphore");
		}
	}

	return NULL;
}

/* Main entry point. */
int main(void)
{
	int ret;
	unsigned int sc;
	pthread_mutexattr_t ma;

	testdata_t *td;
	testdata_t alternativ;

	int do_fork;

	pid_t child_pr = 0, chkpid;
	int status;
	pthread_t child_th;

	long pshared, mf;

	/* Initialize output */
	output_init();

	/* Test system abilities */
	pshared = sysconf(_SC_THREAD_PROCESS_SHARED);
	mf = sysconf(_SC_MAPPED_FILES);

#if VERBOSE > 0
	output("Test starting\n");
	output("System abilities:\n");
	output(" TSH : %li\n", pshared);
	output(" MF  : %li\n", mf);
	if ((mf < 0) || (pshared < 0))
		output("Process-shared attributes won't be tested\n");
#endif

#ifdef WITHOUT_XOPEN
#if VERBOSE > 0
	output
	    ("As XSI extension is disabled, we won't test the feature across process\n");
#endif
	mf = -1;
#endif

/**********
 * Allocate space for the testdata structure
 */
	if (mf < 0) {
		/* Cannot mmap a file (or not interested in this), we use an alternative method */
		td = &alternativ;
		pshared = -1;	/* We won't do this testing anyway */
#if VERBOSE > 0
		output("Testdata allocated in the process memory.\n");
#endif
	}
#ifndef WITHOUT_XOPEN
	else {
		/* We will place the test data in a mmaped file */
		char filename[] = "/tmp/mutex_trylock_1-2-XXXXXX";
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

		td = (testdata_t *) mmaped;

		/* Our datatest structure is now in shared memory */
#if VERBOSE > 1
		output("Testdata allocated in shared memory.\n");
#endif
	}
#endif

/**********
 * For each test scenario, initialize the attributes and other variables.
 * Do the whole thing for each time to test.
 */
	for (sc = 0; sc < NSCENAR; sc++) {
#if VERBOSE > 1
		output("[parent] Preparing attributes for: %s\n",
		       scenarii[sc].descr);
#endif
		/* set / reset everything */
		do_fork = 0;
		ret = pthread_mutexattr_init(&ma);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "[parent] Unable to initialize the mutex attribute object");
		}
#ifndef WITHOUT_XOPEN
		/* Set the mutex type */
		ret = pthread_mutexattr_settype(&ma, scenarii[sc].m_type);
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Unable to set mutex type");
		}
#if VERBOSE > 1
		output("[parent] Mutex type : %i\n", scenarii[sc].m_type);
#endif
#endif

		/* Set the pshared attributes, if supported */
		if ((pshared > 0) && (scenarii[sc].m_pshared != 0)) {
			ret =
			    pthread_mutexattr_setpshared(&ma,
							 PTHREAD_PROCESS_SHARED);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to set the mutex process-shared");
			}
#if VERBOSE > 1
			output("[parent] Mutex is process-shared\n");
#endif
		}
#if VERBOSE > 1
		else {
			output("[parent] Mutex is process-private\n");
		}
#endif

		/* Tell whether the test will be across processes */
		if ((pshared > 0) && (scenarii[sc].fork != 0)) {
			do_fork = 1;
#if VERBOSE > 1
			output("[parent] Child will be a new process\n");
#endif
		}
#if VERBOSE > 1
		else {
			output("[parent] Child will be a new thread\n");
		}
#endif

/**********
 * Initialize the testdata_t structure with the previously defined attributes
 */
		/* Initialize the mutex */
		ret = pthread_mutex_init(&(td->mtx), &ma);
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Mutex init failed");
		}

		/* Initialize the other datas from the test structure */
		td->status = 0;

/**********
 * Proceed to the actual testing
 */
		/* Trylock the mutex twice before creating children */
		ret = pthread_mutex_trylock(&(td->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Unable to trylock the mutex");
		}
		ret = pthread_mutex_trylock(&(td->mtx));
#ifndef WITHOUT_XOPEN
		if (scenarii[sc].m_type == PTHREAD_MUTEX_RECURSIVE) {
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Failed to pthread_mutex_trylock() twice a recursive mutex");
			}

			/* Unlock once so the count is "1" */
			ret = pthread_mutex_unlock(&(td->mtx));
			if (ret != 0) {
				UNRESOLVED(ret, "Failed to unlock the mutex");
			}
		} else
#endif
		if (ret == 0) {
			FAILED
			    ("Main was able to pthread_mutex_trylock() twice without error");
		}

		/* Create the children */
		if (do_fork != 0) {
			/* We are testing across processes */
			child_pr = fork();
			if (child_pr == -1) {
				UNRESOLVED(errno, "[parent] Fork failed");
			}

			if (child_pr == 0) {
#if VERBOSE > 3
				output
				    ("[child] Child process is starting...\n");
#endif

				if (tf((void *)td) != NULL) {
					UNRESOLVED(-1,
						   "[child] Got an unexpected return value from test function");
				} else {
					/* We cannot use the PASSED macro here since it would terminate the output */
					exit(0);
				}
			}
			/* Only the parent process goes further */
		} else {	/* do_fork == 0 */

			/* We are testing across two threads */
			ret = pthread_create(&child_th, NULL, tf, td);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to create the child thread.");
			}
		}

		/* Wait for the child to terminate */
		if (do_fork != 0) {
			/* We were testing across processes */
			ret = 0;
			chkpid = waitpid(child_pr, &status, 0);
			if (chkpid != child_pr) {
				output("Expected pid: %i. Got %i\n",
				       (int)child_pr, (int)chkpid);
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
				UNRESOLVED(-1,
					   "Child process was neither killed nor exited");
			}

			if (ret != 0) {
				exit(ret);	/* Output has already been closed in child */
			}

		} else {	/* child was a thread */

			ret = pthread_join(child_th, NULL);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to join the thread");
			}
		}

		/* Check the child status */
		if (td->status != EBUSY) {
			output("Unexpected return value: %d (%s)\n", td->status,
			       strerror(td->status));
			FAILED
			    ("pthread_mutex_trylock() did not return EBUSY in the child");
		}

		/* Unlock the mutex */
		ret = pthread_mutex_unlock(&(td->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to unlock the mutex");
		}

/**********
 * Destroy the data
 */
		ret = pthread_mutex_destroy(&(td->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to destroy the mutex");
		}

		ret = pthread_mutexattr_destroy(&ma);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Failed to destroy the mutex attribute object");
		}

	}			/* Proceed to the next scenario */

#if VERBOSE > 0
	output("Test passed\n");
#endif

	PASSED;
}
