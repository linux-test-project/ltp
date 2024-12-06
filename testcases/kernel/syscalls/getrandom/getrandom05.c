// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 FUJITSU LIMITED. All Rights Reserved.
 * Copyright (c) Linux Test Project, 2024
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * [Description]
 *
 * Verify that getrandom(2) fails with
 *
 * - EFAULT when buf address is outside the accessible address space
 * - EINVAL when flag is invalid
 */

#include "tst_test.h"
#include "lapi/getrandom.h"
#include "getrandom_var.h"

static char buff_efault[64];
static char buff_einval[64];

static struct test_case_t {
	char *buff;
	size_t size;
	unsigned int flag;
	int expected_errno;
	char *desc;
} tcases[] = {
	{(void *)-1, sizeof(buff_efault), 0, EFAULT,
		"buf address is outside the accessible address space"},
	{buff_einval, sizeof(buff_einval), -1, EINVAL, "flag is invalid"},
};

static void setup(void)
{
	getrandom_info();
}

static void verify_getrandom(unsigned int i)
{
	struct test_case_t *tc = &tcases[i];

	/* EFAULT test can segfault on recent glibc, skip it */
	if (tst_variant == 1 && tc->expected_errno == EFAULT) {
		tst_res(TCONF, "Skipping EFAULT test for libc getrandom()");
		return;
	}

	TST_EXP_FAIL2(do_getrandom(tc->buff, tc->size, tc->flag),
		tc->expected_errno, "%s", tc->desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_getrandom,
	.test_variants = TEST_VARIANTS,
	.setup = setup,
};
