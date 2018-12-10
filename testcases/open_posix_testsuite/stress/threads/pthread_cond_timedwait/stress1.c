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

 * This file is a stress test for the function pthread_cond_timedwait.
 *
 *It aims to check the following assertion:
 *  When inside the function, the thread releases the mutex
 *  before waiting for the conditionnal variable.
 *  Those two operations are atomic in the mean that
 *  no other thread can gain access to the mutex
 *  then signal (or broadcast) the condition
 *  without the blocked thread behaving as if
 *  this signal (or broadcast) had happened
 *  after it blocked on the conditionnal variable.

 * The steps are:
 * -> Create N mutex & N cond vars with different attributes
 * -> Create N threads A, which
 *    -> locks the mutex
 *    -> create a thread B, which
 *       -> locks the mutex
 *       -> while the boolean is false,
 *         -> broadcasts the condvar
 *         -> timedwaits the condition for 10 seconds
 *       -> broadcasts the condvar
 *       -> unlock the mutex
 *    -> while the boolean is false,
 *      -> timedwaits the condvar for 10 seconds
 *      -> signals the condvar
 *    -> unlock the mutex
 *    -> joins the thread B
 * -> sets the boolean True when it receives SIGUSR1
 * -> joins the N threads A.
 *
 * the test fails when a broadcast returns with a timeout.
 *
 * To test for pshared primitive, thread B could be in another process.
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
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
#include "testfrmw.h"
#include "testfrmw.c"
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
#ifndef SCALABILITY_FACTOR
#define SCALABILITY_FACTOR 1
#endif
#ifndef VERBOSE
#define VERBOSE 1
#endif

/* Number of children for each test scenario */
#define NCHILDREN (5)

#define TIMEOUT 120

#ifndef WITHOUT_ALTCLK
#define USE_ALTCLK		/* make tests with MONOTONIC CLOCK if supported */
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

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

#define NSCENAR (sizeof(scenarii)/sizeof(scenarii[0]))

#define NTOT (NSCENAR * SCALABILITY_FACTOR * NCHILDREN)

struct childdata {
	pthread_mutex_t mtx;
	pthread_cond_t cnd;
	clockid_t cid;
	int fork;
	int *pBool;
};

typedef struct {
	struct childdata cd[NTOT];
	int boolean;
} testdata_t;

pthread_attr_t ta;

/***
 * The grand child function (either sub-thread or sub-process)
 */
void *threaded_B(void *arg)
{
	int ret;
	struct timespec ts;
	struct childdata *cd = (struct childdata *)arg;

	ret = pthread_mutex_lock(&(cd->mtx));
	if (ret != 0) {
		UNRESOLVED(ret, "[gchild] Unable to lock mutex");
	}

	while (*(cd->pBool) == 0) {
		ret = pthread_cond_broadcast(&(cd->cnd));
		if (ret != 0) {
			UNRESOLVED(ret, "[gchild] Broadcast failed");
		}

		ret = clock_gettime(cd->cid, &ts);
		if (ret != 0) {
			UNRESOLVED(errno, "[gchild] Unable to read clock");
		}

		ts.tv_sec += TIMEOUT;

		ret = pthread_cond_timedwait(&(cd->cnd), &(cd->mtx), &ts);
		if (ret == ETIMEDOUT) {
			FAILED
			    ("[gchild] Timeout occured. This means a cond signal was lost -- or parent died");
		}
		if (ret != 0) {
			UNRESOLVED(ret, "[gchild] Failed to wait the cond");
		}
	}

	/* We shall broadcast again to be sure the parent is not hung */
	ret = pthread_cond_broadcast(&(cd->cnd));
	if (ret != 0) {
		UNRESOLVED(ret, "[gchild] Broadcast failed");
	}

	ret = pthread_mutex_unlock(&(cd->mtx));
	if (ret != 0) {
		UNRESOLVED(ret, "[gchild] Failed to finally release the mutex");
	}

	return NULL;
}

/***
 * The child function (always in the main thread)
 */
void *threaded_A(void *arg)
{
	struct childdata *cd = (struct childdata *)arg;
	int ret, status;
	pid_t child_p = 0, wrc;
	pthread_t child_t;

	struct timespec ts;

	ret = pthread_mutex_lock(&(cd->mtx));
	if (ret != 0) {
		UNRESOLVED(ret, "[child] Unable to lock mutex");
	}

	/* Create the grand child */
	if (cd->fork == 0) {
		ret = pthread_create(&child_t, &ta, threaded_B, arg);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "[child] Failed to create a grand child thread");
		}
	} else {
		child_p = fork();
		if (child_p == -1) {
			UNRESOLVED(ret,
				   "[child] Failed to create a grand child proces");
		}

		if (child_p == 0) {	/* grand child */
			threaded_B(arg);
			exit(0);
		}
	}

	while (*(cd->pBool) == 0) {
		ret = clock_gettime(cd->cid, &ts);
		if (ret != 0) {
			UNRESOLVED(errno, "[child] Unable to read clock");
		}

		ts.tv_sec += TIMEOUT;

		ret = pthread_cond_timedwait(&(cd->cnd), &(cd->mtx), &ts);
		if (ret == ETIMEDOUT) {
			FAILED
			    ("[child] Timeout occured. This means a cond broadcast was lost -- or gchild died");
		}
		if (ret != 0) {
			UNRESOLVED(ret, "[child] Failed to wait the cond");
		}

		ret = pthread_cond_signal(&(cd->cnd));
		if (ret != 0) {
			UNRESOLVED(ret, "[child] Signal failed");
		}
	}

	ret = pthread_mutex_unlock(&(cd->mtx));
	if (ret != 0) {
		UNRESOLVED(ret, "[gchild] Failed to finally release the mutex");
	}

	/* Wait for the grand child termination */
	if (cd->fork == 0) {
		ret = pthread_join(child_t, NULL);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "[child] Failed to join a grand child thread");
		}
	} else {
		wrc = waitpid(child_p, &status, 0);
		if (wrc != child_p) {
			output("Expected pid: %i. Got %i\n", (int)child_p,
			       (int)wrc);
			UNRESOLVED(errno, "Waitpid failed");
		}

		if (WIFSIGNALED(status)) {
			output("Child process killed with signal %d\n",
			       WTERMSIG(status));
			UNRESOLVED(0, "Child process was killed");
		}

		if (WIFEXITED(status)) {
			ret = WEXITSTATUS(status);
		} else {
			UNRESOLVED(0,
				   "Child process was neither killed nor exited");
		}
	}

	/* the end */
	return NULL;
}

int *pBoolean = NULL;

/***
 * Signal handler
 */
void sighdl(int sig)
{
#if VERBOSE > 1
	output("Received the USR1 signal; stopping everything\n");
#endif
	*pBoolean = 1;
}

int main(int argc, char *argv[])
{
	int ret, i, j;
	struct sigaction sa;

	pthread_mutexattr_t ma;
	pthread_condattr_t ca;
	clockid_t cid = CLOCK_REALTIME;

	testdata_t *td;
	testdata_t alternativ;

	int do_fork;
	long pshared, monotonic, cs, mf;

	pthread_t th[NTOT];

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
		char filename[] = "/tmp/cond_timedwait_st1-XXXXXX";
		size_t sz, ps;
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

		ps = (size_t) sysconf(_SC_PAGESIZE);
		sz = ((sizeof(testdata_t) / ps) + 1) * ps;	/* # pages needed to store the testdata */

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
		output("Testdata allocated in shared memory (%ib).\n",
		       sizeof(testdata_t));
#endif
	}

	/* Init the signal handler variable */
	pBoolean = &(td->boolean);

	/* Init the structure */
	for (i = 0; i < NSCENAR; i++) {
#if VERBOSE > 1
		output("[parent] Preparing attributes for: %s\n",
		       scenarii[i].descr);
#ifdef WITHOUT_XOPEN
		output("[parent] Mutex attributes DISABLED -> not used\n");
#endif
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
#ifndef WITHOUT_XOPEN
		/* Set the mutex type */
		ret = pthread_mutexattr_settype(&ma, scenarii[i].m_type);
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Unable to set mutex type");
		}
#if VERBOSE > 1
		output("[parent] Mutex type : %i\n", scenarii[i].m_type);
#endif
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
		ret = pthread_condattr_getclock(&ca, &cid);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to get clock from cond attr");
		}
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

		/* Initialize all the mutex and condvars which uses those attributes */
		for (j = 0; j < SCALABILITY_FACTOR * NCHILDREN; j++) {
#define CD (td->cd[i+(j*NSCENAR)])
			CD.pBool = &(td->boolean);
			CD.fork = do_fork;
			CD.cid = cid;

			/* initialize the condvar */
			ret = pthread_cond_init(&(CD.cnd), &ca);
			if (ret != 0) {
				UNRESOLVED(ret, "[parent] Cond init failed");
			}

			/* initialize the mutex */
			ret = pthread_mutex_init(&(CD.mtx), &ma);
			if (ret != 0) {
				UNRESOLVED(ret, "[parent] Mutex init failed");
			}
#undef CD
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
	}
#if VERBOSE > 1
	output("[parent] All condvars & mutex are ready\n");
#endif

	ret = pthread_attr_init(&ta);
	if (ret != 0) {
		UNRESOLVED(ret,
			   "[parent] Failed to initialize a thread attribute object");
	}
	ret = pthread_attr_setstacksize(&ta, sysconf(_SC_THREAD_STACK_MIN));
	if (ret != 0) {
		UNRESOLVED(ret, "[parent] Failed to set thread stack size");
	}

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sighdl;
	if ((ret = sigaction(SIGUSR1, &sa, NULL))) {
		UNRESOLVED(ret, "Unable to register signal handler");
	}
#if VERBOSE > 1
	output("[parent] Signal handler registered\n");
#endif

	for (i = 0; i < NTOT; i++) {
		ret = pthread_create(&th[i], &ta, threaded_A, &(td->cd[i]));
		/* In case of failure we can exit; the child process will die after a while */
		if (ret != 0) {
			UNRESOLVED(ret, "[Parent] Failed to create a thread");
		}
#if VERBOSE > 1
		if ((i % 10) == 0)
			output("[parent] %i threads created...\n", i + 1);
#endif
	}

#if VERBOSE > 1
	output("[parent] All %i threads are running...\n", NTOT);
#endif

	for (i = 0; i < NTOT; i++) {
		ret = pthread_join(th[i], NULL);
		if (ret != 0) {
			UNRESOLVED(ret, "[Parent] Failed to join a thread");
		}
	}

	/* Destroy everything */
	for (i = 0; i < NTOT; i++) {
		/* destroy the condvar */
		ret = pthread_cond_destroy(&(td->cd[i].cnd));
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Cond destroy failed");
		}

		/* destroy the mutex */
		ret = pthread_mutex_init(&(td->cd[i].mtx), &ma);
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Mutex destroy failed");
		}
	}

#if VERBOSE > 0
	output("Test passed\n");
#endif

	PASSED;
}
