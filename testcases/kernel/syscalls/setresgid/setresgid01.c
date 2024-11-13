// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that setresgid() syscall correctly sets real user ID, effective user
 * ID and the saved set-user ID in the calling process.
 */

#define _GNU_SOURCE

#include <pwd.h>
#include "tst_test.h"
#include "compat_tst_16.h"

struct tcase {
	uid_t *rgid;
	uid_t *egid;
	uid_t *sgid;
	uid_t *exp_rgid;
	uid_t *exp_egid;
	uid_t *exp_sgid;
};

static uid_t nobody_gid;
static uid_t root_gid;
static uid_t neg = -1;

static struct tcase tcases[] = {
	{
		&neg, &neg, &neg,
		&root_gid, &root_gid, &root_gid,
	},
	{
		&neg, &neg, &nobody_gid,
		&root_gid, &root_gid, &nobody_gid,
	},
	{
		&neg, &nobody_gid, &neg,
		&root_gid, &nobody_gid, &nobody_gid,
	},
	{
		&nobody_gid, &neg, &neg,
		&nobody_gid, &nobody_gid, &nobody_gid,
	},
	{
		&root_gid, &root_gid, &root_gid,
		&root_gid, &root_gid, &root_gid,
	},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	uid_t cur_rgid;
	uid_t cur_egid;
	uid_t cur_sgid;

	TST_EXP_PASS(SETRESGID(*tc->rgid, *tc->egid, *tc->sgid));
	if (!TST_PASS)
		return;

	SAFE_GETRESGID(&cur_rgid, &cur_egid, &cur_sgid);

	TST_EXP_EQ_LI(*tc->exp_rgid, cur_rgid);
	TST_EXP_EQ_LI(*tc->exp_egid, cur_egid);
	TST_EXP_EQ_LI(*tc->exp_sgid, cur_sgid);
}

static void setup(void)
{
	struct passwd *pwd_buf;

	pwd_buf = SAFE_GETPWNAM("root");
	GID16_CHECK(pwd_buf->pw_gid, "setresgid");
	root_gid = pwd_buf->pw_gid;

	pwd_buf = SAFE_GETPWNAM("nobody");
	GID16_CHECK(pwd_buf->pw_gid, "setresgid");
	nobody_gid = pwd_buf->pw_gid;
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
};
