// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2002. All Rights Reserved.
 * Copyright (c) 2019 Petr Vorel <petr.vorel@gmail.com>
 * Author: Saji Kumar.V.R <saji.kumar@wipro.com>
 */

#include "setdomainname.h"

#define ERRNO_DESC(x) .exp_errno = x, .errno_desc = #x

#define MAX_NAME_LENGTH _UTSNAME_DOMAIN_LENGTH - 1

struct test_case {
	char *desc;
	char *name;
	int len;
	int exp_errno;
	char *errno_desc;
} tcases[] = {
	{ "len == -1", TST_VALID_DOMAIN_NAME, -1, ERRNO_DESC(EINVAL) },
	{ "len > allowed maximum", TST_VALID_DOMAIN_NAME, MAX_NAME_LENGTH + 1, ERRNO_DESC(EINVAL) },
	{ "name == NULL", NULL, MAX_NAME_LENGTH, ERRNO_DESC(EFAULT) }
};

void verify_setdomainname(unsigned int nr)
{
	struct test_case *tcase = &tcases[nr];

	TEST(do_setdomainname(tcase->name, (size_t) tcase->len));

	tst_res(TINFO, "testing %s", tcase->desc);
	if (TST_RET != -1) {
		tst_res(TFAIL, "unexpected exit code: %ld", TST_RET);
		return;
	}

	if (TST_ERR != tcase->exp_errno) {
		tst_res(TFAIL | TTERRNO, "unexpected errno: %d, expected: %d",
			TST_ERR, tcase->exp_errno);
		return;
	}

	tst_res(TPASS | TTERRNO, "expected failure");
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.needs_root = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test = verify_setdomainname,
	.test_variants = TEST_VARIANTS,
};
