/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */
/*
 * mq_setattr() test plan:
 * If omqstat is non_NULL, the mq_setattr() function will store, in the
 * location referenced by omqstat, the previous message queue attributes and
 * the current queue status. These values will be the same as would be
 * returned by a call to mq_getattr() at that point.
 *
 *  2/17/2004   call mq_close and mq_unlink before exit to release mq
 *		resources
 */

#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "posixtest.h"

#define TEST "2-1"
#define FUNCTION "mq_setattr"
#define NAMESIZE	50
#define MQFLAGS		0
#define MQMAXMSG	666
#define MQMSGSIZE	777
#define MQCURMSGS	555

int main(void)
{
	char mqname[NAMESIZE];
	mqd_t mqdes;
	struct mq_attr omqstat, mqstat, nmqstat;
	int unresolved = 0;
	int failure = 0;

	sprintf(mqname, "/" FUNCTION "_" TEST "_%d", getpid());

	mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, 0);
	if (mqdes == (mqd_t) - 1) {
		perror("mq_open() did not return success");
		return PTS_UNRESOLVED;
	}
	memset(&mqstat, 0, sizeof(mqstat));
	memset(&omqstat, 0, sizeof(omqstat));
	memset(&nmqstat, 0, sizeof(nmqstat));

	if (mq_getattr(mqdes, &omqstat) == -1) {
		perror("mq_getattr() did not return success");
		unresolved = 1;
	}
	if (mq_unlink(mqname) != 0) {
		perror("mq_unlink()");
		return PTS_UNRESOLVED;
	}
	nmqstat.mq_flags = MQFLAGS;
	nmqstat.mq_maxmsg = MQMAXMSG;
	nmqstat.mq_msgsize = MQMSGSIZE;
	nmqstat.mq_curmsgs = MQCURMSGS;

	if (mq_setattr(mqdes, &nmqstat, &mqstat) != 0) {
		failure = 1;
	}
	if ((omqstat.mq_flags != mqstat.mq_flags) ||
	    (omqstat.mq_maxmsg != mqstat.mq_maxmsg) ||
	    (omqstat.mq_msgsize != mqstat.mq_msgsize) ||
	    (omqstat.mq_curmsgs != mqstat.mq_curmsgs)) {
		failure = 1;
	}

	mq_close(mqdes);
	mq_unlink(mqname);

	if (failure == 1) {
		printf("Test FAILED\n");
		return PTS_FAIL;
	}
	if (unresolved == 1) {
		printf("Test UNRESOLVED\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED \n");
	return PTS_PASS;
}
