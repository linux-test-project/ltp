// SPDX-License-Identifier: GPL-2.0
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
#include "compat_tst_16.h"

static gid_t neg_one = -1;

static struct passwd *ltpuser;

static struct group ltpgroup, root, bin;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

static struct tcase {
	gid_t *real_gid;
	gid_t *eff_gid;
	int exp_errno;
	struct group *exp_real_usr;
	struct group *exp_eff_usr;
	char *test_msg;
} tcases[] = {
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

static struct group get_group_by_name(const char *name)
{
	struct group *ret = SAFE_GETGRNAM(name);

	GID16_CHECK(ret->gr_gid, setregid);

	return *ret;
}

static struct group get_group_by_gid(gid_t gid)
{
	struct group *ret = SAFE_GETGRGID(gid);

	GID16_CHECK(ret->gr_gid, setregid);

	return *ret;
}

void gid_verify(struct group *rg, struct group *eg, char *when)
{
	if ((getgid() != rg->gr_gid) || (getegid() != eg->gr_gid)) {
		tst_res(TFAIL, "ERROR: %s real gid = %d; effective gid = %d",
			 when, getgid(), getegid());
		tst_res(TINFO, "Expected: real gid = %d; effective gid = %d",
			 rg->gr_gid, eg->gr_gid);
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
	gid_verify(tc->exp_real_usr, tc->exp_eff_usr, tc->test_msg);
}

static void setup(void)
{
	ltpuser = SAFE_GETPWNAM("nobody");

	root = get_group_by_name("root");
	ltpgroup = get_group_by_gid(ltpuser->pw_gid);
	bin = get_group_by_name("bin");

	SAFE_SETGID(ltpuser->pw_gid);
	SAFE_SETUID(ltpuser->pw_uid);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.test = run,
	.setup = setup,
};
