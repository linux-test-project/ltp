/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that mq_open() does not add messages to the queue or remove
 * messages from the queue.
 *
 * Test using mq_send and mq_receive:
 * - Call mq_open() for non-blocking queue
 * - Verify mq_receive() fails (because nothing should be in the queue yet)
 * - Call mq_send() to put something in the queue
 * - Call mq_open() again for non-blocking queue
 * - Verify mq_receive() now succeeded (because the sent message should
 *   still be in the queue).
 *
 *   3/13/03 - Added fix from Gregoire Pichon for specifying an attr
 *             with a mq_maxmsg >= BUFFER.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"
#define BUFFER 40

int main(void)
{
	char qname[NAMESIZE], msgrcd[BUFFER];
	const char *msgptr = MSGSTR;
	mqd_t queue;
	struct mq_attr attr;
	int failure = 0;
	unsigned pri;

	sprintf(qname, "/mq_open_19-1_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = BUFFER;

	queue = mq_open(qname, O_CREAT | O_RDWR | O_NONBLOCK,
			S_IRUSR | S_IWUSR, &attr);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	if (mq_receive(queue, msgrcd, BUFFER, &pri) != -1) {
		printf("mq_receive() succeded\n");
		printf("mq_open() may have placed a message in the queue\n");
		failure = 1;
	}

	if (mq_send(queue, msgptr, strlen(msgptr), 1) == -1) {
		perror("mq_send() did not return success");
		printf("Test UNRESOLVED\n");
		/* close queue and exit */
		mq_close(queue);
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}

	queue = mq_open(qname, O_RDWR | O_NONBLOCK, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() second time did not return success");
		printf("Test UNRESOLVED\n");
		/* close queue and exit */
		mq_close(queue);
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}

	if (mq_receive(queue, msgrcd, BUFFER, &pri) == -1) {
		perror("mq_receive() failed");
		printf("mq_open() may have removed a msg from the queue\n");
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
