/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that mq_send() will fail if msg_len is not <= mq_attr->mq_msgsize.
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
#define MSGSTR "01234567890123456789"
#define MSGSIZE 10		// < strlen(MSGSTR)

int main(void)
{
	char qname[NAMESIZE];
	const char *msgptr = MSGSTR;
	mqd_t queue;
	int unresolved = 0, failure = 0;
	struct mq_attr attr;

	sprintf(qname, "/mq_send_2-1_%d", getpid());

	attr.mq_msgsize = MSGSIZE;
	attr.mq_maxmsg = MSGSIZE;

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}

	if (mq_send(queue, msgptr, strlen(msgptr), 1) == 0) {
		printf("mq_send() returned success when msg_len>=mq_msgsize\n");
		failure = 1;
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
