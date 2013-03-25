/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  geoffrey.r.gustafson REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
  Test that a closed message queue descriptor has been disassociated from
  its message queue by attempting to set a notification on the descriptor
  and verifying that mq_notify returns -1 and sets errno to EBADF
 */
#include <stdio.h>
#include <mqueue.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include "posixtest.h"

#define TEST "4-1"
#define FUNCTION "mq_close"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

int main(void)
{
	char qname[50];
	mqd_t queue;
	struct sigevent se;

	sprintf(qname, "/" FUNCTION "_" TEST "_%d", getpid());

	queue = mq_open(qname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
	if (queue == (mqd_t) - 1) {
		perror(ERROR_PREFIX "mq_open");
		return PTS_UNRESOLVED;
	}

	if (mq_close(queue) == -1) {
		perror(ERROR_PREFIX "mq_close");
		return PTS_UNRESOLVED;
	}

	se.sigev_notify = SIGEV_SIGNAL;
	se.sigev_signo = SIGUSR1;

	if (mq_notify(queue, &se) != -1) {
		printf("mq_notify() did not fail as expected\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (errno != EBADF) {
		printf("errno != EBADF\n");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	if (mq_unlink(qname) != 0) {
		perror("mq_unlink() did not return success");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
