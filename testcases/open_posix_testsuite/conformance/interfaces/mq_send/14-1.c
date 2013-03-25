/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that EMSGSIZE is returned if msg_len is not <= mq_attr->mq_msgsize.
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
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "01234567890123456789"
#define NUMINVALID 3

static long messagesize[NUMINVALID] = {
	19, 2, 1
};

int main(void)
{
	char qname[NAMESIZE];
	const char *msgptr = MSGSTR;
	mqd_t queue;
	int unresolved = 0, failure = 0, i;
	struct mq_attr attr;

	sprintf(qname, "/mq_send_14-1_%d", getpid());

	for (i = 0; i < NUMINVALID; i++) {
		attr.mq_msgsize = messagesize[i];
		attr.mq_maxmsg = messagesize[i];

		queue =
		    mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
		if (queue == (mqd_t) - 1) {
			perror("mq_open() did not return success");
			return PTS_UNRESOLVED;
		}

		if (mq_send(queue, msgptr, strlen(msgptr), 1) != -1) {
			printf("mq_send() did not return -1 for EMSGSIZE\n");
			failure = 1;
		}

		if (errno != EMSGSIZE) {
			printf("errno != EMSGSIZE\n");
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
