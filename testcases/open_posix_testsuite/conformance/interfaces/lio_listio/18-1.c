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
 *	lio_listio() shall fail if:
 *	[EINVAL] mode is not LIO_NOWAIT or LIO_WAIT, or nent is greater than
 *	{AIO_LISTIO_MAX}
 *
 * method:
 *
 *	- open a file
 *	- submit a list of requests to lio_listio usign incorrect mode
 *	- check that the error and return codes are consistent
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

#define TNAME "lio_listio/18-1.c"

#define NUM_AIOCBS	1
#define BUF_SIZE	1024

int main(void)
{
	char tmpfname[256];
	int fd;

	struct aiocb *aiocbs[NUM_AIOCBS];
	char *bufs;
	int ret;

	if (sysconf(_SC_ASYNCHRONOUS_IO) < 200112L)
		exit(PTS_UNSUPPORTED);

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_lio_listio_18_1_%d",
		 getpid());
	unlink(tmpfname);

	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);

	if (fd == -1) {
		printf(TNAME " Error at open(): %s\n", strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	bufs = malloc(NUM_AIOCBS * BUF_SIZE);

	if (bufs == NULL) {
		printf(TNAME " Error at malloc(): %s\n", strerror(errno));
		close(fd);
		exit(PTS_UNRESOLVED);
	}

	aiocbs[0] = malloc(sizeof(struct aiocb));
	memset(aiocbs[0], 0, sizeof(struct aiocb));

	aiocbs[0]->aio_fildes = fd;
	aiocbs[0]->aio_offset = 0;
	aiocbs[0]->aio_buf = bufs;
	aiocbs[0]->aio_nbytes = BUF_SIZE;
	aiocbs[0]->aio_lio_opcode = LIO_WRITE;

	/* Submit request list */
	ret = lio_listio(-1, aiocbs, NUM_AIOCBS, NULL);

	if (ret != -1) {
		printf(TNAME
		       " Error lio_listio() should have returned -1: %d\n",
		       ret);

		free(aiocbs[0]);
		free(bufs);
		close(fd);
		exit(PTS_FAIL);
	}

	if (errno != EINVAL) {
		printf(TNAME
		       " Error lio_listio() should have set errno to EINVAL: %d (%s)\n",
		       errno, strerror(errno));

		free(aiocbs[0]);
		free(bufs);
		close(fd);
		exit(PTS_FAIL);
	}

	free(aiocbs[0]);
	free(bufs);
	close(fd);

	printf(TNAME " PASSED\n");

	return PTS_PASS;
}
