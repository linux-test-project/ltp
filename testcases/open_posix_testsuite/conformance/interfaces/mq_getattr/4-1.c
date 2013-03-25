/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * mq_getattr test plan:
 * mq_getattr gets mq_curmsgs value, which will be returned as
 * the number of the message currently on the queue.
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

#define TEST "4-1"
#define FUNCTION "mq_getattr"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE 50
#define MSG_NUM	5
#define MSGSIZE		50
#define MAXMSG		40

int main(void)
{
	char mqname[NAMESIZE];
	const char *msgptr = "test message";
	mqd_t mqdes;
	struct mq_attr mqstat;
	int i;
	int unresolved = 0, failure = 0;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	memset(&mqstat, 0, sizeof(mqstat));
	mqstat.mq_msgsize = MSGSIZE;
	mqstat.mq_maxmsg = MAXMSG;
	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &mqstat);
	if (mqdes == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open()");
		return PTS_UNRESOLVED;
	}
	for (i = 0; i < MSG_NUM; i++) {
		if (mq_send(mqdes, msgptr, strlen(msgptr), 1) == -1) {
			perror(ERROR_PREFIX "mq_send()");
			unresolved = 1;
		}
	}
	memset(&mqstat, 0, sizeof(mqstat));
	if (mq_getattr(mqdes, &mqstat) != 0) {
		perror(ERROR_PREFIX "mq_getattr");
		unresolved = 1;
	} else {
		if (mqstat.mq_curmsgs != MSG_NUM) {
			printf("mq_getattr didn't get the correct "
			       "mq_curmsgs \n");
			failure = 1;
		}
	}

	mq_close(mqdes);
	mq_unlink(mqname);

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
