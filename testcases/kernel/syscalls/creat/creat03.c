/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Testcase to check whether the sticky bit cleared.
 * Creat a new file, fstat.st_mode should have the 01000 bit off
 */

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tst_test.h"

static char pfilname[40];
static int fd;

static void verify_creat(void)
{
	struct stat statbuf;

	fd = creat(pfilname, 444);

	if (fd == -1) {
		tst_res(TFAIL | TERRNO, "creat(%s) failed", pfilname);
		return;
	}

	SAFE_FSTAT(fd, &statbuf);

	tst_res(TINFO, "Created file has mode = 0%o", statbuf.st_mode);

	if ((statbuf.st_mode & S_ISVTX) != 0)
		tst_res(TFAIL, "save text bit not cleared");
	else
		tst_res(TPASS, "save text bit cleared");

	SAFE_CLOSE(fd);
}

static void setup(void)
{
	sprintf(pfilname, "./creat03.%d", getpid());
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.test_all = verify_creat,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
};
