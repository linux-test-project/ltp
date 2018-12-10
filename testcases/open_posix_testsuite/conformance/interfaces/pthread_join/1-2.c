/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis
*
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
* This sample test aims to check the following assertion:
*
* pthread_join() returns only when the joined thread has terminated.
*
* The steps are:
* -> create a thread
* -> the thread yields then read clock and exit
* -> check the read clock value is before pthread_join return.
*
* The test fails if the time read is not coherent
*
*/


#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include <time.h>
#include <errno.h>

#include "../testfrmw/testfrmw.h"
#include "../testfrmw/testfrmw.c"

#ifndef VERBOSE
#define VERBOSE 1
#endif

#include "../testfrmw/threads_scenarii.c"

static void *threaded(void *arg)
{
	int ret = 0;
	int i;

	/* yield the control some times */
	for (i = 0; i < 10; i++)
		sched_yield();

	/* Now tell we're done */
	ret = clock_gettime(CLOCK_REALTIME, arg);
	if (ret != 0)
		UNRESOLVED(errno, "Failed to get clock time");

	return NULL;
}

int main(void)
{
	int ret = 0;
	pthread_t child;

	struct timespec ts_pre, ts_th, ts_post;

	output_init();
	scenar_init();

	for (sc = 0; sc < NSCENAR; sc++) {
		if (scenarii[sc].detached == 1)
			continue;
#if VERBOSE > 0
		output("-----\n");
		output("Starting test with scenario (%i): %s\n",
		       sc, scenarii[sc].descr);
#endif
		ret = clock_gettime(CLOCK_REALTIME, &ts_pre);
		if (ret != 0)
			UNRESOLVED(errno, "Failed to read clock");

		ret = pthread_create(&child, &scenarii[sc].ta,
				     threaded, &ts_th);

		switch (scenarii[sc].result) {
		case 0:
			if (ret != 0)
				UNRESOLVED(ret, "Failed to create this thread");
			break;
		case 1:
			if (ret == 0)
				UNRESOLVED(-1, "An error was expected but the "
					   "thread creation succeeded");
			break;

		case 2:
		default:
#if VERBOSE > 0
			if (ret == 0)
				output("Thread has been created successfully "
				       "for this scenario\n");
			else
				output("Thread creation failed with the error:"
				       " %s\n", strerror(ret));

#endif
		}

		if (ret == 0) {
			ret = pthread_join(child, NULL);
			if (ret != 0)
				UNRESOLVED(ret, "Unable to join a thread");

			ret = clock_gettime(CLOCK_REALTIME, &ts_post);
			if (ret != 0)
				UNRESOLVED(errno, "Failed to read clock");

			/* Now check that ts_pre <= ts_th <= ts_post */
			if ((ts_th.tv_sec < ts_pre.tv_sec) ||
			    ((ts_th.tv_sec == ts_pre.tv_sec) &&
			     (ts_th.tv_nsec < ts_pre.tv_nsec))) {
				output("Pre  : %d.%09d\n", ts_pre.tv_sec,
				       ts_pre.tv_nsec);
				output("child: %d.%09d\n", ts_th.tv_sec,
				       ts_th.tv_nsec);
				output("Post : %d.%09d\n", ts_post.tv_sec,
				       ts_post.tv_nsec);
				FAILED("Child returned before its creation ??");
			}

			if ((ts_post.tv_sec < ts_th.tv_sec) ||
			    ((ts_post.tv_sec == ts_th.tv_sec) &&
			     (ts_post.tv_nsec < ts_th.tv_nsec))) {
				output("Pre  : %d.%09d\n", ts_pre.tv_sec,
				       ts_pre.tv_nsec);
				output("child: %d.%09d\n", ts_th.tv_sec,
				       ts_th.tv_nsec);
				output("Post : %d.%09d\n", ts_post.tv_sec,
				       ts_post.tv_nsec);
				FAILED("pthread_join returned before child "
				       "terminated");
			}

		}
	}

	scenar_fini();
#if VERBOSE > 0
	output("-----\n");
	output("All test data destroyed\n");
	output("Test PASSED\n");
#endif

	PASSED;
}
