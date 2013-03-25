/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if mq_open() is not successful, it will return (mqd_t)-1
 * and set errno to indicate the error.
 *
 * Error case used is when O_CREAT is not set and the named message
 * queue does not exist.
 * Test that errno is set will be part of assertion 29.
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

int main(void)
{
	char qname[NAMESIZE];
	mqd_t queue;

	sprintf(qname, "/mq_open_21-1_%d", getpid());

	queue = mq_open(qname, O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue != (mqd_t) - 1) {
		printf("mq_open() did not return (mqd_t)-1 on error\n");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
