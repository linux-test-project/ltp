/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.
 */
/*
 * Test Description:
 *   Verify that,
 *   The flag of fchownat() is AT_SYMLINK_NOFOLLOW and the pathname would
 *   not be dereferenced if the pathname is a symbolic link.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "test.h"
#include "safe_macros.h"
#include "fchownat.h"
#include "lapi/fcntl.h"

#define TESTFILE	"testfile"
#define TESTFILE_LINK	"testfile_link"

char *TCID = "fchownat02";
int TST_TOTAL = 1;

static int dir_fd;
static uid_t set_uid = 1000;
static gid_t set_gid = 1000;
static void setup(void);
static void cleanup(void);
static void test_verify(void);
static void fchownat_verify(void);

int main(int ac, char **av)
{
	int lc;
	int i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			fchownat_verify();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	struct stat c_buf, l_buf;

	if ((tst_kvercmp(2, 6, 16)) < 0)
		tst_brkm(TCONF, NULL, "This test needs kernel 2.6.16 or newer");

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	dir_fd = SAFE_OPEN(cleanup, "./", O_DIRECTORY);

	SAFE_TOUCH(cleanup, TESTFILE, 0600, NULL);

	SAFE_SYMLINK(cleanup, TESTFILE, TESTFILE_LINK);

	SAFE_STAT(cleanup, TESTFILE_LINK, &c_buf);

	SAFE_LSTAT(cleanup, TESTFILE_LINK, &l_buf);

	if (l_buf.st_uid == set_uid || l_buf.st_gid == set_gid) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "link_uid(%d) == set_uid(%d) or link_gid(%d) == "
			 "set_gid(%d)", l_buf.st_uid, set_uid, l_buf.st_gid,
			 set_gid);
	}
}

static void fchownat_verify(void)
{
	TEST(fchownat(dir_fd, TESTFILE_LINK, set_uid, set_gid,
		      AT_SYMLINK_NOFOLLOW));

	if (TEST_RETURN != 0) {
		tst_resm(TFAIL | TTERRNO, "fchownat() failed, errno=%d : %s",
			 TEST_ERRNO, strerror(TEST_ERRNO));
	} else {
		test_verify();
	}
}

static void test_verify(void)
{
	struct stat c_buf, l_buf;

	SAFE_STAT(cleanup, TESTFILE_LINK, &c_buf);

	SAFE_LSTAT(cleanup, TESTFILE_LINK, &l_buf);

	if (c_buf.st_uid != set_uid && l_buf.st_uid == set_uid &&
	    c_buf.st_gid != set_gid && l_buf.st_gid == set_gid) {
		tst_resm(TPASS, "fchownat() test AT_SYMLINK_NOFOLLOW success");
	} else {
		tst_resm(TFAIL,
			 "fchownat() test AT_SYMLINK_NOFOLLOW fail with uid=%d "
			 "link_uid=%d set_uid=%d | gid=%d link_gid=%d "
			 "set_gid=%d", c_buf.st_uid, l_buf.st_uid, set_uid,
			 c_buf.st_gid, l_buf.st_gid, set_gid);
	}
}

static void cleanup(void)
{
	tst_rmdir();
}
