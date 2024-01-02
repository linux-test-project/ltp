// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2014-2024
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 * Author: Stanislav Kholmanskikh <stanislav.kholmanskikh@oracle.com>
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <fcntl.h>
#include "lapi/fallocate.h"

int tst_fill_fd(int fd, char pattern, size_t bs, size_t bcount)
{
	size_t i;
	char *buf;

	/* Filling a memory buffer with provided pattern */
	buf = malloc(bs);
	if (buf == NULL)
		return -1;

	for (i = 0; i < bs; i++)
		buf[i] = pattern;

	/* Filling the file */
	for (i = 0; i < bcount; i++) {
		if (write(fd, buf, bs) != (ssize_t)bs) {
			free(buf);
			return -1;
		}
	}

	free(buf);

	return 0;
}

int tst_prealloc_size_fd(int fd, size_t bs, size_t bcount)
{
	int ret;

	errno = 0;
	ret = fallocate(fd, 0, 0, bs * bcount);

	if (ret && errno == ENOSPC)
		return ret;

	if (ret)
		ret = tst_fill_fd(fd, 0, bs, bcount);

	return ret;
}

int tst_fill_file(const char *path, char pattern, size_t bs, size_t bcount)
{
	int fd;

	fd = open(path, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
	if (fd < 0)
		return -1;

	if (tst_fill_fd(fd, pattern, bs, bcount)) {
		close(fd);
		unlink(path);
		return -1;
	}

	if (close(fd) < 0) {
		unlink(path);

		return -1;
	}

	return 0;
}

int tst_prealloc_file(const char *path, size_t bs, size_t bcount)
{
	int fd;

	fd = open(path, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
	if (fd < 0)
		return -1;

	if (tst_prealloc_size_fd(fd, bs, bcount)) {
		close(fd);
		unlink(path);
		return -1;
	}

	if (close(fd) < 0) {
		unlink(path);
		return -1;
	}

	return 0;
}
