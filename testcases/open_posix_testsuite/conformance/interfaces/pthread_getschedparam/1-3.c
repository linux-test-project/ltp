/*
* Copyright (c) 2005, Bull S.A..  All rights reserved.
* Created by: Sebastien Decugis

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
* The function gets the scheduling policy and parameter of
* the specified thread. The priority value is the one last
* set with pthread_setschedparam, pthread_setschedprio or
* pthread_create
*
* The steps are:
* -> create a new thread with a known scheduling policy & param.
* -> check the created thread has the required policy & param.
* -> change the policy with pthread_setschedparam & check the result.
* -> change the param with pthread_setschedprio & check the result.
*
* The test fails if an inconsistency is detected.
*
*/
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <posixtest.h>

#define ERR_MSG(f, rc)  printf("Failed: line: %d func: %s rc: %s (%u)\n", \
				__LINE__, f, strerror(rc), rc)

static void check_param(pthread_t thread, int policy, int priority)
{
	int ret;
	int t_pol;
	struct sched_param t_parm;

	ret = pthread_getschedparam(thread, &t_pol, &t_parm);
	if (ret) {
		ERR_MSG("pthread_getscheparam()", ret);
		exit(PTS_UNRESOLVED);
	}

	if (t_pol != policy) {
		printf("Failed: polices: %u != %u\n", t_pol, policy);
		exit(PTS_FAIL);
	}

	if (t_parm.sched_priority != priority) {
		printf("Failed: priorities: %u != %u\n", t_pol, policy);
		exit(PTS_FAIL);
	}
}

/* thread function */
static void *threaded(void *arg)
{
	int ret;
	pthread_t self = pthread_self();

	check_param(self, SCHED_RR, sched_get_priority_min(SCHED_RR));

	ret = pthread_barrier_wait(arg);
	if (ret && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		ERR_MSG("pthread_barrier_wait()", ret);
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_barrier_wait(arg);
	if (ret && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		ERR_MSG("pthread_barrier_wait()", ret);
		exit(PTS_UNRESOLVED);
	}

	check_param(self, SCHED_FIFO, sched_get_priority_min(SCHED_FIFO));

	ret = pthread_barrier_wait(arg);
	if (ret && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		ERR_MSG("pthread_barrier_wait()", ret);
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_barrier_wait(arg);
	if (ret && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		ERR_MSG("pthread_barrier_wait()", ret);
		exit(PTS_UNRESOLVED);
	}

	check_param(self, SCHED_FIFO, sched_get_priority_max(SCHED_FIFO));

	ret = pthread_barrier_wait(arg);
	if (ret && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		ERR_MSG("pthread_barrier_wait()", ret);
		exit(PTS_UNRESOLVED);
	}

	return NULL;
}

/* The main test function. */
int main(void)
{
	int ret;
	pthread_t child;
	pthread_attr_t ta;
	pthread_barrier_t bar;
	struct sched_param sp;

	ret = pthread_barrier_init(&bar, NULL, 2);
	if (ret) {
		ERR_MSG("pthread_barrier_init()", ret);
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_attr_init(&ta);
	if (ret) {
		ERR_MSG("pthread_attr_init()", ret);
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_attr_setinheritsched(&ta, PTHREAD_EXPLICIT_SCHED);
	if (ret) {
		ERR_MSG("pthread_attr_setinheritsched()", ret);
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_attr_setschedpolicy(&ta, SCHED_RR);
	if (ret) {
		ERR_MSG("pthread_attr_setschedpolicy()", ret);
		exit(PTS_UNRESOLVED);
	}

	sp.sched_priority = sched_get_priority_min(SCHED_RR);
	if (sp.sched_priority == -1) {
		ERR_MSG("sched_get_priority_min()", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_attr_setschedparam(&ta, &sp);
	if (ret) {
		ERR_MSG("pthread_attr_setschedparam()", ret);
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_create(&child, &ta, threaded, &bar);
	if (ret) {
		ERR_MSG("pthread_create()", ret);
		exit(PTS_UNRESOLVED);
	}

	check_param(child, SCHED_RR, sp.sched_priority);

	ret = pthread_barrier_wait(&bar);
	if (ret && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		ERR_MSG("pthread_barrier_wait()", ret);
		exit(PTS_UNRESOLVED);
	}

	sp.sched_priority = sched_get_priority_min(SCHED_FIFO);
	if (sp.sched_priority == -1) {
		ERR_MSG("pthread_barrier_wait()", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_setschedparam(child, SCHED_FIFO, &sp);
	if (ret) {
		ERR_MSG("pthread_setschedparam()", ret);
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_barrier_wait(&bar);
	if (ret && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		ERR_MSG("pthread_barrier_wait()", ret);
		exit(PTS_UNRESOLVED);
	}

	check_param(child, SCHED_FIFO, sp.sched_priority);

	ret = pthread_barrier_wait(&bar);
	if (ret && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		ERR_MSG("pthread_barrier_wait()", ret);
		exit(PTS_UNRESOLVED);
	}

	sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
	if (sp.sched_priority == -1) {
		ERR_MSG("sched_get_priority_max()", errno);
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_setschedprio(child, sp.sched_priority);
	if (ret != 0) {
		ERR_MSG("pthread_setschedprio()", ret);
		exit(PTS_UNRESOLVED);
	}

	ret = pthread_barrier_wait(&bar);
	if (ret && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		ERR_MSG("pthread_barrier_wait()", ret);
		exit(PTS_UNRESOLVED);
	}

	check_param(child, SCHED_FIFO, sched_get_priority_max(SCHED_FIFO));

	ret = pthread_barrier_wait(&bar);
	if (ret && (ret != PTHREAD_BARRIER_SERIAL_THREAD)) {
		ERR_MSG("pthread_barrier_wait()", ret);
		exit(PTS_UNRESOLVED);
	}

	pthread_join(child, NULL);

	printf("Test PASSED\n");

	return PTS_PASS;
}
