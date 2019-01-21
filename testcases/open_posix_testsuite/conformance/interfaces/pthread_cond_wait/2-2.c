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
 * When the function returns successfully, everything is as if
 * the thread had locked the mutex.
 *

 * The steps are:
 * -> For each mutex type;
 *   -> with and without process-shared primitive if this is supported;
 *   -> with different clocks if this is supported,
 * -> Initialize a condvar and a mutex.
 * -> Create a new thread (or process for process-shared condvars & mutex)
 * -> The new thread (process) locks the mutex, then waits for the condition .
 * -> The parent thread (process) then locks the mutex, ensures that the child is waiting,
 *    then signals the condition; and checks the child does not leave the wait function.
 * -> The parent unlocks the mutex then joins the child.
 * -> The child checks that it owns the mutex; then it leaves.
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

#ifndef WITHOUT_ALTCLK
#define USE_ALTCLK		/* make tests with MONOTONIC CLOCK if supported */
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/
#ifndef WITHOUT_XOPEN

typedef struct {
	pthread_mutex_t mtx;
	pthread_cond_t cnd;
	clockid_t cid;		/* Clock id used by the cond var */
	int type;		/* Mutex type */
	int ctrl;		/* checkpoints */
	int bool;		/* Boolean predicate for the condition */
	int status;		/* error code */
} testdata_t;

struct _scenar {
	int m_type;		/* Mutex type to use */
	int mc_pshared;		/* 0: mutex and cond are process-private (default) ~ !0: Both are process-shared, if supported */
	int c_clock;		/* 0: cond uses the default clock. ~ !0: Cond uses monotonic clock, if supported. */
	int fork;		/* 0: Test between threads. ~ !0: Test across processes, if supported (mmap) */
	char *descr;		/* Case description */
} scenarii[] = {
	{
	PTHREAD_MUTEX_DEFAULT, 0, 0, 0, "Default mutex"}
	, {
	PTHREAD_MUTEX_NORMAL, 0, 0, 0, "Normal mutex"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 0, 0, 0, "Errorcheck mutex"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 0, 0, 0, "Recursive mutex"}

	, {
	PTHREAD_MUTEX_DEFAULT, 1, 0, 0, "PShared default mutex"}
	, {
	PTHREAD_MUTEX_NORMAL, 1, 0, 0, "Pshared normal mutex"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 1, 0, 0, "Pshared errorcheck mutex"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 1, 0, 0, "Pshared recursive mutex"}

	, {
	PTHREAD_MUTEX_DEFAULT, 1, 0, 1,
		    "Pshared default mutex across processes"}
	, {
	PTHREAD_MUTEX_NORMAL, 1, 0, 1,
		    "Pshared normal mutex across processes"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 1, 0, 1,
		    "Pshared errorcheck mutex across processes"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 1, 0, 1,
		    "Pshared recursive mutex across processes"}

#ifdef USE_ALTCLK
	, {
	PTHREAD_MUTEX_DEFAULT, 1, 1, 1,
		    "Pshared default mutex and alt clock condvar across processes"}
	, {
	PTHREAD_MUTEX_NORMAL, 1, 1, 1,
		    "Pshared normal mutex and alt clock condvar across processes"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 1, 1, 1,
		    "Pshared errorcheck mutex and alt clock condvar across processes"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 1, 1, 1,
		    "Pshared recursive mutex and alt clock condvar across processes"}

	, {
	PTHREAD_MUTEX_DEFAULT, 0, 1, 0,
		    "Default mutex and alt clock condvar"}
	, {
	PTHREAD_MUTEX_NORMAL, 0, 1, 0,
		    "Normal mutex and alt clock condvar"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 0, 1, 0,
		    "Errorcheck mutex and alt clock condvar"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 0, 1, 0,
		    "Recursive mutex and alt clock condvar"}

	, {
	PTHREAD_MUTEX_DEFAULT, 1, 1, 0,
		    "PShared default mutex and alt clock condvar"}
	, {
	PTHREAD_MUTEX_NORMAL, 1, 1, 0,
		    "Pshared normal mutex and alt clock condvar"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, 1, 1, 0,
		    "Pshared errorcheck mutex and alt clock condvar"}
	, {
	PTHREAD_MUTEX_RECURSIVE, 1, 1, 0,
		    "Pshared recursive mutex and alt clock condvar"}
#endif
};

#define NSCENAR (sizeof(scenarii) / sizeof(scenarii[0]))

void *tf(void *arg)
{
	int ret = 0;

	testdata_t *td = (testdata_t *) arg;

	/* Lock the mutex */
	ret = pthread_mutex_lock(&(td->mtx));
	if (ret != 0) {
		td->status = ret;
		UNRESOLVED(ret, "[child] Unable to lock the mutex");
	}

	/* Tell the parent the mutex is locked */
	td->ctrl = 1;

	/* Enter the wait */
	do {
		ret = pthread_cond_wait(&(td->cnd), &(td->mtx));
		td->ctrl = 2;
	} while ((ret == 0) && (td->bool == 0));

	td->ctrl = 3;

	if (ret != 0) {
		td->status = ret;
		UNRESOLVED(ret, "[child] Cond wait returned an error");
	}

	/* Make sure we are owning the mutex */
	ret = pthread_mutex_trylock(&(td->mtx));
	if (td->type == PTHREAD_MUTEX_RECURSIVE) {
#if VERBOSE > 1
		output
		    ("[child] Recursive mutex. Test if we are able to re-lock.\n");
#endif
		if (ret != 0) {
			td->status = ret;
			FAILED("[child] Unable to relock the recursive mutex");
		}
		ret = pthread_mutex_unlock(&(td->mtx));
		if (ret != 0) {
			td->status = ret;
			UNRESOLVED(ret, "[child] Failed to unlock the mutex");
		}
	} else {		/* This was not a recursive mutex; the call must have failed */

		if (ret == 0) {
			td->status = -1;
			FAILED
			    ("[child] Thread did not owned the mutex after the wait return.");
		}
		if (ret != EBUSY) {
			td->status = ret;
			UNRESOLVED(ret,
				   "[child] Mutex trylock did not return EBUSY");
		}
#if VERBOSE > 1
		output("[child] The mutex was busy (normal).\n");
#endif
	}

	ret = pthread_mutex_unlock(&(td->mtx));
	if (ret != 0) {
		td->status = ret;
		output("[child] Got error %i: %s\n", ret, strerror(ret));
		FAILED
		    ("[child] Failed to unlock the mutex - owned by another thread?");
	}

	td->ctrl = 4;
	return NULL;
}

int main(void)
{
	int ret, i;
	pthread_mutexattr_t ma;
	pthread_condattr_t ca;

	testdata_t *td;
	testdata_t alternativ;

	int do_fork;

	pid_t child_pr = 0, chkpid;
	int status;
	pthread_t child_th;

	long pshared, monotonic, cs, mf;
	struct timespec wait_ts = {0, 100000};

	output_init();
	pshared = sysconf(_SC_THREAD_PROCESS_SHARED);
	cs = sysconf(_SC_CLOCK_SELECTION);
	monotonic = sysconf(_SC_MONOTONIC_CLOCK);
	mf = sysconf(_SC_MAPPED_FILES);

#if VERBOSE > 0
	output("Test starting\n");
	output("System abilities:\n");
	output(" TPS : %li\n", pshared);
	output(" CS  : %li\n", cs);
	output(" MON : %li\n", monotonic);
	output(" MF  : %li\n", mf);
	if ((mf < 0) || (pshared < 0))
		output("Process-shared attributes won't be tested\n");
	if ((cs < 0) || (monotonic < 0))
		output("Alternative clock won't be tested\n");
#endif

	/* We are not interested in testing the clock if we have no other clock available.. */
	if (monotonic < 0)
		cs = -1;

#ifndef USE_ALTCLK
	if (cs > 0)
		output
		    ("Implementation supports the MONOTONIC CLOCK but option is disabled in test.\n");
#endif

/**********
 * Allocate space for the testdata structure
 */
	if (mf < 0) {
		/* Cannot mmap a file, we use an alternative method */
		td = &alternativ;
		pshared = -1;	/* We won't do this testing anyway */
#if VERBOSE > 0
		output("Testdata allocated in the process memory.\n");
#endif
	} else {
		/* We will place the test data in a mmaped file */
		char filename[] = "/tmp/cond_wait_2-2-XXXXXX";
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

/**********
 * For each test scenario, initialize the attributes and other variables.
 */
	for (i = 0; i < (int)NSCENAR; i++) {
#if VERBOSE > 1
		output("[parent] Preparing attributes for: %s\n",
		       scenarii[i].descr);
#endif
		/* set / reset everything */
		do_fork = 0;
		ret = pthread_mutexattr_init(&ma);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "[parent] Unable to initialize the mutex attribute object");
		}
		ret = pthread_condattr_init(&ca);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "[parent] Unable to initialize the cond attribute object");
		}

		/* Set the mutex type */
		ret = pthread_mutexattr_settype(&ma, scenarii[i].m_type);
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Unable to set mutex type");
		}
#if VERBOSE > 1
		output("[parent] Mutex type : %i\n", scenarii[i].m_type);
#endif

		/* Set the pshared attributes, if supported */
		if ((pshared > 0) && (scenarii[i].mc_pshared != 0)) {
			ret =
			    pthread_mutexattr_setpshared(&ma,
							 PTHREAD_PROCESS_SHARED);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to set the mutex process-shared");
			}
			ret =
			    pthread_condattr_setpshared(&ca,
							PTHREAD_PROCESS_SHARED);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to set the cond var process-shared");
			}
#if VERBOSE > 1
			output("[parent] Mutex & cond are process-shared\n");
#endif
		}
#if VERBOSE > 1
		else {
			output("[parent] Mutex & cond are process-private\n");
		}
#endif

		/* Set the alternative clock, if supported */
#ifdef USE_ALTCLK
		if ((cs > 0) && (scenarii[i].c_clock != 0)) {
			ret = pthread_condattr_setclock(&ca, CLOCK_MONOTONIC);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to set the monotonic clock for the cond");
			}
#if VERBOSE > 1
			output("[parent] Cond uses the Monotonic clock\n");
#endif
		}
#if VERBOSE > 1
		else {
			output("[parent] Cond uses the default clock\n");
		}
#endif
#endif

		/* Tell whether the test will be across processes */
		if ((pshared > 0) && (scenarii[i].fork != 0)) {
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

		/* initialize the condvar */
		ret = pthread_cond_init(&(td->cnd), &ca);
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Cond init failed");
		}

		/* Initialize the other datas from the test structure */
#ifdef USE_ALTCLK
		ret = pthread_condattr_getclock(&ca, &(td->cid));
		if (ret != 0) {
			UNRESOLVED(ret,
				   "[parent] Unable to read cond clock attribute");
		}
#else
		td->cid = CLOCK_REALTIME;
#endif

		ret = pthread_mutexattr_gettype(&ma, &(td->type));
		if (ret != 0) {
			UNRESOLVED(ret,
				   "[parent] Unable to read mutex type attribute");
		}

		td->ctrl = 0;
		td->bool = 0;
		td->status = 0;

/**********
 * Proceed to the actual testing
 */

		/* Create the child */
		if (do_fork != 0) {
			/* We are testing across two processes */
			child_pr = fork();
			if (child_pr == -1) {
				UNRESOLVED(errno, "[parent] Fork failed");
			}

			if (child_pr == 0) {
#if VERBOSE > 1
				output("[child] Child process starting...\n");
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

		/* Note: in case of an error, the child process will be alive for 10 sec then exit. */

		/* Child is now running and will enter the timedwait */
		/* We are waiting for this; and we have to monitor the status value as well. */
		ret = pthread_mutex_lock(&(td->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Unable to lock the mutex");
		}

		while ((td->ctrl == 0) && (td->status == 0)) {
			ret = pthread_mutex_unlock(&(td->mtx));
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to unlock the mutex");
			}
			sched_yield();
			ret = pthread_mutex_lock(&(td->mtx));
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to lock the mutex");
			}
		}

		if ((td->ctrl == 2) && (td->status == 0)) {	/* Spurious wakeups hapenned */
			output
			    ("Spurious wake ups have happened. Maybe pthread_cond_wait is broken?\n");
			td->ctrl = 1;
		}

		if (td->ctrl == 1) {	/* The child is inside the cond wait */

			ret = pthread_cond_signal(&(td->cnd));
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to signal the condition");
			}

			/* Let the child leave the wait function if something is broken */
			nanosleep(&wait_ts, NULL);

			if (td->ctrl != 1) {
				FAILED
				    ("[parent] Child went out from pthread_cond_wait without locking the mutex");
			}

			/* Allow the child to continue */
			td->bool = 1;
		}

		/* Let the child do its checking */
		ret = pthread_mutex_unlock(&(td->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Unable to unlock the mutex");
		}

		/* Wait for the child to terminate */
		if (do_fork != 0) {
			/* We were testing across two processes */
			chkpid = waitpid(child_pr, &status, 0);
			if (chkpid != child_pr) {
				output("Expected pid: %i. Got %i\n",
				       (int)child_pr, (int)chkpid);
				UNRESOLVED(errno, "Waitpid failed");
			}
			if (WIFSIGNALED(status)) {
				output("Child process killed with signal %d\n",
				       WTERMSIG(status));
				UNRESOLVED(td->status,
					   "Child process was killed");
			}

			if (WIFEXITED(status)) {
				ret = WEXITSTATUS(status);
			} else {
				UNRESOLVED(td->status,
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

/**********
 * Destroy the data
 */
		ret = pthread_cond_destroy(&(td->cnd));
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to destroy the cond var");
		}

		ret = pthread_mutex_destroy(&(td->mtx));
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to destroy the mutex");
		}

		ret = pthread_condattr_destroy(&ca);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Failed to destroy the cond var attribute object");
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

#else /* WITHOUT_XOPEN */
int main(void)
{
	output_init();
	UNTESTED("This test requires XSI features");
}
#endif
