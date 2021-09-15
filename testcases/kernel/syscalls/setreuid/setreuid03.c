// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *    Ported by John George
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * [Description]
 *
 * Test setreuid() when executed by an unpriviledged user.
 */

#include <sys/types.h>
#include <pwd.h>

#include "tst_test.h"
#include "tst_uid.h"
#include "compat_tst_16.h"

static uid_t root_uid, nobody_uid, other_uid, neg_one = -1;

static struct test_data_t {
	uid_t *real_uid;
	uid_t *eff_uid;
	int exp_ret;
	uid_t *exp_real_uid;
	uid_t *exp_eff_uid;
	uid_t *exp_sav_uid;
	const char *test_msg;
} test_data[] = {
	{&nobody_uid, &nobody_uid, 0, &nobody_uid, &nobody_uid, &nobody_uid,
		"setreuid(nobody, nobody)"},
	{&neg_one, &nobody_uid, 0, &nobody_uid, &nobody_uid, &nobody_uid,
		"setreuid(-1, nobody)"},
	{&nobody_uid, &neg_one, 0, &nobody_uid, &nobody_uid, &nobody_uid,
		"setreuid(nobody, -1)"},
	{&neg_one, &neg_one, 0, &nobody_uid, &nobody_uid, &nobody_uid,
		"setreuid(-1, -1)"},
	{&neg_one, &root_uid, -1, &nobody_uid, &nobody_uid, &nobody_uid,
		"setreuid(-1, root)"},
	{&root_uid, &neg_one, -1, &nobody_uid, &nobody_uid, &nobody_uid,
		"setreuid(root, -1)"},
	{&root_uid, &root_uid, -1, &nobody_uid, &nobody_uid,
		&nobody_uid, "setreuid(root, root)"},
	{&root_uid, &nobody_uid, -1, &nobody_uid, &nobody_uid,
		&nobody_uid, "setreuid(root, nobody)"},
	{&root_uid, &other_uid, -1, &nobody_uid, &nobody_uid,
		&nobody_uid, "setreuid(root, other)"},
	{&other_uid, &root_uid, -1, &nobody_uid, &nobody_uid,
		&nobody_uid, "setreuid(other, root)"},
	{&other_uid, &neg_one, -1, &nobody_uid, &nobody_uid,
		&nobody_uid, "setreuid(other, -1)"},
	{&other_uid, &other_uid, -1, &nobody_uid, &nobody_uid,
		&nobody_uid, "setreuid(other, other)"},
	{&other_uid, &nobody_uid, -1, &nobody_uid, &nobody_uid,
		&nobody_uid, "setreuid(other, nobody)"},
	{&nobody_uid, &other_uid, -1, &nobody_uid, &nobody_uid,
		&nobody_uid, "setreuid(nobody, other)"},
};

static void setup(void)
{
	uid_t test_users[2];
	struct passwd *pw;

	root_uid = getuid();
	pw = SAFE_GETPWNAM("nobody");
	nobody_uid = test_users[0] = pw->pw_uid;
	tst_get_uids(test_users, 1, 2);
	other_uid = test_users[1];

	UID16_CHECK(root_uid, setreuid);
	UID16_CHECK(nobody_uid, setreuid);
	UID16_CHECK(other_uid, setreuid);

	SAFE_SETUID(nobody_uid);
}

static void run(unsigned int n)
{
	const struct test_data_t *tc = test_data + n;

	if (tc->exp_ret) {
		TST_EXP_FAIL(SETREUID(*tc->real_uid, *tc->eff_uid), EPERM,
			"%s", tc->test_msg);
	} else {
		TST_EXP_PASS(SETREUID(*tc->real_uid, *tc->eff_uid), "%s",
			tc->test_msg);
	}

	if (!TST_PASS)
		return;

	tst_check_resuid(tc->test_msg, *tc->exp_real_uid, *tc->exp_eff_uid,
		*tc->exp_sav_uid);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(test_data),
	.setup = setup,
	.needs_root = 1,
};
