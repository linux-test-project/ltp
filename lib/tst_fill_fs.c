// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/statvfs.h>
#include <sys/uio.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "tst_fs.h"
#include "tst_rand_data.h"
#include "tst_safe_file_at.h"

void fill_random(const char *path, int verbose)
{
	int i = 0;
	char file[PATH_MAX];
	size_t len;
	ssize_t ret;
	int fd;
	struct statvfs fi;

	statvfs(path, &fi);

	for (;;) {
		len = random() % (1024 * 102400);

		snprintf(file, sizeof(file), "%s/file%i", path, i++);

		if (verbose)
			tst_res(TINFO, "Creating file %s size %zu", file, len);

		fd = open(file, O_WRONLY | O_CREAT, 0700);
		if (fd == -1) {
			if (errno != ENOSPC)
				tst_brk(TBROK | TERRNO, "open()");

			tst_res(TINFO | TERRNO, "open()");
			return;
		}

		while (len) {
			ret = write(fd, tst_rand_data, MIN(len, tst_rand_data_len));

			if (ret < 0) {
				/* retry on ENOSPC to make sure filesystem is really full */
				if (errno == ENOSPC && len >= fi.f_bsize/2) {
					SAFE_FSYNC(fd);
					len /= 2;
					continue;
				}

				SAFE_CLOSE(fd);

				if (errno != ENOSPC)
					tst_brk(TBROK | TERRNO, "write()");

				tst_res(TINFO | TERRNO, "write()");
				return;
			}

			len -= ret;
		}

		SAFE_CLOSE(fd);
	}
}

void fill_flat_vec(const char *path, int verbose)
{
	int dir, fd;
	struct iovec iov[512];
	int iovcnt = ARRAY_SIZE(iov);
	int retries = 3;

	dir = open(path, O_PATH | O_DIRECTORY);
	if (dir == -1) {
		if (errno == ENOSPC) {
			tst_res(TINFO | TERRNO, "open()");
			return;
		}
		tst_brk(TBROK | TERRNO, "open(%s, %d) failed", path, O_PATH | O_DIRECTORY);
	}

	fd = openat(dir, "AOF", O_WRONLY | O_CREAT, 0600);
	if (fd == -1) {
		if (errno == ENOSPC) {
			tst_res(TINFO | TERRNO, "openat()");
			return;
		}
		tst_brk(TBROK | TERRNO, "openat(%s, %d, 0600) failed", dir, O_PATH | O_DIRECTORY);
	}

	SAFE_CLOSE(dir);

	for (int i = 0; i < iovcnt; i++) {
		iov[i] = (struct iovec) {
			(void *)tst_rand_data,
			tst_rand_data_len
		};
	}

	while (retries) {
		const int ret = writev(fd, iov, iovcnt);

		if (!ret)
			tst_res(TWARN | TERRNO, "writev returned 0; not sure what this means");

		if (ret > -1) {
			if (verbose && retries < 3)
				tst_res(TINFO, "writev(\"%s/AOF\", iov, %d) = %d", path, iovcnt, ret);

			retries = 3;
			continue;
		}

		if (errno != ENOSPC)
			tst_brk(TBROK | TERRNO, "writev(\"%s/AOF\", iov, %d)", path, iovcnt);

		if (verbose)
			tst_res(TINFO, "writev(\"%s/AOF\", iov, %d): ENOSPC", path, iovcnt);

		retries--;
	}

	SAFE_CLOSE(fd);
}

void tst_fill_fs(const char *path, int verbose, enum tst_fill_access_pattern pattern)
{

	switch (pattern) {
	case TST_FILL_BLOCKS:
		return fill_flat_vec(path, verbose);
	case TST_FILL_RANDOM:
		return fill_random(path, verbose);
	}
}
