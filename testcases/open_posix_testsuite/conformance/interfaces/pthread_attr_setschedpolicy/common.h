/*
 *
 *   Copyright (c) Novell Inc. 2011
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
 *   Author:  Peter W. Morreale <pmorreale AT novell DOT com>
 *
 *   Date:  20/05/2011
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "posixtest.h"

#define PRIORITY_OTHER -1
#define PRIORITY_FIFO 20
#define PRIORITY_RR 20

#define ERR_MSG(p, f, rc)					\
	printf("Failed: policy: %s, func: %s, rc: %s (%u)\n",	\
				p, f, strerror(rc), rc)

struct params {
	int	policy;
	int	priority;
	char	*policy_label;
	int	status;
};

static void *thread_func(void *data)
{
	pthread_t self;
	struct sched_param sp;
	struct params *p = data;
	int policy;
	int rc;

	self = pthread_self();
	rc = pthread_getschedparam(self, &policy, &sp);
	if (rc != 0) {
		ERR_MSG(p->policy_label, "pthread_getschedparam()", rc);
		goto done;
	}

	p->status = PTS_FAIL;

	if (policy != p->policy) {
		printf("Failed: policy: %s thread policy: %u != %u\n",
			p->policy_label, policy, p->policy);
		goto done;
	}

	if (p->priority != PRIORITY_OTHER && sp.sched_priority != p->priority) {
		printf("Failed: policy: %s thread priority %u != %u\n",
			p->policy_label, sp.sched_priority, p->priority);
		goto done;
	}

	p->status = PTS_PASS;

done:
	return NULL;
}

static int init_attr(pthread_attr_t *attr, struct params *p)
{
	int rc;
	char *func;
	struct sched_param sp;

	func = "pthread_attr_init()";
	rc = pthread_attr_init(attr);
	if (rc != 0)
		goto done;

	func = "pthread_attr_setschedpolicy()";
	rc = pthread_attr_setschedpolicy(attr, p->policy);
	if (rc != 0)
		goto error;

	func = "pthread_attr_setinheritsched()";
	rc = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
	if (rc != 0)
		goto error;

	func = "pthread_attr_setschedparam()";
	sp.sched_priority = p->priority;
	if (p->priority != PRIORITY_OTHER) {
		rc = pthread_attr_setschedparam(attr, &sp);
		if (rc != 0)
			goto error;
	}

	return PTS_PASS;

error:
	pthread_attr_destroy(attr);
done:
	ERR_MSG(p->policy_label, func, rc);
	return PTS_UNRESOLVED;
}

static int create_test_thread(struct params *p)
{
	pthread_t thread;
	pthread_attr_t attr;
	void *status;
	int rc;

	rc = init_attr(&attr, p);
	if (rc != PTS_PASS)
		return rc;

	rc = pthread_create(&thread, &attr, thread_func, p);
	if (rc != 0) {
		ERR_MSG(p->policy_label, "pthread_create()", rc);
		if (rc == EPERM)
			return PTS_UNRESOLVED;
		else
			return PTS_FAIL;
	}

	pthread_join(thread, &status);

	pthread_attr_destroy(&attr);

	return p->status;
}
