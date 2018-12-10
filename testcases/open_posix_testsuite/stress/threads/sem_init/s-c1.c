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

* This scalability sample aims to test the following assertion:
*  -> The sem_init() duration does not depend on the # of semaphores
*     in the system

* The steps are:
* -> Init semaphores until failure

* The test fails if the sem_init duration tends to grow with the # of semaphores,
* or if the failure at last semaphore creation is unexpected.
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

#include <math.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>

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

#define BLOCKSIZE (1000 * SCALABILITY_FACTOR)

#define NSEM_LIMIT (1000 * BLOCKSIZE)

#ifdef PLOT_OUTPUT
#undef VERBOSE
#define VERBOSE 0
#endif

/********************************************************************************************/
/***********************************       Test     *****************************************/
/********************************************************************************************/

/* The next structure is used to save the tests measures */

typedef struct __mes_t {
	int nsem;
	long _data_open;	/* As we store µsec values, a long type should be enough. */
	long _data_close;	/* As we store µsec values, a long type should be enough. */

	struct __mes_t *next;

	struct __mes_t *prev;
} mes_t;

/* Forward declaration */
int parse_measure(mes_t * measures);

/* Structure to store created semaphores */

typedef struct __test_t {
	sem_t sems[BLOCKSIZE];

	struct __test_t *next;

	struct __test_t *prev;
} test_t;

/* Test routine */
int main(int argc, char *argv[])
{
	int ret, status, locerrno;
	int nsem, i;

	struct timespec ts_ref, ts_fin;
	mes_t sentinel;
	mes_t *m_cur, *m_tmp;

	test_t sems;

	struct __test_t *sems_cur = &sems, *sems_tmp;

	long SEM_MAX = sysconf(_SC_SEM_NSEMS_MAX);

	/* Initialize the measure list */
	m_cur = &sentinel;
	m_cur->next = NULL;
	m_cur->prev = NULL;

	/* Initialize output routine */
	output_init();

	/* Initialize sems */
	sems_cur->next = NULL;
	sems_cur->prev = NULL;

#if VERBOSE > 1
	output("SEM_NSEMS_MAX: %ld\n", SEM_MAX);

#endif

#ifdef PLOT_OUTPUT
	output("# COLUMNS 3 Semaphores sem_init sem_destroy\n");

#endif

	nsem = 0;
	status = 0;

	while (1) {		/* we will break */
		/* Create a new block */
		sems_tmp = (test_t *) malloc(sizeof(test_t));

		if (sems_tmp == NULL) {
			/* We stop here */
#if VERBOSE > 0
			output("malloc failed with error %d (%s)\n", errno,
			       strerror(errno));
#endif
			/* We can proceed anyway */
			status = 1;

			break;
		}

		/* read clock */
		ret = clock_gettime(CLOCK_REALTIME, &ts_ref);

		if (ret != 0) {
			UNRESOLVED(errno, "Unable to read clock");
		}

		/* Open all semaphores in the current block */
		for (i = 0; i < BLOCKSIZE; i++) {
			ret = sem_init(&(sems_tmp->sems[i]), i & 1, i & 3);

			if (ret != 0) {
#if VERBOSE > 0
				output("sem_init failed with error %d (%s)\n",
				       errno, strerror(errno));
#endif
				/* Check error code */
				switch (errno) {
				case EMFILE:
				case ENFILE:
				case ENOSPC:
				case ENOMEM:
					status = 2;
					break;
				default:
					UNRESOLVED(errno, "Unexpected error!");

					break;
				}

				if ((SEM_MAX > 0) && (nsem > SEM_MAX)) {
					FAILED
					    ("sem_open opened more than SEM_NSEMS_MAX semaphores");
				}

				nsem++;
			}

			/* read clock */
			ret = clock_gettime(CLOCK_REALTIME, &ts_fin);

			if (ret != 0) {
				UNRESOLVED(errno, "Unable to read clock");
			}

			if (status == 2) {
				/* We were not able to fill this bloc, so we can discard it */

				for (--i; i >= 0; i--) {
					ret = sem_destroy(&(sems_tmp->sems[i]));

					if (ret != 0) {
						UNRESOLVED(errno,
							   "Failed to close");
					}

				}

				free(sems_tmp);
				break;

			}

			sems_tmp->prev = sems_cur;
			sems_cur->next = sems_tmp;
			sems_cur = sems_tmp;
			sems_cur->next = NULL;

			/* add to the measure list */
			m_tmp = (mes_t *) malloc(sizeof(mes_t));

			if (m_tmp == NULL) {
				/* We stop here */
#if VERBOSE > 0
				output("malloc failed with error %d (%s)\n",
				       errno, strerror(errno));
#endif
				/* We can proceed anyway */
				status = 3;

				break;
			}

			m_tmp->nsem = nsem;
			m_tmp->next = NULL;
			m_tmp->prev = m_cur;
			m_cur->next = m_tmp;

			m_cur = m_tmp;

			m_cur->_data_open =
			    ((ts_fin.tv_sec - ts_ref.tv_sec) * 1000000) +
			    ((ts_fin.tv_nsec - ts_ref.tv_nsec) / 1000);
			m_cur->_data_close = 0;

			if (nsem >= NSEM_LIMIT)
				break;
		}

		locerrno = errno;

		/* Free all semaphore blocs */
#if VERBOSE > 0
		output("Detroy and free semaphores\n");

#endif

		/* Reverse list order */

		while (sems_cur != &sems) {
			/* read clock */
			ret = clock_gettime(CLOCK_REALTIME, &ts_ref);

			if (ret != 0) {
				UNRESOLVED(errno, "Unable to read clock");
			}

			/* Empty the sems_cur block */

			for (i = 0; i < BLOCKSIZE; i++) {
				ret = sem_destroy(&(sems_cur->sems[i]));

				if (ret != 0) {
					UNRESOLVED(errno,
						   "Failed to destroy a semaphore");
				}
			}

			/* read clock */
			ret = clock_gettime(CLOCK_REALTIME, &ts_fin);

			if (ret != 0) {
				UNRESOLVED(errno, "Unable to read clock");
			}

			/* add this measure to measure list */

			m_cur->_data_close =
			    ((ts_fin.tv_sec - ts_ref.tv_sec) * 1000000) +
			    ((ts_fin.tv_nsec - ts_ref.tv_nsec) / 1000);

			m_cur = m_cur->prev;

			/* remove the sem bloc */
			sems_cur = sems_cur->prev;

			free(sems_cur->next);

			sems_cur->next = NULL;
		}

#if VERBOSE > 0
		output("Parse results\n");

#endif

		/* Compute the results */
		ret = parse_measure(&sentinel);

		/* Free the resources and output the results */

#if VERBOSE > 5
		output("Dump : \n");

		output("  nsem  |  open  |  close \n");

#endif

		while (sentinel.next != NULL) {
			m_cur = sentinel.next;
#if (VERBOSE > 5) || defined(PLOT_OUTPUT)
			output("%8.8i %1.1li.%6.6li %1.1li.%6.6li\n",
			       m_cur->nsem, m_cur->_data_open / 1000000,
			       m_cur->_data_open % 1000000,
			       m_cur->_data_close / 1000000,
			       m_cur->_data_close % 1000000);

#endif
			sentinel.next = m_cur->next;

			free(m_cur);
		}

		if (ret != 0) {
			FAILED
			    ("The function is not scalable, add verbosity for more information");
		}

		/* Check status */
		if (status) {
			UNRESOLVED(locerrno,
				   "Function is scalable, but test terminated with error");
		}
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
		long X;		/* the X values -- copied from function argument */
		long Y_o;	/* the Y values -- copied from function argument */
		long Y_c;	/* the Y values -- copied from function argument */
		long _x;	/* Value X - Xavg */
		long _y_o;	/* Value Y - Yavg */
		long _y_c;	/* Value Y - Yavg */
		double LnX;	/* Natural logarithm of X values */
		double LnY_o;	/* Natural logarithm of Y values */
		double LnY_c;	/* Natural logarithm of Y values */
		double _lnx;	/* Value LnX - LnXavg */
		double _lny_o;	/* Value LnY - LnYavg */
		double _lny_c;	/* Value LnY - LnYavg */
	};

	int parse_measure(mes_t * measures) {
		int ret, r;

		mes_t *cur;

		double Xavg, Yavg_o, Yavg_c;
		double LnXavg, LnYavg_o, LnYavg_c;

		int N;

		double r1_o, r2_o, r3_o, r4_o;
		double r1_c, r2_c, r3_c, r4_c;

		/* Some more intermediate vars */
		long double _q_o[3];
		long double _d_o[3];
		long double _q_c[3];
		long double _d_c[3];

		long double t;	/* temp value */

		struct row *Table = NULL;

		/* This array contains the last element of each serie */
		int array_max;

		/* Initialize the datas */

		array_max = -1;	/* means no data */
		Xavg = 0.0;
		LnXavg = 0.0;
		Yavg_o = 0.0;
		LnYavg_o = 0.0;
		r1_o = 0.0;
		r2_o = 0.0;
		r3_o = 0.0;
		r4_o = 0.0;
		_q_o[0] = 0.0;
		_q_o[1] = 0.0;
		_q_o[2] = 0.0;
		_d_o[0] = 0.0;
		_d_o[1] = 0.0;
		_d_o[2] = 0.0;
		Yavg_c = 0.0;
		LnYavg_c = 0.0;
		r1_c = 0.0;
		r2_c = 0.0;
		r3_c = 0.0;
		r4_c = 0.0;
		_q_c[0] = 0.0;
		_q_c[1] = 0.0;
		_q_c[2] = 0.0;
		_d_c[0] = 0.0;
		_d_c[1] = 0.0;
		_d_c[2] = 0.0;

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

			if (cur->_data_open != 0) {
				array_max = N;
				Xavg += (double)cur->nsem;
				LnXavg += log((double)cur->nsem);
				Yavg_o += (double)cur->_data_open;
				LnYavg_o += log((double)cur->_data_open);
				Yavg_c += (double)cur->_data_close;
				LnYavg_c += log((double)cur->_data_close);
			}

		}

		/* We have the sum; we can divide to obtain the average values */
		if (array_max != -1) {
			Xavg /= array_max;
			LnXavg /= array_max;
			Yavg_o /= array_max;
			LnYavg_o /= array_max;
			Yavg_c /= array_max;
			LnYavg_c /= array_max;
		}
#if VERBOSE > 1
		output(" Found %d rows\n", N);

#endif

		/* We will now alloc the array ... */

		Table = calloc(N, sizeof(struct row));

		if (Table == NULL) {
			UNRESOLVED(errno,
				   "Unable to alloc space for results parsing");
		}

		/* ... and fill it */
		N = 0;

		cur = measures;

		while (cur->next != NULL) {
			cur = cur->next;

			Table[N].X = (long)cur->nsem;
			Table[N].LnX = log((double)cur->nsem);

			if (array_max > N) {
				Table[N]._x = Table[N].X - Xavg;
				Table[N]._lnx = Table[N].LnX - LnXavg;
				Table[N].Y_o = cur->_data_open;
				Table[N]._y_o = Table[N].Y_o - Yavg_o;
				Table[N].LnY_o = log((double)cur->_data_open);
				Table[N]._lny_o = Table[N].LnY_o - LnYavg_o;
				Table[N].Y_c = cur->_data_close;
				Table[N]._y_c = Table[N].Y_c - Yavg_c;
				Table[N].LnY_c = log((double)cur->_data_close);
				Table[N]._lny_c = Table[N].LnY_c - LnYavg_c;
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
		for (r = 0; r < array_max; r++) {
			r1_o +=
			    ((double)Table[r]._y_o / array_max) *
			    (double)Table[r]._y_o;

			_q_o[0] += Table[r]._y_o * Table[r]._x;
			_d_o[0] += Table[r]._x * Table[r]._x;

			_q_o[1] += Table[r]._lny_o * Table[r]._lnx;
			_d_o[1] += Table[r]._lnx * Table[r]._lnx;

			_q_o[2] += Table[r]._lny_o * Table[r]._x;
			_d_o[2] += Table[r]._x * Table[r]._x;

			r1_c +=
			    ((double)Table[r]._y_c / array_max) *
			    (double)Table[r]._y_c;

			_q_c[0] += Table[r]._y_c * Table[r]._x;
			_d_c[0] += Table[r]._x * Table[r]._x;

			_q_c[1] += Table[r]._lny_c * Table[r]._lnx;
			_d_c[1] += Table[r]._lnx * Table[r]._lnx;

			_q_c[2] += Table[r]._lny_c * Table[r]._x;
			_d_c[2] += Table[r]._x * Table[r]._x;

		}

		/* First pass is terminated; a2 = _q[0]/_d[0]; a3 = _q[1]/_d[1]; a4 = _q[2]/_d[2] */

		/* In the first pass, we'll compute:
		 * -> r2, r3, r4 for each scenar.
		 */

#if VERBOSE > 1
		output("Starting second pass...\n");

#endif
		for (r = 0; r < array_max; r++) {
			/* r2 = avg((y - ax -b)²);  t = (y - ax - b) = (y - yavg) - a (x - xavg); */
			t = (Table[r]._y_o -
			     ((_q_o[0] * Table[r]._x) / _d_o[0]));
			r2_o += t * t / array_max;

			t = (Table[r]._y_c -
			     ((_q_c[0] * Table[r]._x) / _d_c[0]));
			r2_c += t * t / array_max;

			/* r3 = avg((y - c.x^a) ²);
			   t = y - c * x ^ a
			   = y - log (LnYavg - (_q[1]/_d[1]) * LnXavg) * x ^ (_q[1]/_d[1])
			 */
			t = (Table[r].Y_o
			     - (logl(LnYavg_o - (_q_o[1] / _d_o[1]) * LnXavg)
				* powl(Table[r].X, (_q_o[1] / _d_o[1]))
			     ));
			r3_o += t * t / array_max;

			t = (Table[r].Y_c
			     - (logl(LnYavg_c - (_q_c[1] / _d_c[1]) * LnXavg)
				* powl(Table[r].X, (_q_c[1] / _d_c[1]))
			     ));
			r3_c += t * t / array_max;

			/* r4 = avg((y - exp(ax+b))²);
			   t = y - exp(ax+b)
			   = y - exp(_q[2]/_d[2] * x + (LnYavg - (_q[2]/_d[2] * Xavg)));
			   = y - exp(_q[2]/_d[2] * (x - Xavg) + LnYavg);
			 */
			t = (Table[r].Y_o
			     - expl((_q_o[2] / _d_o[2]) * Table[r]._x +
				    LnYavg_o));
			r4_o += t * t / array_max;

			t = (Table[r].Y_c
			     - expl((_q_c[2] / _d_c[2]) * Table[r]._x +
				    LnYavg_c));
			r4_c += t * t / array_max;

		}

#if VERBOSE > 1
		output("All computing terminated.\n");

#endif
		ret = 0;

#if VERBOSE > 1
		output(" # of data: %i\n", array_max);

		output("  Model: Y = k\n");

		output("   sem_open:\n");

		output("       k = %g\n", Yavg_o);

		output("    Divergence %g\n", r1_o);

		output("   sem_close:\n");

		output("       k = %g\n", Yavg_c);

		output("    Divergence %g\n", r1_c);

		output("  Model: Y = a * X + b\n");

		output("   sem_open:\n");

		output("       a = %Lg\n", _q_o[0] / _d_o[0]);

		output("       b = %Lg\n",
		       Yavg_o - ((_q_o[0] / _d_o[0]) * Xavg));

		output("    Divergence %g\n", r2_o);

		output("   sem_close:\n");

		output("       a = %Lg\n", _q_c[0] / _d_c[0]);

		output("       b = %Lg\n",
		       Yavg_c - ((_q_c[0] / _d_c[0]) * Xavg));

		output("    Divergence %g\n", r2_c);

		output("  Model: Y = c * X ^ a\n");

		output("   sem_open:\n");

		output("       a = %Lg\n", _q_o[1] / _d_o[1]);

		output("       c = %Lg\n",
		       logl(LnYavg_o - (_q_o[1] / _d_o[1]) * LnXavg));

		output("    Divergence %g\n", r3_o);

		output("   sem_close:\n");

		output("       a = %Lg\n", _q_c[1] / _d_c[1]);

		output("       c = %Lg\n",
		       logl(LnYavg_c - (_q_c[1] / _d_c[1]) * LnXavg));

		output("    Divergence %g\n", r3_c);

		output("  Model: Y = exp(a * X + b)\n");

		output("   sem_open:\n");

		output("       a = %Lg\n", _q_o[2] / _d_o[2]);

		output("       b = %Lg\n",
		       LnYavg_o - ((_q_o[2] / _d_o[2]) * Xavg));

		output("    Divergence %g\n", r4_o);

		output("   sem_close:\n");

		output("       a = %Lg\n", _q_c[2] / _d_c[2]);

		output("       b = %Lg\n",
		       LnYavg_c - ((_q_c[2] / _d_c[2]) * Xavg));

		output("    Divergence %g\n", r4_c);

#endif

		if (array_max != -1) {
			/* Compare r1 to other values, with some ponderations */

			if ((r1_o > 1.1 * r2_o) || (r1_o > 1.2 * r3_o) ||
			    (r1_o > 1.3 * r4_o) || (r1_c > 1.1 * r2_c) ||
			    (r1_c > 1.2 * r3_c) || (r1_c > 1.3 * r4_c))
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
