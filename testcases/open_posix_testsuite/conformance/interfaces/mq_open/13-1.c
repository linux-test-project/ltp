/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if O_CREAT is set and attr != NULL, then mq_maxmsg and
 * mq_msgsize are as defined in attr.
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
#define MAXMSG 10
#define MSGSIZE 5

int main(void)
{
	char qname[NAMESIZE];
	mqd_t queue;
	struct mq_attr attr;
	struct mq_attr attrget;

	sprintf(qname, "/mq_open_13-1_%d", getpid());

	attr.mq_maxmsg = MAXMSG;
	attr.mq_msgsize = MSGSIZE;
	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (mq_getattr(queue, &attrget) != 0) {
		perror("mq_getattr() failed");
		printf("Test FAILED -- could not get attributes\n");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_FAIL;
	}

	if ((attrget.mq_maxmsg != attr.mq_maxmsg) ||
	    (attrget.mq_msgsize != attr.mq_msgsize)) {
		printf("sent: mq_maxmsg %ld; received %ld\n", attr.mq_maxmsg,
		       attrget.mq_maxmsg);
		printf("sent: mq_msgsize %ld; received %ld\n", attr.mq_msgsize,
		       attrget.mq_msgsize);
		printf("Test FAILED -- attributes do not match\n");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_FAIL;
	}

	mq_close(queue);
	mq_unlink(qname);

	printf("Test PASSED\n");
	return PTS_PASS;
}
