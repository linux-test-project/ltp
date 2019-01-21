/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/* This tests case will send a SIGABRT to sem_timedwait, and should
 * return error EINTR.
 */


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include "posixtest.h"

#define TEST "9-1"
#define FUNCTION "sem_timedwait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define CHILDPASS 1
#define CHILDFAIL 0

void handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("In handler\n");
}

int main(void)
{
	sem_t mysemp;
	struct timespec ts;
	pid_t pid;

	if (sem_init(&mysemp, 0, 1) == -1) {
		perror(ERROR_PREFIX "sem_init");
		return PTS_UNRESOLVED;
	}

	if (sem_wait(&mysemp) == -1) {
		perror(ERROR_PREFIX "sem_wait");
		return PTS_UNRESOLVED;
	}

	pid = fork();
	if (pid == 0) {		// child create the semaphore.
		struct sigaction act;

		act.sa_handler = handler;
		act.sa_flags = 0;
		if (sigemptyset(&act.sa_mask) == -1) {
			perror("Error calling sigemptyset\n");
			return CHILDFAIL;
		}
		if (sigaction(SIGABRT, &act, 0) == -1) {
			perror("Error calling sigaction\n");
			return CHILDFAIL;
		}

		ts.tv_sec = time(NULL) + 3;
		ts.tv_nsec = 0;

		sem_timedwait(&mysemp, &ts);

		if (errno == EINTR) {
			printf("Test PASSED\n");
			return (CHILDPASS);
		}
		puts("TEST FAILED: errno != EINTR");
		return (CHILDFAIL);

	} else {		// parent to send a signal to child
		int i;
		sleep(1);
		(void)kill(pid, SIGABRT);	// send signal to child
		if (wait(&i) == -1) {
			perror("Error waiting for child to exit\n");
			return PTS_UNRESOLVED;
		}

		if (!WEXITSTATUS(i)) {
			return PTS_FAIL;
		}
		puts("TEST PASSED");
		return PTS_PASS;
	}
	return PTS_UNRESOLVED;
}
