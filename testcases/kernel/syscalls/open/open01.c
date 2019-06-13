// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 *    06/2017 Modified by Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
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
	fd = TST_RET;
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
