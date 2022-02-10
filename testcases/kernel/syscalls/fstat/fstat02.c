// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 *  05/2019 Ported to new library: Christian Amann <camann@suse.com>
 */
/*
 * Tests if fstat() returns correctly and reports correct file information
 * using the stat structure.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

#define TESTFILE        "test_file"
#define LINK_TESTFILE   "link_test_file"
#define FILE_SIZE       1024
#define FILE_MODE	0644
#define NLINK	        2

static struct stat stat_buf;
static uid_t user_id;
static gid_t group_id;
static int fildes;

static void run(void)
{
	int fail = 0;

	TEST(fstat(fildes, &stat_buf));

	if (TST_RET != 0) {
		tst_res(TFAIL | TTERRNO, "fstat() failed");
		return;
	}

	fail = 0;
	if (stat_buf.st_uid != user_id) {
		tst_res(TFAIL, "stat_buf.st_uid = %i expected %i",
			stat_buf.st_uid, user_id);
		fail++;
	}

	if (stat_buf.st_gid != group_id) {
		tst_res(TFAIL, "stat_buf.st_gid = %i expected %i",
			stat_buf.st_gid, group_id);
		fail++;
	}

	if (stat_buf.st_size != FILE_SIZE) {
		tst_res(TFAIL, "stat_buf.st_size = %li expected %i",
			(long)stat_buf.st_size, FILE_SIZE);
		fail++;
	}

	if ((stat_buf.st_mode & 0777) != FILE_MODE) {
		tst_res(TFAIL, "stat_buf.st_mode = %o expected %o",
			(stat_buf.st_mode & 0777), FILE_MODE);
		fail++;
	}

	if (stat_buf.st_nlink != NLINK) {
		tst_res(TFAIL, "stat_buf.st_nlink = %li expected %i",
			(long)stat_buf.st_nlink, NLINK);
		fail++;
	}

	if (fail)
		return;

	tst_res(TPASS, "fstat() reported correct values.");
}

static void setup(void)
{
	user_id  = getuid();
	group_id = getgid();

	umask(0);

	fildes = SAFE_OPEN(TESTFILE, O_WRONLY | O_CREAT, FILE_MODE);

	if (tst_fill_file(TESTFILE, 'a', FILE_SIZE, 1))
		tst_brk(TBROK, "Could not fill Testfile!");

	SAFE_LINK(TESTFILE, LINK_TESTFILE);
}

static void cleanup(void)
{
	if (fildes > 0)
		SAFE_CLOSE(fildes);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
