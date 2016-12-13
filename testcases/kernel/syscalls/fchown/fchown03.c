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
 * Test Description:
 *  Verify that, fchown(2) succeeds to change the group of a file specified
 *  by path when called by non-root user with the following constraints,
 *	- euid of the process is equal to the owner of the file.
 *	- the intended gid is either egid, or one of the supplementary gids
 *	  of the process.
 *  Also, verify that fchown() clears the setuid/setgid bits set on the file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <grp.h>
#include <pwd.h>

#include "test.h"
#include "safe_macros.h"
#include "compat_16.h"

#define FILE_MODE (mode_t)(S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define NEW_PERMS (mode_t)(S_IFREG | S_IRWXU | S_IRWXG | S_ISUID | S_ISGID)
#define FCHOWN_PERMS	(mode_t)(NEW_PERMS & ~(S_ISUID | S_ISGID))
#define TESTFILE	"testfile"

TCID_DEFINE(fchown03);
int TST_TOTAL = 1;

static int fildes;
char nobody_uid[] = "nobody";
static struct passwd *ltpuser;

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	struct stat stat_buf;
	int lc;
	uid_t user_id;
	gid_t group_id;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		user_id = geteuid();
		GID16_CHECK((group_id = getegid()), "fchown", cleanup)

		TEST(FCHOWN(cleanup, fildes, -1, group_id));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "fchown() on %s Fails, errno=%d",
				 TESTFILE, TEST_ERRNO);
			continue;
		}

		SAFE_FSTAT(cleanup, fildes, &stat_buf);

		if ((stat_buf.st_uid != user_id) ||
		    (stat_buf.st_gid != group_id)) {
			tst_resm(TFAIL, "%s: Incorrect "
				 "ownership set, Expected %d %d",
				 TESTFILE, user_id, group_id);
			continue;
		}

		if (stat_buf.st_mode != FCHOWN_PERMS) {
			tst_resm(TFAIL, "%s: Incorrect mode permissions"
				 " %#o, Expected %#o", TESTFILE,
				 stat_buf.st_mode, FCHOWN_PERMS);
		} else {
			tst_resm(TPASS, "fchown() on %s succeeds: "
				 "Setuid/gid bits cleared", TESTFILE);
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_require_root();

	ltpuser = SAFE_GETPWNAM(cleanup, nobody_uid);
	SAFE_SETEUID(NULL, ltpuser->pw_uid);

	TEST_PAUSE;

	tst_tmpdir();

	fildes = SAFE_OPEN(cleanup, TESTFILE, O_RDWR | O_CREAT, FILE_MODE);

	SAFE_SETEUID(cleanup, 0);

	SAFE_FCHOWN(cleanup, fildes, -1, 0);
	SAFE_FCHMOD(cleanup, fildes, NEW_PERMS);

	SAFE_SETEGID(cleanup, ltpuser->pw_gid);
	SAFE_SETEUID(cleanup, ltpuser->pw_uid);
}

static void cleanup(void)
{
	if (fildes > 0 && close(fildes))
		tst_resm(TWARN | TERRNO, "close(%s) Failed", TESTFILE);

	tst_rmdir();
}
