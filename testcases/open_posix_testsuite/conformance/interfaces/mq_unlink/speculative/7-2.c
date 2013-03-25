/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 *  mq_unlink() test plan:
 *  mq_unlink() fails with ENOENT, if the named message queue does not
 *  exist.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "7-2"
#define FUNCTION "mq_unlink"

int main(void)
{
	char mqname[50] = "/123";

//      sprintf(mqname, FUNCTION "_" TEST "_%d", getpid());

	mq_unlink(mqname);
	if (ENOENT == errno) {
		printf("Test PASSED\n");
		return PTS_PASS;
	} else {
		printf("Test FAILED, error is %s\n", strerror(errno));
		return PTS_FAIL;
	}
}
