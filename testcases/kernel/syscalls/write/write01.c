/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2017 Fujitsu Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "tst_test.h"

static int fd;

static void verify_write(void)
{
	int i, badcount = 0;
	char buf[BUFSIZ];

	memset(buf, 'w', BUFSIZ);

	SAFE_LSEEK(fd, 0, SEEK_SET);

	for (i = BUFSIZ; i > 0; i--) {
		TEST(write(fd, buf, i));
		if (TEST_RETURN == -1) {
			tst_res(TFAIL | TTERRNO, "write failed");
			return;
		}

		if (TEST_RETURN != i) {
			badcount++;
			tst_res(TINFO, "write() returned %ld, expected %d",
				TEST_RETURN, i);
		}
	}

	if (badcount != 0)
		tst_res(TFAIL, "write() failed to return proper count");
	else
		tst_res(TPASS, "write() passed");
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
