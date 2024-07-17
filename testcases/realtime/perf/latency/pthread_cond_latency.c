/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2005, 2008
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
 *     pthread_cond_latency.c
 *
 * DESCRIPTION
 *     measure pthread_cond_t latencies
 *
 * USAGE:
 *     Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *      Paul E. McKenney <paulmck@us.ibm.com>
 *
 * HISTORY
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <sched.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "librttest.h"

pthread_mutex_t child_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int child_waiting = 0;
double endtime;

void usage(void)
{
	rt_help();
	printf("testpi-1 specific options:\n");
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
	pthread_cond_t *cp = (pthread_cond_t *) arg;

	while (child_waiting == 0) {
		pthread_mutex_lock(&child_mutex);
		child_waiting = 1;
		if (pthread_cond_wait(cp, &child_mutex) != 0) {
			perror("pthread_cond_wait");
			exit(-1);
		}
		endtime = d_gettimeofday();
		child_waiting = 2;
		pthread_mutex_unlock(&child_mutex);
		while (child_waiting == 2) {
			poll(NULL, 0, 10);
		}
	}
	pthread_exit(NULL);
}

void test_signal(int broadcast_flag, int iter)
{
	pthread_attr_t attr;
	pthread_t childid;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
	int i;
	int prio;
	struct sched_param schparm;
	double starttime;

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

	if (pthread_attr_init(&attr) != 0) {
		perror("pthread_attr_init");
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
	if (pthread_create(&childid, &attr, childfunc, (void *)&cond) != 0) {
		perror("pthread_create");
		exit(-1);
	}
	for (i = 0; i < iter; i++) {
		pthread_mutex_lock(&child_mutex);
		child_waiting = 0;
		while (child_waiting == 0) {
			pthread_mutex_unlock(&child_mutex);
			sched_yield();
			pthread_mutex_lock(&child_mutex);
		}
		pthread_mutex_unlock(&child_mutex);
		if (broadcast_flag) {
			starttime = d_gettimeofday();
			if (pthread_cond_broadcast(&cond) != 0) {
				perror("pthread_cond_broadcast");
				exit(-1);
			}
		} else {
			starttime = d_gettimeofday();
			if (pthread_cond_signal(&cond) != 0) {
				perror("pthread_cond_signal");
				exit(-1);
			}
		}
		for (;;) {
			pthread_mutex_lock(&child_mutex);
			if (child_waiting == 2) {
				break;
			}
			pthread_mutex_unlock(&child_mutex);
			poll(NULL, 0, 10);
		}
		printf("%s() latency: %d microseconds\n",
		       (broadcast_flag
			? "pthread_cond_broadcast"
			: "pthread_cond_signal"),
		       (int)((endtime - starttime) * 1000000.));
		pthread_mutex_unlock(&child_mutex);
	}
	pthread_mutex_lock(&child_mutex);
	child_waiting = 3;
	pthread_mutex_unlock(&child_mutex);
	if (pthread_join(childid, NULL) != 0) {
		perror("pthread_join");
		exit(-1);
	}
}

int main(int argc, char *argv[])
{
	struct sched_param sp;
	long iter;
	setup();

	rt_init("h", parse_args, argc, argv);

	sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
	if (sp.sched_priority == -1) {
		perror("sched_get_priority_max");
		exit(-1);
	}
	if (sched_setscheduler(0, SCHED_FIFO, &sp) != 0) {
		perror("sched_setscheduler");
		exit(-1);
	}

	if (argc == 1) {
		fprintf(stderr, "Usage: %s iterations [unicast]\n", argv[0]);
		exit(-1);
	}
	iter = strtol(argv[1], NULL, 0);
	test_signal(argc == 2, iter);

	return 0;
}
