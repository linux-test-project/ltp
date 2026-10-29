// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *    AUTHOR: Madhu T L <madhu.tarikere@wipro.com>
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 */

/*\
 * Verify that setresgid() will successfully set the expected GID when called
 * by root with the following combinations of arguments:
 *
 * - setresgid(-1, -1, -1)
 * - setresgid(-1, -1, other)
 * - setresgid(-1, other, -1)
 * - setresgid(other, -1, -1)
 * - setresgid(root, root, root)
 * - setresgid(root, main, main)
 */

#include "tst_test.h"
#include "tst_uid.h"
#include "compat_tst_16.h"

struct test_case_t {
	gid_t *rgid;
	gid_t *egid;
	gid_t *sgid;
	gid_t *exp_rgid;
	gid_t *exp_egid;
	gid_t *exp_sgid;
	char *desc;
};

static gid_t root_gid, main_gid, other_gid, neg = -1;

/* Don't change order of these test cases */
static struct test_case_t test_cases[] = {
	{&neg, &neg, &neg, &root_gid, &main_gid, &main_gid,
	 "setresgid(-1, -1, -1)"},
	{&neg, &neg, &other_gid, &root_gid, &main_gid, &other_gid,
	 "setresgid(-1, -1, other)"},
	{&neg, &other_gid, &neg, &root_gid, &other_gid, &other_gid,
	 "setresgid(-1, other, -1)"},
	{&other_gid, &neg, &neg, &other_gid, &other_gid, &other_gid,
	 "setresgid(other, -1, -1)"},
	{&root_gid, &root_gid, &root_gid, &root_gid, &root_gid, &root_gid,
	 "setresgid(root, root, root)"},
	{&root_gid, &main_gid, &main_gid, &root_gid, &main_gid, &main_gid,
	 "setresgid(root, main, main)"},
};

static void setup(void)
{
	gid_t test_groups[3];

	root_gid = test_groups[0] = getgid();
	tst_get_gids(test_groups, 1, 3);
	main_gid = test_groups[1];
	other_gid = test_groups[2];

	GID16_CHECK(root_gid, setresgid);
	GID16_CHECK(main_gid, setresgid);
	GID16_CHECK(other_gid, setresgid);

	SAFE_SETRESGID(-1, main_gid, main_gid);
}

static void run(unsigned int n)
{
	const struct test_case_t *tc = test_cases + n;

	TST_EXP_PASS_SILENT(SETRESGID(*tc->rgid, *tc->egid, *tc->sgid), "%s",
		tc->desc);

	if (!TST_PASS)
		return;

	if (tst_check_resgid(tc->desc, *tc->exp_rgid, *tc->exp_egid,
		*tc->exp_sgid))
		tst_res(TPASS, "%s works as expected", tc->desc);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(test_cases),
	.setup = setup,
	.needs_root = 1,
};
