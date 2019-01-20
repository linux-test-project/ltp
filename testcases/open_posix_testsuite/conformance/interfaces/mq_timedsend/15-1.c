/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if abs_timeout has already passed when mq_timedsend is
 * called, then the timeout shall expire.
 *
 * Test by setting the timeout to a time in the past and then sending
 * messages until one would block.  Ensure that the error on this message
 * that would block is
 * ETIMEDOUT and that the time is close to the start time.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"
#define TIMEOUT 5
#define DELTA 1
#define BUFFER 40
#define MAXMSG 5

char gqname[NAMESIZE];
mqd_t gqueue;

void testfailed_handler(int signo LTP_ATTRIBUTE_UNUSED)
{
	printf("Timeout never happened\n");
	printf("Test FAILED\n");
	mq_close(gqueue);
	mq_unlink(gqname);
	exit(PTS_FAIL);
}

int main(void)
{
	char *msgptr = MSGSTR;
	struct timespec ts;
	struct sigaction act;
	struct mq_attr attr;
	time_t currsec;
	int maxreached = 0, i;

	sprintf(gqname, "/mq_timedsend_15-1_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = MAXMSG;
	gqueue = mq_open(gqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (gqueue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	currsec = time(NULL);

	ts.tv_sec = currsec - 5;	//time in past
	ts.tv_nsec = 0;

	/*
	 * If timeout never happens, set up an alarm that will go off
	 * after TIMEOUT seconds and call a handler to end the test
	 */
	act.sa_handler = testfailed_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGALRM, &act, 0);
	alarm(TIMEOUT);

	for (i = 0; i < MAXMSG + 1; i++) {
		if (mq_timedsend(gqueue, msgptr, strlen(msgptr), 1, &ts)
		    == -1) {
			maxreached = 1;
			if (errno != ETIMEDOUT) {
				printf("errno != ETIMEDOUT\n");
				printf("Test FAILED\n");
				mq_close(gqueue);
				mq_unlink(gqname);
				return PTS_FAIL;
			}
			break;
		}
	}

	mq_close(gqueue);
	mq_unlink(gqname);

	if (maxreached == 0) {
		printf("Test UNRESOLVED:  Couldn't fill message queue\n");
		return PTS_UNRESOLVED;
	}

	if (time(NULL) > currsec + DELTA) {
		printf("Timeout lasted too long\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
