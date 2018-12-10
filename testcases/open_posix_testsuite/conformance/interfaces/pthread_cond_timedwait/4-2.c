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
 * When the abstime parameter is invalid,
 * the function must return EINVAL and
 * the mutex state must not have changed during the call.

 * The steps are:
 *  -> parent (for each mutex type and each condvar options, across threads or processes)
 *     -> locks the mutex m
 *     -> sets ctrl = 0
 *     -> creates a bunch of children, which:
 *        -> lock the mutex m
 *        -> if ctrl == 0, test has failed
 *        -> unlock the mutex then exit
 *     -> calls pthread_cond_timedwait with invalid values (nsec > 999999999)
 *     -> sets ctrl = non-zero value
 *     -> unlocks the mutex m
 */

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

#define NCHILDREN (20)

#ifndef WITHOUT_ALTCLK
#define USE_ALTCLK		/* make tests with MONOTONIC CLOCK if supported */
#endif

#ifndef WITHOUT_XOPEN

typedef struct {
	pthread_mutex_t mtx;
	int ctrl;		/* Control value */
	int gotit;		/* Thread locked the mutex while ctrl == 0 */
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

struct {
	long sec_val;		/* Value for seconds */
	short sec_is_offset;	/* Seconds value is added to current time or is absolute */
	long nsec_val;		/* Value for nanoseconds */
	short nsec_is_offset;	/* Nanoseconds value is added to current time or is absolute */
} junks_ts[] = {
	{
	-2, 1, 1000000000, 1}
	, {
	-2, 1, -1, 0}
	, {
	-3, 1, 2000000000, 0}
};

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

	/* Checks whether the parent release the lock inside the timedwait function */
	if (td->ctrl == 0)
		td->gotit += 1;

	/* Unlock and exit */
	ret = pthread_mutex_unlock(&(td->mtx));
	if (ret != 0) {
		td->status = ret;
		UNRESOLVED(ret, "[child] Failed to unlock the mutex.");
	}
	return NULL;
}

int main(void)
{
	int ret, k;
	unsigned int i, j;
	pthread_mutexattr_t ma;
	pthread_condattr_t ca;
	pthread_cond_t cnd;
	clockid_t cid = CLOCK_REALTIME;
	struct timespec ts, ts_junk;

	testdata_t *td;
	testdata_t alternativ;

	int do_fork;

	pid_t child_pr[NCHILDREN], chkpid;
	int status;
	pthread_t child_th[NCHILDREN];

	long pshared, monotonic, cs, mf;

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
	fflush(stdout);
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
		char filename[] = "/tmp/cond_timedwait_2-4-XXXXXX";
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
 * Do the whole thing for each time to test.
 */
	for (i = 0; i < (sizeof(scenarii) / sizeof(scenarii[0])); i++) {
		for (j = 0; j < (sizeof(junks_ts) / sizeof(junks_ts[0])); j++) {
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
			ret =
			    pthread_mutexattr_settype(&ma, scenarii[i].m_type);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to set mutex type");
			}
#if VERBOSE > 1
			output("[parent] Mutex type : %i\n",
			       scenarii[i].m_type);
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
				output
				    ("[parent] Mutex & cond are process-shared\n");
#endif
			}
#if VERBOSE > 1
			else {
				output
				    ("[parent] Mutex & cond are process-private\n");
			}
#endif

			/* Set the alternative clock, if supported */
#ifdef USE_ALTCLK
			if ((cs > 0) && (scenarii[i].c_clock != 0)) {
				ret =
				    pthread_condattr_setclock(&ca,
							      CLOCK_MONOTONIC);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "[parent] Unable to set the monotonic clock for the cond");
				}
#if VERBOSE > 1
				output
				    ("[parent] Cond uses the Monotonic clock\n");
#endif
			}
#if VERBOSE > 1
			else {
				output
				    ("[parent] Cond uses the default clock\n");
			}
#endif
			ret = pthread_condattr_getclock(&ca, &cid);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to get clock from cond attr");
			}
#endif

			/* Tell whether the test will be across processes */
			if ((pshared > 0) && (scenarii[i].fork != 0)) {
				do_fork = 1;
#if VERBOSE > 1
				output
				    ("[parent] Child will be a new process\n");
#endif
			}
#if VERBOSE > 1
			else {
				output("[parent] Child will be a new thread\n");
			}
#endif

			/* initialize the condvar */
			ret = pthread_cond_init(&cnd, &ca);
			if (ret != 0) {
				UNRESOLVED(ret, "[parent] Cond init failed");
			}

/**********
 * Initialize the testdata_t structure with the previously defined attributes
 */
			/* Initialize the mutex */
			ret = pthread_mutex_init(&(td->mtx), &ma);
			if (ret != 0) {
				UNRESOLVED(ret, "[parent] Mutex init failed");
			}

			/* Initialize the other datas from the test structure */
			td->ctrl = 0;
			td->gotit = 0;
			td->status = 0;

/**********
 * Proceed to the actual testing
 */
			/* Lock the mutex before creating children */
			ret = pthread_mutex_lock(&(td->mtx));
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to lock the mutex");
			}

			/* Create the children */
			if (do_fork != 0) {
				/* We are testing across processes */
				for (k = 0; k < NCHILDREN; k++) {
					child_pr[k] = fork();
					if (child_pr[k] == -1) {
						UNRESOLVED(errno,
							   "[parent] Fork failed");
					}

					if (child_pr[k] == 0) {
#if VERBOSE > 3
						output
						    ("[child] Child process %i starting...\n",
						     k);
#endif

						if (tf((void *)td) != NULL) {
							UNRESOLVED(-1,
								   "[child] Got an unexpected return value from test function");
						} else {
							/* We cannot use the PASSED macro here since it would terminate the output */
							exit(0);
						}
					}
				}
				/* Only the parent process goes further */
			} else {	/* do_fork == 0 */

				/* We are testing across two threads */
				for (k = 0; k < NCHILDREN; k++) {
					ret =
					    pthread_create(&child_th[k], NULL,
							   tf, td);
					if (ret != 0) {
						UNRESOLVED(ret,
							   "[parent] Unable to create the child thread.");
					}
				}
			}

			/* Children are now running and trying to lock the mutex. */

			ret = clock_gettime(cid, &ts);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to read clock");
			}

			/* Do the junk timedwaits */
			ts_junk.tv_sec =
			    junks_ts[j].sec_val +
			    (junks_ts[j].sec_is_offset ? ts.tv_sec : 0);
			ts_junk.tv_nsec =
			    junks_ts[j].nsec_val +
			    (junks_ts[j].nsec_is_offset ? ts.tv_nsec : 0);

#if VERBOSE > 2
			output("TS: s = %s%li ; ns = %s%li\n",
			       junks_ts[j].sec_is_offset ? "n + " : " ",
			       junks_ts[j].sec_val,
			       junks_ts[j].nsec_is_offset ? "n + " : " ",
			       junks_ts[j].nsec_val);
			output("Now is: %i.%09li\n", ts.tv_sec, ts.tv_nsec);
			output("Junk is: %i.%09li\n", ts_junk.tv_sec,
			       ts_junk.tv_nsec);
#endif

			do {
				ret =
				    pthread_cond_timedwait(&cnd, &(td->mtx),
							   &ts_junk);
			} while (ret == 0);
#if VERBOSE > 2
			output("timedwait returns %d (%s) - gotit = %d\n", ret,
			       strerror(ret), td->gotit);
#endif

			/* check that when EINVAL is returned, the mutex has not been released */
			if (ret == EINVAL) {
				if (td->gotit != 0) {
					FAILED
					    ("The mutex was released when an invalid timestamp was detected in the function");
				}
#if VERBOSE > 0
			} else {
				output
				    ("Warning, struct timespec with tv_sec = %i and tv_nsec = %li was not invalid\n",
				     ts_junk.tv_sec, ts_junk.tv_nsec);
			}
#endif

			/* Finally unlock the mutex */
			td->ctrl = 1;
			ret = pthread_mutex_unlock(&(td->mtx));
			if (ret != 0) {
				UNRESOLVED(ret,
					   "[parent] Unable to unlock the mutex");
			}

			/* Wait for the child to terminate */
			if (do_fork != 0) {
				/* We were testing across processes */
				ret = 0;
				for (k = 0; k < NCHILDREN; k++) {
					chkpid =
					    waitpid(child_pr[k], &status, 0);
					if (chkpid != child_pr[k]) {
						output
						    ("Expected pid: %i. Got %i\n",
						     (int)child_pr[k],
						     (int)chkpid);
						UNRESOLVED(errno,
							   "Waitpid failed");
					}
					if (WIFSIGNALED(status)) {
						output
						    ("Child process killed with signal %d\n",
						     WTERMSIG(status));
						UNRESOLVED(-1,
							   "Child process was killed");
					}

					if (WIFEXITED(status)) {
						ret |= WEXITSTATUS(status);
					} else {
						UNRESOLVED(-1,
							   "Child process was neither killed nor exited");
					}
				}
				if (ret != 0) {
					exit(ret);	/* Output has already been closed in child */
				}

			} else {	/* child was a thread */

				for (k = 0; k < NCHILDREN; k++) {
					ret = pthread_join(child_th[k], NULL);
					if (ret != 0) {
						UNRESOLVED(ret,
							   "[parent] Unable to join the thread");
					}
				}
			}

/**********
 * Destroy the data
 */
			ret = pthread_cond_destroy(&cnd);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Failed to destroy the cond var");
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

		}		/* Proceed to the next junk timedwait value */
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
