/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that mq_open() fails with EMFILE if there are too many message
 * queue descriptors or file descriptors in use by the process calling mq_open.
 *
 * Test by calling mq_open() _POSIX_OPEN_MAX+1 or _POSIX_MQ_OPEN_MAX+1 times.
 * If it fails, assume it was because there were too many message queue
 * descriptors or file descriptors open and check that errno == EMFILE.
 *
 * If it does not fail, print that test results are inconclusive and
 * return PTS_PASS.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include "posixtest.h"

#define NAMESIZE 50

int main(void)
{
#ifndef _POSIX_OPEN_MAX
	printf("_POSIX_OPEN_MAX not defined as expected\n");
	return PTS_UNRESOLVED;
#else
#ifndef _POSIX_MQ_OPEN_MAX
	printf("_POSIX_MQ_OPEN_MAX not defined as expected\n");
	return PTS_UNRESOLVED;
#else
	char qname[NAMESIZE];
	mqd_t queue[_POSIX_OPEN_MAX + _POSIX_MQ_OPEN_MAX + 1];
	int i, unresolved = 0, failure = 0, numqueues = 0;

	for (i = 0; (i < _POSIX_OPEN_MAX) && (i < _POSIX_MQ_OPEN_MAX);
	     i++, numqueues++) {
		sprintf(qname, "/msgqueue%d_%d", i, getpid());

		queue[i] = mq_open(qname, O_CREAT | O_RDWR,
				   S_IRUSR | S_IWUSR, NULL);
		if (queue[i] == (mqd_t) - 1) {
			printf("mq_open() failed before expected\n");
			unresolved = 1;
			break;
		}
	}

	queue[numqueues] = mq_open(qname, O_CREAT | O_RDWR,
				   S_IRUSR | S_IWUSR, NULL);
	if (queue[numqueues] != (mqd_t) - 1) {
		printf("mq_open() did not fail on > ");
		printf("_POSIX_OPEN_MAX  or _POSIX_MQ_OPEN_MAX queues\n");
	} else {
		if (errno != EMFILE) {
			printf("errno != EMFILE on > ");
			printf
			    ("_POSIX_OPEN_MAX  or _POSIX_MQ_OPEN_MAX queues\n");
			failure = 1;
		}
	}

	for (i = 0; i <= numqueues; i++)
		mq_close(queue[i]);

	for (i = 0; i <= numqueues; i++) {
		sprintf(qname, "/msgqueue%d_%d", i, getpid());
		mq_unlink(qname);
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
#endif
#endif
}
