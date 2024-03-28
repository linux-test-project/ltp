// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) International Business Machines  Corp., 2001
 *
 *  HISTORY
 *	07/2001 John George - Ported
 * Copyright (C) 2024 SUSE LLC Andrea Manzini <andrea.manzini@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that sigaltstack() fails with return value -1 and set expected errno:
 *
 * - EINVAL on invalid value
 * - ENOMEM on stack is < MINSIGSTKSZ.
 */

#include <stdlib.h>
#include "tst_test.h"

#define INVAL_FLAGS	9999

static stack_t sigstk;

static struct test_case {
	int flag;
	int size;
	char *desc;
	int exp_errno;
} tcases[] = {
	{INVAL_FLAGS, SIGSTKSZ, "Invalid Flag value", EINVAL},
	/* use value low enough for all kernel versions
	 * avoid using MINSIGSTKSZ defined by glibc as it could be different
	 * from the one in kernel ABI
	 */
	{0, (2048 - 1), "alternate stack is < MINSIGSTKSZ", ENOMEM} };

static void check_sigaltstack(unsigned int nr)
{
	struct test_case *tc = &tcases[nr];

	sigstk.ss_size = tc->size;
	sigstk.ss_flags = tc->flag;
	TST_EXP_FAIL(sigaltstack(&sigstk, NULL), tc->exp_errno, "%s", tc->desc);
}

static void setup(void)
{
	sigstk.ss_sp = SAFE_MALLOC(SIGSTKSZ);
}

static void cleanup(void)
{
	free(sigstk.ss_sp);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = check_sigaltstack,
	.tcnt = ARRAY_SIZE(tcases),
	.needs_tmpdir = 1
};
