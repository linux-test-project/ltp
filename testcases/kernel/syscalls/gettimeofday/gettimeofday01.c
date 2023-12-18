// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *   Copyright (c) Linux Test Project, 2001-2023
 */

/*\
 * DESCRIPTION
 *
 * Test for EFAULT error.
 *
 * - gettimeofday fail with EFAULT when one of tv or tz pointed outside the accessible
 *   address space
 */

#include "tst_test.h"
#include "lapi/syscalls.h"

static void verify_gettimeofday(void)
{
	TST_EXP_FAIL(tst_syscall(__NR_gettimeofday, (void *)-1, (void *)-1), EFAULT);
}


static struct tst_test test = {
	.test_all  = verify_gettimeofday,
};
