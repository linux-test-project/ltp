/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 *  mq_unlink() test plan:
 *  mq_unlink() will remove the message queue named by the pathname name
 *  and return 0 on success. After the success call to mq_unlink() with name,
 *  a call to mq_open() with name will fail if the flag O_CREAT is not set.
 *
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "mq_unlink"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE	50

int main(void)
{
	char mqname[NAMESIZE];
	mqd_t mqdes;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
	if (mqdes == (mqd_t)-1) {
		perror(ERROR_PREFIX "mq_open");
		return PTS_UNRESOLVED;
	}
	if (mq_unlink(mqname) == 0) {
		if (mq_open(mqname, O_RDWR, S_IRUSR | S_IWUSR, 0) == (mqd_t)-1) {
			printf("Test PASSED\n");
			return PTS_PASS;
		} else {
			printf("mq_open succeed unexpectly \n");
			return PTS_FAIL;
		}
	} else {
		perror(ERROR_PREFIX "mq_unlink");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	return PTS_UNRESOLVED;
}
