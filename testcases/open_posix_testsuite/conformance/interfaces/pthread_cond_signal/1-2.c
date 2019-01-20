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
 * pthread_cond_signal() unblocks at least one thread which is blocked
 * on the conditional variable, if any.

 * The steps are:
 *  -> Create a lot of threads/process which will wait on a condition variable
 *  -> Cascade-signal the condition and check that all threads/processes are awaken.
 *
 *  The test will fail when the threads are not terminated within a certain duration.
 *
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
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/wait.h>

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
#define UNRESOLVED_KILLALL(error, text, Tchild) { \
	if (td->fork) \
	{ \
		int _nch; \
		for (_nch=0; _nch<NTHREADS; _nch++) \
			kill(Tchild[_nch], SIGKILL); \
	} \
	UNRESOLVED(error, text); \
	}
#define FAILED_KILLALL(text, Tchild) { \
	if (td->fork) \
	{ \
		int _nch; \
		for (_nch=0; _nch<NTHREADS; _nch++) \
			kill(Tchild[_nch], SIGKILL); \
	} \
	FAILED(text); \
	}
/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

#define NTHREADS (20)

#define TIMEOUT  (120)

#ifndef WITHOUT_ALTCLK
#define USE_ALTCLK		/* make tests with MONOTONIC CLOCK if supported */
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

#ifdef WITHOUT_XOPEN
/* We define those to avoid compilation errors, but they won't be used */
#define PTHREAD_MUTEX_DEFAULT 0
#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_ERRORCHECK 0
#define PTHREAD_MUTEX_RECURSIVE 0

#endif

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

/* The shared data */
typedef struct {
	int count;		/* number of children currently waiting */
	pthread_cond_t cnd;
	pthread_mutex_t mtx;
	int predicate;		/* Boolean associated to the condvar */
	clockid_t cid;		/* clock used in the condvar */
	char fork;		/* the children are processes */
} testdata_t;
testdata_t *td;

/* Child function (either in a thread or in a process) */
void *child(void *arg LTP_ATTRIBUTE_UNUSED)
{
	int ret = 0;
	struct timespec ts;
	char timed;

	/* lock the mutex */
	ret = pthread_mutex_lock(&td->mtx);
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to lock mutex in child");
	}

	/* increment count */
	td->count++;

	timed = td->count & 1;

	if (timed) {
		/* get current time if we are a timedwait */
		ret = clock_gettime(td->cid, &ts);
		if (ret != 0) {
			UNRESOLVED(errno, "Unable to read clock");
		}
		ts.tv_sec += TIMEOUT;
	}

	do {
		/* Wait while the predicate is false */
		if (timed)
			ret = pthread_cond_timedwait(&td->cnd, &td->mtx, &ts);
		else
			ret = pthread_cond_wait(&td->cnd, &td->mtx);
#if VERBOSE > 5
		output("[child] Wokenup timed=%i, Predicate=%i, ret=%i\n",
		       timed, td->predicate, ret);
#endif
	} while ((ret == 0) && (td->predicate == 0));
	if (ret == ETIMEDOUT) {
		FAILED
		    ("Timeout occured. This means a cond signal was lost -- or parent died");
	}
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to wait for the cond");
	}

	/* Signal the condition to cascade */
	ret = pthread_cond_signal(&td->cnd);
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to cascade signal the cond");
	}

	/* unlock the mutex */
	ret = pthread_mutex_unlock(&td->mtx);
	if (ret != 0) {
		UNRESOLVED(ret, "Failed to unlock the mutex.");
	}

	return NULL;
}

/* Timeout thread */
void *timer(void *arg)
{
	pid_t *pchildren = (pid_t *) arg;
	unsigned int to = TIMEOUT;
	do {
		to = sleep(to);
	}
	while (to > 0);
	FAILED_KILLALL("Operation timed out. A signal was lost.", pchildren);
	return NULL;		/* For compiler */
}

/* main function */

int main(void)
{
	int ret;

	pthread_mutexattr_t ma;
	pthread_condattr_t ca;

	int scenar;
	long pshared, monotonic, cs, mf;

	pid_t p_child[NTHREADS];
	pthread_t t_child[NTHREADS];
	int ch;
	pid_t pid;
	int status;

	pthread_t t_timer;

	testdata_t alternativ;

	output_init();

	/* check the system abilities */
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
		char filename[] = "/tmp/cond_wait_stress-XXXXXX";
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

	/* Do the test for each test scenario */
	for (scenar = 0; scenar < (int)NSCENAR; scenar++) {
		/* set / reset everything */
		td->fork = 0;
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
		ret = pthread_mutexattr_settype(&ma, scenarii[scenar].m_type);
		if (ret != 0) {
			UNRESOLVED(ret, "[parent] Unable to set mutex type");
		}
#endif

		/* Set the pshared attributes, if supported */
		if ((pshared > 0) && (scenarii[scenar].mc_pshared != 0)) {
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
		}

		/* Set the alternative clock, if supported */
#ifdef USE_ALTCLK
		if ((cs > 0) && (scenarii[scenar].c_clock != 0)) {
			ret = pthread_condattr_setclock(&ca, CLOCK_MONOTONIC);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to set the monotonic clock for the cond");
			}
		}
		ret = pthread_condattr_getclock(&ca, &td->cid);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to get clock from cond attr");
		}
#else
		td->cid = CLOCK_REALTIME;
#endif

		/* Tell whether the test will be across processes */
		if ((pshared > 0) && (scenarii[scenar].fork != 0)) {
			td->fork = 1;
		}

		/* initialize the condvar */
		ret = pthread_cond_init(&td->cnd, &ca);
		if (ret != 0) {
			UNRESOLVED(ret, "Cond init failed");
		}

		/* initialize the mutex */
		ret = pthread_mutex_init(&td->mtx, &ma);
		if (ret != 0) {
			UNRESOLVED(ret, "Mutex init failed");
		}

		/* Destroy the attributes */
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
#if VERBOSE > 2
		output("[parent] Starting test %s\n", scenarii[scenar].descr);
#endif

		td->count = 0;
		/* Create all the children */
		for (ch = 0; ch < NTHREADS; ch++) {
			if (td->fork == 0) {
				ret =
				    pthread_create(&t_child[ch], NULL, child,
						   NULL);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Failed to create a child thread");
				}
			} else {
				p_child[ch] = fork();
				if (p_child[ch] == -1) {
					ret = errno;
					for (--ch; ch >= 0; ch--)
						kill(p_child[ch], SIGKILL);
					UNRESOLVED(ret,
						   "Failed to create a child process");
				}

				if (p_child[ch] == 0) {	/* We are the child */
					child(NULL);
					exit(0);
				}
			}
		}
#if VERBOSE > 4
		output("[parent] All children are running\n");
#endif

		/* Make sure all children are waiting */
		ret = pthread_mutex_lock(&td->mtx);
		if (ret != 0) {
			UNRESOLVED_KILLALL(ret, "Failed to lock mutex",
					   p_child);
		}
		ch = td->count;
		while (ch < NTHREADS) {
			ret = pthread_mutex_unlock(&td->mtx);
			if (ret != 0) {
				UNRESOLVED_KILLALL(ret,
						   "Failed to unlock mutex",
						   p_child);
			}
			sched_yield();
			ret = pthread_mutex_lock(&td->mtx);
			if (ret != 0) {
				UNRESOLVED_KILLALL(ret, "Failed to lock mutex",
						   p_child);
			}
			ch = td->count;
		}

#if VERBOSE > 4
		output("[parent] All children are waiting\n");
#endif

		/* create the timeout thread */
		ret = pthread_create(&t_timer, NULL, timer, p_child);
		if (ret != 0) {
			UNRESOLVED_KILLALL(ret, "Unable to create timer thread",
					   p_child);
		}

		/* Wakeup the children */
		td->predicate = 1;
		ret = pthread_cond_signal(&td->cnd);
		if (ret != 0) {
			UNRESOLVED_KILLALL(ret,
					   "Failed to signal the condition.",
					   p_child);
		}
#if VERBOSE > 4
		output("[parent] Condition was signaled\n");
#endif

		ret = pthread_mutex_unlock(&td->mtx);
		if (ret != 0) {
			UNRESOLVED_KILLALL(ret, "Failed to unlock mutex",
					   p_child);
		}
#if VERBOSE > 4
		output("[parent] Joining the children\n");
#endif

		/* join the children */
		for (ch = (NTHREADS - 1); ch >= 0; ch--) {
			if (td->fork == 0) {
				ret = pthread_join(t_child[ch], NULL);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Failed to join a child thread");
				}
			} else {
				pid = waitpid(p_child[ch], &status, 0);
				if (pid != p_child[ch]) {
					ret = errno;
					output
					    ("Waitpid failed (expected: %i, got: %i)\n",
					     p_child[ch], pid);
					for (; ch >= 0; ch--) {
						kill(p_child[ch], SIGKILL);
					}
					UNRESOLVED(ret, "Waitpid failed");
				}
				if (WIFEXITED(status)) {
					/* the child should return only failed or unresolved or passed */
					if (ret != PTS_FAIL)
						ret |= WEXITSTATUS(status);
				}
			}
		}
		if (ret != 0) {
			output_fini();
			exit(ret);
		}
#if VERBOSE > 4
		output("[parent] All children terminated\n");
#endif

		/* cancel the timeout thread */
		ret = pthread_cancel(t_timer);
		if (ret != 0) {
			/* Strange error here... the thread cannot be terminated (app would be killed) */
			UNRESOLVED(ret, "Failed to cancel the timeout handler");
		}

		/* join the timeout thread */
		ret = pthread_join(t_timer, NULL);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to join the timeout handler");
		}

		/* Destroy the datas */
		ret = pthread_cond_destroy(&td->cnd);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to destroy the condvar");
		}

		ret = pthread_mutex_destroy(&td->mtx);
		if (ret != 0) {
			UNRESOLVED(ret, "Failed to destroy the mutex");
		}

	}

	/* exit */
	PASSED;
}
