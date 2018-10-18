/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2006, 2008
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * NAME
 *    sbrk-mutex.c
 *
 * DESCRIPTION
 *    Create NUM_THREADS to walk through an array of malloc'd pthread mutexes.
 *    Each thread holds up to NUM_CONCURRENT locks at a time.
 *
 * USAGE:
 *    Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *    Darren Hart <dvhltc@us.ibm.com>
 *
 * HISTORY
 *    2006-02-28: Initial version by Darren Hart
 *    2006-03-01: Changed mutexes to PTHREAD_MUTEX_ROBUST_NP type -Sripathi Kodi
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include "librttest.h"

#if HAS_PTHREAD_MUTEXTATTR_ROBUST_APIS

#define NUM_MUTEXES 5000
#define NUM_THREADS 50
#define NUM_CONCURRENT_LOCKS 50
#define DELAY 1000		/* how long to sleep in the worker thread in us */

static pthread_mutex_t *mutexes[NUM_MUTEXES];

void usage(void)
{
	rt_help();
	printf("sbrk_mutex specific options:\n");
}

int parse_args(int c, char *v)
{

	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	default:
		handled = 0;
		break;
	}
	return handled;
}

void *worker_thread(void *arg)
{
	int i;

	for (i = 0; i < NUM_MUTEXES + NUM_CONCURRENT_LOCKS; i++) {
		/* release prior lock */
		if (i >= NUM_CONCURRENT_LOCKS) {
			pthread_mutex_unlock(mutexes[i - NUM_CONCURRENT_LOCKS]);
		}
		/* grab a new lock */
		if (i < NUM_MUTEXES) {
			pthread_mutex_lock(mutexes[i]);
		}

		usleep(DELAY);

		if (_dbg_lvl)
			printf("thread %ld @ %d\n", (long)arg, i);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	int m, ret, robust;
	intptr_t t;
	pthread_mutexattr_t mutexattr;
	setup();

	rt_init("h", parse_args, argc, argv);

	if (pthread_mutexattr_init(&mutexattr) != 0) {
		printf("Failed to init mutexattr\n");
	}
	if (pthread_mutexattr_setrobust(&mutexattr, PTHREAD_MUTEX_ROBUST)
	    != 0) {
		printf("Can't set mutexattr robust\n");
	}
	if (pthread_mutexattr_getrobust(&mutexattr, &robust) != 0) {
		printf("Can't get mutexattr robust\n");
	} else {
		printf("robust in mutexattr is %d\n", robust);
	}

	/* malloc and initialize the mutexes */
	printf("allocating and initializing %d mutexes\n", NUM_MUTEXES);
	for (m = 0; m < NUM_MUTEXES; m++) {
		if (!(mutexes[m] = malloc(sizeof(pthread_mutex_t)))) {
			perror("malloc failed\n");
		}
		if ((ret = pthread_mutex_init(mutexes[m], &mutexattr))) {
			perror("pthread_mutex_init() failed\n");
		}
	}
	printf("mutexes allocated and initialized successfully\n");

	/* start children threads to walk the array, grabbing the locks */
	for (t = 0; t < NUM_THREADS; t++) {
		create_fifo_thread(worker_thread, (void *)t,
				   sched_get_priority_min(SCHED_FIFO));
	}
	/* wait for the children to complete */
	printf("joining threads\n");
	join_threads();
	/* destroy all the mutexes */
	for (m = 0; m < NUM_MUTEXES; m++) {
		if (mutexes[m]) {
			if ((ret = pthread_mutex_destroy(mutexes[m])))
				perror("pthread_mutex_destroy() failed\n");
			free(mutexes[m]);
		}
	}

	return 0;
}

#else
int main(void)
{
	printf
	    ("Your system doesn't support the pthread robust mutexattr APIs\n");
	return 1;
}
#endif
