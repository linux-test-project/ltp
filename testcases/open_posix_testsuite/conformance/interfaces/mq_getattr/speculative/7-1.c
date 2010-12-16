/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */
/*
 * mq_getattr() test plan:
 * Test if mqdes argument is not a valid message queue descriptor,
 * mq_getattr() function may fail with EBADF, and the function will
 * return a value of -1.
 * Since this is a "may" assertion either option will be a pass.
 */

#include <stdio.h>
#include <errno.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define TEST "7-1"
#define FUNCTION "mq_getattr"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define NAMESIZE	50

int main()
{
	char mqname[NAMESIZE];
	mqd_t mqdes;
	struct mq_attr mqstat;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
	if (mqdes == (mqd_t)-1) {
		perror(ERROR_PREFIX "mq_open()");
		return PTS_UNRESOLVED;
	}
	mqdes = mqdes + 1;
	memset(&mqstat,0,sizeof(mqstat));
	if (mq_getattr(mqdes, &mqstat) == -1)	{
		if (EBADF != errno) {
			printf("fcn returned -1, but errno != EBADF \n");
			printf("Test FAILED \n");
			return PTS_FAIL;
		}
	} else {
		printf("fcn returned success\n");
	       	return PTS_PASS;
	}
	printf("fcn returned -1 and errno == EBADF\n");
	return PTS_PASS;
}