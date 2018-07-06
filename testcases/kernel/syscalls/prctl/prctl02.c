// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *
 * 1) prctl() fails with EINVAL when an invalid value is given for option
 * 2) prctl() fails with EINVAL when option is PR_SET_PDEATHSIG & arg2 is
 * not zero or a valid signal number
 */

#include <errno.h>
#include <signal.h>
#include <sys/prctl.h>

#include "tst_test.h"

#define OPTION_INVALID 999
#define INVALID_ARG 999

static struct tcase {
	int option;
	unsigned long arg2;
	int exp_errno;
} tcases[] = {
	{OPTION_INVALID, 0, EINVAL},
	{PR_SET_PDEATHSIG, INVALID_ARG, EINVAL},
};

static void verify_prctl(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(prctl(tc->option, tc->arg2));
	if (TST_RET == 0) {
		tst_res(TFAIL, "prctl() succeeded unexpectedly");
		return;
	}

	if (tc->exp_errno == TST_ERR) {
		tst_res(TPASS | TTERRNO, "prctl() failed as expected");
	} else {
		tst_res(TPASS | TTERRNO, "prctl() failed unexpectedly, expected %s",
				tst_strerrno(tc->exp_errno));
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_prctl,
};
