/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * Copyright (c) 2011 Cyril Hrubis <chrubis@suse.cz>
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */
/*
 * mq_getattr() test plan:
 * Test if mqdes argument is not a valid message queue descriptor,
 * mq_getattr() function may fail with EBADF, and the function will
 * return a value of -1.
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

#define TEST "7-1"
#define FUNCTION "mq_getattr"

#define NAMESIZE 50

int main(void)
{
	char mqname[NAMESIZE];
	mqd_t mqdes, mqdes_invalid;
	struct mq_attr mqstat;
	int ret, saved_errno;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);

	if (mqdes == (mqd_t) - 1) {
		perror("mq_open()");
		return PTS_UNRESOLVED;
	}

	mqdes_invalid = mqdes + 1;
	memset(&mqstat, 0, sizeof(mqstat));

	ret = mq_getattr(mqdes_invalid, &mqstat);
	saved_errno = errno;

	mq_close(mqdes);
	mq_unlink(mqname);

	switch (ret) {
	case 0:
		printf("mq_getattr() returned success\n");
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	case -1:
		break;
	default:
		printf("mq_getattr returned %i\n", ret);
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (saved_errno != EBADF) {
		printf("mq_getattr() returned -1, but errno != EBADF (%s)\n",
		       strerror(saved_errno));
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("mq_getattr() returned -1 and errno == EBADF\n");
	printf("Test PASSED\n");
	return PTS_PASS;
}
