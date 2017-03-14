/*
 * Copyright (c) International Business Machines Corp., 2001
 *
 * This program is free software;  you can rdistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 */

/*
 * Basic test for pipe().
 */

#include <errno.h>
#include <string.h>
#include "tst_test.h"

static int fds[2];

static void verify_pipe(void)
{
	int rd_size, wr_size;
	char wrbuf[] = "abcdefghijklmnopqrstuvwxyz";
	char rdbuf[128];

	memset(rdbuf, 0, sizeof(rdbuf));

	TEST(pipe(fds));

	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "pipe()");
		return;
	}

	wr_size = SAFE_WRITE(1, fds[1], wrbuf, sizeof(wrbuf));
	rd_size = SAFE_READ(0, fds[0], rdbuf, sizeof(rdbuf));

	if (rd_size != wr_size) {
		tst_res(TFAIL, "read() returned %d, expected %d",
		        rd_size, wr_size);
		return;
	}

	if ((strncmp(rdbuf, wrbuf, wr_size)) != 0) {
		tst_res(TFAIL, "Wrong data were read back");
		return;
	}

	SAFE_CLOSE(fds[0]);
	SAFE_CLOSE(fds[1]);

	tst_res(TPASS, "pipe() functionality is correct");
}

static struct tst_test test = {
	.test_all = verify_pipe,
};
