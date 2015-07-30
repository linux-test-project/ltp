/*
 * Copyright (c) Dan Kegel 2003
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
 * Test Name: setegid01
 *
 * Test Description:
 *  Verify that setegid does not modify the saved gid or real gid.
 */

#define _GNU_SOURCE 1
#include <pwd.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "test.h"
#include "safe_macros.h"

char *TCID = "setegid01";
int TST_TOTAL = 1;
static void setup(void);
static void setegid_verify(void);
static void cleanup(void);

static gid_t nobody_gid;

int main(int argc, char **argv)
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
	struct passwd *nobody;

	tst_require_root();

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	nobody = SAFE_GETPWNAM(cleanup, "nobody");

	nobody_gid = nobody->pw_gid;
}

static void setegid_verify(void)
{
	gid_t cur_rgid, cur_egid, cur_sgid;
	gid_t orig_rgid, orig_egid, orig_sgid;

	SAFE_GETRESGID(cleanup, &orig_rgid, &orig_egid, &orig_sgid);
	tst_resm(TINFO, "getresgid reports rgid %d, egid %d, sgid %d",
		 orig_rgid, orig_egid, orig_sgid);

	tst_resm(TINFO, "calling setegid(nobody_gid %d)", nobody_gid);
	SAFE_SETEGID(cleanup, nobody_gid);

	SAFE_GETRESGID(cleanup, &cur_rgid, &cur_egid, &cur_sgid);
	tst_resm(TINFO, "getresgid reports rgid %d, egid %d, sgid %d", cur_rgid,
		 cur_egid, cur_sgid);

	/* make sure it at least does what its name says */
	if (nobody_gid != cur_egid) {
		tst_resm(TFAIL, "setegid() failed to change the effective gid");
		return;
	}

	/* SUSv3 says the real group ID and saved set-gid must
	 * remain unchanged by setgid.  See
	 * http://www.opengroup.org/onlinepubs/007904975/functions/setegid.html
	 */
	if (orig_sgid != cur_sgid) {
		tst_resm(TFAIL, "setegid() changed the saved set-gid");
		return;
	}
	if (orig_rgid != cur_rgid) {
		tst_resm(TFAIL, "setegid() changed the real gid");
		return;
	}

	SAFE_SETEGID(cleanup, orig_egid);

	SAFE_GETRESGID(cleanup, &cur_rgid, &cur_egid, &orig_sgid);

	if (orig_egid != cur_egid) {
		tst_resm(TFAIL, "setegid() failed to reset effective gid back");
		return;
	}

	tst_resm(TPASS, "setegid() passed functional test");
}

static void cleanup(void)
{
}
