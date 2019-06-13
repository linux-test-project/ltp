// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_fs.h"

void tst_fill_fs(const char *path, int verbose)
{
	int i = 0;
	char file[PATH_MAX];
	char buf[4096];
	size_t len;
	ssize_t ret;
	int fd;

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
			ret = write(fd, buf, MIN(len, sizeof(buf)));

			if (ret < 0) {
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
