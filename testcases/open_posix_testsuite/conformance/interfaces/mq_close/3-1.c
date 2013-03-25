/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  geoffrey.r.gustafson REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
  mq_close test plan:
  1. Create a new message queue
  2. Close the message queue, verify success
  3. Attempt to close the same descriptor again, and verify that mq_close
      returns EBADF
*/

#include <stdio.h>
#include <mqueue.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "posixtest.h"

#define TEST "3-1"
#define FUNCTION "mq_close"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	char qname[50];
	mqd_t queue;

	sprintf(qname, "/" FUNCTION "_" TEST "_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open");
		return PTS_UNRESOLVED;
	}

	if (mq_close(queue) == -1) {
		perror(ERROR_PREFIX "mq_close");
		mq_unlink(qname);
		return PTS_UNRESOLVED;
	}

	if (mq_unlink(qname) != 0) {
		perror("mq_unlink() did not return success");
		return PTS_UNRESOLVED;
	}

	if (mq_close(queue) != -1) {
		printf("mq_close() did not return failure\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	if (errno != EBADF) {
		printf("errno != EBADF\n");
		return PTS_FAIL;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
