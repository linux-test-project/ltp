/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that errno == EINVAL if msg_prio is >= MQ_PRIO_MAX or < 0 (<0 N/A
 * for an unsigned, so not tested).
 */

#include <stdio.h>
#include <limits.h>
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
#define NUMINVALID 3

static unsigned invalidpri[NUMINVALID] = {
	MQ_PRIO_MAX, MQ_PRIO_MAX + 1, MQ_PRIO_MAX + 5
};

int main(void)
{
	char qname[NAMESIZE];
	const char *msgptr = MSGSTR;
	struct timespec ts;
	mqd_t queue;
	int unresolved = 0, failure = 0, i;

	sprintf(qname, "/mq_timedsend_13-1_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}

	ts.tv_sec = time(NULL) + 1;
	ts.tv_nsec = 0;
	for (i = 0; i < NUMINVALID; i++) {
		if (mq_timedsend(queue, msgptr,
				 strlen(msgptr), invalidpri[i], &ts) == 0) {
			printf("mq_timedsend() ret success on invalid %d\n",
			       invalidpri[i]);
			failure = 1;
		}
		if (errno != EINVAL) {
			printf("errno not == EINVAL for invalid %d\n",
			       invalidpri[i]);
			failure = 1;
		}
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
