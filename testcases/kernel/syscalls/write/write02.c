/*
 * Copyright (c) 2017 Carlo Marcelo Arenas Belon <carlo@gmail.com>
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
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
 * Tests for a special case NULL buffer with size 0 is expected to return 0.
 */

#include <errno.h>
#include "tst_test.h"

static int fd;

static void verify_write(void)
{
	TEST(write(fd, NULL, 0));

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO,
			"write() should have succeeded with ret=0");
		return;
	}

	tst_res(TPASS, "write(fd, NULL, 0) == 0");
}

static void setup(void)
{
	fd = SAFE_OPEN("test_file", O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = verify_write,
	.needs_tmpdir = 1,
};
