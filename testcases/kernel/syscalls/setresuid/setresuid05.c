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
 *	File system UID is always set to the same value as the (possibly new)
 *	effective UID.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include "test.h"
#include "safe_macros.h"
#include "compat_16.h"

TCID_DEFINE(setresuid05);
int TST_TOTAL = 1;
static struct passwd *ltpuser;
static void setup(void);
static void setresuid_verify(void);
static void cleanup(void);

int main(int argc, char **argv)
{
	int i, lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		for (i = 0; i < TST_TOTAL; i++)
			setresuid_verify();
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

	UID16_CHECK(ltpuser->pw_uid, "setresuid", cleanup)
}

static void setresuid_verify(void)
{
	struct stat buf;

	TEST(SETRESUID(cleanup, -1, ltpuser->pw_uid, -1));

	if (TEST_RETURN != 0) {
		tst_resm(TFAIL | TTERRNO, "setresuid failed unexpectedly");
		return;
	}

	SAFE_TOUCH(cleanup, "test_file", 0644, NULL);

	SAFE_STAT(cleanup, "test_file", &buf);

	if (ltpuser->pw_uid == buf.st_uid) {
		tst_resm(TPASS, "setresuid succeeded as expected");
	} else {
		tst_resm(TFAIL,
			 "setresuid failed unexpectedly; euid(%d) - st_uid(%d)",
			 ltpuser->pw_uid, buf.st_uid);
	}
}

static void cleanup(void)
{
	if (seteuid(0) < 0)
		tst_resm(TWARN | TERRNO, "seteuid failed");

	tst_rmdir();
}
