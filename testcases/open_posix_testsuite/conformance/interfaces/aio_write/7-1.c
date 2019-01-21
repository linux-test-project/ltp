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
 *	aio_write() shall fail with [EAGAIN] if:
 *	The requested AIO operation was not queued to the system
 *	due to system resource limitations.
 *
 * method:
 *
 *	- open file
 *	- queue NUM_AIOCBS 512-byte aio_write
 *	- wait until one returns EAGAIN
 *
 *	NUM_AIOCBS might need to be adjusted for the system
 *
 */

#include <sys/stat.h>
#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define TNAME "aio_write/7-1.c"

#define NUM_AIOCBS 1024

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 512
	char buf[BUF_SIZE];
	int fd;
	int i;
	struct aiocb aiocbs[NUM_AIOCBS];
	int last_req;
	int err;
	int ret;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L
	    || sysconf(_SC_AIO_MAX) == -1)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_write_4_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	for (i = 0; i < NUM_AIOCBS; i++) {
		memset(&aiocbs[i], 0, sizeof(struct aiocb));
		aiocbs[i].aio_fildes = fd;
		aiocbs[i].aio_buf = buf;
		aiocbs[i].aio_nbytes = BUF_SIZE;

		last_req = i + 1;

		ret = aio_write(&aiocbs[i]);
		if (ret == -1)
			break;
	}

	for (i = 0; i < last_req - 1; i++) {
		err = aio_error(&aiocbs[i]);
		ret = aio_return(&aiocbs[i]);

	}

	if (last_req == NUM_AIOCBS) {
		printf(TNAME " Could not fail queuing %d request\n",
		       NUM_AIOCBS);
		close(fd);
		exit(PTS_UNRESOLVED);
	}

	printf("Failed at %d\n", last_req);

	/*
	 * XXX (ngie): this check seems incorrect, given the
	 *             requirements/description for the API and test,
	 *             respectively.
	 */
	if (ret != -1 && errno != EAGAIN) {
		printf(TNAME " failed with code %d: %s\n", errno,
		       strerror(errno));
		close(fd);
		exit(PTS_FAIL);
	}

	printf(TNAME " PASSED\n");

	return PTS_PASS;
}
