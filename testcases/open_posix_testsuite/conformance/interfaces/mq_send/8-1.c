/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that mq_send() returns 0 on success.
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
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"
#define BUFFER 40
#define MAXMSG 10

int main(void)
{
	char qname[NAMESIZE], msgrcd[BUFFER];
	const char *msgptr = MSGSTR;
	mqd_t queue;
	struct mq_attr attr;
	int unresolved = 0, ret;
	unsigned pri;

	sprintf(qname, "/mq_send_8-1_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = MAXMSG;
	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}

	ret = mq_send(queue, msgptr, strlen(msgptr), 1);

	if (mq_receive(queue, msgrcd, BUFFER, &pri) == -1) {
		perror("mq_receive() returned failure");
		unresolved = 1;
	}

	if (strncmp(msgptr, msgrcd, strlen(msgptr)) != 0) {
		printf("send was unsuccessful:  sent %s received %s\n",
		       msgptr, msgrcd);
		unresolved = 1;
	}

	if (mq_close(queue) != 0) {
		perror("mq_close() did not return success");
		unresolved = 1;
	}

	if (mq_unlink(qname) != 0) {
		perror("mq_unlink() did not return success");
		unresolved = 1;
	}

	if (unresolved == 1) {
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	if (ret != 0) {
		printf("mq_send() did not return 0 on success\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
