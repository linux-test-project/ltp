/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if O_NONBLOCK is set and the message queue is full, mq_send()
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
#include "posixtest.h"

#define NAMESIZE 50
#define MESSAGESIZE 50

#define BUFFER 40
#define MAXMSG 10

int main(void)
{
	char qname[NAMESIZE];
	char msgptr[MESSAGESIZE];
	mqd_t queue;
	struct mq_attr attr;
	int unresolved = 0, failure = 0, i, maxreached = 0;

	sprintf(qname, "/mq_send_10-1_%d", getpid());

	attr.mq_maxmsg = MAXMSG;
	attr.mq_msgsize = BUFFER;
	queue = mq_open(qname, O_CREAT | O_RDWR | O_NONBLOCK,
			S_IRUSR | S_IWUSR, &attr);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < MAXMSG + 1; i++) {
		sprintf(msgptr, "message %d", i);
		if (mq_send(queue, msgptr, strlen(msgptr), 1) == -1) {
			maxreached = 1;
			if (errno != EAGAIN) {
				printf("mq_send() did not fail on EAGAIN\n");
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
