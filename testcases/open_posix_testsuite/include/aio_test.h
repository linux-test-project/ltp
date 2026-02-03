/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 SUSE LLC <mdoucha@suse.cz>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <aio.h>
#include <sys/socket.h>

static int setup_aio(const char *testname, int fd_pair[2], struct aiocb *reqs,
	unsigned int count)
{
	unsigned int i;
	int ret;
	int bufsize;
	socklen_t argsize = sizeof(bufsize);

	/* Open anonymous sockets */
	ret = socketpair(AF_UNIX, SOCK_DGRAM, 0, fd_pair);

	if (ret) {
		printf("%s Error creating sockets(): %s\n", testname,
			strerror(errno));
		return -1;
	}

	ret = getsockopt(fd_pair[0], SOL_SOCKET, SO_SNDBUF, &bufsize, &argsize);
	if (ret) {
		printf("%s Error reading socket buffer size: %s\n", testname,
			strerror(errno));
		close(fd_pair[0]);
		close(fd_pair[1]);
		return -1;
	}

	/* Socket buffer size is twice the maximum message size */
	bufsize /= 2;
	memset(reqs, 0, sizeof(struct aiocb) * count);

	/* Setup basic AIO requests */
	for (i = 0; i < count; i++) {
		reqs[i].aio_fildes = fd_pair[0];
		reqs[i].aio_buf = malloc(bufsize);

		if (!reqs[i].aio_buf) {
			ret = errno;
			break;
		}

		reqs[i].aio_nbytes = bufsize;
		reqs[i].aio_sigevent.sigev_notify = SIGEV_NONE;
		memset((void *)reqs[i].aio_buf, 0xaa, bufsize);
	}

	/* Setup successful */
	if (i >= count)
		return 0;

	/* malloc() failed above */
	for (i = 0; i < count; i++)
		free((void *)reqs[i].aio_buf);

	printf("%s malloc() failed: %s\n", testname, strerror(ret));
	close(fd_pair[0]);
	close(fd_pair[1]);
	return -1;
}

static void cleanup_aio(int fd_pair[2], struct aiocb *reqs, unsigned int count)
{
	unsigned int i;
	int ret;

	for (i = 0; i < count; i++) {
		if (!reqs[i].aio_buf)
			break;

		ret = aio_error(reqs + i);

		/* flush written data from the socket */
		if (ret == 0 || ret == EINPROGRESS) {
			read(fd_pair[1], (void *)reqs[i].aio_buf,
				reqs[i].aio_nbytes);
		}

		free((void *)reqs[i].aio_buf);
	}

	close(fd_pair[0]);
	close(fd_pair[1]);
}
