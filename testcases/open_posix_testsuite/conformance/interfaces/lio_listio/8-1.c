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
 *	LIO_READ causes the entry to be submited as if by a call to aio_read().
 *
 * method:
 *
 *	- open a file
 *	- write data
 *	- submit read request to lio_listio
 *	- check data is read correctly
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

#define TNAME "lio_listio/8-1.c"

int main(void)
{
	char tmpfname[256];
#define BUF_SIZE 512
	unsigned char buf[BUF_SIZE];
	unsigned char check[BUF_SIZE];
	int fd;
	struct aiocb aiocb;
	struct aiocb *list[1];
	int i;
	int err;
	int ret;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		exit(PTS_UNSUPPORTED);

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_lio_listio_8_1_%d",
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

	if (write(fd, buf, BUF_SIZE) != BUF_SIZE) {
		printf(TNAME " Error at write(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	memset(check, 0xaa, BUF_SIZE);
	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = check;
	aiocb.aio_nbytes = BUF_SIZE;
	aiocb.aio_lio_opcode = LIO_READ;

	list[0] = &aiocb;

	if (lio_listio(LIO_WAIT, list, 1, NULL) == -1) {
		printf(TNAME " Error at lio_listio(): %s\n", strerror(errno));

		close(fd);
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
