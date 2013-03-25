/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * mq_timedreceive test plan:
 * mq_timedreceive will receive the oldest of the highest priority messages
 * from the message queue. The selected message will be removed from the
 * queue and copied to the buffer pointed by the msg_ptr argument.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "mq_timedreceive"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE 50
#define BUFFER 40

int main(void)
{
	char mqname[NAMESIZE], msgrv1[BUFFER], msgrv2[BUFFER];
	const char *msgptr1 = "test message 1";
	const char *msgptr2 = "test message 2";
	mqd_t mqdes;
	unsigned rvprio, sdprio1 = 1, sdprio2 = 2;
	struct timespec ts;
	struct mq_attr attr;
	int unresolved = 0, failure = 0;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = BUFFER;
	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
	if (mqdes == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open");
		unresolved = 1;
	}

	if (mq_send(mqdes, msgptr1, strlen(msgptr1), sdprio1) != 0) {
		perror(ERROR_PREFIX "mq_send");
		unresolved = 1;
	}
	if (mq_send(mqdes, msgptr2, strlen(msgptr2), sdprio2) != 0) {
		perror(ERROR_PREFIX "mq_send");
		unresolved = 1;
	}

	ts.tv_sec = time(NULL) + 1;
	ts.tv_nsec = 0;
	if (mq_timedreceive(mqdes, msgrv1, BUFFER, &rvprio, &ts) == -1) {
		perror(ERROR_PREFIX "mq_timedreceive");
		failure = 1;
	}

	if (strncmp(msgptr2, msgrv1, strlen(msgptr2)) != 0) {
		printf("FAIL: mq_timedreceive didn't receive the highest "
		       "priority message\n");
		failure = 1;
	}
	if (rvprio != sdprio2) {
		printf("FAIL: receive priority %d != send priority %d\n",
		       rvprio, sdprio2);
		failure = 1;
	}
	ts.tv_sec = time(NULL) + 1;
	ts.tv_nsec = 0;
	if (mq_timedreceive(mqdes, msgrv2, BUFFER, &rvprio, &ts) == -1) {
		perror(ERROR_PREFIX "mq_timedreceive");
		failure = 1;
	}
	if (strncmp(msgptr1, msgrv2, strlen(msgptr1)) != 0) {
		printf("FAIL: mq_timedreceive didn't receive the correct "
		       "message\n");
		failure = 1;
	}
	if (rvprio != sdprio1) {
		printf("FAIL: receive priority %d != send priority %d\n",
		       rvprio, sdprio1);
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
