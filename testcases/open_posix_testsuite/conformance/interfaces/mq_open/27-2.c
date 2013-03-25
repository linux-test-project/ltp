/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that mq_open() fails with ENAMETOOLONG if a component of the
 * name is greater than NAME_MAX.
 *
 * Since a component == the full name, this test will be identical to
 * 27-1.c for NAME_MAX.
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "posixtest.h"

int main(void)
{
	char qname[NAME_MAX * 2];
	mqd_t queue;
	int i;

	sprintf(qname, "/mq_open_27-1_%d", getpid());

	/* Ensures queue name will have > NAME_MAX chars */
	for (i = 0; i < NAME_MAX; i++)
		strcat(qname, "0");

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue != (mqd_t) - 1) {
		printf("mq_open() should have failed with queue name %s\n",
		       qname);
		printf("Test FAILED\n");
		mq_close(queue);
		mq_unlink(qname);
		return PTS_FAIL;
	}
#ifdef DEBUG
	printf("mq_open() failed as expected\n");
#endif

	if (errno != ENAMETOOLONG) {
		printf("errno != ENAMETOOLONG\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
#ifdef DEBUG
	printf("errno == ENAMETOOLONG\n");
#endif

	printf("Test PASSED\n");
	return PTS_PASS;
}
