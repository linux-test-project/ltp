// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) Manas Kumar Nayak <maknayak@in.ibm.com>
 * Copyright (c) Cyril Hrubis <chrubis@suse.cz> 2011
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Tests set_thread_area and get_thread_area syscalls for their expected errors:
 *
 * - EINVAL u_info->entry_number is out of bounds.
 * - EFAULT u_info is an invalid pointer.
 */

#include "tst_test.h"

#include "lapi/ldt.h"

static struct user_desc *u_info;

static struct tcase {
	struct user_desc *u_info;
	int exp_errno;
} tcases[] = {
	{ NULL, EINVAL },
	{ (void *)-9, EFAULT },
};

static void run(unsigned int i)
{
	struct tcase tc = tcases[i];

	if (tst_variant)
		TST_EXP_FAIL(get_thread_area(tc.u_info), tc.exp_errno);
	else
		TST_EXP_FAIL(set_thread_area(tc.u_info), tc.exp_errno);
}

static void setup(void)
{
	/* This makes *entry invalid */
	u_info->entry_number = -2;
	tcases[0].u_info = u_info;
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.test_variants = 2,
	.supported_archs = (const char *const[]){ "x86", NULL },
	.bufs = (struct tst_buffers[]) {
			{ &u_info, .size = sizeof(struct user_desc) },
			{},
		},
};
