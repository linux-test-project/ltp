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
 *	LIO_WRITE causes the entry to be submited as if by a call to
 *	aio_write().
 *
 * method:
 *
 *	- open a file
 *	- write data using lio_listio
 *	- read data
 *	- check data is read correctly
 *
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "posixtest.h"

#define TNAME "lio_listio/9-1.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 512
	unsigned char buf[BUF_SIZE];
	unsigned char check[BUF_SIZE];
	int fd;
	struct aiocb aiocb;
	struct aiocb *list[1];
	int err;
	int ret;
	int i;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		exit(PTS_UNSUPPORTED);

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_lio_listio_9_1_%d",
		 getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	for (i = 0; i < BUF_SIZE; i++)
		buf[i] = i;

	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = buf;
	aiocb.aio_nbytes = BUF_SIZE;
	aiocb.aio_lio_opcode = LIO_WRITE;

	list[0] = &aiocb;

	if (lio_listio(LIO_WAIT, list, 1, NULL) == -1) {
		printf(TNAME " Error at lio_listio(): %s\n", strerror(errno));
		exit(PTS_FAIL);
	}

	/* Check return values */
	err = aio_error(&aiocb);
	ret = aio_return(&aiocb);

	if (err != 0) {
		printf(TNAME " Error at aio_error(): %s\n", strerror(errno));

		close(fd);
		exit(PTS_FAIL);
	}

	if (ret != BUF_SIZE) {
		printf(TNAME " Error at aio_return(): %d\n", ret);

		close(fd);
		exit(PTS_FAIL);
	}

	memset(check, 0xaa, BUF_SIZE);

	if (read(fd, check, BUF_SIZE) != BUF_SIZE) {
		printf(TNAME " Error at read(): %s\n", strerror(errno));

		close(fd);
		exit(PTS_UNRESOLVED);
	}

	/* check it */

	for (i = 0; i < BUF_SIZE; i++) {
		if (buf[i] != check[i]) {
			printf(TNAME " read values are corrupted\n");
			exit(PTS_FAIL);
		}
	}

	close(fd);
	printf("Test PASSED\n");
	return PTS_PASS;
}
