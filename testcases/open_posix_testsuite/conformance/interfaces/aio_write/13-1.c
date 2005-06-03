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
 *	aio_write() shall fail or the error status of the operation shall be [EFBIG]:
 *	if aio_nbytes is greater than 0 and aio_offset is at or beyond the
 *	offset maximum of aio_fildes.
 *
 * method:
 *
 *	- open file for writing
 *	- drop RLIMIT_FSIZE to 512
 *	- write 1024 bytes at offset 512 using aio_write
 *	- check error code is EFBIG
 *
 */

#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <aio.h>

#include "posixtest.h"

#define TNAME "aio_write/13-1.c"

int got_sigxfsz = 0;

void
sigxfsz_handler(int signum, siginfo_t *info, void *context)
{
	got_sigxfsz = 1;
}

int main()
{
	char tmpfname[256];
#define BUF_SIZE 1024
	char buf[BUF_SIZE];
	int fd;
	struct aiocb aiocb;
	struct rlimit limit;
	struct sigaction action;

#if _POSIX_ASYNCHRONOUS_IO != 200112L
	exit(PTS_UNSUPPORTED);
#endif

	/* Set file size soft limit to 512 */
	getrlimit(RLIMIT_FSIZE , &limit);

	limit.rlim_cur = BUF_SIZE / 2;

	if (setrlimit(RLIMIT_FSIZE , &limit) == -1)
	{
		printf(TNAME " Error at setlimit(): %s\n",
		       strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_aio_write_13_1_%d", 
		  getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL,
		  S_IRUSR | S_IWUSR);
	if (fd == -1)
	{
		printf(TNAME " Error at open(): %s\n",
		       strerror(errno));
		exit(PTS_UNRESOLVED);
	}

	unlink(tmpfname);

	/* Setup handler for SIGXFSZ */
	action.sa_sigaction = sigxfsz_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_SIGINFO|SA_RESTART;
	sigaction(SIGXFSZ, &action, NULL);

	memset(buf, 0xaa, BUF_SIZE);
	memset(&aiocb, 0, sizeof(struct aiocb));
	aiocb.aio_fildes = fd;
	aiocb.aio_buf = buf;
	aiocb.aio_nbytes = BUF_SIZE;
	aiocb.aio_offset = 512;

	if (aio_write(&aiocb) != -1)
	{
		while (aio_error (&aiocb) == EINPROGRESS);

		int err = aio_error (&aiocb);
		int ret = aio_return (&aiocb);

		if (ret != -1) {
			printf(TNAME " bad aio_write return value\n");
			close (fd);
			exit(PTS_FAIL);
		} else if (err != EFBIG) {
			printf(TNAME " error code is not EFBIG %s\n", strerror(errno));
			close (fd);
			exit(PTS_FAIL);
		}
	}  else {

		if (errno != EFBIG)
		{
			printf(TNAME " errno is not EFBIG %s\n", strerror(errno));
			close(fd);
			exit(PTS_FAIL);
		}
	}

	close(fd);
	printf ("Test PASSED\n");
	return PTS_PASS;
}
