/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * Test that a message queue can be opened more than once for read-only
 * in the same process.
 *
 * Note:  Just testing that it can be opened.  No tests that it truly
 * is read-only (that's 7-1.c and 7-2.c)
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
	mqd_t roqueue, roqueue2;

	sprintf(qname, "/mq_open_7-3_%d", getpid());

	roqueue = mq_open(qname, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, NULL);
	if (roqueue == (mqd_t) - 1) {
		perror("mq_open() for read-only queue did not return success");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	roqueue2 = mq_open(qname, O_RDONLY, S_IRUSR | S_IWUSR, NULL);
	if (roqueue2 == (mqd_t) - 1) {
		perror("Second mq_open() for rd-only queue didn't ret success");
		printf("Test FAILED\n");
		return PTS_FAIL;
	}

	mq_close(roqueue);
	mq_close(roqueue2);
	mq_unlink(qname);

	printf("Test PASSED\n");
	return PTS_PASS;
}
