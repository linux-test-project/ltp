/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Basic test that O_NONBLOCK can be set.  The functionality of O_NONBLOCK
 * is tested in the mq_send and mq_receive test cases.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define NAMESIZE 50
#define MSGSTR "O123456789"

int main(void)
{
	char qname[NAMESIZE];
	const char *msgptr = MSGSTR;
	mqd_t queue;

	sprintf(qname, "/mq_open_18-1_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR | O_NONBLOCK,
			S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror("mq_open() did not return success w/O_NONBLOCK set");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (mq_send(queue, msgptr, strlen(msgptr), 1) != 0) {
		perror("mq_send() did not return success");
		printf("Test FAILED\n");
		/* close queue and exit */
		mq_close(queue);
		mq_unlink(qname);
		return PTS_FAIL;
	}

	mq_close(queue);
	mq_unlink(qname);

	printf("Test PASSED\n");
	return PTS_PASS;
}
