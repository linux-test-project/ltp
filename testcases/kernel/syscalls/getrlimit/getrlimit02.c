// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies, 2002. All Rights Reserved.
 * AUTHOR: Suresh Babu V. <suresh.babu@wipro.com>
 */

/*\
 * Test for checking error conditions for getrlimit(2):
 *
 * 1. getrlimit(2) returns -1 and sets errno to EFAULT if an invalid
 *	  address is given for address parameter.
 * 2. getrlimit(2) returns -1 and sets errno to EINVAL if an invalid
 *	  resource type (RLIM_NLIMITS is a out of range resource type) is
 *	  passed.
 */

#include <sys/resource.h>
#include "tst_test.h"

#define INVALID_RES_TYPE 1000

static struct rlimit rlim;

static struct tcase {
	int exp_errno;
	char *desc;
	struct rlimit *rlim;
	int res_type;
} tcases[] = {
	{EFAULT, "invalid address", (void *)-1, RLIMIT_CORE},
	{EINVAL, "invalid resource type", &rlim, INVALID_RES_TYPE}
};

static void verify_getrlimit(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(getrlimit(tc->res_type, tc->rlim),
				tc->exp_errno,
				"getrlimit() with %s",
				tc->desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_getrlimit,
};
