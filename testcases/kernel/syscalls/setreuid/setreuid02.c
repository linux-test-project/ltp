// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *    Ported by John George
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * [Description]
 *
 * Test setreuid() when executed by root.
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
	uid_t *exp_real_uid;
	uid_t *exp_eff_uid;
	uid_t *exp_sav_uid;
	const char *test_msg;
} test_data[] = {
	{&neg_one, &neg_one, &root_uid, &root_uid, &root_uid,
		"setreuid(-1, -1)"},
	{&nobody_uid, &neg_one, &nobody_uid, &root_uid, &root_uid,
		"setreuid(nobody, -1)"},
	{&root_uid, &neg_one, &root_uid, &root_uid, &root_uid,
		"setreuid(root, -1)"},
	{&neg_one, &nobody_uid, &root_uid, &nobody_uid, &nobody_uid,
		"setreuid(-1, nobody)"},
	{&neg_one, &root_uid, &root_uid, &root_uid, &nobody_uid,
		"setreuid(-1, root)"},
	{&other_uid, &neg_one, &other_uid, &root_uid, &root_uid,
		"setreuid(other, -1)"},
	{&root_uid, &neg_one, &root_uid, &root_uid, &root_uid,
		"setreuid(root, -1)"},
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

	/* Make sure that saved UID is also set to root */
	SAFE_SETUID(root_uid);
}

static void run(unsigned int n)
{
	const struct test_data_t *tc = test_data + n;

	TST_EXP_PASS_SILENT(SETREUID(*tc->real_uid, *tc->eff_uid), "%s",
		tc->test_msg);

	if (!TST_PASS)
		return;

	if (tst_check_resuid(tc->test_msg, *tc->exp_real_uid, *tc->exp_eff_uid,
		*tc->exp_sav_uid))
		tst_res(TPASS, "%s works as expected", tc->test_msg);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(test_data),
	.setup = setup,
	.needs_root = 1,
};
