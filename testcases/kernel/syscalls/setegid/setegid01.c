// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Dan Kegel 2003
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that setegid() sets the effective UID of the calling process
 * correctly, and does not modify the saved GID and real GID.
 */

#include <pwd.h>
#include "tst_test.h"

static gid_t nobody_gid;

static void setup(void)
{
	struct passwd *nobody;

	nobody = SAFE_GETPWNAM("nobody");
	nobody_gid = nobody->pw_gid;
}

static void setegid_verify(void)
{
	gid_t cur_rgid, cur_egid, cur_sgid;
	gid_t orig_rgid, orig_egid, orig_sgid;

	SAFE_GETRESGID(&orig_rgid, &orig_egid, &orig_sgid);
	tst_res(TINFO, "getresgid() reports rgid: %d, egid: %d, sgid: %d",
			orig_rgid, orig_egid, orig_sgid);

	tst_res(TINFO, "call setegid(nobody_gid %d)", nobody_gid);
	SAFE_SETEGID(nobody_gid);

	SAFE_GETRESGID(&cur_rgid, &cur_egid, &cur_sgid);
	tst_res(TINFO, "getresgid() reports rgid: %d, egid: %d, sgid: %d",
			cur_rgid, cur_egid, cur_sgid);

	TST_EXP_EQ_LU(nobody_gid, cur_egid);
	TST_EXP_EQ_LU(orig_rgid, cur_rgid);
	TST_EXP_EQ_LU(orig_sgid, cur_sgid);

	SAFE_SETEGID(orig_egid);
	SAFE_GETRESGID(&cur_rgid, &cur_egid, &orig_sgid);
	TST_EXP_EQ_LU(orig_egid, cur_egid);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = setegid_verify,
	.needs_root = 1
};
