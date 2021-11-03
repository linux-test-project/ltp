// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies, 2002. All Rights Reserved.
 * AUTHOR: Suresh Babu V. <suresh.babu@wipro.com>
 */

/*\
 * [Description]
 *
 * Test for checking error conditions for getrlimit(2)
 *   1) getrlimit(2) returns -1 and sets errno to EFAULT if an invalid
 *	address is given for address parameter.
 *   2) getrlimit(2) returns -1 and sets errno to EINVAL if an invalid
 *	resource type (RLIM_NLIMITS is a out of range resource type) is
 *	passed.
 */

#include <sys/resource.h>
#include "tst_test.h"

#define RLIMIT_TOO_HIGH 1000

static struct rlimit rlim;

static struct tcase {
	int exp_errno;		/* Expected error no            */
	char *exp_errval;	/* Expected error value string  */
	struct rlimit *rlim;	/* rlimit structure             */
	int res_type;		/* resource type                */
} tcases[] = {
	{ EINVAL, "EINVAL", &rlim, RLIMIT_TOO_HIGH}
};

static void verify_getrlimit(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TEST(getrlimit(tc->res_type, tc->rlim));

	if ((TST_RET == -1) && (TST_ERR == tc->exp_errno)) {
		tst_res(TPASS, "expected failure; got %s",
			 tc->exp_errval);
	} else {
		tst_res(TFAIL, "call failed to produce "
			 "expected error;  errno: %d : %s",
			 TST_ERR, strerror(TST_ERR));
	}
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_getrlimit,
};
