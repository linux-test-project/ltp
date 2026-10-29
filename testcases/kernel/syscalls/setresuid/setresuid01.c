// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 ported by John George
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Test setresuid() when executed by root.
 */

#include "tst_test.h"
#include "tst_uid.h"
#include "compat_tst_16.h"

static uid_t root_uid, main_uid, other_uid, neg_one = -1;

static struct test_data_t {
	uid_t *real_uid;
	uid_t *eff_uid;
	uid_t *sav_uid;
	uid_t *exp_real_uid;
	uid_t *exp_eff_uid;
	uid_t *exp_sav_uid;
	char *test_msg;
} test_data[] = {
	{&neg_one, &neg_one, &neg_one, &root_uid, &root_uid, &root_uid,
		"After setresuid(-1, -1, -1),"},
	{&neg_one, &neg_one, &main_uid, &root_uid, &root_uid, &main_uid,
		"After setresuid(-1, -1, main),"},
	{&neg_one, &other_uid, &neg_one, &root_uid, &other_uid, &main_uid,
		"After setresuid(-1, other, -1),"},
	{&neg_one, &neg_one, &root_uid, &root_uid, &other_uid, &root_uid,
		"After setresuid(-1, -1, root),"},
	{&neg_one, &neg_one, &other_uid, &root_uid, &other_uid, &other_uid,
		"After setresuid(-1, -1, other),"},
	{&neg_one, &root_uid, &neg_one, &root_uid, &root_uid, &other_uid,
		"After setresuid(-1, root, -1),"},
	{&main_uid, &neg_one, &neg_one, &main_uid, &root_uid, &other_uid,
		"After setresuid(main, -1, -1)"},
	{&neg_one, &root_uid, &neg_one, &main_uid, &root_uid, &other_uid,
		"After setresuid(-1, root, -1),"},
	{&root_uid, &neg_one, &root_uid, &root_uid, &root_uid, &root_uid,
		"After setresuid(root, -1, -1),"},
};

static void setup(void)
{
	uid_t test_users[2];

	root_uid = getuid();
	tst_get_uids(test_users, 0, 2);
	main_uid = test_users[0];
	other_uid = test_users[1];

	UID16_CHECK(root_uid, setresuid);
	UID16_CHECK(main_uid, setresuid);
	UID16_CHECK(other_uid, setresuid);
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
