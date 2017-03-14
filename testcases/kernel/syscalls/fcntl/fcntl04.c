/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
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
 /*
  * Basic test for fcntl(2) using F_GETFL argument.
  */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "tst_test.h"

static int fd;
static char fname[255];

static void verify_fcntl(void)
{
	TEST(fcntl(fd, F_GETFL, 0));

	if (TEST_RETURN == -1) {
		tst_res(TFAIL | TTERRNO, "fcntl(%s, F_GETFL, 0) failed",
			fname);
		return;
	}

	if ((TEST_RETURN & O_ACCMODE) != O_RDWR) {
		tst_res(TFAIL, "fcntl(%s, F_GETFL, 0) returned wrong "
			"access mode %li, expected %i", fname,
			TEST_RETURN & O_ACCMODE, O_RDWR);
		return;
	}

	tst_res(TPASS, "fcntl(%s, F_GETFL, 0) returned %lx",
		fname, TEST_RETURN);
}

static void setup(void)
{
	sprintf(fname, "fcntl04_%d", getpid());
	fd = SAFE_OPEN(fname, O_RDWR | O_CREAT, 0700);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test_all = verify_fcntl,
	.setup = setup,
	.cleanup = cleanup,
};
