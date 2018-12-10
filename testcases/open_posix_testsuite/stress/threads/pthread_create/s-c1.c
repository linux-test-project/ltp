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

 * This scalability sample aims to test the following assertions:
 *  -> If pthread_create fails because of a lack of a resource, or
 	PTHREAD_THREADS_MAX threads already exist, EAGAIN shall be returned.
 *  -> It also tests that the thread creation time does not depend on the # of threads
 *     already created.

 * The steps are:
 * -> Create threads until failure

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

#define RESOLUTION (5 * SCALABILITY_FACTOR)

#ifdef PLOT_OUTPUT
#undef VERBOSE
#define VERBOSE 0
#endif

/********************************************************************************************/
/***********************************    Test cases  *****************************************/
/********************************************************************************************/

#include "threads_scenarii.c"

/* This file will define the following objects:
 * scenarii: array of struct __scenario type.
 * NSCENAR : macro giving the total # of scenarii
 * scenar_init(): function to call before use the scenarii array.
 * scenar_fini(): function to call after end of use of the scenarii array.
 */

/********************************************************************************************/
/***********************************    Real Test   *****************************************/
/********************************************************************************************/

/* The next structure is used to save the tests measures */
typedef struct __mes_t {
	int nthreads;
	long _data[NSCENAR];	/* As we store µsec values, a long type should be amply enough. */
	struct __mes_t *next;
} mes_t;

/* Forward declaration */
int parse_measure(mes_t * measures);

pthread_mutex_t m_synchro = PTHREAD_MUTEX_INITIALIZER;

void *threaded(void *arg)
{
	int ret = 0;

	/* Signal we're done */
	do {
		ret = sem_post(&scenarii[sc].sem);
	}
	while ((ret == -1) && (errno == EINTR));
	if (ret == -1) {
		UNRESOLVED(errno, "Failed to wait for the semaphore");
	}

	/* Wait for all threads being created */
	ret = pthread_mutex_lock(&m_synchro);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex lock failed");
	}
	(*(int *)arg) += 1;
	ret = pthread_mutex_unlock(&m_synchro);
	if (ret != 0) {
		UNRESOLVED(ret, "Mutex unlock failed");
	}

	return arg;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	pthread_t child;
	pthread_t *th;

	int nthreads, ctl, i, tmp;

	struct timespec ts_ref, ts_fin;

	mes_t sentinel;
	mes_t *m_cur, *m_tmp;

	long PTHREAD_THREADS_MAX = sysconf(_SC_THREAD_THREADS_MAX);
	long my_max = 1000 * SCALABILITY_FACTOR;

	/* Initialize the measure list */
	m_cur = &sentinel;
	m_cur->next = NULL;

	/* Initialize output routine */
	output_init();

	if (PTHREAD_THREADS_MAX > 0)
		my_max = PTHREAD_THREADS_MAX;

	th = (pthread_t *) calloc(1 + my_max, sizeof(pthread_t));
	if (th == NULL) {
		UNRESOLVED(errno, "Not enough memory for thread storage");
	}

	/* Initialize thread attribute objects */
	scenar_init();

#ifdef PLOT_OUTPUT
	printf("# COLUMNS %d #threads", NSCENAR + 1);
	for (sc = 0; sc < NSCENAR; sc++)
		printf(" %i", sc);
	printf("\n");
#endif

	for (sc = 0; sc < NSCENAR; sc++) {
		if (scenarii[sc].bottom == NULL) {	/* skip the alternate stacks as we could create only 1 */
#if VERBOSE > 0
			output("-----\n");
			output("Starting test with scenario (%i): %s\n", sc,
			       scenarii[sc].descr);
#endif

			/* Block every (about to be) created threads */
			ret = pthread_mutex_lock(&m_synchro);
			if (ret != 0) {
				UNRESOLVED(ret, "Mutex lock failed");
			}

			ctl = 0;
			nthreads = 0;
			m_cur = &sentinel;

			/* Create 1 thread for testing purpose */
			ret =
			    pthread_create(&child, &scenarii[sc].ta, threaded,
					   &ctl);
			switch (scenarii[sc].result) {
			case 0:	/* Operation was expected to succeed */
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Failed to create this thread");
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
				;
			}
			if (ret == 0) {	/* The new thread is running */

				while (1) {	/* we will break */
					/* read clock */
					ret =
					    clock_gettime(CLOCK_REALTIME,
							  &ts_ref);
					if (ret != 0) {
						UNRESOLVED(errno,
							   "Unable to read clock");
					}

					/* create a new thread */
					ret =
					    pthread_create(&th[nthreads],
							   &scenarii[sc].ta,
							   threaded, &ctl);

					/* stop here if we've got EAGAIN */
					if (ret == EAGAIN)
						break;

// temporary hack
					if (ret == ENOMEM)
						break;
					nthreads++;

					/* FAILED if error is != EAGAIN or nthreads > PTHREAD_THREADS_MAX */
					if (ret != 0) {
						output
						    ("pthread_create returned: %i (%s)\n",
						     ret, strerror(ret));
						FAILED
						    ("pthread_create did not return EAGAIN on a lack of resource");
					}
					if (nthreads > my_max) {
						if (PTHREAD_THREADS_MAX > 0) {
							FAILED
							    ("We were able to create more than PTHREAD_THREADS_MAX threads");
						} else {
							break;
						}
					}

					/* wait for the semaphore */
					do {
						ret =
						    sem_wait(&scenarii[sc].sem);
					}
					while ((ret == -1) && (errno == EINTR));
					if (ret == -1) {
						UNRESOLVED(errno,
							   "Failed to wait for the semaphore");
					}

					/* read clock */
					ret =
					    clock_gettime(CLOCK_REALTIME,
							  &ts_fin);
					if (ret != 0) {
						UNRESOLVED(errno,
							   "Unable to read clock");
					}

					/* add to the measure list if nthreads % resolution == 0 */
					if ((nthreads % RESOLUTION) == 0) {
						if (m_cur->next == NULL) {
							/* Create an empty new element */
							m_tmp = malloc(sizeof(mes_t));
							if (m_tmp == NULL) {
								UNRESOLVED
								    (errno,
								     "Unable to alloc memory for measure saving");
							}
							m_tmp->nthreads =
							    nthreads;
							m_tmp->next = NULL;
							for (tmp = 0;
							     tmp < NSCENAR;
							     tmp++)
								m_tmp->
								    _data[tmp] =
								    0;
							m_cur->next = m_tmp;
						}

						/* Add this measure to the next element */
						m_cur = m_cur->next;
						m_cur->_data[sc] =
						    ((ts_fin.tv_sec -
						      ts_ref.tv_sec) *
						     1000000) +
						    ((ts_fin.tv_nsec -
						      ts_ref.tv_nsec) / 1000);

#if VERBOSE > 5
						output
						    ("Added the following measure: sc=%i, n=%i, v=%li\n",
						     sc, nthreads,
						     m_cur->_data[sc]);
#endif
					}
				}
#if VERBOSE > 3
				output
				    ("Could not create anymore thread. Current count is %i\n",
				     nthreads);
#endif

				/* Unblock every created threads */
				ret = pthread_mutex_unlock(&m_synchro);
				if (ret != 0) {
					UNRESOLVED(ret, "Mutex unlock failed");
				}

				if (scenarii[sc].detached == 0) {
#if VERBOSE > 3
					output("Joining the threads\n");
#endif
					for (i = 0; i < nthreads; i++) {
						ret = pthread_join(th[i], NULL);
						if (ret != 0) {
							UNRESOLVED(ret,
								   "Unable to join a thread");
						}
					}

					ret = pthread_join(child, NULL);
					if (ret != 0) {
						UNRESOLVED(ret,
							   "Unalbe to join a thread");
					}

				}
#if VERBOSE > 3
				output
				    ("Waiting for threads (almost) termination\n");
#endif
				do {
					ret = pthread_mutex_lock(&m_synchro);
					if (ret != 0) {
						UNRESOLVED(ret,
							   "Mutex lock failed");
					}

					tmp = ctl;

					ret = pthread_mutex_unlock(&m_synchro);
					if (ret != 0) {
						UNRESOLVED(ret,
							   "Mutex unlock failed");
					}
				} while (tmp != nthreads + 1);

			}
			/* The thread was created */
		}
	}			/* next scenario */

	/* Free some memory before result parsing */
	free(th);

	/* Compute the results */
	ret = parse_measure(&sentinel);

	/* Free the resources and output the results */

#if VERBOSE > 5
	printf("Dump : \n");
	printf("%8.8s", "nth");
	for (i = 0; i < NSCENAR; i++)
		printf("|   %2.2i   ", i);
	printf("\n");
#endif
	while (sentinel.next != NULL) {
		m_cur = sentinel.next;
#if (VERBOSE > 5) || defined(PLOT_OUTPUT)
		printf("%8.8i", m_cur->nthreads);
		for (i = 0; i < NSCENAR; i++)
			printf(" %1.1li.%6.6li", m_cur->_data[i] / 1000000,
			       m_cur->_data[i] % 1000000);
		printf("\n");
#endif
		sentinel.next = m_cur->next;
		free(m_cur);
	}

	scenar_fini();

#if VERBOSE > 0
	output("-----\n");
	output("All test data destroyed\n");
	output("Test PASSED\n");
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
	long _x[NSCENAR];	/* Value X - Xavg */
	long _y[NSCENAR];	/* Value Y - Yavg */
	double LnX;		/* Natural logarithm of X values */
	double LnY[NSCENAR];	/* Natural logarithm of Y values */
	double _lnx[NSCENAR];	/* Value LnX - LnXavg */
	double _lny[NSCENAR];	/* Value LnY - LnYavg */
};

int parse_measure(mes_t * measures)
{
	int ret, i, r;

	mes_t *cur;

	double Xavg[NSCENAR], Yavg[NSCENAR];
	double LnXavg[NSCENAR], LnYavg[NSCENAR];

	int N;

	double r1[NSCENAR], r2[NSCENAR], r3[NSCENAR], r4[NSCENAR];

	/* Some more intermediate vars */
	long double _q[3][NSCENAR];
	long double _d[3][NSCENAR];

	long double t;		/* temp value */

	struct row *Table = NULL;

	/* This array contains the last element of each serie */
	int array_max[NSCENAR];

	/* Initialize the datas */
	for (i = 0; i < NSCENAR; i++) {
		array_max[i] = -1;	/* means no data */
		Xavg[i] = 0.0;
		LnXavg[i] = 0.0;
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
	 * -> number of elements, to assign an array.
	 * -> average values
	 */
	while (cur->next != NULL) {
		cur = cur->next;

		N++;

		for (i = 0; i < NSCENAR; i++) {
			if (cur->_data[i] != 0) {
				array_max[i] = N;
				Xavg[i] += (double)cur->nthreads;
				LnXavg[i] += log((double)cur->nthreads);
				Yavg[i] += (double)cur->_data[i];
				LnYavg[i] += log((double)cur->_data[i]);
			}
		}
	}

	/* We have the sum; we can divide to obtain the average values */
	for (i = 0; i < NSCENAR; i++) {
		if (array_max[i] != -1) {
			Xavg[i] /= array_max[i];
			LnXavg[i] /= array_max[i];
			Yavg[i] /= array_max[i];
			LnYavg[i] /= array_max[i];
		}
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
		Table[N].LnX = log((double)cur->nthreads);
		for (i = 0; i < NSCENAR; i++) {
			if (array_max[i] > N) {
				Table[N]._x[i] = Table[N].X - Xavg[i];
				Table[N]._lnx[i] = Table[N].LnX - LnXavg[i];
				Table[N].Y[i] = cur->_data[i];
				Table[N]._y[i] = Table[N].Y[i] - Yavg[i];
				Table[N].LnY[i] = log((double)cur->_data[i]);
				Table[N]._lny[i] = Table[N].LnY[i] - LnYavg[i];
			}
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
	for (i = 0; i < NSCENAR; i++) {
		for (r = 0; r < array_max[i]; r++) {
			r1[i] +=
			    ((double)Table[r]._y[i] / array_max[i]) *
			    (double)Table[r]._y[i];

			_q[0][i] += Table[r]._y[i] * Table[r]._x[i];
			_d[0][i] += Table[r]._x[i] * Table[r]._x[i];

			_q[1][i] += Table[r]._lny[i] * Table[r]._lnx[i];
			_d[1][i] += Table[r]._lnx[i] * Table[r]._lnx[i];

			_q[2][i] += Table[r]._lny[i] * Table[r]._x[i];
			_d[2][i] += Table[r]._x[i] * Table[r]._x[i];
		}
	}

	/* First pass is terminated; a2 = _q[0]/_d[0]; a3 = _q[1]/_d[1]; a4 = _q[2]/_d[2] */

	/* In the first pass, we'll compute:
	 * -> r2, r3, r4 for each scenar.
	 */

#if VERBOSE > 1
	output("Starting second pass...\n");
#endif
	for (i = 0; i < NSCENAR; i++) {
		for (r = 0; r < array_max[i]; r++) {
			/* r2 = avg((y - ax -b)²);  t = (y - ax - b) = (y - yavg) - a (x - xavg); */
			t = (Table[r]._y[i] -
			     ((_q[0][i] * Table[r]._x[i]) / _d[0][i]));
			r2[i] += t * t / array_max[i];

			/* r3 = avg((y - c.x^a) ²);
			   t = y - c * x ^ a
			   = y - log (LnYavg - (_q[1]/_d[1]) * LnXavg) * x ^ (_q[1]/_d[1])
			 */
			t = (Table[r].Y[i]
			     -
			     (logl
			      (LnYavg[i] - (_q[1][i] / _d[1][i]) * LnXavg[i])
			      * powl(Table[r].X, (_q[1][i] / _d[1][i]))
			     ));
			r3[i] += t * t / array_max[i];

			/* r4 = avg((y - exp(ax+b))²);
			   t = y - exp(ax+b)
			   = y - exp(_q[2]/_d[2] * x + (LnYavg - (_q[2]/_d[2] * Xavg)));
			   = y - exp(_q[2]/_d[2] * (x - Xavg) + LnYavg);
			 */
			t = (Table[r].Y[i]
			     - expl((_q[2][i] / _d[2][i]) * Table[r]._x[i] +
				    LnYavg[i]));
			r4[i] += t * t / array_max[i];

		}
	}

#if VERBOSE > 1
	output("All computing terminated.\n");
#endif
	ret = 0;
	for (i = 0; i < NSCENAR; i++) {
#if VERBOSE > 1
		output("\nScenario: %s\n", scenarii[i].descr);

		output(" # of data: %i\n", array_max[i]);

		output("  Model: Y = k\n");
		output("       k = %g\n", Yavg[i]);
		output("    Divergence %g\n", r1[i]);

		output("  Model: Y = a * X + b\n");
		output("       a = %Lg\n", _q[0][i] / _d[0][i]);
		output("       b = %Lg\n",
		       Yavg[i] - ((_q[0][i] / _d[0][i]) * Xavg[i]));
		output("    Divergence %g\n", r2[i]);

		output("  Model: Y = c * X ^ a\n");
		output("       a = %Lg\n", _q[1][i] / _d[1][i]);
		output("       c = %Lg\n",
		       logl(LnYavg[i] - (_q[1][i] / _d[1][i]) * LnXavg[i]));
		output("    Divergence %g\n", r2[i]);

		output("  Model: Y = exp(a * X + b)\n");
		output("       a = %Lg\n", _q[2][i] / _d[2][i]);
		output("       b = %Lg\n",
		       LnYavg[i] - ((_q[2][i] / _d[2][i]) * Xavg[i]));
		output("    Divergence %g\n", r2[i]);
#endif

		if (array_max[i] != -1) {
			/* Compare r1 to other values, with some ponderations */
			if ((r1[i] > 1.1 * r2[i]) || (r1[i] > 1.2 * r3[i])
			    || (r1[i] > 1.3 * r4[i]))
				ret++;
#if VERBOSE > 1
			else
				output(" Sanction: OK\n");
#endif
		}
	}

	/* We need to free the array */
	free(Table);

	/* We're done */
	return ret;
}
