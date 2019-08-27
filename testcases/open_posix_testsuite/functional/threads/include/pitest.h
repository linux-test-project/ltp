
/*
 *  Copyright (c) 2003, Intel Corporation. All rights reserved.
 *  Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 *  This file is licensed under the GPL license.  For the full content
 *  of this license, see the COPYING file at the top level of this
 *  source tree.
 */

#include <sys/time.h>
#include <string.h>
#include "test.h"

#define PROTOCOL                PTHREAD_PRIO_INHERIT

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
};

static inline
struct thread_param *setup_thread_param(struct thread_param *tp_template, int nElements)
{
	int i, tf_index;
	struct thread_param *tp;
	int cpus;

	cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (cpus < 2)
		cpus = 2;
	tp = malloc(sizeof(struct thread_param) * (nElements + cpus - 2));
	if (!tp) {
		EPRINTF("UNRESOLVED: malloc: %d %s", errno, strerror(errno));
		exit(UNRESOLVED);
	}
	memcpy(tp, tp_template, sizeof(struct thread_param) * nElements);

	tf_index = nElements - 1;
	for (i = 1; i < cpus - 1; ++i) {
		tp[tf_index + i] = tp_template[tf_index];
		tp[tf_index + i].index += i;
		tp[tf_index + i].cpu += i;
	}

	return tp;
}

static inline
double seconds_read(void)
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec + 1e-6 * tv.tv_usec;
}

static inline
int test_set_priority(pthread_t pid, unsigned policy, unsigned prio)
{
	struct sched_param sched_param;
	memset(&sched_param, 0, sizeof(sched_param));
	sched_param.sched_priority = prio;
	if (pthread_setschedparam(pid, policy, &sched_param) == -1)
	{
		EPRINTF("UNRESOLVED: Can't set policy to %d and prio to %d",
		  	policy, prio);
  	  	exit(UNRESOLVED);
  	}
	return 0;
}

static inline
void mutex_attr_init(pthread_mutexattr_t *attr)
{
	unsigned rc;

	rc = pthread_mutexattr_init(attr);
        if (rc != 0) {
                EPRINTF("UNRESOLVED: pthread_mutexattr_init: %d %s",
                        rc, strerror(rc));
                exit(UNRESOLVED);
        }

	rc = pthread_mutexattr_setprotocol(attr, PROTOCOL);
        if (rc != 0) {
                EPRINTF("UNRESOLVED: pthread_mutexattr_setprotocol: %d %s",
	                rc, strerror(rc));
                exit(UNRESOLVED);
        }
}

static inline
int mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
	unsigned rc;

	rc = pthread_mutex_init(mutex, attr);
        if (rc != 0) {
                EPRINTF("UNRESOLVED: pthread_mutex_init: %d %s",
                        rc, strerror(rc));
                exit(UNRESOLVED);
        }
	return 0;
}

static inline
int threadattr_init(pthread_attr_t *threadattr)
{
	unsigned rc;
	rc = pthread_attr_init(threadattr);
	if (rc != 0) {
                EPRINTF("UNRESOLVED: pthread_attr_init: %d %s",
                        rc, strerror(rc));
	        exit(UNRESOLVED);
        }
	return 0;
}

