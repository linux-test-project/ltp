// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) International Business Machines  Corp., 2005
 *   Robbie Williamson <robbiew@us.ibm.com>
 *
 * Copyright (c) Wipro Technologies Ltd, 2005.  All Rights Reserved.
 *   Prashant P Yendigeri <prashant.yendigeri@wipro.com>
 *
 * Copyright (c) 2022 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that getpagesize(2) returns the number of bytes in a
 * memory page as expected.
 */

#include "tst_test.h"

static void run(void)
{
	int pagesize_sysconf;

	pagesize_sysconf = sysconf(_SC_PAGESIZE);
	TST_EXP_VAL(getpagesize(), pagesize_sysconf);
}

static struct tst_test test = {
	.test_all = run
};
