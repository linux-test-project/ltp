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
 * Test Name: lstat03
 *
 * Test Description:
 *  Verify that, lstat(2) succeeds to get the status of a file pointed to by
 *  symlink and fills the stat structure elements.
 *
 * Expected Result:
 *  lstat() should return value 0 on success and the stat structure elements
 *  should be filled with the symlink file information.
 */
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"

#define FILE_MODE	S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define TESTFILE	"testfile"
#define SFILE		"sfile"
#define FILE_SIZE       1024
#define BUF_SIZE	256
#define PERMS		0644

char *TCID = "lstat03";
int TST_TOTAL = 1;
static uid_t user_id;
static gid_t group_id;

static char nobody_uid[] = "nobody";
static struct passwd *ltpuser;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	struct stat stat_buf;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(lstat(SFILE, &stat_buf));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO, "lstat(%s) failed", SFILE);
			continue;
		}

		if ((stat_buf.st_uid != user_id) ||
		    (stat_buf.st_gid != group_id) ||
		    ((stat_buf.st_mode & S_IFMT) != S_IFLNK) ||
		    (stat_buf.st_size != strlen(TESTFILE))) {
			tst_resm(TFAIL, "Functionality of lstat(2) on "
				 "'%s' Failed", SFILE);
		} else {
			tst_resm(TPASS, "Functionality of lstat(2) on "
				 "'%s' Succcessful", SFILE);
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root();

	ltpuser = SAFE_GETPWNAM(NULL, nobody_uid);
	SAFE_SETUID(NULL, ltpuser->pw_uid);

	TEST_PAUSE;

	tst_tmpdir();

	if (tst_fill_file(TESTFILE, 'a', 1024, 1))
		tst_brkm(TBROK, cleanup, "Failed to create " TESTFILE);

	SAFE_SYMLINK(cleanup, TESTFILE, SFILE);

	user_id = getuid();
	group_id = getgid();
}

static void cleanup(void)
{
	tst_rmdir();
}
