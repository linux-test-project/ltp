/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Basic test that mq_open() returns a valid message descriptor.
 * Test by calling mq_open() and then verifying that that descriptor can
 * be used to call mq_send().
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

int main(void)
{
	char qname[NAMESIZE];
	const char *msgptr = MSGSTR;
	mqd_t queue;
	int failure = 0;

	sprintf(qname, "/mq_open_1-1_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (mq_send(queue, msgptr, strlen(msgptr), 1) != 0) {
		perror("mq_send() did not return success");
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
