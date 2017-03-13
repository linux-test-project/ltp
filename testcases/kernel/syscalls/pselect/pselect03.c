/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
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
 /*
  * This is basic test for pselect() returning without error.
  */
#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "tst_test.h"

static int fd;

static void verify_pselect(void)
{
	fd_set readfds;
	struct timespec tv = {0};

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	TEST(pselect(fd, &readfds, 0, 0, &tv, NULL));
	if (TEST_RETURN >= 0)
		tst_res(TPASS, "pselect() succeeded retval=%li", TEST_RETURN);
	else
		tst_res(TFAIL | TTERRNO, "pselect() failed unexpectedly");
}

static void setup(void)
{
	fd = SAFE_OPEN("pselect03_file", O_CREAT | O_RDWR, 0777);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tid = "pselect03",
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_pselect,
};
