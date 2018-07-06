/* Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* DESCRIPTION
 * This test will verify the mkdir(2) creates a new directory successfully and
 * it is owned by the effective UID and GID of the process.
 */

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include "tst_test.h"

#define PERMS		0777
#define TESTDIR		"testdir"

static void verify_mkdir(void)
{
	struct stat buf;

	TEST(mkdir(TESTDIR, PERMS));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "mkdir() Failed");
		return;
	}

	SAFE_STAT(TESTDIR, &buf);

	if (buf.st_uid != geteuid()) {
		tst_res(TFAIL, "mkdir() FAILED to set owner ID "
			"as process's effective ID");
		return;
	}

	if (buf.st_gid != getegid()) {
		tst_res(TFAIL, "mkdir() failed to set group ID "
			"as the process's group ID");
		return;
	}

	tst_res(TPASS, "mkdir() functionality is correct");

	SAFE_RMDIR(TESTDIR);
}

void setup(void)
{
	struct passwd *pw;

	pw = SAFE_GETPWNAM("nobody");

	SAFE_SETUID(pw->pw_uid);
}

static struct tst_test test = {
	.test_all = verify_mkdir,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.setup = setup,
};
