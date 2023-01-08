/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that mq_open() fails with EINVAL if O_CREAT was set in oflag,
 * attr != NULL, and either mq_maxmsg <= 0 or mq_msgsize <= 0.
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
#include "posixtest.h"

#define NAMESIZE 50
#define VALIDVAL 10

#define NUMTESTS 3

static int invalid_values[NUMTESTS] = { 0, -1, INT32_MIN };

int main(void)
{
	char qname[NAMESIZE];
	mqd_t queue;
	struct mq_attr attr;
	int i, failed = 0;

	sprintf(qname, "/mq_open_25-1_%d", getpid());

	attr.mq_msgsize = VALIDVAL;
	for (i = 0; i < NUMTESTS; i++) {
		attr.mq_maxmsg = invalid_values[i];
		queue = mq_open(qname, O_CREAT | O_RDWR,
				S_IRUSR | S_IWUSR, &attr);
		if (queue != (mqd_t) - 1) {
			printf("mq_open() succeeded w/invalid mq_maxmsg %ld\n",
			       attr.mq_maxmsg);
			mq_close(queue);
			mq_unlink(qname);
			failed++;
#ifdef DEBUG
		} else {
			printf("mq_maxmsg %ld failed as expected\n",
			       attr.mq_maxmsg);
#endif
		}

		if (errno != EINVAL) {
			printf("errno(%s) != EINVAL for mq_maxmsg %ld\n",
			       strerror(errno), attr.mq_maxmsg);
			failed++;
#ifdef DEBUG
		} else {
			printf("mq_maxmsg %ld set errno==EINVAL as expected\n",
			       attr.mq_maxmsg);
#endif
		}
	}

	attr.mq_maxmsg = VALIDVAL;
	for (i = 0; i < NUMTESTS; i++) {
		attr.mq_msgsize = invalid_values[i];
		queue = mq_open(qname, O_CREAT | O_RDWR,
				S_IRUSR | S_IWUSR, &attr);
		if (queue != (mqd_t) - 1) {
			printf("mq_open() succeeded w/invalid mq_msgsize %ld\n",
			       attr.mq_msgsize);
			mq_close(queue);
			mq_unlink(qname);
			failed++;
#ifdef DEBUG
		} else {
			printf("mq_msgsize %ld failed as expected\n",
			       attr.mq_msgsize);
#endif
		}

		if (errno != EINVAL) {
			printf("errno(%s) != EINVAL for mq_msgsize %ld\n",
			       strerror(errno), attr.mq_msgsize);
			failed++;
#ifdef DEBUG
		} else {
			printf("mq_msgsize %ld set errno==EINVAL as expected\n",
			       attr.mq_msgsize);
#endif
		}
	}

	if (failed > 0) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
