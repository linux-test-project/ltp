/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Test having two timers in different processes set to expire at the
 * same time, and ensure they both expire at the same time.
 */
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "posixtest.h"

#define EXPIREDELTA 2

#define CHILDPASS 1

int main(int argc, char *argv[])
{
	int pid;
	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
		perror("clock_gettime() did not return success\n");
		return PTS_UNRESOLVED;
	}

	if ((pid = fork()) == 0) {
		/*child */
		struct sigevent ev;
		timer_t tid;
		struct itimerspec its;
		sigset_t set;
		int sig;
		int flags = 0;

		if (sigemptyset(&set) == -1) {
			perror("sigemptyset() failed\n");
			return PTS_UNRESOLVED;
		}

		if (sigaddset(&set, SIGABRT) == -1) {
			perror("sigaddset() failed\n");
			return PTS_UNRESOLVED;
		}

		if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
			perror("sigprocmask() failed\n");
			return PTS_UNRESOLVED;
		}
		ev.sigev_notify = SIGEV_SIGNAL;
		ev.sigev_signo = SIGABRT;
		if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
			perror("timer_create() did not return success\n");
			return PTS_UNRESOLVED;
		}

		its.it_value.tv_sec = ts.tv_sec + EXPIREDELTA;
		its.it_value.tv_nsec = ts.tv_nsec;
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;

		flags |= TIMER_ABSTIME;
		if (timer_settime(tid, flags, &its, NULL) != 0) {
			perror("timer_settime() did not return success\n");
			return PTS_UNRESOLVED;
		}

		if (sigwait(&set, &sig) == -1) {
			perror("sigwait() failed\n");
			return PTS_UNRESOLVED;
		}

		if (sig == SIGABRT) {
			printf("Got it! Child\n");
			return CHILDPASS;
		}

		printf("Got another signal! Child\n");
		return PTS_FAIL;
	} else {
		/*parent */
		struct sigevent ev;
		timer_t tid;
		struct itimerspec its;
		sigset_t set;
		int sig;
		int flags = 0;
		int i;

		if (sigemptyset(&set) == -1) {
			perror("sigemptyset() failed\n");
			return PTS_UNRESOLVED;
		}

		if (sigaddset(&set, SIGALRM) == -1) {
			perror("sigaddset() failed\n");
			return PTS_UNRESOLVED;
		}

		if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
			perror("sigaprocmask() failed\n");
			return PTS_UNRESOLVED;
		}

		ev.sigev_notify = SIGEV_SIGNAL;
		ev.sigev_signo = SIGALRM;
		if (timer_create(CLOCK_REALTIME, &ev, &tid) != 0) {
			perror("timer_create() did not return success\n");
			return PTS_UNRESOLVED;
		}

		its.it_value.tv_sec = ts.tv_sec + EXPIREDELTA;
		its.it_value.tv_nsec = ts.tv_nsec;
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;

		flags |= TIMER_ABSTIME;
		if (timer_settime(tid, flags, &its, NULL) != 0) {
			perror("timer_settime() did not return success\n");
			return PTS_UNRESOLVED;
		}

		if (sigwait(&set, &sig) == -1) {
			perror("sigwait() failed\n");
			return PTS_UNRESOLVED;
		}

		if (sig != SIGALRM) {
			printf("Got another signal! Parent\n");
			return PTS_FAIL;
		}

		printf("Got it! Parent\n");

		if (wait(&i) == -1) {
			perror("Error waiting for child to exit\n");
			return PTS_UNRESOLVED;
		}
		if (WEXITSTATUS(i)) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
	}
	return PTS_UNRESOLVED;
}
