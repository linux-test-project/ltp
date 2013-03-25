/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if O_NONBLOCK is set and the message queue is full, mq_timedsend()
 * will set errno == EAGAIN (subset of 7-1.c).
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include "posixtest.h"

#define NAMESIZE 50
#define MESSAGESIZE 50

#define BUFFER 100
#define MAXMSG 5

int main(void)
{
	char qname[NAMESIZE];
	char msgptr[MESSAGESIZE];
	struct timespec ts;
	struct mq_attr attr;
	mqd_t queue;
	int unresolved = 0, failure = 0, i, maxreached = 0;

	sprintf(qname, "/mq_timedsend_10-1_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = MAXMSG;
	queue = mq_open(qname, O_CREAT | O_RDWR | O_NONBLOCK,
			S_IRUSR | S_IWUSR, &attr);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}

	ts.tv_sec = time(NULL) + 1;
	ts.tv_nsec = 0;
	for (i = 0; i < MAXMSG + 1; i++) {
		sprintf(msgptr, "message %d", i);
		if (mq_timedsend(queue, msgptr, strlen(msgptr), 1, &ts) == -1) {
			maxreached = 1;
			if (errno != EAGAIN) {
				printf("mq_timedsend() did not w/EAGAIN\n");
				failure = 1;
			}
			break;
		}
	}

	if (mq_close(queue) != 0) {
		perror("mq_close() did not return success");
		unresolved = 1;
	}

	if (mq_unlink(qname) != 0) {
		perror("mq_unlink() did not return success");
		unresolved = 1;
	}

	if (maxreached == 0) {
		printf("Test inconclusive:  Couldn't fill message queue\n");
		return PTS_UNRESOLVED;
	}
	if (failure == 1) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (unresolved == 1) {
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
