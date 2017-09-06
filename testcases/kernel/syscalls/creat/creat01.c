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
 * 1. creat() a file using 0444 mode, write to the fildes, write
 *    should return a positive count.
 *
 * 2. creat() should truncate a file to 0 bytes if it already
 *    exists, and should not fail.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "tst_test.h"

static char filename[40];
static int fd;

static void setup(void)
{
	sprintf(filename, "creat01.%d", getpid());
}

static struct tcase {
	int mode;
} tcases[] = {
	{0644},
	{0444}
};

static void verify_creat(unsigned int i)
{
	struct stat buf;
	char c;

	fd = creat(filename, tcases[i].mode);

	if (fd == -1)
		tst_brk(TBROK | TERRNO, "creat() failed");

	SAFE_STAT(filename, &buf);

	if (buf.st_size != 0)
		tst_res(TFAIL, "creat() failed to truncate file to 0 bytes");
	else
		tst_res(TPASS, "creat() truncated file to 0 bytes");

	if (write(fd, "A", 1) != 1)
		tst_res(TFAIL | TERRNO, "write was unsuccessful");
	else
		tst_res(TPASS, "file was created and written to successfully");

	if (read(fd, &c, 1) != -1)
		tst_res(TFAIL, "read succeeded unexpectedly");
	else
		tst_res(TPASS | TERRNO, "read failed expectedly");

	SAFE_CLOSE(fd);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = 2,
	.test = verify_creat,
	.needs_tmpdir = 1,
	.cleanup = cleanup,
	.setup = setup,
};
