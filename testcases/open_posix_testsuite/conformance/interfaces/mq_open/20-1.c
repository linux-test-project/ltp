/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Another basic test that mq_open() returns a valid message queue
 * descriptor on success.
 *
 * Test as in 1-1.c.  To make this different, call mq_notify on the
 * message queue descriptor returned.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define NAMESIZE 50

void handler(int signo)
{
	(void) signo;
#ifdef DEBUG
	printf("in handler\n");
#endif
}

int main(void)
{
	char qname[NAMESIZE];
	mqd_t queue;
	struct sigevent ev;
	struct sigaction act;
	int failure = 0;

	sprintf(qname, "/mq_open_20-1_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	/* set up notification */
	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGUSR1;
	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGUSR1, &act, NULL);
	if (mq_notify(queue, &ev) != 0) {
		perror("mq_notify() did not return success");
		failure = 1;
	}

	mq_close(queue);
	mq_unlink(qname);

	if (failure == 1) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
