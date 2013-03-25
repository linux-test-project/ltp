/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */
/*
 *  mq_setattr() test plan:
 *  mq_setattr() set attributes of the open message queue,
 *  then verify that mq_setattr() returns 0 on success.
 *
 *  2/17/2004   call mq_close and mq_unlink before exit to release mq
 *		resources
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "mq_setattr"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE	50
#define MQFLAGS		O_NONBLOCK

int main(void)
{
	char mqname[NAMESIZE];
	mqd_t mqdes;
	struct mq_attr mqstat, nmqstat;
	int unresolved = 0;
	int failure = 0;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
	if (mqdes == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}
	memset(&mqstat, 0, sizeof(mqstat));
	memset(&nmqstat, 0, sizeof(nmqstat));
	if (mq_getattr(mqdes, &mqstat) == -1) {
		perror(ERROR_PREFIX "mq_getattr");
		unresolved = 1;
	}
	mqstat.mq_flags |= MQFLAGS;
	if (mq_setattr(mqdes, &mqstat, NULL) != 0) {
		perror(ERROR_PREFIX "mq_setattr");
		failure = 1;
	}
	if (mq_getattr(mqdes, &nmqstat) == -1) {
		perror(ERROR_PREFIX "mq_getattr");
		unresolved = 1;
	}
	mq_close(mqdes);
	if (mq_unlink(mqname) != 0) {
		perror(ERROR_PREFIX "mq_unlink");
		return PTS_UNRESOLVED;
	}
	if (nmqstat.mq_flags != mqstat.mq_flags) {
		printf("FAIL: mq_flags getted is %ld , while mq_flags "
		       "setted is %ld\n", nmqstat.mq_flags, mqstat.mq_flags);
		failure = 1;
	}

	if (failure == 1) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	if (unresolved == 1) {
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED \n");
	return PTS_PASS;
}
