// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 */

/*
 * DESCRIPTION
 * Testcase to test that getcwd(2) sets errno correctly.
 * 1) getcwd(2) fails if buf points to a bad address.
 * 2) getcwd(2) fails if the size is invalid.
 * 3) getcwd(2) fails if the size is set to 0.
 * 4) getcwd(2) fails if the size is set to 1.
 * 5) getcwd(2) fails if buf points to NULL and the size is set to 1.
 *
 * Expected Result:
 * 1) getcwd(2) should return NULL and set errno to EFAULT.
 * 2) getcwd(2) should return NULL and set errno to EFAULT.
 * 3) getcwd(2) should return NULL and set errno to ERANGE.
 * 4) getcwd(2) should return NULL and set errno to ERANGE.
 * 5) getcwd(2) should return NULL and set errno to ERANGE.
 */

#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

static char buffer[5];

static struct t_case {
	char *buf;
	size_t size;
	int exp_err;
} tcases[] = {
	{(void *)-1, PATH_MAX, EFAULT},
	{NULL, (size_t)-1, EFAULT},
	{buffer, 0, ERANGE},
	{buffer, 1, ERANGE},
	{NULL, 1, ERANGE}
};


static void verify_getcwd(unsigned int n)
{
	struct t_case *tc = &tcases[n];

	TST_EXP_FAIL2(tst_syscall(__NR_getcwd, tc->buf, tc->size), tc->exp_err);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_getcwd
};
