// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2002. All Rights Reserved.
 * Copyright (c) 2019 Petr Vorel <petr.vorel@gmail.com>
 * Author: Saji Kumar.V.R <saji.kumar@wipro.com>
 */

#include <errno.h>
#include <pwd.h>

#include "setdomainname.h"

struct passwd *ltpuser;

static void do_test(void)
{
	char *new = TST_VALID_DOMAIN_NAME;

	TEST(do_setdomainname(new, strlen(new)));

	if (TST_RET != -1) {
		tst_res(TFAIL, "unexpected exit code: %ld", TST_RET);
		return;
	}

	if (TST_ERR != EPERM) {
		tst_res(TFAIL | TTERRNO, "unexpected errno: %d, expected: EPERM",
			TST_ERR);
		return;
	}

	tst_res(TPASS | TTERRNO, "expected failure");
}

void setup_setuid(void)
{
	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(ltpuser->pw_uid);
	setup();
}

static void cleanup_setuid(void)
{
	SAFE_SETEUID(0);
	cleanup();
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup_setuid,
	.cleanup = cleanup_setuid,
	.test_all = do_test,
	.test_variants = TEST_VARIANTS,
};
