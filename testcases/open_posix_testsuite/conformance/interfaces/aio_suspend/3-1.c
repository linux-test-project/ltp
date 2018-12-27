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
 *	The list array may contain NULL pointers which are ignored.
 *
 * method:
 *
 *	- submit a bunch of aio_write
 *	- prepare suspend list with NULL pointers
 *	- call aio_suspend with this list
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
#include <time.h>
#include <aio.h>

#include "posixtest.h"

#define TNAME "aio_suspend/2-1.c"

#define NENT	8
#define NAIOCB	3

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 1024
	char buf[BUF_SIZE];
	int fd;
	struct aiocb aiocb[NAIOCB];
	const struct aiocb *list[NENT];
	struct timespec processing_completion_ts = {0, 10000000};
	int i;
	int ret;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		return PTS_UNSUPPORTED;

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_suspend_2_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	for (i = 0; i < NAIOCB; i++) {
		memset(&aiocb[i], 0, sizeof(struct aiocb));
		aiocb[i].aio_fildes = fd;
		aiocb[i].aio_buf = buf;
		aiocb[i].aio_offset = i * BUF_SIZE;
		aiocb[i].aio_nbytes = BUF_SIZE;

		if (aio_write(&aiocb[i]) == -1) {
			printf(TNAME " Error at aio_write(): %s\n",
			       strerror(errno));
			exit(PTS_FAIL);
		}
	}

	memset(&list, 0, sizeof(list));
	list[2] = &aiocb[0];
	list[5] = &aiocb[1];
	list[6] = &aiocb[2];

	if (aio_suspend(list, NENT, NULL) != 0) {
		printf(TNAME " Error at aio_suspend(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	for (i = 0; i < NAIOCB; ++i) {
		do {
			nanosleep(&processing_completion_ts, NULL);
			ret = aio_error(&aiocb[i]);
		} while (ret == EINPROGRESS);
		if (aio_return(&aiocb[i]) == -1) {
			printf(TNAME " Error at aio_return(): %s\n",
			       strerror(errno));
			exit(PTS_FAIL);
		}
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
