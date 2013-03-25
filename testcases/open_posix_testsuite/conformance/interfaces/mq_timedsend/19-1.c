/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that mq_timedsend() will return EINVAL if the message queue is
 * full and abs_timeout has a tv_nsec < 0 or >= 1000 million.
 *
 * Fill message queue until full and then call mq_timedsend with invalid
 * abs_timeout values.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "0123456789"
#define NUMTESTS 5
#define BUFFER 40
#define MAXMSG 5

int invalid_tests[NUMTESTS] = { -1, INT32_MIN, 1000000000, 1000000001,
	INT32_MAX
};

int main(void)
{
	char qname[NAMESIZE];
	char *msgptr = MSGSTR;
	struct timespec ts;
	mqd_t queue;
	struct mq_attr attr;
	int failure = 0, i, maxreached = 0;

	sprintf(qname, "/mq_timedsend_19-1_%d", getpid());

	attr.mq_maxmsg = MAXMSG;
	attr.mq_msgsize = BUFFER;
	queue = mq_open(qname, O_CREAT | O_RDWR | O_NONBLOCK,
			S_IRUSR | S_IWUSR, &attr);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	ts.tv_sec = time(NULL);
	ts.tv_nsec = 0;
	for (i = 0; i < MAXMSG + 1; i++) {
		ts.tv_sec++;
		if (mq_timedsend(queue, msgptr, strlen(msgptr), 1, &ts)
		    == -1) {
			maxreached = 1;
			break;
		}
	}

	if (maxreached == 0) {
		printf("Test UNRESOLVED:  Couldn't fill message queue\n");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}

	/*
	 * Message queue is full -- call mq_timedsend() with invalid
	 * timespec values
	 * First, open message queue as blocking
	 */
	mq_close(queue);
	queue = mq_open(qname, O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	for (i = 0; i < NUMTESTS; i++) {
		ts.tv_nsec = invalid_tests[i];
		if (mq_timedsend(queue, msgptr, strlen(msgptr), 1, &ts) != -1) {
			printf("mq_timedsend() didn't fail w/invalid ts\n");
			printf("ts.tv_nsec = %ld\n", ts.tv_nsec);
			failure = 1;
		} else {
			if (errno != EINVAL) {
				printf("errno != EINVAL\n");
				printf("ts.tv_nsec = %ld\n", ts.tv_nsec);
				failure = 1;
			}
		}
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
