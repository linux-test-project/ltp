/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * mq_receive test plan:
 * On success, mq_receive will return the length of the selected message
 * and the message will be removed from the queue.
 * step:
 * 1. send two messages to the message queue
 * 2. call mq_receive() twice, if the two received messages are the same,
 *    which means the first mq_receive does not remove the message from
 *    the queue, the test will fail. Otherwise, the test will pass.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define TEST "8-1"
#define FUNCTION "mq_receive"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE 50
#define BUFFER 40

int main(void)
{
	char mqname[NAMESIZE], msgrv1[BUFFER], msgrv2[BUFFER];
	const char *msgptr1 = "test message1";
	const char *msgptr2 = "test message2 with differnet length";
	mqd_t mqdes;
	int prio1 = 1, prio2 = 2;
	struct mq_attr attr = {
		.mq_msgsize = BUFFER,
		.mq_maxmsg = BUFFER
	};
	int unresolved = 0, failure = 0;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (mqdes == (mqd_t)-1) {
		perror(ERROR_PREFIX "mq_open");
		unresolved = 1;
	}

	if (mq_send(mqdes, msgptr1, strlen(msgptr1), prio1) != 0) {
		perror(ERROR_PREFIX "mq_send");
		unresolved = 1;
	}
	if (mq_send(mqdes, msgptr2, strlen(msgptr2), prio2) != 0) {
		perror(ERROR_PREFIX "mq_send");
		unresolved = 1;
	}

	if (mq_receive(mqdes, msgrv1, BUFFER, NULL) !=
	    (ssize_t)strlen(msgptr2)) {
		printf("FAIL: mq_receive didn't return the selected message "
		    "size correctly\n");
		failure = 1;
	}
	if (mq_receive(mqdes, msgrv2, BUFFER, NULL) !=
	    (ssize_t)strlen(msgptr1)) {
		printf("FAIL: mq_receive didn't return the selected message "
		    "size correctly\n");
		failure = 1;
	}
	if (!strcmp(msgrv1, msgrv2)) {
		printf("FAIL: mq_receive received the same message twice\n");
		failure = 1;
	}
	if (mq_close(mqdes) != 0) {
		perror(ERROR_PREFIX "mq_close");
		unresolved = 1;
	}
	if (mq_unlink(mqname) != 0) {
		perror(ERROR_PREFIX "mq_unlink");
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
