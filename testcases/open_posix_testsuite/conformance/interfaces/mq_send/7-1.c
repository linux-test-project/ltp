/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if O_NONBLOCK is set and the message queue is full, mq_send()
 * will just return an error message and the message will not be queued.
 *
 * Test by sending messages with increasing priority number until a failure
 * is received.  Then test that the message just sent (highest priority) is
 * not the one received.
 *
 * 3/13/03 - Added fix from Gregoire Pichon for specifying an attr
 *           with a mq_maxmsg >= BUFFER.
 *
 */

#include <stdio.h>
#include <limits.h>
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
#define MSGSTR "0123456789"
#define BUFFER 40
#define MAXMSG 10		// send should end after MAXMSG

int main(void)
{
	char qname[NAMESIZE], msgrcd[BUFFER];
	char msgptr[MESSAGESIZE];
	mqd_t queue;
	struct mq_attr attr;
	int unresolved = 0, failure = 0, spri = 1, i, maxreached = 0;
	unsigned pri;

	sprintf(qname, "/mq_send_7-1_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = MAXMSG;
	queue = mq_open(qname, O_CREAT | O_RDWR | O_NONBLOCK,
			S_IRUSR | S_IWUSR, &attr);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < MQ_PRIO_MAX; i++) {	// assuming MAXMSG < MQ_PRIO_MAX
		sprintf(msgptr, "message %d", i);
		if (mq_send(queue, msgptr, strlen(msgptr), spri++) == -1) {
			maxreached = 1;
			if (errno != EAGAIN) {
				printf("mq_send() did not fail on EAGAIN\n");
				failure = 1;
			}
			break;
		}
	}

	if (mq_receive(queue, msgrcd, BUFFER, &pri) == -1) {
		perror("mq_receive() returned failure");
		unresolved = 1;
	} else {
		if ((strcmp(msgptr, msgrcd) == 0) && (maxreached != 0)) {
			printf("Error: Received message that caused EAGAIN\n");
			failure = 1;
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
