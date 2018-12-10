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

* This scalability sample aims to test the following assertion:
*  -> The fork() duration does not depend on the # of processes in the system

* The steps are:
* -> Create processes until failure
* -> wait for each created process starting before creating the next one.
* -> processes are destroyed once we have reached the max processes in the system.

* The test fails if the fork duration tends to grow with the # of processes.
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

#include <sys/wait.h>
#include <errno.h>

#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
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
/***********************************       Test     *****************************************/
/********************************************************************************************/

/* The next structure is used to save the tests measures */

typedef struct __mes_t {
	int nprocess;
	long _data;		/* As we store µsec values, a long type should be enough. */

	struct __mes_t *next;
} mes_t;

/* Forward declaration */
int parse_measure(mes_t * measures);

sem_t *sem_synchro;
sem_t *sem_ending;

int main(int argc, char *argv[])
{
	int ret, status;
	pid_t pidctl;
	pid_t *pr;

	int nprocesses, i;

	struct timespec ts_ref, ts_fin;

	mes_t sentinel;
	mes_t *m_cur, *m_tmp;

	long CHILD_MAX = sysconf(_SC_CHILD_MAX);
	long my_max = 1000 * SCALABILITY_FACTOR;

	/* Initialize the measure list */
	m_cur = &sentinel;
	m_cur->next = NULL;

	/* Initialize output routine */
	output_init();

	if (CHILD_MAX > 0)
		my_max = CHILD_MAX;

	pr = (pid_t *) calloc(1 + my_max, sizeof(pid_t));

	if (pr == NULL) {
		UNRESOLVED(errno, "Not enough memory for process IDs storage");
	}
#if VERBOSE > 1
	output("CHILD_MAX: %d\n", CHILD_MAX);

#endif

#ifdef PLOT_OUTPUT
	output("# COLUMNS 2 #Process Duration\n");

#endif

	/* Initilaize the semaphores */
	sem_synchro = sem_open("/fork_scal_sync", O_CREAT, O_RDWR, 0);

	if (sem_synchro == SEM_FAILED) {
		UNRESOLVED(errno, "Failed to open a named semaphore\n");
	}

	sem_unlink("/fork_scal_sync");

	sem_ending = sem_open("/fork_scal_end", O_CREAT, O_RDWR, 0);

	if (sem_ending == SEM_FAILED) {
		UNRESOLVED(errno, "Failed to open a named semaphore\n");
	}

	sem_unlink("/fork_scal_end");

	nprocesses = 0;
	m_cur = &sentinel;

	while (1) {		/* we will break */
		/* read clock */
		ret = clock_gettime(CLOCK_REALTIME, &ts_ref);

		if (ret != 0) {
			UNRESOLVED(errno, "Unable to read clock");
		}

		/* create a new child */
		pr[nprocesses] = fork();

		if (pr[nprocesses] == -1) {
			if (errno == EAGAIN || errno == ENOMEM)
				break;

			FAILED
			    ("Failed to fork and received an unexpected error");
			/* Post the semaphore so running processes will terminate */

			do {
				ret = sem_post(sem_ending);
			} while (ret != 0 && errno == EINTR);

			if (ret != 0)
				output
				    ("Failed to post the semaphore on termination: error %d\n",
				     errno);

		}

		if (pr[nprocesses] == 0) {
			/* Child */
			/* Post the synchro semaphore */

			do {
				ret = sem_post(sem_synchro);
			} while ((ret != 0) && (errno == EINTR));

			if (ret != 0) {
				/* In this case the test will hang... */
				UNRESOLVED(errno,
					   "Failed post the sync semaphore");
			}

			/* Wait the end semaphore */
			do {
				ret = sem_wait(sem_ending);
			} while ((ret != 0) && (errno == EINTR));

			if (ret != 0) {
				UNRESOLVED(errno,
					   "Failed wait for the end semaphore");
			}

			/* Cascade-post the end semaphore */
			do {
				ret = sem_post(sem_ending);
			} while ((ret != 0) && (errno == EINTR));

			if (ret != 0) {
				UNRESOLVED(errno,
					   "Failed post the end semaphore");
			}

			/* Exit */
			exit(PTS_PASS);
		}

		/* Parent */
		nprocesses++;

		/* FAILED if nprocesses > CHILD_MAX */
		if (nprocesses > my_max) {
			errno = 0;

			if (CHILD_MAX > 0) {
#if VERBOSE > 0
				output
				    ("WARNING! We were able to create more than CHILD_MAX processes\n");
#endif

			}

			break;
		}

		/* wait for the semaphore */
		do {
			ret = sem_wait(sem_synchro);
		} while ((ret == -1) && (errno == EINTR));

		if (ret == -1) {
			sem_post(sem_ending);
			UNRESOLVED(errno,
				   "Failed to wait for the sync semaphore");
		}

		/* read clock */
		ret = clock_gettime(CLOCK_REALTIME, &ts_fin);

		if (ret != 0) {
			UNRESOLVED(errno, "Unable to read clock");
		}

		/* add to the measure list if nprocesses % resolution == 0 */
		if (((nprocesses % RESOLUTION) == 0) && (nprocesses != 0)) {
			/* Create an empty new element */
			m_tmp = malloc(sizeof(mes_t));

			if (m_tmp == NULL) {
				sem_post(sem_ending);
				UNRESOLVED(errno,
					   "Unable to alloc memory for measure saving");
			}

			m_tmp->nprocess = nprocesses;
			m_tmp->next = NULL;
			m_tmp->_data = 0;
			m_cur->next = m_tmp;

			m_cur = m_cur->next;

			m_cur->_data =
			    ((ts_fin.tv_sec - ts_ref.tv_sec) * 1000000) +
			    ((ts_fin.tv_nsec - ts_ref.tv_nsec) / 1000);

#if VERBOSE > 5
			output("Added the following measure: n=%i, v=%li\n",
			       nprocesses, m_cur->_data);
#endif

		}

	}
#if VERBOSE > 3

	if (errno)
		output
		    ("Could not create anymore processes. Current count is %i\n",
		     nprocesses);
	else
		output
		    ("Should not create anymore processes. Current count is %i\n",
		     nprocesses);

#endif

	/* Unblock every created children: post once, then cascade signaling */

	do {
		ret = sem_post(sem_ending);
	}
	while ((ret != 0) && (errno == EINTR));

	if (ret != 0) {
		UNRESOLVED(errno, "Failed post the end semaphore");
	}
#if VERBOSE > 3
	output("Waiting children termination\n");

#endif

	for (i = 0; i < nprocesses; i++) {
		pidctl = waitpid(pr[i], &status, 0);

		if (pidctl != pr[i]) {
			UNRESOLVED(errno, "Waitpid returned the wrong PID");
		}

		if ((!WIFEXITED(status)) || (WEXITSTATUS(status) != PTS_PASS)) {
			FAILED("Child exited abnormally");
		}

	}

	/* Free some memory before result parsing */
	free(pr);

	/* Compute the results */
	ret = parse_measure(&sentinel);

	/* Free the resources and output the results */

#if VERBOSE > 5
	output("Dump : \n");

	output("  nproc  |  dur  \n");

#endif
	while (sentinel.next != NULL) {
		m_cur = sentinel.next;
#if (VERBOSE > 5) || defined(PLOT_OUTPUT)
		output("%8.8i %1.1li.%6.6li\n", m_cur->nprocess,
		       m_cur->_data / 1000000, m_cur->_data % 1000000);

#endif
		sentinel.next = m_cur->next;

		free(m_cur);
	}

	if (ret != 0) {
		FAILED
		    ("The function is not scalable, add verbosity for more information");
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
	long X;			/* the X values -- copied from function argument */
	long Y;			/* the Y values -- copied from function argument */
	long _x;		/* Value X - Xavg */
	long _y;		/* Value Y - Yavg */
	double LnX;		/* Natural logarithm of X values */
	double LnY;		/* Natural logarithm of Y values */
	double _lnx;		/* Value LnX - LnXavg */
	double _lny;		/* Value LnY - LnYavg */
};

int parse_measure(mes_t * measures)
{
	int ret, r;

	mes_t *cur;

	double Xavg, Yavg;
	double LnXavg, LnYavg;

	int N;

	double r1, r2, r3, r4;

	/* Some more intermediate vars */
	long double _q[3];
	long double _d[3];

	long double t;		/* temp value */

	struct row *Table = NULL;

	/* This array contains the last element of each serie */
	int array_max;

	/* Initialize the datas */

	array_max = -1;		/* means no data */
	Xavg = 0.0;
	LnXavg = 0.0;
	Yavg = 0.0;
	LnYavg = 0.0;
	r1 = 0.0;
	r2 = 0.0;
	r3 = 0.0;
	r4 = 0.0;
	_q[0] = 0.0;
	_q[1] = 0.0;
	_q[2] = 0.0;
	_d[0] = 0.0;
	_d[1] = 0.0;
	_d[2] = 0.0;

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

		if (cur->_data != 0) {
			array_max = N;
			Xavg += (double)cur->nprocess;
			LnXavg += log((double)cur->nprocess);
			Yavg += (double)cur->_data;
			LnYavg += log((double)cur->_data);
		}
	}

	/* We have the sum; we can divide to obtain the average values */
	if (array_max != -1) {
		Xavg /= array_max;
		LnXavg /= array_max;
		Yavg /= array_max;
		LnYavg /= array_max;
	}
#if VERBOSE > 1
	output(" Found %d rows\n", N);

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

		Table[N].X = (long)cur->nprocess;
		Table[N].LnX = log((double)cur->nprocess);

		if (array_max > N) {
			Table[N]._x = Table[N].X - Xavg;
			Table[N]._lnx = Table[N].LnX - LnXavg;
			Table[N].Y = cur->_data;
			Table[N]._y = Table[N].Y - Yavg;
			Table[N].LnY = log((double)cur->_data);
			Table[N]._lny = Table[N].LnY - LnYavg;
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
		r1 += ((double)Table[r]._y / array_max) * (double)Table[r]._y;

		_q[0] += Table[r]._y * Table[r]._x;
		_d[0] += Table[r]._x * Table[r]._x;

		_q[1] += Table[r]._lny * Table[r]._lnx;
		_d[1] += Table[r]._lnx * Table[r]._lnx;

		_q[2] += Table[r]._lny * Table[r]._x;
		_d[2] += Table[r]._x * Table[r]._x;
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
		t = (Table[r]._y - ((_q[0] * Table[r]._x) / _d[0]));
		r2 += t * t / array_max;

		/* r3 = avg((y - c.x^a) ²);
		   t = y - c * x ^ a
		   = y - log (LnYavg - (_q[1]/_d[1]) * LnXavg) * x ^ (_q[1]/_d[1])
		 */
		t = (Table[r].Y - (logl(LnYavg - (_q[1] / _d[1]) * LnXavg)
				   * powl(Table[r].X, (_q[1] / _d[1]))
		     ));
		r3 += t * t / array_max;

		/* r4 = avg((y - exp(ax+b))²);
		   t = y - exp(ax+b)
		   = y - exp(_q[2]/_d[2] * x + (LnYavg - (_q[2]/_d[2] * Xavg)));
		   = y - exp(_q[2]/_d[2] * (x - Xavg) + LnYavg);
		 */
		t = (Table[r].Y - expl((_q[2] / _d[2]) * Table[r]._x + LnYavg));
		r4 += t * t / array_max;

	}

#if VERBOSE > 1
	output("All computing terminated.\n");

#endif
	ret = 0;

#if VERBOSE > 1
	output(" # of data: %i\n", array_max);

	output("  Model: Y = k\n");

	output("       k = %g\n", Yavg);

	output("    Divergence %g\n", r1);

	output("  Model: Y = a * X + b\n");

	output("       a = %Lg\n", _q[0] / _d[0]);

	output("       b = %Lg\n", Yavg - ((_q[0] / _d[0]) * Xavg));

	output("    Divergence %g\n", r2);

	output("  Model: Y = c * X ^ a\n");

	output("       a = %Lg\n", _q[1] / _d[1]);

	output("       c = %Lg\n", logl(LnYavg - (_q[1] / _d[1]) * LnXavg));

	output("    Divergence %g\n", r2);

	output("  Model: Y = exp(a * X + b)\n");

	output("       a = %Lg\n", _q[2] / _d[2]);

	output("       b = %Lg\n", LnYavg - ((_q[2] / _d[2]) * Xavg));

	output("    Divergence %g\n", r2);

#endif

	if (array_max != -1) {
		/* Compare r1 to other values, with some ponderations */

		if ((r1 > 1.1 * r2) || (r1 > 1.2 * r3) || (r1 > 1.3 * r4))
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
