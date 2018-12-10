/*
 * Copyright (c) 2004, Bull SA. All rights reserved.
 * Created by:  Laurent.Vivier@bull.net
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * assertion:
 *
 *	aio_error() may fail if:
 *	[EINVAL] The aiocbp argument does not refer to an asynchronous
 *	operation whose return status has not yet been retrieved.
 *
 * method:
 *
 *	call aio_error() with an invalid aiocbp
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <aio.h>

#include "posixtest.h"

#define TNAME "aio_error/3-1.c"

int main(void)
{

	char tmpfname[256];
#define BUF_SIZE 512
	char buf[BUF_SIZE];
	int fd;
	struct aiocb aiocb;
	int ret = 0;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_error_3_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	memset(&aiocb, 0, sizeof(struct aiocb));

	aiocb.aio_fildes = fd;
	aiocb.aio_buf = buf;
	aiocb.aio_reqprio = -1;
	aiocb.aio_nbytes = BUF_SIZE;

	ret = aio_error(&aiocb);

	if (ret != EINVAL) {
		printf(TNAME " return code didn't match expected "
		       "value (%d != %d).\n", ret, EINVAL);
		return PTS_UNTESTED;
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
