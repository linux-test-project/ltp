// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2002. All Rights Reserved.
 * Copyright (c) 2019 Petr Vorel <petr.vorel@gmail.com>
 * Author: Saji Kumar.V.R <saji.kumar@wipro.com>
 */

#include "setdomainname.h"

static void do_test(void)
{
	char *new = TST_VALID_DOMAIN_NAME;
	static char tmp[_UTSNAME_DOMAIN_LENGTH];

	TEST(do_setdomainname(new, sizeof(new)));

	if (TST_RET != 0)
		tst_brk(TFAIL | TTERRNO, "setdomainname() failed: %d", TST_ERR);

	if (getdomainname(tmp, sizeof(tmp)) != 0)
		tst_brk(TFAIL | TERRNO, "getdomainname() failed");

	if (strcmp(tmp, new))
		tst_res(TFAIL, "getdomainname() returned wrong domainname: '%s'", tmp);
	else
		tst_res(TPASS, "setdomainname() succeed");
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = do_test,
	.test_variants = TEST_VARIANTS,
};
