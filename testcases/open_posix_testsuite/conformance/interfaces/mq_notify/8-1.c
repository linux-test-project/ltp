/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 *  mq_notify() test plan:
 *  mq_notify() will fail with EBADF if the mqdes argument is not a
 *  valid message queue descriptor.
 *
 *  2/17/2004   call mq_close and mq_unlink before exit to release mq
 *		resources
 */

#include <stdio.h>
#include <errno.h>
#include <mqueue.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "8-1"
#define FUNCTION "mq_notify"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE	50

int main(void)
{
	mqd_t mqdes;
	struct sigevent notification;

	mqdes = (mqd_t) - 1;

	notification.sigev_notify = SIGEV_SIGNAL;
	notification.sigev_signo = SIGUSR1;

	if (mq_notify(mqdes, &notification) == -1) {
		if (EBADF == errno) {
			printf("Test PASSED \n");
			return PTS_PASS;
		} else {
			printf("Test FAILED (errno != EBADF)\n");
			return PTS_FAIL;
		}
	}
	printf("Test FAILED\n");
	return PTS_FAIL;
}
