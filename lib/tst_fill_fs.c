/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
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
