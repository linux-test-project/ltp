/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2014 Cyril Hrubis <chrubis@suse.cz>
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
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test Description:
 *  Verify that, when fchown(2) invoked by super-user to change the owner and
 *  group of a file specified by file descriptor to any numeric
 *  owner(uid)/group(gid) values,
 *	- clears setuid and setgid bits set on an executable file.
 *	- preserves setgid bit set on a non-group-executable file.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "test.h"
#include "safe_macros.h"
#include "compat_16.h"

#define FILE_MODE	S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define NEW_PERMS1	S_IFREG | S_IRWXU | S_IRWXG | S_ISUID | S_ISGID
#define NEW_PERMS2	S_IFREG | S_IRWXU | S_ISGID
#define EXP_PERMS	S_IFREG | S_IRWXU | S_IRWXG
#define TESTFILE1	"testfile1"
#define TESTFILE2	"testfile2"

TCID_DEFINE(fchown02);
int TST_TOTAL = 2;
static int fd1;
static int fd2;

struct test_case {
	int *fd;
	char *pathname;
	char *desc;
	uid_t user_id;
	gid_t group_id;
	int test_flag;
} tc[] = {
	{&fd1, TESTFILE1, "Setuid/Setgid bits cleared", 700, 701, 1},
	{&fd2, TESTFILE2, "Setgid bit not cleared", 700, 701, 2},
	{0, NULL, NULL, 0, 0, 0}
};

static void setup(void);
static void cleanup(void);

static void verify_fchown(struct test_case *t)
{
	struct stat stat_buf;

	TEST(FCHOWN(cleanup, *t->fd, t->user_id, t->group_id));

	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO, "fchown() Fails on %s", t->pathname);
		return;
	}

	SAFE_FSTAT(cleanup, *t->fd, &stat_buf);

	if ((stat_buf.st_uid != t->user_id) ||
	    (stat_buf.st_gid != t->group_id)) {
		tst_resm(TFAIL,
		         "%s: Incorrect ownership expected %d %d, have %d %d",
		         t->pathname, t->user_id, t->group_id,
		         stat_buf.st_uid, stat_buf.st_gid);
	}

	switch (t->test_flag) {
	case 1:
		if (((stat_buf.st_mode & (S_ISUID | S_ISGID)))) {
			tst_resm(TFAIL, "%s: Incorrect mode "
					"permissions %#o, Expected "
					"%#o", t->pathname, NEW_PERMS1,
					 EXP_PERMS);
			return;
		}
	break;
	case 2:
		if ((!(stat_buf.st_mode & S_ISGID))) {
			tst_resm(TFAIL,
				 "%s: Incorrect mode "
				 "permissions %#o, Expected "
				 "%#o", t->pathname,
				 stat_buf.st_mode, NEW_PERMS2);
			return;
		}
	break;
	}

	tst_resm(TPASS, "fchown() on %s succeeds : %s", t->pathname, t->desc);
}

int main(int ac, char **av)
{
	int lc, i;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		for (i = 0; tc[i].desc != NULL; i++)
			verify_fchown(tc + i);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	fd1 = SAFE_OPEN(cleanup, TESTFILE1, O_RDWR | O_CREAT, FILE_MODE);
	SAFE_CHMOD(cleanup, TESTFILE1, NEW_PERMS1);
	fd2 = SAFE_OPEN(cleanup, TESTFILE2, O_RDWR | O_CREAT, FILE_MODE);
	SAFE_CHMOD(cleanup, TESTFILE2, NEW_PERMS2);
}

static void cleanup(void)
{
	if (fd1 > 0 && close(fd1))
		tst_resm(TWARN | TERRNO, "Failed to close fd1");

	if (fd2 > 0 && close(fd2))
		tst_resm(TWARN | TERRNO, "Failed to close fd2");

	tst_rmdir();
}
