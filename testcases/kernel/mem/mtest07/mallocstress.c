/******************************************************************************/
/*									      */
/* Copyright (c) International Business Machines  Corp., 2001		      */
/*									      */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.					      */
/*									      */
/* This program is distributed in the hope that it will be useful,	      */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of	      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See	              */
/* the GNU General Public License for more details.			      */
/*									      */
/* You should have received a copy of the GNU General Public License	      */
/* along with this program;  if not, write to the Free Software		      */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*									      */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* History:     Nov - 04 - 2001 Created - Manoj Iyer, IBM Austin TX.          */
/*                               email:manjo@austin.ibm.com                   */
/*                                                                            */
/*		Nov - 06 - 2001 Modified - Manoj Iyer, IBM Austin TX.         */
/*				- added function alloc_mem()                  */
/*    								              */
/*		Nov - 08 - 2001 Modified - Manoj Iyer, IBM Austin TX.         */
/*				- added logic to allocate memory in the size  */
/*				  of fibanocci numbers.                       */
/*				- fixed segmetation fault.                    */
/*									      */
/*		Nov - 09 - 2001 Modified - Manoj Iyer, IBM Austin TX.         */
/*				- separated alocation logic to allocate_free()*/
/*				  function.                                   */
/*				- introduced logic to randomly pick allocation*/
/*				  scheme. size = fibannoci number, pow of 2 or*/
/*				  power of 3.                                 */
/*				- changed comments.                           */
/*				- Added test to LTP.                          */
/*                                                                            */
/*		Nov - 09 - 2001 Modified - Manoj Iyer,IBM Austin TX.	      */
/*				- Removed compile errors.                     */
/*				- too many missing arguments.                 */
/*								              */
/*		Nov - 19 - 2001 Modified - Manoj Iyer, IBM Austin TX.	      */
/*				- fixed segmentation fault. 		      */
/*				  changed variable th_status from dynamic to  */
/*				  static array.			              */
/*                                                                            */
/*		May - 15 - 2002 Dan Kegel (dank@kegel.com)                    */
/*		                - Fixed crash on > 30 threads                 */
/*		                - Cleaned up, fixed compiler warnings         */
/*		                - Removed mallocs that could fail             */
/*		                - Note that pthread_create fails with EINTR   */
/*                                                                            */
/* File:	mallocstress.c						      */
/*									      */
/* Description:	This program stresses the VMM and C library                   */
/*              by spawning N threads which                                   */
/*              malloc blocks of increasing size until malloc returns NULL.   */
/******************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>

#include "tst_test.h"
#include "tst_safe_pthread.h"

/* Number of loops per-thread */
#define NUM_LOOPS	100

/* Number of threads to create */
#define NUM_THREADS     60

/* Define SPEW_SIGNALS to tickle thread_create bug (it fails if interrupted). */
#define SPEW_SIGNALS

static pthread_t *thread_id;	/* Spawned thread */

static void my_yield(void)
{
#ifdef SPEW_SIGNALS
	/* usleep just happens to use signals in glibc at moment.
	 * This is good because it allows us to test whether pthread_create
	 * improperly returns EINTR (which would violate SUSv3)
	 */
	usleep(0);
#else
	/* If you want this test to pass, don't define SPEW_SIGNALS,
	 * as pthread_create is broken at moment, and fails if interrupted
	 */
	static const struct timespec t0 = { 0, 0 };
	nanosleep(&t0, NULL);
#endif
}

/*
 * allocate_free() - Allocate and free test called per-thread
 *
 * @scheme: 0-3; selects how fast memory size grows
 *
 * This function does the allocation and free by calling malloc
 * and free functions. The size of the memory to be malloced is
 * determined by the caller of this function. The size can be
 * a number from the fibannoaci series, power of 2 or 3 or 5
 *
 * Return:
 *  0: success
 *  1: failure
 */
int allocate_free(int scheme)
{
	int loop;
	const int MAXPTRS = 50;	/* only 42 or so get used on 32 bit machine */

	for (loop = 0; loop < NUM_LOOPS; loop++) {
		size_t oldsize = 5;
		size_t size = sizeof(long);
		long *ptrs[MAXPTRS];
		int num_alloc;
		int i;

		/* loop terminates in one of three ways:
		 * 1. after MAXPTRS iterations
		 * 2. if malloc fails
		 * 3. if new size overflows
		 */
		for (num_alloc = 0; num_alloc < MAXPTRS; num_alloc++) {
			size_t newsize = 0;

			/* Malloc the next block */
			ptrs[num_alloc] = malloc(size);
			/* terminate loop if malloc fails */
			if (!ptrs[num_alloc])
				break;
			ptrs[num_alloc][0] = num_alloc;

			/* Increase size according to one of four schedules. */
			switch (scheme) {
			case 0:
				newsize = size + oldsize;
				oldsize = size;
				break;
			case 1:
				newsize = size * 2;
				break;
			case 2:
				newsize = size * 3;
				break;
			case 3:
				newsize = size * 5;
				break;
			default:
				assert(0);
			}
			/* terminate loop on overflow */
			if (newsize < size)
				break;
			size = newsize;

			my_yield();
		}

		for (i = 0; i < num_alloc; i++) {
			if (ptrs[i][0] != i) {
				tst_res(TFAIL,
					"pid[%d]: fail: bad sentinel value\n",
					getpid());
				return 1;
			}
			free(ptrs[i]);
			my_yield();
		}

		my_yield();
	}

	/* Success! */
	return 0;
}

void *alloc_mem(void *threadnum)
{
	int err;

	/* waiting for other threads starting */
	TST_CHECKPOINT_WAIT(0);

	/* thread N will use growth scheme N mod 4 */
	err = allocate_free(((uintptr_t)threadnum) % 4);
	tst_res(TINFO,
		"Thread [%d]: allocate_free() returned %d, %s.  Thread exiting.\n",
		(int)(uintptr_t)threadnum, err,
		(err ? "failed" : "succeeded"));
	return (void *)(uintptr_t) (err ? -1 : 0);
}

static void stress_malloc(void)
{
	int thread_index;

	for (thread_index = 0; thread_index < NUM_THREADS; thread_index++) {
		SAFE_PTHREAD_CREATE(&thread_id[thread_index], NULL, alloc_mem,
				    (void *)(uintptr_t)thread_index);
	}

	/* Wake up all threads */
	TST_CHECKPOINT_WAKE2(0, NUM_THREADS);

	/* wait for all threads to finish */
	for (thread_index = 0; thread_index < NUM_THREADS; thread_index++) {
		void *status;

		SAFE_PTHREAD_JOIN(thread_id[thread_index], &status);
		if ((intptr_t)status != 0) {
			tst_res(TFAIL, "thread [%d] - exited with errors\n",
				thread_index);
		}
	}

	tst_res(TPASS, "malloc stress test finished successfully");
}

static void setup(void)
{
	thread_id = SAFE_MALLOC(sizeof(pthread_t) * NUM_THREADS);
}

static void cleanup(void)
{
	if (thread_id) {
		free(thread_id);
		thread_id = NULL;
	}
}

static struct tst_test test = {
	.needs_checkpoints = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = stress_malloc,
};
