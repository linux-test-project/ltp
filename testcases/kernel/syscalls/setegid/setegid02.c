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
 * DESCRIPTION
 *	The calling process is not privileged and euid is not appropriate,
 *	EPERM should return.
 */

#include <errno.h>
#include <pwd.h>
#include "test.h"
#include "safe_macros.h"

char *TCID = "setegid02";
int TST_TOTAL = 1;
static void setup(void);
static void setegid_verify(void);
static void cleanup(void);

static struct passwd *ltpuser;

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;
		setegid_verify();
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	ltpuser = SAFE_GETPWNAM(cleanup, "nobody");

	SAFE_SETEUID(cleanup, ltpuser->pw_uid);
}

static void setegid_verify(void)
{
	TEST(setegid(ltpuser->pw_gid));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "setegid(%d) succeeded unexpectedly",
			 ltpuser->pw_gid);
		return;
	}

	if (TEST_ERRNO == EPERM) {
		tst_resm(TPASS | TTERRNO, "setegid failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "setegid failed unexpectedly; expected: %d - %s",
			 EPERM, strerror(EPERM));
	}
}

static void cleanup(void)
{
}
