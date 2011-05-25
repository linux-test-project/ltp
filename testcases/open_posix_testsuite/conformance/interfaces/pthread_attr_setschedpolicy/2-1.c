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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   Author:  Peter W. Morreale <pmorreale AT novell DOT com>
 *
 *   Date:  20/05/2011
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <posixtest.h>

/* Priorities for the threads, must be unique, non-zero, and ordered */
#define PRIO_HIGH	20
#define PRIO_MED	10
#define PRIO_LOW	5

static int priorities[3];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t c_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static int thread_started;

#define ERR_MSG(f, rc)	printf("Failed: function: %s status: %s(%u)\n", \
						f, strerror(rc), rc)

static void *thread_func(void *data)
{
	struct sched_param sp;
	int policy;
	int rc;

	rc = pthread_getschedparam(pthread_self(), &policy, &sp);
	if (rc) {
		ERR_MSG("pthread_getschedparam()", rc);
		goto done;
	}

	thread_started = 1;
	rc = pthread_cond_signal(&cond);
	if (rc) {
		ERR_MSG("pthread_cond_signal()", rc);
		goto done;
	}

	rc = pthread_mutex_lock(&mutex);
	if (rc) {
		ERR_MSG("pthread_mutex_lock()", rc);
		goto done;
	}

	/* Stuff the priority in execution order */
	if (!priorities[0])
		priorities[0] = sp.sched_priority;
	else if (!priorities[1])
		priorities[1] = sp.sched_priority;
	else
		priorities[2] = sp.sched_priority;


	rc = pthread_mutex_unlock(&mutex);
	if (rc) {
		ERR_MSG("pthread_mutex_unlock()", rc);
		goto done;
	}

done:
	return (void *) (long) rc;
}

static int create_thread(int prio, pthread_t *tid)
{
	int rc;
	char *func;
	struct sched_param sp;
	pthread_attr_t attr;

	func = "pthread_attr_init()";
	rc = pthread_attr_init(&attr);
	if (rc != 0)
		goto done;

	func = "pthread_attr_setschedpolicy()";
	rc = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (rc != 0)
		goto error;

	func = "pthread_attr_setinheritsched()";
	rc = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (rc != 0)
		goto error;

	func = "pthread_attr_setschedparam()";
	sp.sched_priority = prio;
	rc = pthread_attr_setschedparam(&attr, &sp);
	if (rc != 0)
		goto error;

	thread_started = 0;

	rc = pthread_create(tid, &attr, thread_func, NULL);
	if (rc) {
		ERR_MSG("pthread_create()", rc);
		goto error;
	}

	while (!thread_started) {
		func = "pthread_mutex_lock()";
		rc = pthread_mutex_lock(&c_mutex);
		if (rc)
			goto error;

		func = "pthread_cond_wait()";
		rc = pthread_cond_wait(&cond, &c_mutex);
		if (rc)
			goto unlock;

		func = "pthread_mutex_unlock()";
		rc = pthread_mutex_unlock(&c_mutex);
		if (rc)
			goto error;
	}

	pthread_attr_destroy(&attr);

	return 0;

unlock:
	(void) pthread_mutex_unlock(&c_mutex);
error:
	pthread_attr_destroy(&attr);
done:
	ERR_MSG(func, rc);
	return -1;
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

	status = PTS_UNRESOLVED;

	rc = pthread_mutex_lock(&mutex);
	if (rc) {
		ERR_MSG("pthread_mutex_lock()", rc);
		goto done;
	}

	rc = create_thread(PRIO_LOW, &t3);
	if (rc)
		goto done;

	rc = create_thread(PRIO_MED, &t2);
	if (rc)
		goto done;

	rc = create_thread(PRIO_HIGH, &t1);
	if (rc)
		goto done;

	rc = pthread_mutex_unlock(&mutex);
	if (rc)
		ERR_MSG("pthread_mutex_unlock()", rc);

	rc = pthread_join(t1, &r1);
	if (rc) {
		ERR_MSG("pthread_join(t1)", rc);
		goto done;
	}

	rc = pthread_join(t2, &r2);
	if (rc) {
		ERR_MSG("pthread_join(t2)", rc);
		goto done;
	}

	rc = pthread_join(t3, &r3);
	if (rc) {
		ERR_MSG("pthread_join(t3)", rc);
		goto done;
	}

	/* Threads fail? */
	if ((long) r1 || (long) r2 || (long) r2)
		goto done;

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

done:
	if (status == PTS_PASS)
		printf("Test PASS\n");

	return status;
}
