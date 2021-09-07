// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 * Ported by John George
 */

/*
 * Test setregid() when executed by root.
 */

#include "tst_test.h"
#include "tst_uid.h"
#include "compat_tst_16.h"

static gid_t first_gid, second_gid, root_gid, neg_one = -1;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	gid_t *real_gid;
	gid_t *eff_gid;
	gid_t *exp_real_usr;
	gid_t *exp_eff_usr;
	const char *test_msg;
} test_data[] = {
	{
	&root_gid, &root_gid, &root_gid, &root_gid,
		    "After setregid(root, root),"}, {
	&first_gid, &neg_one, &first_gid, &root_gid,
		    "After setregid(first, -1)"}, {
	&root_gid, &neg_one, &root_gid, &root_gid,
		    "After setregid(root,-1),"}, {
	&neg_one, &neg_one, &root_gid, &root_gid,
		    "After setregid(-1, -1),"}, {
	&neg_one, &root_gid, &root_gid, &root_gid,
		    "After setregid(-1, root)"}, {
	&root_gid, &neg_one, &root_gid, &root_gid,
		    "After setregid(root, -1),"}, {
	&second_gid, &first_gid, &second_gid, &first_gid,
		    "After setregid(second, first)"}, {
	&neg_one, &neg_one, &second_gid, &first_gid,
		    "After setregid(-1, -1)"}, {
	&neg_one, &first_gid, &second_gid, &first_gid,
		    "After setregid(-1, first)"}
};

static void gid_verify(gid_t rg, gid_t eg, const char *when)
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
	/* Set the real or effective group id */
	TEST(SETREGID(*test_data[i].real_gid, *test_data[i].eff_gid));

	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "setregid(%d, %d) failed",
			*test_data[i].real_gid, *test_data[i].eff_gid);
		return;
	}

	gid_verify(*test_data[i].exp_real_usr, *test_data[i].exp_eff_usr,
		   test_data[i].test_msg);
}

static void setup(void)
{
	gid_t test_groups[3];

	root_gid = test_groups[0] = getgid();
	tst_get_gids(test_groups, 1, 3);
	first_gid = test_groups[1];
	second_gid = test_groups[2];
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(test_data),
	.needs_root = 1,
	.test = run,
	.setup = setup,
};
