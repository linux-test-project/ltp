/*
 * Copyright (c) 2003 - 2004, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 * adam.li@intel.com
 */
/*
 * If Timers option is supported, then abs_timeout is based on
 * CLOCK_REALTIME.
 * Otherwise, the timeout is based on the system clock (time() function).
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "8-1"
#define FUNCTION "mq_timedreceive"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE 50
#define BUFFER 40
#define TIMEOUT	3

int blocking;
void exit_handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("FAIL: the case is blocking, exit anyway\n");
	blocking = 1;
	return;
}

int main(void)
{
	char mqname[NAMESIZE], msgrv[BUFFER];
	mqd_t mqdes;
	struct timespec ts;
	time_t oldtime, newtime;
	struct mq_attr attr;
	pid_t pid;
	int unresolved = 0, failure = 0;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = BUFFER;
	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (mqdes == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open");
		unresolved = 1;
	}

	pid = fork();
	if (pid != 0) {
		/* Parent process */
		struct sigaction act;
		act.sa_handler = exit_handler;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		sigaction(SIGABRT, &act, 0);
#ifdef _POSIX_TIMERS
		printf("Using CLOCK_REALTIME\n");
		clock_gettime(CLOCK_REALTIME, &ts);
		oldtime = ts.tv_sec;
		ts.tv_sec = ts.tv_sec + TIMEOUT;
#else
		ts.tv_sec = time(NULL) + TIMEOUT;
		oldtime = time(NULL);
#endif
		ts.tv_nsec = 0;
		mq_timedreceive(mqdes, msgrv, BUFFER, NULL, &ts);
#ifdef _POSIX_TIMERS
		clock_gettime(CLOCK_REALTIME, &ts);
		newtime = ts.tv_sec;
#else
		newtime = time(NULL);
#endif
		if ((newtime - oldtime) < TIMEOUT) {
			printf("FAIL: mq_timedreceive didn't block until "
			       "timout expires\n");
			failure = 1;
		}
		/* Parent is not blocking, let child abort */
		kill(pid, SIGABRT);
		wait(NULL);
		if (mq_close(mqdes) != 0) {
			perror(ERROR_PREFIX "mq_close");
			unresolved = 1;
		}
		if (mq_unlink(mqname) != 0) {
			perror(ERROR_PREFIX "mq_unlink");
			unresolved = 1;
		}
		if (failure == 1 || blocking == 1) {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}
		if (unresolved == 1) {
			printf("Test UNRESOLVED\n");
			return PTS_UNRESOLVED;
		}
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		sleep(TIMEOUT + 3);	/* Parent is probably blocking
					   send a signal to let it abort */
		kill(getppid(), SIGABRT);
		return 0;
	}
}
