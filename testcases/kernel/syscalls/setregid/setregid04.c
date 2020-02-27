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
#include "compat_tst_16.h"

static gid_t neg_one = -1;

static struct group nobody_gr, daemon_gr, root_gr, bin_gr;

/*
 * The following structure contains all test data.  Each structure in the array
 * is used for a separate test.  The tests are executed in the for loop below.
 */

struct test_data_t {
	gid_t *real_gid;
	gid_t *eff_gid;
	struct group *exp_real_usr;
	struct group *exp_eff_usr;
	const char *test_msg;
} test_data[] = {
	{
	&root_gr.gr_gid, &root_gr.gr_gid, &root_gr, &root_gr,
		    "After setregid(root, root),"}, {
	&nobody_gr.gr_gid, &neg_one, &nobody_gr, &root_gr,
		    "After setregid(nobody, -1)"}, {
	&root_gr.gr_gid, &neg_one, &root_gr, &root_gr,
		    "After setregid(root,-1),"}, {
	&neg_one, &neg_one, &root_gr, &root_gr,
		    "After setregid(-1, -1),"}, {
	&neg_one, &root_gr.gr_gid, &root_gr, &root_gr,
		    "After setregid(-1, root)"}, {
	&root_gr.gr_gid, &neg_one, &root_gr, &root_gr,
		    "After setregid(root, -1),"}, {
	&daemon_gr.gr_gid, &nobody_gr.gr_gid, &daemon_gr, &nobody_gr,
		    "After setregid(daemon, nobody)"}, {
	&neg_one, &neg_one, &daemon_gr, &nobody_gr,
		    "After setregid(-1, -1)"}, {
	&neg_one, &nobody_gr.gr_gid, &daemon_gr, &nobody_gr,
		    "After setregid(-1, nobody)"}
};

static void gid_verify(struct group *rg, struct group *eg, const char *when)
{
	if ((getgid() != rg->gr_gid) || (getegid() != eg->gr_gid)) {
		tst_res(TFAIL, "ERROR: %s real gid = %d; effective gid = %d",
			 when, getgid(), getegid());
		tst_res(TINFO, "Expected: real gid = %d; effective gid = %d",
			 rg->gr_gid, eg->gr_gid);
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

	gid_verify(test_data[i].exp_real_usr, test_data[i].exp_eff_usr,
		   test_data[i].test_msg);
}

static void setup(void)
{
	root_gr = *SAFE_GETGRNAM("root");
	nobody_gr = *SAFE_GETGRNAM_FALLBACK("nobody", "nogroup");
	daemon_gr = *SAFE_GETGRNAM("daemon");
	bin_gr = *SAFE_GETGRNAM("bin");
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(test_data),
	.needs_root = 1,
	.test = run,
	.setup = setup,
};
