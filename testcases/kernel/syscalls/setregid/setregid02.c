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
#include "usctest.h"
#include "compat_16.h"

TCID_DEFINE(setregid02);

static gid_t neg_one = -1;

static gid_t inval_user;
static struct passwd *ltpuser;

static struct group nobody, root, bin;

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
	&neg_one, &root.gr_gid, EPERM, &nobody, &nobody,
		    "After setregid(-1, root),"}, {
	&neg_one, &bin.gr_gid, EPERM, &nobody, &nobody,
		    "After setregid(-1, bin)"}, {
	&root.gr_gid, &neg_one, EPERM, &nobody, &nobody,
		    "After setregid(root,-1),"}, {
	&bin.gr_gid, &neg_one, EPERM, &nobody, &nobody,
		    "After setregid(bin, -1),"}, {
	&root.gr_gid, &bin.gr_gid, EPERM, &nobody, &nobody,
		    "After setregid(root, bin)"}, {
	&bin.gr_gid, &root.gr_gid, EPERM, &nobody, &nobody,
		    "After setregid(bin, root),"}, {
	&inval_user, &neg_one, EINVAL, &nobody, &nobody,
		    "After setregid(invalid group, -1),"}, {
	&neg_one, &inval_user, EINVAL, &nobody, &nobody,
		    "After setregid(-1, invalid group),"},};

int TST_TOTAL = sizeof(test_data) / sizeof(test_data[0]);

static void setup(void);
static void cleanup(void);
static void gid_verify(struct group *ru, struct group *eu, char *when);

int main(int ac, char **av)
{
	int lc;
	char *msg;

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		int i;

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			/* Set the real or effective group id */
			TEST(SETREGID(cleanup, *test_data[i].real_gid,
				      *test_data[i].eff_gid));

			if (TEST_RETURN == -1) {
				TEST_ERROR_LOG(TEST_ERRNO);
				if (TEST_ERRNO == test_data[i].exp_errno) {
					tst_resm(TPASS, "setregid(%d, %d) "
						 "failed as expected.",
						 *test_data[i].real_gid,
						 *test_data[i].eff_gid);
				} else if (TEST_ERRNO == test_data[0].exp_errno) {
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
			if (STD_FUNCTIONAL_TEST) {
				gid_verify(test_data[i].exp_real_usr,
					   test_data[i].exp_eff_usr,
					   test_data[i].test_msg);
			} else {
				tst_resm(TINFO, "Call succeeded.");
			}
		}
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{
	struct group *junk;

	tst_require_root(NULL);

	tst_sig(FORK, DEF_HANDLER, cleanup);

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

#define GET_GID(group)	do {		\
	junk = getgrnam(#group);	\
	if (junk == NULL) {		\
		tst_brkm(TBROK|TERRNO, NULL, "getgrnam(\"%s\") failed", #group); \
	}				\
	GID16_CHECK(junk->gr_gid, setregid, NULL); \
	group = *(junk); \
} while (0)

	GET_GID(root);
	GET_GID(nobody);
	GET_GID(bin);

	inval_user = GET_UNUSED_GID();
	if (inval_user == -1)
		tst_brkm(TBROK, NULL, "No free gid found");

	TEST_PAUSE;
}

static void cleanup(void)
{
	TEST_CLEANUP;
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
