/*
 * Copyright (c) 2015, Cyril Hrubis <chrubis@suse.cz>
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
 * pthread_create creates a thread with attributes as specified in the attr parameter.
 *
 * This test tests scheduller attributes are set correctly and schedulling works.
 *
 * The steps are:
 *
 *  - create thread with given scheduler policy and minimal priority for the
 *    scheduling policy
 *
 *  - get the scheduler attributes of the running thread and check
 *    that they are set as requested
 *
 *  - start a thread(s) with higher priority and check that the thread with
 *    lower priority does not finish until the high priority threads finished
 */


/* Must be included first */
#include "affinity.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include "posixtest.h"
#include "ncpu.h"

static volatile sig_atomic_t flag;
static int n_threads;

static void alarm_handler()
{
	flag = 0;
}

void *do_work(void *arg)
{
	(void) arg;

	while (flag)
		sched_yield();

	return NULL;
}

static void init_attr(pthread_attr_t *attr, int sched_policy, int prio)
{
	struct sched_param sched_param = {.sched_priority = prio};
	int ret;

	ret = pthread_attr_init(attr);
	if (ret) {
		fprintf(stderr, "pthread_attr_init(): %s\n", strerror(ret));
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_attr_setschedpolicy(attr, sched_policy);
	if (ret) {
		fprintf(stderr, "pthread_setschedpolicy(): %s\n", strerror(ret));
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
	if (ret) {
		fprintf(stderr, "pthread_attr_setinheritsched(): %s\n", strerror(ret));
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_attr_setschedparam(attr, &sched_param);
	if (ret) {
		fprintf(stderr, "pthread_attr_setschedparam(): %s\n", strerror(ret));
		exit(PTS_UNRESOLVED);
	}
}

static void run_hp_threads(int sched_policy, int sched_prio)
{
	struct itimerval it;
	pthread_t threads[n_threads];
	pthread_attr_t attr;
	int i, ret;

	flag = 1;

	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = 0;
	it.it_value.tv_sec = n_threads / 20;
	it.it_value.tv_usec = (n_threads % 20) * 50000;

	init_attr(&attr, sched_policy, sched_prio);

	if (signal(SIGPROF, alarm_handler) == SIG_ERR) {
		perror("signal()");
		exit(PTS_UNRESOLVED);
	}

	if (setitimer(ITIMER_PROF, &it, NULL)) {
		perror("setitimer(ITIMER_VIRTUAL, ...)");
		exit(PTS_UNRESOLVED);
	}

	for (i = 0; i < n_threads; i++) {
		ret = pthread_create(&threads[i], &attr, do_work, NULL);
		if (ret) {
			fprintf(stderr, "pthread_create(): %s\n",
			        strerror(ret));
			exit(PTS_UNRESOLVED);
		}
	}

	if (flag) {
		printf("FAILED: low priority thread scheduled\n");
		exit(PTS_FAIL);
	}

	pthread_attr_destroy(&attr);

	for (i = 0; i < n_threads; i++)
		pthread_join(threads[i], NULL);

}

struct params {
	int sched_policy;
	int sched_priority;
};

static void *do_test(void *arg)
{
	int ret, sched_policy;
	struct sched_param param;
	struct params *p = arg;

	/* First check that the scheduler parameters are set correctly */
	ret = pthread_getschedparam(pthread_self(), &sched_policy, &param);
	if (ret) {
		fprintf(stderr, "pthread_getschedparam(): %s\n", strerror(ret));
		exit(PTS_UNRESOLVED);
	}

	if (p->sched_policy != sched_policy) {
		printf("FAILED: have scheduler policy %i expected %i\n",
		       sched_policy, p->sched_policy);
		exit(PTS_FAIL);
	}

	if (p->sched_priority != param.sched_priority) {
		printf("FAILED: have scheduler priority %i expected %i\n",
		       p->sched_priority, param.sched_priority);
		exit(PTS_FAIL);
	}

	/* Now check that priorities actually work */
	run_hp_threads(p->sched_policy, p->sched_priority + 1);

	return NULL;
}

struct tcase {
	int sched_policy;
	int prio;
};

enum tprio {
	MIN,
	HALF,
	MAX_1,
};

struct tcase tcases[] = {
	{SCHED_FIFO, MIN},
	{SCHED_FIFO, HALF},
	{SCHED_FIFO, MAX_1},
	{SCHED_RR, MIN},
	{SCHED_RR, HALF},
	{SCHED_RR, MAX_1},
};

static int get_prio(struct tcase *self)
{
	switch (self->prio) {
	case MIN:
		return sched_get_priority_min(self->sched_policy);
	break;
	case HALF:
		 return (sched_get_priority_min(self->sched_policy) +
		         sched_get_priority_max(self->sched_policy)) / 2;
	break;
	case MAX_1:
		return sched_get_priority_max(self->sched_policy) - 1;
	break;
	}

	printf("Wrong self->prio %i\n", self->prio);
	exit(PTS_UNRESOLVED);
}

static const char *sched_policy_name(int policy)
{
	switch (policy) {
	case SCHED_FIFO:
		return "SCHED_FIFO";
	case SCHED_RR:
		return "SCHED_RR";
	default:
		return "UNKNOWN";
	}
}

int main(void)
{
	pthread_attr_t attr;
	pthread_t th;
	struct params p;
	int ret;
	unsigned int i;

	ret = set_affinity_single();
	if (ret) {
		n_threads = get_ncpu();
		if (n_threads == -1) {
			printf("Cannot get number of CPUs\n");
			return PTS_UNRESOLVED;
		}
		printf("INFO: Affinity not supported, running %i threads.\n",
		       n_threads);
	} else {
		printf("INFO: Affinity works, will use only one thread.\n");
		n_threads = 1;
	}

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		p.sched_policy = tcases[i].sched_policy;
		p.sched_priority = get_prio(&tcases[i]);

		init_attr(&attr, p.sched_policy, p.sched_priority);

		printf("INFO: Testing %s prio %i\n",
		       sched_policy_name(p.sched_policy), p.sched_priority);

		ret = pthread_create(&th, &attr, do_test, &p);
		if (ret) {
			fprintf(stderr, "pthread_create(): %s\n", strerror(ret));
			return PTS_UNRESOLVED;
		}

		pthread_join(th, NULL);

		pthread_attr_destroy(&attr);
	}

	printf("Test PASSED\n");
	return 0;
}
