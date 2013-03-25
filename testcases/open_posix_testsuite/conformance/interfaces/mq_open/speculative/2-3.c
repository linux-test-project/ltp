/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that if there are two /s in a message queue name, the behavior
 * is implementation defined.
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

	sprintf(qname, "/tmp/msgqueue_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		printf("This implementation does not appear to support\n");
		printf("message queue names with two /s in them.\n");
		return PTS_PASS;
	}

	mq_close(queue);
	mq_unlink(qname);

	printf("This implementation may support message queue\n");
	printf("names with two /s in them.\n");
	printf("Test PASSED\n");
	return PTS_PASS;
}
