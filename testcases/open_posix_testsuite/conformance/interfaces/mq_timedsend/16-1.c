/*
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 * adam.li@intel.com - 2004-04-29
 */

/*
 * Test that if the message queue is full and O_NONBLOCK is not set,
 * mq_timedsend() will block until abs_timeout is reached.
 *
 * Test by sending messages in a child process until the message queue is full.
 * At this point, the child should be blocking on sending.  Then, have the
 * parent wait for the timeout and return pass when the next message is sent
 * to the message queue.
 *
 * If Timers is supported, then abs_timeout is based on CLOCK_REALTIME.
 * Otherwise, the timeout is based on the system clock (time() function).
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"
#define BUFFER 40
#define MAXMSG 5

#define TIMEOUT 7

#define CHILDPASS 1
#define CHILDFAIL 0

char gqname[NAMESIZE];
mqd_t gqueue;

/*
 * This handler is just used to catch the signal and stop sleep (so the
 * parent knows the child is still busy sending signals).
 */
void stopsleep_handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	return;
}

int main(void)
{
	int pid;
	struct mq_attr attr;
	const char *msgptr = MSGSTR;

	sprintf(gqname, "/mq_timedsend_16-1_%d", getpid());

	attr.mq_maxmsg = MAXMSG;
	attr.mq_msgsize = BUFFER;
	gqueue = mq_open(gqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	mq_unlink(gqname);
	if (gqueue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}

	if ((pid = fork()) == 0) {
		/* child here */
		int i, sig;
		struct timespec ts;
		sigset_t mask;

		/* wait for parent to set up handler */
		sigemptyset(&mask);
		sigaddset(&mask, SIGUSR1);
		sigprocmask(SIG_BLOCK, &mask, NULL);
		sigwait(&mask, &sig);

		/* child should block in < TIMEOUT seconds */
#ifdef _POSIX_TIMERS
		printf("Using CLOCK_REALTIME\n");
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += TIMEOUT;
#else
		ts.tv_sec = time(NULL) + TIMEOUT;
#endif
		ts.tv_nsec = 0;

		for (i = 0; i < MAXMSG + 1; i++) {
			if (mq_timedsend(gqueue, msgptr,
					 strlen(msgptr), 1, &ts) != 0) {
				/* send will fail after timeout occurs */
				kill(getppid(), SIGABRT);
				return CHILDPASS;
			}
			/* send signal to parent each time message is sent */
			kill(getppid(), SIGABRT);
		}
		printf("Child never interrupted\n");
		return CHILDFAIL;
	} else {
		/* parent here */
		struct sigaction act;
		int j;

		/* parent runs stopsleep_handler when sleep is interrupted
		   by child */
		act.sa_handler = stopsleep_handler;
		act.sa_flags = 0;
		sigemptyset(&act.sa_mask);
		sigaction(SIGABRT, &act, 0);

		/* wait 1 second and tell child handler is set up */
		struct timespec ts;
		ts.tv_sec = 1;
		ts.tv_nsec = 0;
		nanosleep(&ts, NULL);
		kill(pid, SIGUSR1);

		/* wait for heartbeats from child */
		for (j = 0; j < MAXMSG + 1; j++) {
			ts.tv_sec = 3;
			ts.tv_nsec = 0;
			if (nanosleep(&ts, NULL)
			    == 0) {
				/* If sleep finished, child is probably blocking */
				break;
			}
		}

		if (j == MAXMSG + 1) {
			printf("Child never blocked\n");
			printf("Test FAILED\n");
			kill(pid, SIGKILL);	//kill child
			mq_close(gqueue);
			mq_unlink(gqname);
			return PTS_FAIL;
		}

		/*
		 * Wait for timeout to complete.
		 */
		ts.tv_sec = TIMEOUT;
		ts.tv_nsec = 0;
		if (nanosleep(&ts, NULL) == 0) {
			/*
			 * If sleep lasted the full time, child never timed out
			 */
			printf("Child never timed out\n");
			kill(pid, SIGKILL);	//kill child
			mq_close(gqueue);
			mq_unlink(gqname);
			printf("Test FAILED\n");
			return PTS_FAIL;
		}

		mq_close(gqueue);
		mq_unlink(gqname);
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	return PTS_UNRESOLVED;
}
