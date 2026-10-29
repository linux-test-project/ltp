// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2001
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

#include "tst_test.h"
#include "tst_uid.h"
#include "compat_tst_16.h"

static gid_t root_gid, nobody_gid, other_gid, neg_one = -1;

static struct passwd *ltpuser;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

static struct tcase {
	gid_t *real_gid;
	gid_t *eff_gid;
	int exp_errno;
	gid_t *exp_real_usr;
	gid_t *exp_eff_usr;
	char *test_msg;
} tcases[] = {
	{
	&neg_one, &root_gid, EPERM, &nobody_gid, &nobody_gid,
		    "After setregid(-1, root),"}, {
	&neg_one, &other_gid, EPERM, &nobody_gid, &nobody_gid,
		    "After setregid(-1, other)"}, {
	&root_gid, &neg_one, EPERM, &nobody_gid, &nobody_gid,
		    "After setregid(root,-1),"}, {
	&other_gid, &neg_one, EPERM, &nobody_gid, &nobody_gid,
		    "After setregid(other, -1),"}, {
	&root_gid, &other_gid, EPERM, &nobody_gid, &nobody_gid,
		    "After setregid(root, other)"}, {
	&other_gid, &root_gid, EPERM, &nobody_gid, &nobody_gid,
		    "After setregid(other, root),"}
};

void gid_verify(gid_t rg, gid_t eg, char *when)
{
	if ((getgid() != rg) || (getegid() != eg)) {
		tst_res(TFAIL, "ERROR: %s real gid = %d; effective gid = %d",
			 when, getgid(), getegid());
		tst_res(TINFO, "Expected: real gid = %d; effective gid = %d",
			 rg, eg);
		return;
	}

	tst_res(TPASS, "real or effective gid wasn't modified as expected");
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	/* Set the real or effective group id */
	TEST(SETREGID(*tc->real_gid, *tc->eff_gid));

	if (TST_RET == -1) {
		if (tc->exp_errno == TST_ERR) {
			tst_res(TPASS | TTERRNO,
				"setregid(%d, %d) failed as expected",
				*tc->real_gid, *tc->eff_gid);
		} else {
			tst_res(TFAIL | TTERRNO,
				"setregid(%d, %d) failed unexpectedly, expected %s",
				*tc->real_gid, *tc->eff_gid,
				tst_strerrno(tc->exp_errno));
		}
	} else {
		tst_res(TFAIL,
			"setregid(%d, %d) did not fail (ret: %ld) as expected (ret: -1).",
			*tc->real_gid, *tc->eff_gid, TST_RET);
	}
	gid_verify(*tc->exp_real_usr, *tc->exp_eff_usr, tc->test_msg);
}

static void setup(void)
{
	gid_t test_groups[3];

	ltpuser = SAFE_GETPWNAM("nobody");
	nobody_gid = test_groups[0] = ltpuser->pw_gid;
	root_gid = test_groups[1] = getgid();
	tst_get_gids(test_groups, 2, 3);
	other_gid = test_groups[2];

	GID16_CHECK(root_gid, setregid);
	GID16_CHECK(nobody_gid, setregid);
	GID16_CHECK(other_gid, setregid);

	SAFE_SETGID(ltpuser->pw_gid);
	SAFE_SETUID(ltpuser->pw_uid);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.test = run,
	.setup = setup,
};
