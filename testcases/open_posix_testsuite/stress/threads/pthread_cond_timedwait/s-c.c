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
 *

 * This file is a scalability test for the pthread_cond_timedwait function.
 *
 * It aims to measure the time between end of timeout and actual wakeup
 * with different factors.

 * The steps are:
 * -*- Number of threads waiting on the conditionnal variable
 *     -> for an increaing number of threads,
 *        -> create the other threads which will do a pthread_cond_wait on the same cond/mutex
 *        -> When the threads are waiting, create one thread which will measure the time
 *        -> once the timeout has expired and the measure is done, broadcast the condition.
 *     -> do each measure 10 times (with different attributes i.e.)
 *
 * -*- other possible influencial parameters
 *     -> To be defined.
 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <time.h>
#include <errno.h>
#include <math.h>

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

#ifndef WITHOUT_ALTCLK
#define USE_ALTCLK		/* make tests with MONOTONIC CLOCK if supported */
#endif

#define MES_TIMEOUT  (1000000)	/* ns, offset for the pthread_cond_timedwait call */

#ifdef PLOT_OUTPUT
#undef VERBOSE
#define VERBOSE 0
#endif

// #define USE_CANCEL  /* Will cancel the threads instead of signaling the cond */#define

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

long altclk_ok, pshared_ok;

typedef struct {
	pthread_cond_t *cnd;
	pthread_mutex_t *mtx;
	int *predicate;
	int *tnum;
} test_t;

struct {
	int mutex_type;
	int pshared;
	clockid_t cid;
	char *desc;
} test_scenar[] = {
	{
	PTHREAD_MUTEX_DEFAULT, PTHREAD_PROCESS_PRIVATE, CLOCK_REALTIME,
		    "Default"}
	, {
	PTHREAD_MUTEX_DEFAULT, PTHREAD_PROCESS_SHARED, CLOCK_REALTIME,
		    "PShared"}
#ifndef WITHOUT_XOPEN
	, {
	PTHREAD_MUTEX_NORMAL, PTHREAD_PROCESS_PRIVATE, CLOCK_REALTIME,
		    "Normal"}
	, {
	PTHREAD_MUTEX_NORMAL, PTHREAD_PROCESS_SHARED, CLOCK_REALTIME,
		    "Normal+PShared"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, PTHREAD_PROCESS_PRIVATE,
		    CLOCK_REALTIME, "Errorcheck"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, PTHREAD_PROCESS_SHARED,
		    CLOCK_REALTIME, "Errorcheck+PShared"}
	, {
	PTHREAD_MUTEX_RECURSIVE, PTHREAD_PROCESS_PRIVATE,
		    CLOCK_REALTIME, "Recursive"}
	, {
	PTHREAD_MUTEX_RECURSIVE, PTHREAD_PROCESS_SHARED, CLOCK_REALTIME,
		    "Recursive+PShared"}
#endif
#ifdef USE_ALTCLK
	, {
	PTHREAD_MUTEX_DEFAULT, PTHREAD_PROCESS_PRIVATE, CLOCK_MONOTONIC,
		    "Monotonic"}
	, {
	PTHREAD_MUTEX_DEFAULT, PTHREAD_PROCESS_SHARED, CLOCK_MONOTONIC,
		    "PShared+Monotonic"}
#ifndef WITHOUT_XOPEN
	, {
	PTHREAD_MUTEX_NORMAL, PTHREAD_PROCESS_PRIVATE, CLOCK_MONOTONIC,
		    "Normal+Monotonic"}
	, {
	PTHREAD_MUTEX_NORMAL, PTHREAD_PROCESS_SHARED, CLOCK_MONOTONIC,
		    "Normal+PShared+Monotonic"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, PTHREAD_PROCESS_PRIVATE,
		    CLOCK_MONOTONIC, "Errorcheck+Monotonic"}
	, {
	PTHREAD_MUTEX_ERRORCHECK, PTHREAD_PROCESS_SHARED,
		    CLOCK_MONOTONIC, "Errorcheck+PShared+Monotonic"}
	, {
	PTHREAD_MUTEX_RECURSIVE, PTHREAD_PROCESS_PRIVATE,
		    CLOCK_MONOTONIC, "Recursive+Monotonic"}
	, {
	PTHREAD_MUTEX_RECURSIVE, PTHREAD_PROCESS_SHARED,
		    CLOCK_MONOTONIC, "Recursive+PShared+Monotonic"}
#endif
#endif
};

#define NSCENAR (sizeof(test_scenar) / sizeof(test_scenar[0]))

pthread_attr_t ta;

/* The next structure is used to save the tests measures */
typedef struct __mes_t {
	int nthreads;
	long _data[NSCENAR];	/* As we store µsec values, a long type should be amply enough. */
	struct __mes_t *next;
} mes_t;

/**** do_measure
 * This function will do a timedwait on the cond cnd after locking mtx.
 * Once the timedwait times out, it will read the clock cid then
 * compute the difference and put it into ts.
 * This function must be called once test is ready, as the timeout will be very short. */
void do_measure(pthread_mutex_t * mtx, pthread_cond_t * cnd, clockid_t cid,
		struct timespec *ts)
{
	int ret, rc;

	struct timespec ts_cnd, ts_clk;

	/* lock the mutex */
	ret = pthread_mutex_lock(mtx);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to lock mutex");
	}

	/* Prepare the timeout parameter */
	ret = clock_gettime(cid, &ts_cnd);
	if (ret != 0) {
		UNRESOLVED(errno, "Unable to read clock");
	}

	ts_cnd.tv_nsec += MES_TIMEOUT;
	while (ts_cnd.tv_nsec >= 1000000000) {
		ts_cnd.tv_nsec -= 1000000000;
		ts_cnd.tv_sec++;
	}

	/* Do the timedwait */
	do {
		rc = pthread_cond_timedwait(cnd, mtx, &ts_cnd);
		/* Re-read the clock as soon as possible */
		ret = clock_gettime(cid, &ts_clk);
		if (ret != 0) {
			UNRESOLVED(errno, "Unable to read clock");
		}
	}
	while (rc == 0);
	if (rc != ETIMEDOUT) {
		UNRESOLVED(rc,
			   "Timedwait returned an unexpected error (expected ETIMEDOUT)");
	}

	/* Compute the difference time */
	ts->tv_sec = ts_clk.tv_sec - ts_cnd.tv_sec;
	ts->tv_nsec = ts_clk.tv_nsec - ts_cnd.tv_nsec;
	if (ts->tv_nsec < 0) {
		ts->tv_nsec += 1000000000;
		ts->tv_sec -= 1;
	}

	if (ts->tv_sec < 0) {
		FAILED
		    ("The function returned from wait with a timeout before the time has passed\n");
	}

	/* unlock the mutex */
	ret = pthread_mutex_unlock(mtx);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to unlock mutex");
	}

	return;
}

void *waiter(void *arg)
{
	test_t *dt = (test_t *) arg;

	int ret;

	ret = pthread_mutex_lock(dt->mtx);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex lock failed in waiter");
	}
#ifdef USE_CANCEL
	pthread_cleanup_push((void *)pthread_mutex_unlock, (void *)(dt->mtx));
#endif

	/* This thread is ready to wait */
	*(dt->tnum) += 1;

	do {
		ret = pthread_cond_wait(dt->cnd, dt->mtx);
	}
	while ((ret == 0) && (*(dt->predicate) == 0));
	if (ret != 0) {
		UNRESOLVED(ret, "pthread_cond_wait failed in waiter");
	}
#ifdef USE_CANCEL
	pthread_cleanup_pop(0);	/* We could put 1 and avoid the next line, but we would miss the return code */
#endif

	ret = pthread_mutex_unlock(dt->mtx);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex unlock failed in waiter");
	}

	return NULL;
}

/**** do_threads_test
 * This function is responsible for all the testing with a given # of threads
 *  nthreads is the amount of threads to create.
 * the return value is:
 *  < 0 if function was not able to create enough threads.
 *  cumulated # of nanoseconds otherwise.
 */
long do_threads_test(int nthreads, mes_t * measure)
{
	int ret;

	int scal, i, j;

	pthread_t *th;

	int s;
	pthread_mutexattr_t ma;
	pthread_condattr_t ca;

	pthread_cond_t cnd;
	pthread_mutex_t mtx;
	int predicate;
	int tnum;

	test_t td;

	struct timespec ts, ts_cumul;

	td.mtx = &mtx;
	td.cnd = &cnd;
	td.predicate = &predicate;
	td.tnum = &tnum;

	/* Allocate space for the threads structures */
	th = (pthread_t *) calloc(nthreads, sizeof(pthread_t));
	if (th == NULL) {
		UNRESOLVED(errno, "Not enough memory for thread storage");
	}
#ifdef PLOT_OUTPUT
	output("%d", nthreads);
#endif
	/* For each test scenario (mutex and cond attributes) */
	for (s = 0; s < NSCENAR; s++) {
		/* Initialize the attributes */
		ret = pthread_mutexattr_init(&ma);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to initialize mutex attribute object");
		}

		ret = pthread_condattr_init(&ca);
		if (ret != 0) {
			UNRESOLVED(ret,
				   "Unable to initialize cond attribute object");
		}

		/* Set the attributes according to the scenar and the impl capabilities */
		ret = pthread_mutexattr_settype(&ma, test_scenar[s].mutex_type);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to set mutex type");
		}

		if (pshared_ok > 0) {
			ret =
			    pthread_mutexattr_setpshared(&ma,
							 test_scenar[s].
							 pshared);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to set mutex process-shared");
			}

			ret =
			    pthread_condattr_setpshared(&ca,
							test_scenar[s].pshared);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to set cond process-shared");
			}
		}
#ifdef USE_ALTCLK
		if (altclk_ok > 0) {
			ret =
			    pthread_condattr_setclock(&ca, test_scenar[s].cid);
			if (ret != 0) {
				UNRESOLVED(ret, "Unable to set clock for cond");
			}
		}
#endif

		ts_cumul.tv_sec = 0;
		ts_cumul.tv_nsec = 0;

#if VERBOSE > 1
		output("Starting case %s\n", test_scenar[s].desc);
#endif

		for (scal = 0; scal < 5 * SCALABILITY_FACTOR; scal++) {
			/* Initialize the mutex, the cond, and other data */
			ret = pthread_mutex_init(&mtx, &ma);
			if (ret != 0) {
				UNRESOLVED(ret, "Mutex init failed");
			}

			ret = pthread_cond_init(&cnd, &ca);
			if (ret != 0) {
				UNRESOLVED(ret, "Cond init failed");
			}

			predicate = 0;
			tnum = 0;

			/* Create the waiter threads */
			for (i = 0; i < nthreads; i++) {
				ret =
				    pthread_create(&th[i], &ta, waiter,
						   (void *)&td);
				if (ret != 0) {	/* We reached the limits */
#if VERBOSE > 1
					output
					    ("Limit reached with %i threads\n",
					     i);
#endif
#ifdef USE_CANCEL
					for (j = i - 1; j >= 0; j--) {
						ret = pthread_cancel(th[j]);
						if (ret != 0) {
							UNRESOLVED(ret,
								   "Unable to cancel a thread");
						}
					}
#else
					predicate = 1;
					ret = pthread_cond_broadcast(&cnd);
					if (ret != 0) {
						UNRESOLVED(ret,
							   "Unable to broadcast the condition");
					}
#endif
					for (j = i - 1; j >= 0; j--) {
						ret = pthread_join(th[j], NULL);
						if (ret != 0) {
							UNRESOLVED(ret,
								   "Unable to join a canceled thread");
						}
					}
					free(th);
					return -1;
				}
			}
			/* All waiter threads are created now */
#if VERBOSE > 5
			output("%i waiter threads created successfully\n", i);
#endif

			ret = pthread_mutex_lock(&mtx);
			if (ret != 0) {
				UNRESOLVED(ret, "Unable to lock mutex");
			}

			/* Wait for all threads be waiting */
			while (tnum < nthreads) {
				ret = pthread_mutex_unlock(&mtx);
				if (ret != 0) {
					UNRESOLVED(ret, "Mutex unlock failed");
				}

				sched_yield();

				ret = pthread_mutex_lock(&mtx);
				if (ret != 0) {
					UNRESOLVED(ret, "Mutex lock failed");
				}
			}

			/* All threads are now waiting - we do the measure */

			ret = pthread_mutex_unlock(&mtx);
			if (ret != 0) {
				UNRESOLVED(ret, "Mutex unlock failed");
			}
#if VERBOSE > 5
			output("%i waiter threads are waiting; start measure\n",
			       tnum);
#endif

			do_measure(&mtx, &cnd, test_scenar[s].cid, &ts);

#if VERBOSE > 5
			output("Measure for %s returned %d.%09d\n",
			       test_scenar[s].desc, ts.tv_sec, ts.tv_nsec);
#endif

			ts_cumul.tv_sec += ts.tv_sec;
			ts_cumul.tv_nsec += ts.tv_nsec;
			if (ts_cumul.tv_nsec >= 1000000000) {
				ts_cumul.tv_nsec -= 1000000000;
				ts_cumul.tv_sec += 1;
			}

			/* We can release the threads */
			predicate = 1;
			ret = pthread_cond_broadcast(&cnd);
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Unable to broadcast the condition");
			}
#if VERBOSE > 5
			output("Joining the waiters...\n");
#endif

			/* We will join every threads */
			for (i = 0; i < nthreads; i++) {
				ret = pthread_join(th[i], NULL);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Unable to join a thread");
				}

			}

			/* Destroy everything */
			ret = pthread_mutex_destroy(&mtx);
			if (ret != 0) {
				UNRESOLVED(ret, "Unable to destroy mutex");
			}

			ret = pthread_cond_destroy(&cnd);
			if (ret != 0) {
				UNRESOLVED(ret, "Unable to destroy cond");
			}
		}

#ifdef PLOT_OUTPUT
		output(" %d.%09d", ts_cumul.tv_sec, ts_cumul.tv_nsec);
#endif

		measure->_data[s] = ts_cumul.tv_sec * 1000000 + (ts_cumul.tv_nsec / 1000);	/* We reduce precision */

		/* Destroy the mutex attributes */
		ret = pthread_mutexattr_destroy(&ma);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to destroy mutex attribute");
		}

		ret = pthread_condattr_destroy(&ca);
		if (ret != 0) {
			UNRESOLVED(ret, "Unable to destroy cond attribute");
		}
	}

	/* Free the memory */
	free(th);

#if VERBOSE > 2
	output("%5d threads; %d.%09d s (%i loops)\n", nthreads, ts_cumul.tv_sec,
	       ts_cumul.tv_nsec, scal);
#endif

#ifdef PLOT_OUTPUT
	output("\n");
#endif

	return ts_cumul.tv_sec * 1000000000 + ts_cumul.tv_nsec;
}

/* Forward declaration */
int parse_measure(mes_t * measures);

/* Main
 */
int main(int argc, char *argv[])
{
	int ret, nth;
	long dur;

	/* Initialize the measure list */
	mes_t sentinel;
	mes_t *m_cur, *m_tmp;
	m_cur = &sentinel;
	m_cur->next = NULL;

	/* Initialize the output */
	output_init();

	/* Test machine capabilities */
	/* -> clockid_t; pshared; ... */
	altclk_ok = sysconf(_SC_CLOCK_SELECTION);
	if (altclk_ok > 0)
		altclk_ok = sysconf(_SC_MONOTONIC_CLOCK);
#ifndef USE_ALTCLK
	if (altclk_ok > 0)
		output
		    ("Implementation supports the MONOTONIC CLOCK but option is disabled in test.\n");
#endif

	pshared_ok = sysconf(_SC_THREAD_PROCESS_SHARED);

#if VERBOSE > 0
	output("Test starting\n");
	output(" Process-shared primitive %s be tested\n",
	       (pshared_ok > 0) ? "will" : "won't");
	output(" Alternative clock for cond %s be tested\n",
	       (altclk_ok > 0) ? "will" : "won't");
#endif

	/* Prepare thread attribute */
	ret = pthread_attr_init(&ta);
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to initialize thread attributes");
	}

	ret = pthread_attr_setstacksize(&ta, sysconf(_SC_THREAD_STACK_MIN));
	if (ret != 0) {
		UNRESOLVED(ret, "Unable to set stack size to minimum value");
	}
#ifdef PLOT_OUTPUT
	output("# COLUMNS %d #threads", NSCENAR + 1);
	for (nth = 0; nth < NSCENAR; nth++)
		output(" %s", test_scenar[nth].desc);
	output("\n");
#endif

	/* Do the testing */
	nth = 0;
	do {
		nth += 100 * SCALABILITY_FACTOR;

		/* Create a new measure item */
		m_tmp = malloc(sizeof(mes_t));
		if (m_tmp == NULL) {
			UNRESOLVED(errno,
				   "Unable to alloc memory for measure saving");
		}
		m_tmp->nthreads = nth;
		m_tmp->next = NULL;

		/* Run the test */
		dur = do_threads_test(nth, m_tmp);

		/* If test was success, add this measure to the list. Otherwise, free the mem */
		if (dur >= 0) {
			m_cur->next = m_tmp;
			m_cur = m_tmp;
		} else {
			free(m_tmp);
		}
	}
	while (dur >= 0);

	/* We will now parse the results to determine if the measure is ~ constant or is growing. */

	ret = parse_measure(&sentinel);

	/* Free the memory from the list */
	m_cur = sentinel.next;
	while (m_cur != NULL) {
		m_tmp = m_cur;
		m_cur = m_tmp->next;
		free(m_tmp);
	}

	if (ret != 0) {
		FAILED("This function is not scalable");
	}
#if VERBOSE > 0
	output("The function is scalable\n");
#endif

	PASSED;
}

/***
 * The next function will seek for the better model for each series of measurements.
 *
 * The tested models are: -- X = # threads; Y = latency
 * -> Y = a;      -- Error is r1 = avg((Y - Yavg)²);
 * -> Y = aX + b; -- Error is r2 = avg((Y -aX -b)²);
 *                -- where a = avg ((X - Xavg)(Y - Yavg)) / avg((X - Xavg)²)
 *                --         Note: We will call _q = sum((X - Xavg) * (Y - Yavg));
 *                --                       and  _d = sum((X - Xavg)²);
 *                -- and   b = Yavg - a * Xavg
 * -> Y = c * X^a;-- Same as previous, but with log(Y) = a log(X) + b; and b = log(c). Error is r3
 * -> Y = exp(aX + b); -- log(Y) = aX + b. Error is r4
 *
 * We compute each error factor (r1, r2, r3, r4) then search which is the smallest (with ponderation).
 * The function returns 0 when r1 is the best for all cases (latency is constant) and !0 otherwise.
 */

struct row {
	long X;			/* the X values -- copied from function argument */
	long Y[NSCENAR];	/* the Y values -- copied from function argument */
	long _x;		/* Value X - Xavg */
	long _y[NSCENAR];	/* Value Y - Yavg */
	double LnX;		/* Natural logarithm of X values */
	double LnY[NSCENAR];	/* Natural logarithm of Y values */
	double _lnx;		/* Value LnX - LnXavg */
	double _lny[NSCENAR];	/* Value LnY - LnYavg */
};

int parse_measure(mes_t * measures)
{
	int ret, i, r;

	mes_t *cur;

	double Xavg, Yavg[NSCENAR];
	double LnXavg, LnYavg[NSCENAR];

	int N;

	double r1[NSCENAR], r2[NSCENAR], r3[NSCENAR], r4[NSCENAR];

	/* Some more intermediate vars */
	long double _q[3][NSCENAR];
	long double _d[3][NSCENAR];

	long double t;		/* temp value */

	struct row *Table = NULL;

	/* Initialize the datas */
	Xavg = 0.0;
	LnXavg = 0.0;
	for (i = 0; i < NSCENAR; i++) {
		Yavg[i] = 0.0;
		LnYavg[i] = 0.0;
		r1[i] = 0.0;
		r2[i] = 0.0;
		r3[i] = 0.0;
		r4[i] = 0.0;
		_q[0][i] = 0.0;
		_q[1][i] = 0.0;
		_q[2][i] = 0.0;
		_d[0][i] = 0.0;
		_d[1][i] = 0.0;
		_d[2][i] = 0.0;
	}
	N = 0;
	cur = measures;

#if VERBOSE > 1
	output("Data analysis starting\n");
#endif

	/* We start with reading the list to find:
	 * -> number of elements, to assign an array
	 * -> average values
	 */
	while (cur->next != NULL) {
		cur = cur->next;

		N++;

		Xavg += (double)cur->nthreads;
		LnXavg += log((double)cur->nthreads);

		for (i = 0; i < NSCENAR; i++) {
			Yavg[i] += (double)cur->_data[i];
			LnYavg[i] += log((double)cur->_data[i]);
		}
	}

	/* We have the sum; we can divide to obtain the average values */
	Xavg /= N;
	LnXavg /= N;

	for (i = 0; i < NSCENAR; i++) {
		Yavg[i] /= N;
		LnYavg[i] /= N;
	}

#if VERBOSE > 1
	output(" Found %d rows and %d columns\n", N, NSCENAR);
#endif

	/* We will now alloc the array ... */
	Table = calloc(N, sizeof(struct row));
	if (Table == NULL) {
		UNRESOLVED(errno, "Unable to alloc space for results parsing");
	}

	/* ... and fill it */
	N = 0;
	cur = measures;

	while (cur->next != NULL) {
		cur = cur->next;

		Table[N].X = (long)cur->nthreads;
		Table[N]._x = Table[N].X - Xavg;
		Table[N].LnX = log((double)cur->nthreads);
		Table[N]._lnx = Table[N].LnX - LnXavg;
		for (i = 0; i < NSCENAR; i++) {
			Table[N].Y[i] = cur->_data[i];
			Table[N]._y[i] = Table[N].Y[i] - Yavg[i];
			Table[N].LnY[i] = log((double)cur->_data[i]);
			Table[N]._lny[i] = Table[N].LnY[i] - LnYavg[i];
		}

		N++;
	}

	/* We won't need the list anymore -- we'll work with the array which should be faster. */
#if VERBOSE > 1
	output(" Data was stored in an array.\n");
#endif

	/* We need to read the full array at least twice to compute all the error factors */

	/* In the first pass, we'll compute:
	 * -> r1 for each scenar.
	 * -> "a" factor for linear (0), power (1) and exponential (2) approximations -- with using the _d and _q vars.
	 */
#if VERBOSE > 1
	output("Starting first pass...\n");
#endif
	for (r = 0; r < N; r++) {
		for (i = 0; i < NSCENAR; i++) {
			r1[i] +=
			    ((double)Table[r]._y[i] / N) *
			    (double)Table[r]._y[i];

			_q[0][i] += Table[r]._y[i] * Table[r]._x;
			_d[0][i] += Table[r]._x * Table[r]._x;

			_q[1][i] += Table[r]._lny[i] * Table[r]._lnx;
			_d[1][i] += Table[r]._lnx * Table[r]._lnx;

			_q[2][i] += Table[r]._lny[i] * Table[r]._x;
			_d[2][i] += Table[r]._x * Table[r]._x;
		}
	}

	/* First pass is terminated; a2 = _q[0]/_d[0]; a3 = _q[1]/_d[1]; a4 = _q[2]/_d[2] */

	/* In the first pass, we'll compute:
	 * -> r2, r3, r4 for each scenar.
	 */

#if VERBOSE > 1
	output("Starting second pass...\n");
#endif
	for (r = 0; r < N; r++) {
		for (i = 0; i < NSCENAR; i++) {
			/* r2 = avg((y - ax -b)²);  t = (y - ax - b) = (y - yavg) - a (x - xavg); */
			t = (Table[r]._y[i] -
			     ((_q[0][i] * Table[r]._x) / _d[0][i]));
			r2[i] += t * t / N;

			/* r3 = avg((y - c.x^a) ²);
			   t = y - c * x ^ a
			   = y - log (LnYavg - (_q[1]/_d[1]) * LnXavg) * x ^ (_q[1]/_d[1])
			 */
			t = (Table[r].Y[i]
			     - (logl(LnYavg[i] - (_q[1][i] / _d[1][i]) * LnXavg)
				* powl(Table[r].X, (_q[1][i] / _d[1][i]))
			     ));
			r3[i] += t * t / N;

			/* r4 = avg((y - exp(ax+b))²);
			   t = y - exp(ax+b)
			   = y - exp(_q[2]/_d[2] * x + (LnYavg - (_q[2]/_d[2] * Xavg)));
			   = y - exp(_q[2]/_d[2] * (x - Xavg) + LnYavg);
			 */
			t = (Table[r].Y[i]
			     - expl((_q[2][i] / _d[2][i]) * Table[r]._x +
				    LnYavg[i]));
			r4[i] += t * t / N;

		}
	}

#if VERBOSE > 1
	output("All computing terminated.\n");
#endif
	ret = 0;
	for (i = 0; i < NSCENAR; i++) {
#if VERBOSE > 1
		output("\nScenario: %s\n", test_scenar[i].desc);

		output("  Model: Y = k\n");
		output("       k = %g\n", Yavg[i]);
		output("    Divergence %g\n", r1[i]);

		output("  Model: Y = a * X + b\n");
		output("       a = %Lg\n", _q[0][i] / _d[0][i]);
		output("       b = %Lg\n",
		       Yavg[i] - ((_q[0][i] / _d[0][i]) * Xavg));
		output("    Divergence %g\n", r2[i]);

		output("  Model: Y = c * X ^ a\n");
		output("       a = %Lg\n", _q[1][i] / _d[1][i]);
		output("       c = %Lg\n",
		       logl(LnYavg[i] - (_q[1][i] / _d[1][i]) * LnXavg));
		output("    Divergence %g\n", r2[i]);

		output("  Model: Y = exp(a * X + b)\n");
		output("       a = %Lg\n", _q[2][i] / _d[2][i]);
		output("       b = %Lg\n",
		       LnYavg[i] - ((_q[2][i] / _d[2][i]) * Xavg));
		output("    Divergence %g\n", r2[i]);
#endif
		/* Compare r1 to other values, with some ponderations */
		if ((r1[i] > 1.1 * r2[i]) || (r1[i] > 1.2 * r3[i])
		    || (r1[i] > 1.3 * r4[i]))
			ret++;
#if VERBOSE > 1
		else
			output(" Sanction: OK\n");
#endif
	}

	/* We need to free the array */
	free(Table);

	/* We're done */
	return ret;
}
