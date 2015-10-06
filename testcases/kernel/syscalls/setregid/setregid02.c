/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 *   Ported by John George
 */

/*
 * Test that setregid() fails and sets the proper errno values when a
 * non-root user attemps to change the real or effective group id to a
 * value other than the current gid or the current effective gid.
 */

#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "compat_16.h"

TCID_DEFINE(setregid02);

static gid_t neg_one = -1;

static struct passwd *ltpuser;

static struct group ltpgroup, root, bin;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	gid_t *real_gid;
	gid_t *eff_gid;
	int exp_errno;
	struct group *exp_real_usr;
	struct group *exp_eff_usr;
	char *test_msg;
} test_data[] = {
	{
	&neg_one, &root.gr_gid, EPERM, &ltpgroup, &ltpgroup,
		    "After setregid(-1, root),"}, {
	&neg_one, &bin.gr_gid, EPERM, &ltpgroup, &ltpgroup,
		    "After setregid(-1, bin)"}, {
	&root.gr_gid, &neg_one, EPERM, &ltpgroup, &ltpgroup,
		    "After setregid(root,-1),"}, {
	&bin.gr_gid, &neg_one, EPERM, &ltpgroup, &ltpgroup,
		    "After setregid(bin, -1),"}, {
	&root.gr_gid, &bin.gr_gid, EPERM, &ltpgroup, &ltpgroup,
		    "After setregid(root, bin)"}, {
	&bin.gr_gid, &root.gr_gid, EPERM, &ltpgroup, &ltpgroup,
		    "After setregid(bin, root),"}
};

int TST_TOTAL = ARRAY_SIZE(test_data);

static void setup(void);
static void gid_verify(struct group *ru, struct group *eu, char *when);
static struct group get_group_by_name(const char *name);
static struct group get_group_by_gid(gid_t gid);

int main(int ac, char **av)
{
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int i;

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			/* Set the real or effective group id */
			TEST(SETREGID(NULL, *test_data[i].real_gid,
				      *test_data[i].eff_gid));

			if (TEST_RETURN == -1) {
				if (TEST_ERRNO == test_data[i].exp_errno) {
					tst_resm(TPASS, "setregid(%d, %d) "
						 "failed as expected.",
						 *test_data[i].real_gid,
						 *test_data[i].eff_gid);
				} else {
					tst_resm(TFAIL, "setregid(%d, %d) "
						 "failed (%d) but did not set the "
						 "expected errno (%d).",
						 *test_data[i].real_gid,
						 *test_data[i].eff_gid,
						 TEST_ERRNO,
						 test_data[i].exp_errno);
				}
			} else {
				tst_resm(TFAIL, "setregid(%d, %d) "
					 "did not fail (ret: %ld) as expected (ret: -1).",
					 *test_data[i].real_gid,
					 *test_data[i].eff_gid, TEST_RETURN);
			}
			gid_verify(test_data[i].exp_real_usr,
				   test_data[i].exp_eff_usr,
				   test_data[i].test_msg);
		}
	}

	tst_exit();
}

static void setup(void)
{
	tst_require_root();

	tst_sig(FORK, DEF_HANDLER, NULL);

	ltpuser = getpwnam("nobody");
	if (ltpuser == NULL)
		tst_brkm(TBROK, NULL, "getpwnam(\"nobody\") failed");

	if (setgid(ltpuser->pw_gid) == -1) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "setgid failed to set the effective gid to %d",
			 ltpuser->pw_gid);
	}
	if (setuid(ltpuser->pw_uid) == -1) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "setuid failed to to set the effective uid to %d",
			 ltpuser->pw_uid);
	}

	root = get_group_by_name("root");
	ltpgroup = get_group_by_gid(ltpuser->pw_gid);
	bin = get_group_by_name("bin");

	TEST_PAUSE;
}

static struct group get_group_by_name(const char *name)
{
	struct group *ret = getgrnam(name);

	if (ret == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "getgrnam(\"%s\") failed", name);

	GID16_CHECK(ret->gr_gid, setregid, NULL);

	return *ret;
}

static struct group get_group_by_gid(gid_t gid)
{
	struct group *ret = getgrgid(gid);

	if (ret == NULL)
		tst_brkm(TBROK|TERRNO, NULL, "getgrgid(\"%d\") failed", gid);

	GID16_CHECK(ret->gr_gid, setregid, NULL);

	return *ret;
}

void gid_verify(struct group *rg, struct group *eg, char *when)
{
	if ((getgid() != rg->gr_gid) || (getegid() != eg->gr_gid)) {
		tst_resm(TFAIL, "ERROR: %s real gid = %d; effective gid = %d",
			 when, getgid(), getegid());
		tst_resm(TINFO, "Expected: real gid = %d; effective gid = %d",
			 rg->gr_gid, eg->gr_gid);
	} else {
		tst_resm(TPASS, "real or effective gid was modified as expected");
	}
}
