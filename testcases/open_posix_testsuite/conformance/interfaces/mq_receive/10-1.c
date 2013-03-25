/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * mq_receive() test plan:
 * mq_receive() will fail with EAGAIN, if  message queue is empty and
 * O_NONBLOCK is set in the message queue,
 */

#include <stdio.h>
#include <errno.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "posixtest.h"

#define TEST "10-1"
#define FUNCTION "mq_receive"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE 50
#define BUFFER 40

int main(void)
{
	char mqname[NAMESIZE], msgrv[BUFFER];
	mqd_t mqdes;
	struct mq_attr attr;
	int unresolved = 0, failure = 0;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	attr.mq_msgsize = BUFFER;
	attr.mq_maxmsg = BUFFER;
	mqdes =
	    mq_open(mqname, O_CREAT | O_NONBLOCK | O_RDWR, S_IRUSR | S_IWUSR,
		    &attr);
	if (mqdes == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open");
		unresolved = 1;
	}
	if (mq_receive(mqdes, msgrv, BUFFER, NULL) == -1) {
		if (EAGAIN != errno) {
			printf("errno != EAGAIN \n");
			failure = 1;
		}
	} else {
		printf("mq_receive() succeed unexpectly\n");
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
