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
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 */

#include <errno.h>
#include "tst_test.h"

#define SIZE 512

static int fd;
static char buf[SIZE];

static void verify_read(void)
{
	SAFE_LSEEK(fd, 0, SEEK_SET);

	TEST(read(fd, buf, SIZE));

	if (TEST_RETURN == -1)
		tst_res(TFAIL | TTERRNO, "read(2) failed");
	else
		tst_res(TPASS, "read(2) returned %ld", TEST_RETURN);
}

static void setup(void)
{
	memset(buf, '*', SIZE);
	fd = SAFE_OPEN("testfile", O_RDWR | O_CREAT, 0700);
	SAFE_WRITE(1, fd, buf, SIZE);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_read,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
