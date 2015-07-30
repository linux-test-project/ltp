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
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * Test Description:
 *  Verify that,
 *	File system GID is always set to the same value as the (possibly new)
 *	effective GID.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include "test.h"
#include "safe_macros.h"
#include "compat_16.h"

TCID_DEFINE(setresgid04);
int TST_TOTAL = 1;
static struct passwd *ltpuser;
static void setup(void);
static void setresgid_verify(void);
static void cleanup(void);

int main(int argc, char **argv)
{
	int i, lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			setresgid_verify();
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

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");

	GID16_CHECK(ltpuser->pw_gid, "setresgid", cleanup)
}

static void setresgid_verify(void)
{
	struct stat buf;

	TEST(SETRESGID(cleanup, -1, ltpuser->pw_gid, -1));

	if (TEST_RETURN != 0) {
		tst_resm(TFAIL | TTERRNO, "setresgid failed unexpectedly");
		return;
	}

	SAFE_TOUCH(cleanup, "test_file", 0644, NULL);

	SAFE_STAT(cleanup, "test_file", &buf);

	if (ltpuser->pw_gid == buf.st_gid) {
		tst_resm(TPASS, "setresgid succeeded as expected");
	} else {
		tst_resm(TFAIL,
			 "setresgid failed unexpectedly; egid(%d) - st_gid(%d)",
			 ltpuser->pw_gid, buf.st_gid);
	}
}

static void cleanup(void)
{
	tst_rmdir();
}
