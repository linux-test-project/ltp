/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2005-2008
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
 *      pthread_cond_many.c
 *
 * DESCRIPTION
 *      Measure pthread_cond_t latencies , but in presence of many processes.
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *      Paul E. McKenney <paulmck@us.ibm.com>
 *
 * HISTORY
 *      librttest parsing, threading, and mutex initialization - Darren Hart
 *
 *
 *      This line has to be added to avoid a stupid CVS problem
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <sched.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <librttest.h>
#include <libstats.h>
#define PASS_US 100
pthread_mutex_t child_mutex;
volatile int *child_waiting = NULL;
double endtime;
pthread_cond_t *condlist = NULL;
int iterations = 0;
int nthreads = 0;
int realtime = 0;
int broadcast_flag = 0;
unsigned long latency = 0;
int fail = 0;
/*
 * Return time as a floating-point number rather than struct timeval.
 */

double d_gettimeofday(void)
{
	int retval;
	struct timeval tv;

	retval = gettimeofday(&tv, NULL);
	if (retval != 0) {
		perror("gettimeofday");
		exit(-1);
	}
	return (tv.tv_sec + ((double)tv.tv_usec) / 1000000.);
}

void *childfunc(void *arg)
{
	int myid = (intptr_t) arg;
	pthread_cond_t *cp;
	volatile int *cw;

	cp = &condlist[myid];
	cw = &child_waiting[myid];
	while (*cw == 0) {
		pthread_mutex_lock(&child_mutex);
		*cw = 1;
		if (pthread_cond_wait(cp, &child_mutex) != 0) {
			perror("pthread_cond_wait");
			exit(-1);
		}
		endtime = d_gettimeofday();
		*cw = 2;
		pthread_mutex_unlock(&child_mutex);
		while (*cw == 2) {
			poll(NULL, 0, 10);
		}
	}
	pthread_exit(NULL);
}

pthread_t create_thread_(int itsid)
{
	pthread_attr_t attr;
	pthread_t childid;
	int prio;
	struct sched_param schparm;

	if (pthread_attr_init(&attr) != 0) {
		perror("pthread_attr_init");
		exit(-1);
	}
	if (realtime) {
		prio = sched_get_priority_max(SCHED_FIFO);
		if (prio == -1) {
			perror("sched_get_priority_max");
			exit(-1);
		}
		schparm.sched_priority = prio;
		if (sched_setscheduler(getpid(), SCHED_FIFO, &schparm) != 0) {
			perror("sched_setscheduler");
			exit(-1);
		}

		if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO) != 0) {
			perror("pthread_attr_setschedpolicy");
			exit(-1);
		}
		if (pthread_attr_setschedparam(&attr, &schparm) != 0) {
			perror("pthread_attr_setschedparam");
			exit(-1);
		}
	}
	if (pthread_attr_setstacksize(&attr, (size_t) (32 * 1024)) != 0) {
		perror("pthread_attr_setstacksize");
		exit(-1);
	}
	if (pthread_cond_init(&condlist[itsid], NULL) != 0) {
		perror("pthread_cond_init");
		exit(-1);
	}
	if (pthread_create(&childid, &attr, childfunc, (void *)(intptr_t) itsid)
	    != 0) {
		perror("pthread_create");
		exit(-1);
	}
	return (childid);
}

void wake_child(int itsid, int broadcast_flag)
{
	double starttime;

	pthread_mutex_lock(&child_mutex);
	while (child_waiting[itsid] == 0) {
		pthread_mutex_unlock(&child_mutex);
		sched_yield();
		pthread_mutex_lock(&child_mutex);
	}
	pthread_mutex_unlock(&child_mutex);
	if (broadcast_flag) {
		starttime = d_gettimeofday();
		if (pthread_cond_broadcast(&condlist[itsid]) != 0) {
			perror("pthread_cond_broadcast");
			exit(-1);
		}
	} else {
		starttime = d_gettimeofday();
		if (pthread_cond_signal(&condlist[itsid]) != 0) {
			perror("pthread_cond_signal");
			exit(-1);
		}
	}
	for (;;) {
		pthread_mutex_lock(&child_mutex);
		if (child_waiting[itsid] == 2) {
			break;
		}
		pthread_mutex_unlock(&child_mutex);
		poll(NULL, 0, 10);
	}
	latency = (unsigned long)((endtime - starttime) * 1000000.);
	pthread_mutex_unlock(&child_mutex);
}

void test_signal(long iter, long nthreads)
{
	int i;
	int j;
	int k;
	pthread_t *pt;
	unsigned long max = 0;
	unsigned long min = 0;
	stats_container_t dat;
	stats_record_t rec;

	stats_container_init(&dat, iter * nthreads);

	pt = malloc(sizeof(*pt) * nthreads);
	if (pt == NULL) {
		fprintf(stderr, "Out of memory\n");
		exit(-1);
	}
	for (j = 0; j < nthreads; j++) {
		child_waiting[j] = 0;
		pt[j] = create_thread_(j);
	}
	for (i = 0; i < (iter - 1) * nthreads; i += nthreads) {
		for (j = 0, k = i; j < nthreads; j++, k++) {
			wake_child(j, broadcast_flag);
			rec.x = k;
			rec.y = latency;
			stats_container_append(&dat, rec);
			pthread_mutex_lock(&child_mutex);
			child_waiting[j] = 0;
			pthread_mutex_unlock(&child_mutex);
		}
	}
	for (j = 0; j < nthreads; j++) {
		wake_child(j, broadcast_flag);
		pthread_mutex_lock(&child_mutex);
		child_waiting[j] = 3;
		pthread_mutex_unlock(&child_mutex);
		if (pthread_join(pt[j], NULL) != 0) {
			fprintf(stderr, "%d: ", j);
			perror("pthread_join");
			exit(-1);
		}
	}
	min = (unsigned long)-1;
	for (i = 0; i < iter * nthreads; i++) {
		latency = dat.records[i].y;
		if (latency > PASS_US)
			fail = 1;
		min = MIN(min, latency);
		max = MAX(max, latency);
	}
	printf("Recording statistics...\n");
	printf("Minimum: %lu us\n", min);
	printf("Maximum: %lu us\n", max);
	printf("Average: %f us\n", stats_avg(&dat));
	printf("Standard Deviation: %f\n", stats_stddev(&dat));
}

void usage(void)
{
	rt_help();
	printf("pthread_cond_many specific options:\n");
	printf("  -r,--realtime   run with realtime priority\n");
	printf("  -b,--broadcast  use cond_broadcast instead of cond_signal\n");
	printf("  -iITERATIONS    iterations (required)\n");
	printf("  -nNTHREADS      number of threads (required)\n");
	printf("deprecated unnamed arguments:\n");
	printf("  pthread_cond_many [options] iterations nthreads\n");
}

int parse_args(int c, char *v)
{
	int handled = 1;
	switch (c) {
	case 'h':
		usage();
		exit(0);
	case 'a':
		broadcast_flag = 1;
		break;
	case 'i':
		iterations = atoi(v);
		break;
	case 'n':
		nthreads = atoi(v);
		break;
	case 'r':
		realtime = 1;
		break;
	default:
		handled = 0;
		break;
	}
	return handled;
}

int main(int argc, char *argv[])
{
	struct option longopts[] = {
		{"broadcast", 0, NULL, 'a'},
		{"realtime", 0, NULL, 'r'},
		{NULL, 0, NULL, 0},
	};
	setup();

	init_pi_mutex(&child_mutex);
	rt_init_long("ahi:n:r", longopts, parse_args, argc, argv);

	/* Legacy command line arguments support, overrides getopt args. */
	if (optind < argc)
		iterations = strtol(argv[optind++], NULL, 0);
	if (optind < argc)
		nthreads = strtol(argv[optind++], NULL, 0);

	/* Ensure we have the required arguments. */
	if (iterations == 0 || nthreads == 0) {
		usage();
		exit(1);
	}

	child_waiting = malloc(sizeof(*child_waiting) * nthreads);
	condlist = malloc(sizeof(*condlist) * nthreads);
	if ((child_waiting == NULL) || (condlist == NULL)) {
		fprintf(stderr, "Out of memory\n");
		exit(-1);
	}
	test_signal(iterations, nthreads);
	printf("\nCriteria: latencies < %d us\n", PASS_US);
	printf("Result: %s\n", fail ? "FAIL" : "PASS");

	return 0;
}
