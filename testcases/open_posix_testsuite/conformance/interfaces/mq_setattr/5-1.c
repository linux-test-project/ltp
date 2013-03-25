/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */
/*
 * mq_setattr() test plan:
 * Test if mqdes argument is not a valid message queue descriptor,
 * mq_setattr() function will fail with EBADF, and the function will
 * return a value of -1.
 *
 *  2/17/2004   call mq_close and mq_unlink before exit to release mq
 *		resources
 */

#include <stdio.h>
#include <errno.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define TEST "5-1"
#define FUNCTION "mq_setattr"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define MQFLAGS		1
#define NAMESIZE	50

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
		perror(ERROR_PREFIX "mq_open()");
		return PTS_UNRESOLVED;
	}
	if (mq_unlink(mqname) != 0) {
		perror(ERROR_PREFIX "mq_unlink()");
		return PTS_UNRESOLVED;
	}
	mqdes = mqdes + 1;

	memset(&mqstat, 0, sizeof(mqstat));
	memset(&nmqstat, 0, sizeof(nmqstat));
	nmqstat.mq_flags = MQFLAGS;

	if (mq_setattr(mqdes, &mqstat, NULL) == -1) {
		if (EBADF != errno) {
			printf("errno != EBADF \n");
			failure = 1;
		}
	} else {
		printf("Test FAILED\n");
		failure = 1;
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

	printf("Test PASSED \n");
	return PTS_PASS;
}
