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

 * This file is a scalability test for the pthread_cond_init function.

 * The steps are:
 * -> Restrict the memory to 32Mb * SCALABILITY_FACTOR
 * -> While there is free memory
 *      -> allocate memory for 10 cond vars
 *      -> time = 0
 *      -> init the 10 cond vars with different attributes
 *      -> output time
 * -> When memory is full; undo everything:
 *      -> time=0
 *      -> destroy the 10 cond vars
 *      -> output time
 *      -> free memory
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

#ifndef WITHOUT_XOPEN

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef SCALABILITY_FACTOR
#define SCALABILITY_FACTOR 1
#endif
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

typedef struct _teststruct {
	pthread_cond_t cnd[10 * SCALABILITY_FACTOR];
	pthread_condattr_t ca[4];
	pthread_condattr_t *pca[10 * SCALABILITY_FACTOR];
	struct _teststruct *prev;
} teststruct_t;

int main(int argc, char *argv[])
{
	struct rlimit rl;
	int ret;
	int i;
	teststruct_t *cur, *prev;
	struct timeval time_zero, time_cour, time_res, time_sav[8];
	long sav = 0;

	long monotonic, pshared;

	pshared = sysconf(_SC_THREAD_PROCESS_SHARED);
	monotonic = sysconf(_SC_MONOTONIC_CLOCK);
#if VERBOSE > 1
	output("Support for process-shared condvars: %li\n", pshared);
	output("Support for monotonic clock        : %li\n", monotonic);
#endif

	/* Limit the process memory to a small value (32Mb for example). */
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
		 * pca[0] = NULL
		 * pca[1] = Default cond attribute
		 * pca[2] = (if supported) pshared cond attribute
		 * pca[3] = (if supported) monotonic clock cond attribute
		 * pca[4] = (if supported) pshared + monotonic
		 * pca[5] = pca[0]...
		 */
		for (i = 0; i < 4; i++) {
			ret = pthread_condattr_init(&(cur->ca[i]));
			if (ret != 0) {
				UNRESOLVED(ret, "Cond attribute init failed");
			}

			if ((monotonic > 0) && ((i == 2) || (i == 3))) {
				ret =
				    pthread_condattr_setclock(&(cur->ca[i]),
							      CLOCK_MONOTONIC);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Set monotonic clock failed");
				}
			}
			if ((pshared > 0) && ((i == 1) || (i == 3))) {
				ret =
				    pthread_condattr_setpshared(&(cur->ca[i]),
								PTHREAD_PROCESS_SHARED);
				if (ret != 0) {
					UNRESOLVED(ret,
						   "Set process shared attribute failed");
				}
			}

		}

		for (i = 0; i < (10 * SCALABILITY_FACTOR); i++) {
			cur->pca[i] = (i % 5) ? &(cur->ca[i % 4]) : NULL;
		}		/* The mutex attributes are now initialized */

		/* Save the time */
		gettimeofday(&time_zero, NULL);

		/* For each condvar, we will:
		 * - init the condvar
		 * - destroy the condvar
		 * - init the condvar
		 * - destroy the condvar
		 * - init the condvar
		 */
		for (i = 0; i < 10 * SCALABILITY_FACTOR; i++) {
			ret = pthread_cond_init(&(cur->cnd[i]), cur->pca[i]);
			if (ret) {
				UNRESOLVED(ret, "Cond 1st init failed");
			}
			ret = pthread_cond_destroy(&(cur->cnd[i]));
			if (ret) {
				UNRESOLVED(ret, "Cond 1st destroy failed");
			}
			ret = pthread_cond_init(&(cur->cnd[i]), cur->pca[i]);
			if (ret) {
				UNRESOLVED(ret, "Cond 2nd init failed");
			}
			ret = pthread_cond_destroy(&(cur->cnd[i]));
			if (ret) {
				UNRESOLVED(ret, "Cond 2nd destroy failed");
			}
			ret = pthread_cond_init(&(cur->cnd[i]), cur->pca[i]);
			if (ret) {
				UNRESOLVED(ret, "Cond 3rd init failed");
			}
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
	}
	if (errno != ENOMEM) {
		UNRESOLVED(errno, "Memory not full");
	}

	/* Now we just have to cleanup everything. */
	while (prev != NULL) {
		cur = prev;
		prev = cur->prev;

		/* Free the condvar resources in the cur element */
		for (i = 0; i < 10 * SCALABILITY_FACTOR; i++) {
			ret = pthread_cond_destroy(&(cur->cnd[i]));
			if (ret) {
				UNRESOLVED(ret, "Cond final destroy failed");
			}
		}
		/* Free the cond attributes resources in the cur element */
		for (i = 0; i < 4; i++) {
			ret = pthread_condattr_destroy(&(cur->ca[i]));
			if (ret != 0) {
				UNRESOLVED(ret,
					   "Cond attribute destroy failed");
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
