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

	TEST(do_setdomainname(new, strlen(new)));

	if (TST_RET != 0)
		tst_brk(TFAIL | TTERRNO, "set" SYSCALL_NAME "() failed: %d", TST_ERR);

	if (GET_SYSCALL(tmp, sizeof(tmp)) != 0)
		tst_brk(TFAIL | TERRNO, "get" SYSCALL_NAME "() failed");

	if (strcmp(tmp, new))
		tst_res(TFAIL, "get" SYSCALL_NAME "() returned wrong domainname: '%s'", tmp);
	else
		tst_res(TPASS, "set" SYSCALL_NAME "() succeed");
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = do_test,
	.test_variants = TEST_VARIANTS,
};
