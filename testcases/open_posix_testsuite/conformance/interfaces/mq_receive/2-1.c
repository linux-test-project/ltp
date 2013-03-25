/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * mq_receive test plan:
 * If the buffer size is less than the mq_msgsize attribute, the function
 * will fail and return an error.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define TEST "2-1"
#define FUNCTION "mq_receive"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE 50
#define BUFFER 20

int main(void)
{
	char mqname[NAMESIZE], msgrv[BUFFER];
	const char *msgptr = "test message";
	mqd_t mqdes;
	int prio = 1;
	struct mq_attr mqstat;
	int unresolved = 0, failure = 0;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());
	memset(&mqstat, 0, sizeof(mqstat));
	mqstat.mq_msgsize = BUFFER + 1;
	mqstat.mq_maxmsg = BUFFER + 1;

	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &mqstat);
	if (mqdes == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open");
		unresolved = 1;
	}

	if (mq_send(mqdes, msgptr, strlen(msgptr), prio) != 0) {
		perror(ERROR_PREFIX "mq_send");
		unresolved = 1;
	}

	if (mq_receive(mqdes, msgrv, BUFFER, NULL) > 0) {
		printf("FAIL: mq_receive succeed unexpectly \n");
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
