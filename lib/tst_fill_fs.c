// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/statvfs.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_fs.h"
#include "tst_rand_data.h"

void tst_fill_fs(const char *path, int verbose)
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
