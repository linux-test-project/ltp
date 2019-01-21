
/*
 *  Copyright (c) 2003, Intel Corporation. All rights reserved.
 *  Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 *  This file is licensed under the GPL license.  For the full content
 *  of this license, see the COPYING file at the top level of this
 *  source tree.
 */

/* There are n TF threads, n is equal to the processors in the system minus
 * one. TFs are used to keep busy these CPUs, which have priority 3. A
 * TL thread with lower priority 1 is created, which locks a mutex and
 * does workload. A TB thread with higher priority 4 is created and try
 * to lock TL's mutex. A TP thread with priority 2 is created to reflect the
 * priority change of TL. Main thread has the highest priority 6, which will
 * control the running steps of those threads, including creating threads,
 * stopping threads. There is another thread to collect the sample data with
 * priority 5.
 *
 * Steps:
 * 1.	Create n TF threads, n is equal to processors number minus one. TF
 * 	will do workload.
 * 2.	Create 1 TP thread and do workload. The thread will keep running when
 * 	TL is created.
 * 3.	Create 1 TL thread to lock a mutex. TL will get a chance to run
 *      when TP sleep a wee bit in between.
 * 4.	Create 1 TB thread to lock the mutex. TL's priority will boost to
 *  	TB's priority, which will cause TP having no chance to run.
 * 5.	Set TB's prioirty to 1, TL's priority will decrease. So TP and TL
 * 	will keep working as before.
 * 6.	Stop these threads.
 *
 * NOTE: Most of the code is ported from test-11 written by inkay.
 */

#ifdef	__linux__
#define	_GNU_SOURCE
#endif
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "test.h"
#include "posixtest.h"
#include "pitest.h"

int cpus;
pthread_mutex_t mutex;
volatile int ts_stop = 0;
volatile double base_time;

struct thread_param {
	int index;
	volatile int stop;
	int sleep_ms;
	int priority;
	int policy;
	const char *name;
	int cpu;
	volatile unsigned futex;
	volatile unsigned should_stall;
	volatile unsigned progress;
} tp[] = {
	{
	0, 0, 0, 1, SCHED_FIFO, "TL", 0, 0, 0, 0}, {
	1, 0, 200, 2, SCHED_FIFO, "TP", 0, 0, 0, 0}, {
	2, 0, 0, 3, SCHED_FIFO, "TF", 1, 0, 0, 0}, {
	3, 0, 0, 3, SCHED_FIFO, "TF", 2, 0, 0, 0}, {
	4, 0, 0, 3, SCHED_FIFO, "TF", 3, 0, 0, 0}, {
	5, 0, 0, 3, SCHED_FIFO, "TF", 4, 0, 0, 0}, {
	6, 0, 0, 3, SCHED_FIFO, "TF", 5, 0, 0, 0}, {
	7, 0, 0, 3, SCHED_FIFO, "TF", 6, 0, 0, 0}
};

volatile unsigned do_work_dummy;
void do_work(unsigned granularity_top, volatile unsigned *progress)
{
	unsigned granularity_cnt, i;
	unsigned top = 5 * 1000 * 1000;
	unsigned dummy = do_work_dummy;

	for (granularity_cnt = 0; granularity_cnt < granularity_top;
	     granularity_cnt++) {
		for (i = 0; i < top; i++)
			dummy = i | dummy;
		(*progress)++;
	}
	return;
}

void *thread_fn(void *param)
{
	struct thread_param *tp = param;
	struct timespec ts;
	int rc;

#if __linux__
	unsigned long mask = 1 << tp->cpu;

	rc = sched_setaffinity(0, sizeof(mask), &mask);
	if (rc < 0) {
		EPRINTF("UNRESOLVED: Thread %s index %d: Can't set affinity: "
			"%d %s", tp->name, tp->index, rc, strerror(rc));
		exit(UNRESOLVED);
	}
#endif
	test_set_priority(pthread_self(), SCHED_FIFO, tp->priority);
	DPRINTF(stderr, "Thread %s index %d: started\n", tp->name, tp->index);
	DPRINTF(stdout, "#EVENT %f Thread %s Started\n",
		seconds_read() - base_time, tp->name);

	tp->progress = 0;
	ts.tv_sec = 0;
	ts.tv_nsec = tp->sleep_ms * 1000 * 1000;
	while (!tp->stop) {
		do_work(5, &tp->progress);
		if (tp->sleep_ms == 0)
			continue;
		rc = nanosleep(&ts, NULL);
		if (rc < 0) {
			EPRINTF("UNRESOLVED: Thread %s %d: nanosleep returned "
				"%d %s", tp->name, tp->index, rc, strerror(rc));
			exit(UNRESOLVED);
		}
	}

	DPRINTF(stdout, "#EVENT %f Thread %s Started\n",
		seconds_read() - base_time, tp->name);
	return NULL;
}

void *thread_tl(void *param)
{
	struct thread_param *tp = param;

#if __linux__
	unsigned long mask = 1 << tp->cpu;
	int rc;

	rc = sched_setaffinity((pid_t) 0, sizeof(mask), &mask);
	if (rc < 0) {
		EPRINTF
		    ("UNRESOLVED: Thread %s index %d: Can't set affinity: %d %s",
		     tp->name, tp->index, rc, strerror(rc));
		exit(UNRESOLVED);
	}
#endif

	test_set_priority(pthread_self(), SCHED_FIFO, tp->priority);

	DPRINTF(stderr, "Thread %s index %d: started\n", tp->name, tp->index);
	DPRINTF(stdout, "#EVENT %f Thread %s Started\n",
		seconds_read() - base_time, tp->name);
	tp->progress = 0;
	pthread_mutex_lock(&mutex);
	while (!tp->stop) {
		do_work(5, &tp->progress);
	}
	pthread_mutex_unlock(&mutex);
	DPRINTF(stdout, "#EVENT %f Thread %s Stopted\n",
		seconds_read() - base_time, tp->name);
	return NULL;
}

void *thread_sample(void *arg LTP_ATTRIBUTE_UNUSED)
{
	char buffer[1024];
	struct timespec ts;
	double period = 250;
	size_t size;
	int i;
	int rc;

	test_set_priority(pthread_self(), SCHED_FIFO, 5);
	DPRINTF(stderr, "Thread Sampler: started\n");
	DPRINTF(stdout, "# COLUMNS %d Time TP TL ", 2 + cpus);
	for (i = 0; i < (cpus - 1); i++)
		DPRINTF(stdout, "TF%d ", i);
	DPRINTF(stdout, "\n");
	ts.tv_sec = 0;
	ts.tv_nsec = period * 1000 * 1000;

	while (!ts_stop) {
		size =
		    snprintf(buffer, 1023, "%f ", seconds_read() - base_time);
		for (i = 0; i < cpus + 1; i++)
			size +=
			    snprintf(buffer + size, 1023 - size, "%u ",
				     tp[i].progress);
		DPRINTF(stdout, "%s\n", buffer);
		rc = nanosleep(&ts, NULL);
		if (rc < 0)
			EPRINTF("UNRESOLVED: Thread %s %d: nanosleep returned "
				"%d %s", tp->name, tp->index, rc, strerror(rc));
	}
	return NULL;
}

void *thread_tb(void *arg)
{
	struct timespec boost_time;
	double t0, t1;
	int rc;

	test_set_priority(pthread_self(), SCHED_FIFO, 4);
	DPRINTF(stderr, "Thread TB: started\n");

	DPRINTF(stdout, "#EVENT %f TB Started, waiting for mutex for %lu s\n",
		seconds_read() - base_time, *(time_t *) arg);

	boost_time.tv_sec = time(NULL) + *(time_t *) arg;
	boost_time.tv_nsec = 0;

	t0 = seconds_read();
	rc = pthread_mutex_timedlock(&mutex, &boost_time);
	t1 = seconds_read();
	DPRINTF(stdout, "#EVENT %f TB Thread Waited for %.2f s\n",
		seconds_read() - base_time, t1 - t0);

	if (rc != ETIMEDOUT && rc != 0) {
		EPRINTF("FAIL: Thread TB: lock returned %d %s, "
			"slept %f", rc, strerror(rc), t1 - t0);
		exit(FAIL);
	}

	DPRINTF(stdout, "#EVENT %f TB Stopped\n", seconds_read() - base_time);
	return NULL;
}

int main(void)
{
	cpus = sysconf(_SC_NPROCESSORS_ONLN);
	pthread_mutexattr_t mutex_attr;
	pthread_attr_t threadattr;
	pthread_t threads[cpus - 1], threadsample, threadtp, threadtl, threadtb;

	time_t multiplier = 1;
	int i;
	int rc;

	test_set_priority(pthread_self(), SCHED_FIFO, 6);
	base_time = seconds_read();

	/* Initialize a mutex with PTHREAD_PRIO_INHERIT protocol */
	mutex_attr_init(&mutex_attr);
	mutex_init(&mutex, &mutex_attr);

	/* Initialize thread attr */
	threadattr_init(&threadattr);

	/* Start the sample thread */
	DPRINTF(stderr, "Main Thread: Creating sample thread\n");
	rc = pthread_create(&threadsample, &threadattr, thread_sample, NULL);
	if (rc != 0) {
		EPRINTF("UNRESOLVED: pthread_create: %d %s", rc, strerror(rc));
		exit(UNRESOLVED);
	}

	/* Start the TF threads */
	DPRINTF(stderr, "Main Thread: Creating %d TF threads\n", cpus - 1);
	for (i = 0; i < cpus - 1; i++) {
		rc = pthread_create(&threads[i], &threadattr, thread_fn,
				    &tp[i + 2]);
		if (rc != 0) {
			EPRINTF("UNRESOLVED: pthread_create: %d %s",
				rc, strerror(rc));
			exit(UNRESOLVED);
		}
	}

	sleep(base_time + multiplier * 10 - seconds_read());

	/* Start TP thread */
	DPRINTF(stderr, "Main Thread: Creating TP thread\n");
	rc = pthread_create(&threadtp, &threadattr, thread_fn, &tp[1]);
	if (rc != 0) {
		EPRINTF("UNRESOLVED: pthread_create: %d %s", rc, strerror(rc));
		exit(UNRESOLVED);
	}
	sleep(base_time + multiplier * 20 - seconds_read());

	/* Start TL thread */
	DPRINTF(stderr, "Main Thread: Creating TL thread\n");
	rc = pthread_create(&threadtl, &threadattr, thread_tl, &tp[0]);
	if (rc != 0) {
		EPRINTF("UNRESOLVED: pthread_create: %d %s", rc, strerror(rc));
		exit(UNRESOLVED);
	}
	sleep(base_time + multiplier * 30 - seconds_read());

	/* Start TB thread (boosting thread) */
	unsigned long timeout = multiplier * 20;
	rc = pthread_create(&threadtb, &threadattr, thread_tb, &timeout);
	if (rc != 0) {
		EPRINTF("UNRESOLVED: pthread_create: %d %s", rc, strerror(rc));
		exit(UNRESOLVED);
	}
	sleep(base_time + multiplier * 40 - seconds_read());

	/* Decrease TB's priority */
	DPRINTF(stderr, "Main Thread: decrease TB's priority to 1\n");
	test_set_priority(threadtb, SCHED_FIFO, 1);
	DPRINTF(stdout, "#EVENT %f TB's priority decreased\n",
		seconds_read() - base_time);
	sleep(base_time + multiplier * 60 - seconds_read());

	/* Stop TL thread */
	tp[0].stop = 1;
	sleep(base_time + multiplier * 70 - seconds_read());

	/* Stop TP thread */
	tp[1].stop = 1;
	sleep(base_time + multiplier * 80 - seconds_read());

	/* Stop TF threads */
	for (i = 2; i < cpus - 1; i++) {
		tp[i].stop = 1;
	}

	/* Stop sampler */
	ts_stop = 1;
	DPRINTF(stderr, "Main Thread: stop sampler thread\n");
	return 0;
}
