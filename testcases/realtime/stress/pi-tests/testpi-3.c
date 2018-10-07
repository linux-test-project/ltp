/******************************************************************************
 *
 *	 Copyright © International Business Machines	Corp., 2005, 2008
 *
 *	 This program is free software;	you can redistribute it and/or modify
 *	 it under the terms of the GNU General Public License as published by
 *	 the Free Software Foundation; either version 2 of the License, or
 *	 (at your option) any later version.
 *
 *	 This program is distributed in the hope that it will be useful,
 *	 but WITHOUT ANY WARRANTY;	without even the implied warranty of
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See
 *	 the GNU General Public License for more details.
 *
 *	 You should have received a copy of the GNU General Public License
 *	 along with this program;	if not, write to the Free Software
 *	 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * NAME
 *	testpi-3.c
 *
 * DESCRIPTION
 *
 *
 * USAGE:
 *	Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *
 *
 * HISTORY
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <librttest.h>

void usage(void)
{
	rt_help();
	printf("testpi-3 specific options:\n");
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

int gettid(void)
{
	return syscall(__NR_gettid);
}

typedef void *(*entrypoint_t) (void *);

#define THREAD_STOP		1

pthread_mutex_t glob_mutex;

/*typedef struct thread {
	int priority;
	int policy;
	entrypoint_t func;
	pthread_attr_t attr;
	pthread_t handle;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int flags;
	int count;
} Thread;*/

typedef struct thread Thread;

Thread arg1, arg2, arg3, arg4, arg5;

int strartThread(Thread * thr);
void stopThread(Thread * thr);
void joinThread(Thread * thr);

void *func_nonrt(void *arg)
{
	Thread *pthr = (Thread *) arg;
	int rc, i, j, policy, tid = gettid();
	struct sched_param schedp;
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);

	rc = sched_setaffinity(0, sizeof(mask), &mask);
	if (rc < 0) {
		printf("Thread %d: Can't set affinity: %d %s\n", tid, rc,
		       strerror(rc));
		exit(-1);
	}
	rc = sched_getaffinity(0, sizeof(mask), &mask);

	printf("Thread started %d on CPU %ld\n", pthr->priority,
	       (long)mask.__bits[0]);
	pthread_getschedparam(pthr->pthread, &policy, &schedp);
	printf("Thread running %d\n", pthr->priority);

	while (1) {
		pthread_mutex_lock(&glob_mutex);
		printf
		    ("Thread %d at start pthread pol %d pri %d - Got global lock\n",
		     pthr->priority, policy, schedp.sched_priority);
		sleep(2);
		for (i = 0; i < 10000; i++) {
			if ((i % 100) == 0) {
				sched_getparam(tid, &schedp);
				policy = sched_getscheduler(tid);
				printf("Thread %d(%d) loop %d pthread pol %d "
				       "pri %d\n", tid, pthr->priority, i,
				       policy, schedp.sched_priority);
				fflush(NULL);
			}
			pthr->id++;
			for (j = 0; j < 5000; j++) {
				pthread_mutex_lock(&(pthr->mutex));
				pthread_mutex_unlock(&(pthr->mutex));
			}
		}
		pthread_mutex_unlock(&glob_mutex);
		sched_yield();
	}
	return NULL;
}

void *func_rt(void *arg)
{
	Thread *pthr = (Thread *) arg;
	int rc, i, j, policy, tid = gettid();
	struct sched_param schedp;
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);

	rc = sched_setaffinity(0, sizeof(mask), &mask);
	if (rc < 0) {
		printf("Thread %d: Can't set affinity: %d %s\n", tid, rc,
		       strerror(rc));
		exit(-1);
	}
	rc = sched_getaffinity(0, sizeof(mask), &mask);

	printf("Thread started %d on CPU %ld\n", pthr->priority,
	       (long)mask.__bits[0]);
	pthread_getschedparam(pthr->pthread, &policy, &schedp);

	while (1) {
		sleep(2);
		printf("Thread running %d\n", pthr->priority);
		pthread_mutex_lock(&glob_mutex);
		printf
		    ("Thread %d at start pthread pol %d pri %d - Got global lock\n",
		     pthr->priority, policy, schedp.sched_priority);

		/* we just use the mutex as something to slow things down */
		/* say who we are and then do nothing for a while.      The aim
		 * of this is to show that high priority threads make more
		 * progress than lower priority threads..
		 */
		for (i = 0; i < 1000; i++) {
			if (i % 100 == 0) {
				sched_getparam(tid, &schedp);
				policy = sched_getscheduler(tid);
				printf
				    ("Thread %d(%d) loop %d pthread pol %d pri %d\n",
				     tid, pthr->priority, i, policy,
				     schedp.sched_priority);
				fflush(NULL);
			}
			pthr->id++;
			for (j = 0; j < 5000; j++) {
				pthread_mutex_lock(&(pthr->mutex));
				pthread_mutex_unlock(&(pthr->mutex));
			}
		}
		pthread_mutex_unlock(&glob_mutex);
		sleep(2);
	}
	return NULL;
}

void *func_noise(void *arg)
{
	Thread *pthr = (Thread *) arg;
	int rc, i, j, policy, tid = gettid();
	struct sched_param schedp;
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);

	rc = sched_setaffinity(0, sizeof(mask), &mask);
	if (rc < 0) {
		printf("Thread %d: Can't set affinity: %d %s\n", tid, rc,
		       strerror(rc));
		exit(-1);
	}
	rc = sched_getaffinity(0, sizeof(mask), &mask);

	printf("Noise Thread started %d on CPU %ld\n", pthr->priority,
	       (long)mask.__bits[0]);
	pthread_getschedparam(pthr->pthread, &policy, &schedp);

	while (1) {
		sleep(1);
		printf("Noise Thread running %d\n", pthr->priority);

		for (i = 0; i < 10000; i++) {
			if ((i % 100) == 0) {
				sched_getparam(tid, &schedp);
				policy = sched_getscheduler(tid);
				printf
				    ("Noise Thread %d(%d) loop %d pthread pol %d pri %d\n",
				     tid, pthr->priority, i, policy,
				     schedp.sched_priority);
				fflush(NULL);
			}
			pthr->id++;
			for (j = 0; j < 5000; j++) {
				pthread_mutex_lock(&(pthr->mutex));
				pthread_mutex_unlock(&(pthr->mutex));
			}
		}
		sched_yield();
	}
	return NULL;
}

int startThread(Thread * thrd)
{
	struct sched_param schedp;
	pthread_condattr_t condattr;
	int retc, policy, inherit;

	printf("Start thread priority %d\n", thrd->priority);
	if (pthread_attr_init(&(thrd->attr)) != 0) {
		printf("Attr init failed");
		exit(2);
	}
	thrd->flags = 0;
	memset(&schedp, 0, sizeof(schedp));
	schedp.sched_priority = thrd->priority;
	policy = thrd->policy;

	if (pthread_attr_setschedpolicy(&(thrd->attr), policy) != 0) {
		printf("Can't set policy %d\n", policy);
	}
	if (pthread_attr_getschedpolicy(&(thrd->attr), &policy) != 0) {
		printf("Can't get policy\n");
	} else {
		printf("Policy in attribs is %d\n", policy);
	}
	if (pthread_attr_setschedparam(&(thrd->attr), &schedp) != 0) {
		printf("Can't set params");
	}
	if (pthread_attr_getschedparam(&(thrd->attr), &schedp) != 0) {
		printf("Can't get params");
	} else {
		printf("Priority in attribs is %d\n", schedp.sched_priority);
	}
	if (pthread_attr_setinheritsched(&(thrd->attr), PTHREAD_EXPLICIT_SCHED)
	    != 0) {
		printf("Can't set inheritsched\n");
	}
	if (pthread_attr_getinheritsched(&(thrd->attr), &inherit) != 0) {
		printf("Can't get inheritsched\n");
	} else {
		printf("inherit sched in attribs is %d\n", inherit);
	}
	if ((retc = pthread_mutex_init(&(thrd->mutex), NULL)) != 0) {
		printf("Failed to init mutex: %d\n", retc);
	}
	if (pthread_condattr_init(&condattr) != 0) {
		printf("Failed to init condattr\n");
	}
	if (pthread_cond_init(&(thrd->cond), &condattr) != 0) {
		printf("Failed to init cond\n");
	}
	retc =
	    pthread_create(&(thrd->pthread), &(thrd->attr), thrd->func, thrd);
	printf("Create returns %d\n\n", retc);
	return retc;
}

void stopThread(Thread * thr)
{
	thr->flags += THREAD_STOP;
	joinThread(thr);
}

void joinThread(Thread * thr)
{
	void *ret = NULL;
	if (pthread_join(thr->pthread, &ret) != 0) {
		printf("Join failed\n");
	}
	printf("Join gave %p\n", ret);
}

/*
 * Test pthread creation at different thread priorities.
 */
int main(int argc, char *argv[])
{
	int i, retc, nopi = 0;
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(0, &mask);
	setup();

	rt_init("h", parse_args, argc, argv);

	retc = sched_setaffinity(0, sizeof(mask), &mask);
	if (retc < 0) {
		printf("Main Thread: Can't set affinity: %d %s\n", retc,
		       strerror(retc));
		exit(1);
	}
	retc = sched_getaffinity(0, sizeof(mask), &mask);

	/*
	 * XXX: Have you ever heard of structures with c89/c99?
	 * Inline assignment is a beautiful thing.
	 */
	arg1.policy = SCHED_OTHER;
	arg1.priority = 0;
	arg1.func = func_nonrt;
	arg2.policy = SCHED_RR;
	arg2.priority = 20;
	arg2.func = func_rt;
	arg3.policy = SCHED_RR;
	arg3.priority = 30;
	arg3.func = func_rt;
	arg4.policy = SCHED_RR;
	arg4.priority = 40;
	arg4.func = func_rt;
	arg5.policy = SCHED_RR;
	arg5.priority = 40;
	arg5.func = func_noise;

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "nopi") == 0)
			nopi = 1;
	}

	printf("Start %s\n", argv[0]);

#if HAS_PRIORITY_INHERIT
	if (!nopi) {
		pthread_mutexattr_t mutexattr;
		int protocol;

		if (pthread_mutexattr_init(&mutexattr) != 0) {
			printf("Failed to init mutexattr\n");
		};
		if (pthread_mutexattr_setprotocol
		    (&mutexattr, PTHREAD_PRIO_INHERIT) != 0) {
			printf("Can't set protocol prio inherit\n");
		}
		if (pthread_mutexattr_getprotocol(&mutexattr, &protocol) != 0) {
			printf("Can't get mutexattr protocol\n");
		} else {
			printf("protocol in mutexattr is %d\n", protocol);
		}
		if ((retc = pthread_mutex_init(&glob_mutex, &mutexattr)) != 0) {
			printf("Failed to init mutex: %d\n", retc);
		}
	}
#endif

	startThread(&arg1);
	startThread(&arg2);
	startThread(&arg3);
	startThread(&arg4);
	startThread(&arg5);

	sleep(10);

	printf("Stopping threads\n");
	stopThread(&arg1);
	stopThread(&arg2);
	stopThread(&arg3);
	stopThread(&arg4);
	stopThread(&arg5);

	printf("Thread counts %d %d %d %d %d\n", arg1.id, arg2.id, arg3.id,
	       arg4.id, arg5.id);
	printf("Done\n");

	return 0;
}
