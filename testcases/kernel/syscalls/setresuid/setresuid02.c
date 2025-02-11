// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 ported by John George
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Test that a non-root user can change the real, effective and saved uid
 * values through the setresuid system call.
 */

#define _GNU_SOURCE 1
#include <sys/types.h>
#include <pwd.h>

#include "tst_test.h"
#include "tst_uid.h"
#include "compat_tst_16.h"

static uid_t nobody_uid, other_uid, neg_one = -1;

static struct test_data_t {
	uid_t *real_uid;
	uid_t *eff_uid;
	uid_t *sav_uid;
	uid_t *exp_real_uid;
	uid_t *exp_eff_uid;
	uid_t *exp_sav_uid;
	char *test_msg;
} test_data[] = {
	{&neg_one, &neg_one, &other_uid, &nobody_uid, &other_uid, &other_uid,
		"setresuid(-1, -1, other)"},
	{&neg_one, &nobody_uid, &neg_one, &nobody_uid, &nobody_uid, &other_uid,
		"setresuid(-1, nobody -1)"},
	{&other_uid, &neg_one, &neg_one, &other_uid, &nobody_uid, &other_uid,
		"setresuid(other, -1 -1)"},
	/* Return to initial state */
	{&nobody_uid, &other_uid, &nobody_uid, &nobody_uid, &other_uid,
		&nobody_uid, "setresuid(nobody, other, nobody)"},
};

static void setup(void)
{
	uid_t test_users[2];
	struct passwd *pw = SAFE_GETPWNAM("nobody");

	nobody_uid = test_users[0] = pw->pw_uid;
	tst_get_uids(test_users, 1, 2);
	other_uid = test_users[1];

	UID16_CHECK(nobody_uid, setresuid);
	UID16_CHECK(other_uid, setresuid);

	SAFE_SETRESUID(nobody_uid, other_uid, nobody_uid);
}

static void run(unsigned int n)
{
	const struct test_data_t *tc = test_data + n;

	TST_EXP_PASS_SILENT(SETRESUID(*tc->real_uid, *tc->eff_uid,
		*tc->sav_uid), "%s", tc->test_msg);

	if (!TST_PASS)
		return;

	if (tst_check_resuid(tc->test_msg, *tc->exp_real_uid,
		*tc->exp_eff_uid, *tc->exp_sav_uid))
		tst_res(TPASS, "%s works as expected", tc->test_msg);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(test_data),
	.setup = setup,
	.needs_root = 1,
};
