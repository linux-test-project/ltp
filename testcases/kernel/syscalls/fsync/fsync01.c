/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *    AUTHOR		: William Roske
 *    CO-PILOT		: Dave Fenner
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
 *
 */

#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "tst_test.h"

static char fname[255];
static int fd;
#define BUF "davef"

static void verify_fsync(void)
{
	unsigned int i;

	for (i = 0; i < 10; i++) {
		SAFE_WRITE(1, fd, BUF, sizeof(BUF));

		TEST(fsync(fd));

		if (TEST_RETURN == -1)
			tst_res(TFAIL | TTERRNO, "fsync failed");
		else
			tst_res(TPASS, "fsync() returned %ld", TEST_RETURN);
	}
}

static void setup(void)
{
	sprintf(fname, "mntpoint/tfile_%d", getpid());
	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.cleanup = cleanup,
	.setup = setup,
	.test_all = verify_fsync,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = "mntpoint",
	.all_filesystems = 1,
};
