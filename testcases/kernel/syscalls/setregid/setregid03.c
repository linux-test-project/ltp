// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * Ported by John George
 */

/*
 * Test setregid() when executed by a non-root user.
 */

#include <pwd.h>

#include "tst_test.h"
#include "tst_uid.h"
#include "compat_tst_16.h"

static int fail = -1;
static int pass;
static gid_t primary_gid, secondary_gid, neg_one = -1;

struct tcase {
	gid_t *real_gid;
	gid_t *eff_gid;
	int *exp_ret;
	gid_t *exp_real_usr;
	gid_t *exp_eff_usr;
	char *test_msg;
} tcases[] = {
	{
	&primary_gid, &secondary_gid, &pass, &primary_gid, &secondary_gid,
		    "After setregid(primary, secondary),"}, {
	&neg_one, &primary_gid, &pass, &primary_gid, &primary_gid,
		    "After setregid(-1, primary)"}, {
	&neg_one, &secondary_gid, &pass, &primary_gid, &secondary_gid,
		    "After setregid(-1, secondary),"}, {
	&secondary_gid, &neg_one, &pass, &secondary_gid, &secondary_gid,
		    "After setregid(secondary, -1),"}, {
	&neg_one, &neg_one, &pass, &secondary_gid, &secondary_gid,
		    "After setregid(-1, -1),"}, {
	&neg_one, &secondary_gid, &pass, &secondary_gid, &secondary_gid,
		    "After setregid(-1, secondary),"}, {
	&secondary_gid, &neg_one, &pass, &secondary_gid, &secondary_gid,
		    "After setregid(secondary, -1),"}, {
	&secondary_gid, &secondary_gid, &pass, &secondary_gid, &secondary_gid,
		    "After setregid(secondary, secondary),"}, {
	&primary_gid, &neg_one, &fail, &secondary_gid, &secondary_gid,
		    "After setregid(primary, -1)"}, {
	&neg_one, &primary_gid, &fail, &secondary_gid, &secondary_gid,
		    "After setregid(-1, primary)"}, {
	&primary_gid, &primary_gid, &fail, &secondary_gid, &secondary_gid,
		    "After setregid(primary, primary)"},};

static void setup(void)
{
	struct passwd *nobody;
	gid_t test_groups[2];

	nobody = SAFE_GETPWNAM("nobody");

	tst_get_gids(test_groups, 0, 2);
	primary_gid = test_groups[0];
	secondary_gid = test_groups[1];
	GID16_CHECK(primary_gid, setregid);
	GID16_CHECK(secondary_gid, setregid);

	/* set the appropriate ownership values */
	SAFE_SETREGID(primary_gid, secondary_gid);
	SAFE_SETEUID(nobody->pw_uid);
}

static void test_success(struct tcase *tc)
{
	if (TST_RET != 0)
		tst_res(TFAIL | TTERRNO, "setregid(%d, %d) failed unexpectedly",
			*tc->real_gid, *tc->eff_gid);
	else
		tst_res(TPASS, "setregid(%d, %d) succeeded as expected",
			*tc->real_gid, *tc->eff_gid);
}

static void test_failure(struct tcase *tc)
{
	if (TST_RET == 0)
		tst_res(TFAIL, "setregid(%d, %d) succeeded unexpectedly",
			*tc->real_gid, *tc->eff_gid);
	else if (TST_ERR == EPERM)
		tst_res(TPASS, "setregid(%d, %d) failed as expected",
			*tc->real_gid, *tc->eff_gid);
	else
		tst_res(TFAIL | TTERRNO,
			"setregid(%d, %d) did not set errno value as expected",
			*tc->real_gid, *tc->eff_gid);
}

static void gid_verify(gid_t rg, gid_t eg, char *when)
{
	if ((getgid() != rg) || (getegid() != eg)) {
		tst_res(TFAIL, "ERROR: %s real gid = %d; effective gid = %d",
			 when, getgid(), getegid());
		tst_res(TINFO, "Expected: real gid = %d; effective gid = %d",
			 rg, eg);
	} else {
		tst_res(TPASS,
			"real or effective gid was modified as expected");
	}
}

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	/* Set the real or effective group id */
	TEST(SETREGID(*tc->real_gid, *tc->eff_gid));

	if (*tc->exp_ret == 0)
		test_success(tc);
	else
		test_failure(tc);

	gid_verify(*tc->exp_real_usr, *tc->exp_eff_usr, tc->test_msg);
}

void run_all(void)
{
	unsigned int i;

	if (!SAFE_FORK()) {
		for (i = 0; i < ARRAY_SIZE(tcases); i++)
			run(i);
	}
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.test_all = run_all,
	.setup = setup,
};
