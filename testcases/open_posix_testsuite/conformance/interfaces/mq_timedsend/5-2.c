/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if the message queue is full and O_NONBLOCK is not set,
 * mq_timedsend() will block until it is interrupted by a signal.
 *
 * Have a child send signals until it starts to block.  At that point, have
 * the parent send a signal to the child.  Test passes if send was interrupted
 * by the signal.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <errno.h>
#include "posixtest.h"
#include "timespec.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"
#define BUFFER 40
#define MAXMSG 5

#define CHILDPASS 1
#define CHILDFAIL 0

char gqname[NAMESIZE];
mqd_t gqueue;

/*
 * This handler is just used to catch the signal and stop sleep (so the
 * parent knows the child is still busy sending signals).
 */
void justreturn_handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	return;
}

int main(void)
{
	int pid;
	const char *msgptr = MSGSTR;
	struct mq_attr attr;
	struct sigaction act;

	sprintf(gqname, "/mq_timedsend_5-2_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = MAXMSG;
	gqueue = mq_open(gqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (gqueue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}

	/* parent and child use justreturn_handler to just return out of
	 * situations -- parent uses to stop it's sleep and wait again for
	 * the child; child uses to stop its mq_timedsend
	 */
	act.sa_handler = justreturn_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGABRT, &act, 0);

	if ((pid = fork()) == 0) {
		/* child here */
		int i;
		struct timespec ts;
		/* set up timeout to be as long as possible */
		ts.tv_sec = TIME_T_MAX;
		ts.tv_nsec = 0;

		sleep(1);	// give parent time to set up handler
		for (i = 0; i < MAXMSG + 1; i++) {
			if (mq_timedsend(gqueue, msgptr,
					 strlen(msgptr), 1, &ts) == -1) {
				if (errno == EINTR) {
					printf
					    ("mq_timedsend interrupted by signal\n");
					return CHILDPASS;
				} else {
					printf
					    ("mq_timedsend not interrupted by signal\n");
					return CHILDFAIL;
				}
			}
			/* send signal to parent each time message is sent */
			kill(getppid(), SIGABRT);
		}

		printf("Child never blocked\n");
		return CHILDFAIL;
	} else {
		/* parent here */
		int j, k, blocking = 0;

		for (j = 0; j < MAXMSG + 1; j++) {
			if (sleep(3) == 0) {
				/* If sleep finished, child is probably blocking */
				blocking = 1;	//set blocking flag
				kill(pid, SIGABRT);	//signal child
				break;
			}
		}

		if (blocking != 1) {
			printf("Signal never blocked\n");
			kill(pid, SIGKILL);	//kill child if not gone
			mq_close(gqueue);
			mq_unlink(gqname);
			return PTS_UNRESOLVED;
		}
		mq_close(gqueue);
		if (mq_unlink(gqname) != 0) {
			perror("mq_unlink()");
			kill(pid, SIGKILL);	//kill child if not gone
			return PTS_UNRESOLVED;
		}

		if (wait(&k) == -1) {
			perror("Error waiting for child to exit\n");
			kill(pid, SIGKILL);	//kill child if not gone
			return PTS_UNRESOLVED;
		}

		if (!WIFEXITED(k) || !WEXITSTATUS(k)) {
			printf("Test FAILED\n");
			return PTS_FAIL;
		}

		printf("Test PASSED\n");
		return PTS_PASS;
	}

	return PTS_UNRESOLVED;
}
