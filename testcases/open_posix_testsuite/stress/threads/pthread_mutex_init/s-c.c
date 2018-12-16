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

 * This file is a scalability test for the pthread_mutex_init function.

 * The steps are:
 * -> Restrict the memory to 32Mb * SCALABILITY_FACTOR
 * -> While there is free memory
 *      -> allocate memory for 10 mutex
 *      -> time = 0
 *      -> init the 10 mutex with different attributes
 *      -> output time
 * -> When memory is full; undo everything:
 *      -> time=0
 *      -> destroy the 10 mutexes
 *      -> output time
 *      -> free memory
 * -> We could additionally lock each mutex after init, and unlock before destroy.
 */

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/resource.h>
#include <sys/time.h>

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

#define WITH_LOCKS

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

typedef struct _teststruct {
	pthread_mutex_t mtx[10 * SCALABILITY_FACTOR];
	pthread_mutexattr_t ma[5];
	pthread_mutexattr_t *pma[10 * SCALABILITY_FACTOR];
	struct _teststruct *prev;
} teststruct_t;

int types[] = { PTHREAD_MUTEX_NORMAL,
	PTHREAD_MUTEX_ERRORCHECK,
	PTHREAD_MUTEX_RECURSIVE,
	PTHREAD_MUTEX_DEFAULT
};

int main(int argc, char *argv[])
{
	struct rlimit rl;
	int ret;
	int i;
	teststruct_t *cur, *prev;
	struct timeval time_zero, time_cour, time_res, time_sav[8];
	long sav = 0;

	/* Limit the process memory to a small value (64Mb for example). */
	rl.rlim_max = 1024 * 1024 * 32 * SCALABILITY_FACTOR;
	rl.rlim_cur = rl.rlim_max;
	if ((ret = setrlimit(RLIMIT_AS, &rl))) {
		UNRESOLVED(ret, "Memory limitation failed");
	}
#if VERBOSE > 1
	output(";Memory is now limited to %dMb\n", rl.rlim_max >> 20);
#endif

	prev = NULL;
	cur = NULL;

	/* Loop while we have memory left */
	while (1) {
		/* Allocate memory for 10 mutex and related stuff */
		cur = malloc(sizeof(teststruct_t));
		if (cur == NULL)	/* No memory left */
			break;

		/* Link to the previous so we are able to free memory */
		cur->prev = prev;
		prev = cur;

		/* Initialize the mutex attributes */
		/* We will have:
		 * pma[0] = NULL
		 * pma[1] = NORMAL type mutex attribute
		 * pma[2] = RECURSIVE type mutex attribute
		 * pma[3] = ERRORCHECK type mutex attribute
		 * pma[4] = DEFAULT type mutex attribute
		 * pma[5] = default mutex attribute
		 * pma[6] = NORMAL type mutex attribute
		 * pma[7] = RECURSIVE type mutex attribute
		 * pma[8] = ERRORCHECK type mutex attribute
		 * pma[9] = DEFAULT type mutex attribute
		 * pma[10] = pma[5] ...
		 */
		for (i = 0; i < 5; i++) {
			if ((ret = pthread_mutexattr_init(&(cur->ma[i])))) {
				UNRESOLVED(ret, "Mutex attribute init failed");
			}
			if (i) {
				if ((ret =
				     pthread_mutexattr_settype(&(cur->ma[i]),
							       types[i - 1]))) {
					UNRESOLVED(ret, "Mutex settype failed");
				}
			}
		}
		cur->pma[0] = NULL;
		for (i = 1; i < (10 * SCALABILITY_FACTOR); i++) {
			cur->pma[i] = &(cur->ma[i % 5]);
		}		/* The mutex attributes are now initialized */

		/* Save the time */
		gettimeofday(&time_zero, NULL);

		/* For each mutex, we will:
		 * - init the mutex
		 * - destroy the mutex
		 * - init the mutex
		 * - lock the mutex
		 * - unlock the mutex
		 * if WITH_LOCKS,
		 * - lock the mutex
		 */
		for (i = 0; i < 10 * SCALABILITY_FACTOR; i++) {
			ret = pthread_mutex_init(&(cur->mtx[i]), cur->pma[i]);
			if (ret) {
				UNRESOLVED(ret, "Mutex 1st init failed");
			}
			ret = pthread_mutex_destroy(&(cur->mtx[i]));
			if (ret) {
				UNRESOLVED(ret, "Mutex 1st destroy failed");
			}
			ret = pthread_mutex_init(&(cur->mtx[i]), cur->pma[i]);
			if (ret) {
				UNRESOLVED(ret, "Mutex 2nd init failed");
			}
			ret = pthread_mutex_lock(&(cur->mtx[i]));
			if (ret) {
				UNRESOLVED(ret, "Mutex 1st lock failed");
			}
			ret = pthread_mutex_unlock(&(cur->mtx[i]));
			if (ret) {
				UNRESOLVED(ret, "Mutex 1st unlock failed");
			}
#ifdef WITH_LOCKS
			ret = pthread_mutex_lock(&(cur->mtx[i]));
			if (ret) {
				UNRESOLVED(ret, "Mutex 2st lock failed");
			}
#endif
		}
		/* Compute the operation duration */
		gettimeofday(&time_cour, NULL);
		time_res.tv_usec =
		    time_cour.tv_usec + 1000000 - time_zero.tv_usec;
		if (time_res.tv_usec < 1000000) {
			time_res.tv_sec =
			    time_cour.tv_sec - 1 - time_zero.tv_sec;
		} else {
			time_res.tv_sec = time_cour.tv_sec - time_zero.tv_sec;
			time_res.tv_usec -= 1000000;
		}

		if (sav > 3) {
			time_sav[4].tv_sec = time_sav[5].tv_sec;
			time_sav[4].tv_usec = time_sav[5].tv_usec;
			time_sav[5].tv_sec = time_sav[6].tv_sec;
			time_sav[5].tv_usec = time_sav[6].tv_usec;
			time_sav[6].tv_sec = time_sav[7].tv_sec;
			time_sav[6].tv_usec = time_sav[7].tv_usec;
			time_sav[7].tv_sec = time_res.tv_sec;
			time_sav[7].tv_usec = time_res.tv_usec;
		} else {
			time_sav[sav].tv_sec = time_res.tv_sec;
			time_sav[sav].tv_usec = time_res.tv_usec;
		}
		sav++;
#if VERBOSE > 2
		output("%4i.%06i;\n", time_res.tv_sec, time_res.tv_usec);
#endif
	}
	if (errno != ENOMEM) {
		UNRESOLVED(errno, "Memory not full");
	}

	/* Now we just have to cleanup everything. */
	while (prev != NULL) {
		cur = prev;
		prev = cur->prev;

		/* Free the mutex resources in the cur element */
		for (i = 0; i < 10 * SCALABILITY_FACTOR; i++) {
#ifdef WITH_LOCKS
			ret = pthread_mutex_unlock(&(cur->mtx[i]));
			if (ret) {
				UNRESOLVED(ret, "Mutex 2nd unlock failed");
			}
#endif
			ret = pthread_mutex_destroy(&(cur->mtx[i]));
			if (ret) {
				UNRESOLVED(ret, "Mutex 2nd destroy failed");
			}
		}
		/* Free the mutex attributes resources in the cur element */
		for (i = 0; i < 5; i++) {
			if ((ret = pthread_mutexattr_destroy(&(cur->ma[i])))) {
				UNRESOLVED(ret,
					   "Mutex attribute destroy failed");
			}
		}
		/* Free the element memory */
		free(cur);
	}
#if VERBOSE > 0
	if (sav < 8) {
		output("Not enough iterations to build statistics\n");
	} else {
		output("Duration for the operations:\n");
		output(" %8i : %2i.%06i s\n", 0, time_sav[0].tv_sec,
		       time_sav[0].tv_usec);
		output(" %8i : %2i.%06i s\n", 1, time_sav[1].tv_sec,
		       time_sav[1].tv_usec);
		output(" %8i : %2i.%06i s\n", 2, time_sav[2].tv_sec,
		       time_sav[2].tv_usec);
		output(" %8i : %2i.%06i s\n", 3, time_sav[3].tv_sec,
		       time_sav[3].tv_usec);
		output(" [...]\n");
		output(" %8i : %2i.%06i s\n", sav - 3, time_sav[4].tv_sec,
		       time_sav[4].tv_usec);
		output(" %8i : %2i.%06i s\n", sav - 2, time_sav[5].tv_sec,
		       time_sav[5].tv_usec);
		output(" %8i : %2i.%06i s\n", sav - 1, time_sav[6].tv_sec,
		       time_sav[6].tv_usec);
		output(" %8i : %2i.%06i s\n", sav, time_sav[7].tv_sec,
		       time_sav[7].tv_usec);
	}
#endif

	PASSED;
}

#else /* WITHOUT_XOPEN */
int main(int argc, char *argv[])
{
	output_init();
	UNRESOLVED(0, "This test requires XSI features");
}
#endif
