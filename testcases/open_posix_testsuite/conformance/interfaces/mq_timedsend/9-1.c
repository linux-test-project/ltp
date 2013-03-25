/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that mq_timedsend() returns -1 on failure, no message is queued, and
 * errno is set.
 * The failure case chosen is an invalid message queue descriptor.
 *
 * 3/13/03 - Added fix from Gregoire Pichon for specifying an attr
 *           with a mq_maxmsg >= BUFFER.
 *
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"
#define BUFFER 40

int main(void)
{
	char qname[NAMESIZE], msgrcd[BUFFER];
	const char *msgptr = MSGSTR;
	struct timespec ts;
	mqd_t queue;
	struct mq_attr attr;
	int unresolved = 0, failure = 0;
	unsigned pri;

	sprintf(qname, "/mq_timedsend_9-1_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = BUFFER;
	queue = mq_open(qname, O_CREAT | O_RDWR | O_NONBLOCK,
			S_IRUSR | S_IWUSR, &attr);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}
	// Verify mq_timedsend() returns -1
	ts.tv_sec = time(NULL) + 1;
	ts.tv_nsec = 0;
	if (mq_timedsend(queue + 1, msgptr, strlen(msgptr), 1, &ts) != -1) {
		printf("mq_timedsend() did not return -1 on invalid queue\n");
		failure = 1;
	}
	// Verify errno is set
	if (errno != EBADF) {
		printf("errno was not set on invalid queue\n");
		failure = 1;
	}
	// Verify message was not queued (cannot be received)
	if (mq_receive(queue, msgrcd, BUFFER, &pri) != -1) {
		if (strcmp(msgptr, msgrcd) == 0) {
			printf("Message ended up being sent\n");
			failure = 1;
		} else {
			printf("Error with mq_receive()\n");
			unresolved = 1;
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
