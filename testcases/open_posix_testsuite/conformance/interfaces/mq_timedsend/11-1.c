/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that mq_timedsend() returns errno == EBADF if mqdes is not a valid
 * message queue descriptor.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"

int main(void)
{
	char qname[NAMESIZE];
	const char *msgptr = MSGSTR;
	struct timespec ts;
	mqd_t queue;
	int unresolved = 0, failure = 0;

	sprintf(qname, "/mq_timedsend_11-1_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}

	ts.tv_sec = time(NULL) + 1;
	ts.tv_nsec = 0;
	if (mq_timedsend(queue + 1, msgptr, strlen(msgptr), 1, &ts) != -1) {
		printf("mq_timedsend() did not return -1 on invalid queue\n");
		failure = 1;
	}

	if (errno != EBADF) {
		printf("errno != EBADF\n");
		failure = 1;
	}

	if (mq_close(queue) != 0) {
		perror("mq_close() did not return success");
		unresolved = 1;
	}

	if (mq_unlink(qname) != 0) {
		perror("mq_unlink() did not return success");
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
