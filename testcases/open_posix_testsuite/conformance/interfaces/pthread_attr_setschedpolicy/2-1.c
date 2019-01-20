/*
 *
 *   Copyright (c) Novell Inc. 2011
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms in version 2 of the GNU General Public License as published by
 *   the Free Software Foundation.
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
 *   Author:  Peter W. Morreale <pmorreale AT novell DOT com>
 *
 *   Date:  20/05/2011
 */

#include "affinity.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <posixtest.h>

/* Priorities for the threads, must be unique, non-zero, and ordered */
#define PRIO_HIGH	20
#define PRIO_MED	10
#define PRIO_LOW	5
#define PRIO_MAIN	1

static int priorities[3];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t c_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static int thread_started;

#define FAIL_AND_EXIT(f, rc) { \
	printf("Failed: function: %s status: %s(%u)\n", f, strerror(rc), rc); \
	exit(PTS_UNRESOLVED); \
}

static void *thread_func(void *data LTP_ATTRIBUTE_UNUSED)
{
	struct sched_param sp;
	int policy;
	int rc;

	rc = pthread_getschedparam(pthread_self(), &policy, &sp);
	if (rc)
		FAIL_AND_EXIT("pthread_getschedparam()", rc);

	rc = pthread_mutex_lock(&c_mutex);
	if (rc)
		FAIL_AND_EXIT("pthread_mutex_lock()", rc);
	thread_started = 1;
	rc = pthread_cond_signal(&cond);
	if (rc)
		FAIL_AND_EXIT("pthread_cond_signal()", rc);
	rc = pthread_mutex_unlock(&c_mutex);
	if (rc)
		FAIL_AND_EXIT("pthread_mutex_unlock()", rc);

	rc = pthread_mutex_lock(&mutex);
	if (rc)
		FAIL_AND_EXIT("pthread_mutex_lock()", rc);

	/* Stuff the priority in execution order */
	if (!priorities[0])
		priorities[0] = sp.sched_priority;
	else if (!priorities[1])
		priorities[1] = sp.sched_priority;
	else
		priorities[2] = sp.sched_priority;

	rc = pthread_mutex_unlock(&mutex);
	if (rc)
		FAIL_AND_EXIT("pthread_mutex_unlock()", rc);

	return (void *)(long)rc;
}

static int create_thread(int prio, pthread_t * tid)
{
	int rc;
	struct sched_param sp;
	pthread_attr_t attr;

	rc = pthread_attr_init(&attr);
	if (rc != 0)
		FAIL_AND_EXIT("pthread_attr_init()", rc);

	rc = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (rc != 0)
		FAIL_AND_EXIT("pthread_attr_setschedpolicy()", rc);

	rc = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (rc != 0)
		FAIL_AND_EXIT("pthread_attr_setinheritsched()", rc);

	sp.sched_priority = prio;
	rc = pthread_attr_setschedparam(&attr, &sp);
	if (rc != 0)
		FAIL_AND_EXIT("pthread_attr_setschedparam()", rc);

	thread_started = 0;

	rc = pthread_create(tid, &attr, thread_func, NULL);
	if (rc)
		FAIL_AND_EXIT("pthread_create()", rc);

	rc = pthread_mutex_lock(&c_mutex);
	if (rc)
		FAIL_AND_EXIT("pthread_mutex_lock()", rc);
	while (!thread_started) {
		rc = pthread_cond_wait(&cond, &c_mutex);
		if (rc)
			FAIL_AND_EXIT("pthread_cond_wait()", rc);
	}
	rc = pthread_mutex_unlock(&c_mutex);
	if (rc)
		FAIL_AND_EXIT("pthread_mutex_unlock()", rc);

	pthread_attr_destroy(&attr);

	return 0;
}

int main(void)
{
	int status;
	int rc;
	void *r1;
	void *r2;
	void *r3;
	pthread_t t1;
	pthread_t t2;
	pthread_t t3;
	struct sched_param sp;

	status = PTS_UNRESOLVED;


	rc = set_affinity_single();
	if (rc)
		FAIL_AND_EXIT("set_affinity_single", errno);

	sp.sched_priority = PRIO_MAIN;
	rc = pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp);
	if (rc)
		FAIL_AND_EXIT("pthread_setschedparam()", rc);

	rc = pthread_mutex_lock(&mutex);
	if (rc)
		FAIL_AND_EXIT("pthread_mutex_lock()", rc);

	rc = create_thread(PRIO_LOW, &t3);
	if (rc)
		FAIL_AND_EXIT("create_thread LOW", rc);

	rc = create_thread(PRIO_MED, &t2);
	if (rc)
		FAIL_AND_EXIT("create_thread MED", rc);

	rc = create_thread(PRIO_HIGH, &t1);
	if (rc)
		FAIL_AND_EXIT("create_thread HIGH", rc);

	rc = pthread_mutex_unlock(&mutex);
	if (rc)
		FAIL_AND_EXIT("pthread_mutex_unlock()", rc);

	rc = pthread_join(t1, &r1);
	if (rc)
		FAIL_AND_EXIT("pthread_join(t1)", rc);

	rc = pthread_join(t2, &r2);
	if (rc)
		FAIL_AND_EXIT("pthread_join(t2)", rc);

	rc = pthread_join(t3, &r3);
	if (rc)
		FAIL_AND_EXIT("pthread_join(t3)", rc);

	/* priorities must be high to low */
	status = PTS_FAIL;
	if (priorities[0] != PRIO_HIGH)
		printf("Failed: first is prio: %u, should be: %u\n",
		       priorities[0], PRIO_HIGH);
	else if (priorities[1] != PRIO_MED)
		printf("Failed: second is prio: %u, should be: %u\n",
		       priorities[1], PRIO_MED);
	else if (priorities[2] != PRIO_LOW)
		printf("Failed: third is prio: %u, should be: %u\n",
		       priorities[2], PRIO_LOW);
	else
		status = PTS_PASS;

	if (status == PTS_PASS)
		printf("Test PASSED\n");

	return status;
}
