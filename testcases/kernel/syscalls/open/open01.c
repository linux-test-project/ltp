/*
 *   Copyright (c) International Business Machines  Corp., 2001
 *    07/2001 Ported by Wayne Boyer
 *    06/2017 Modified by Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * DESCRIPTION
 *	Open a file with oflag = O_CREAT set, does it set the sticky bit off?
 *	Open a dir with O_DIRECTORY, does it set the S_IFDIR bit on?
 *
 * ALGORITHM
 *	1. open a new file with O_CREAT, fstat.st_mode should not have the
 *	   01000 bit on. In Linux, the save text bit is *NOT* cleared.
 *	2. open a new dir with O_DIRECTORY, fstat.st_mode should have the
 *	   040000 bit on.
 */

#define _GNU_SOURCE		/* for O_DIRECTORY */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "tst_test.h"

#define TEST_FILE	"testfile"
#define TEST_DIR	"testdir"

static int fd;

static struct tcase {
	char *filename;
	int flag;
	mode_t mode;
	unsigned short tst_bit;
	char *desc;
} tcases[] = {
	{TEST_FILE, O_RDWR | O_CREAT, 01444, S_ISVTX, "Sticky bit"},
	{TEST_DIR, O_DIRECTORY, 0, S_IFDIR, "Directory bit"}
};

static void verify_open(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	struct stat buf;

	TEST(open(tc->filename, tc->flag, tc->mode));
	fd = TEST_RETURN;
	if (fd == -1) {
		tst_res(TFAIL, "Cannot open a file");
		return;
	}

	SAFE_FSTAT(fd, &buf);
	if (!(buf.st_mode & tc->tst_bit))
		tst_res(TFAIL, "%s is cleared unexpectedly", tc->desc);
	else
		tst_res(TPASS, "%s is set as expected", tc->desc);

	SAFE_CLOSE(fd);
	if (S_ISREG(buf.st_mode))
		SAFE_UNLINK(tc->filename);
}

static void setup(void)
{
	SAFE_MKDIR(TEST_DIR, 0755);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_open,
};
