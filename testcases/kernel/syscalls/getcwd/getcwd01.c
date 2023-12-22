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
 * linux syscall
 * 1) getcwd(2) should return NULL and set errno to EFAULT.
 * 2) getcwd(2) should return NULL and set errno to EFAULT.
 * 3) getcwd(2) should return NULL and set errno to ERANGE.
 * 4) getcwd(2) should return NULL and set errno to ERANGE.
 * 5) getcwd(2) should return NULL and set errno to ERANGE.
 *
 * glibc
 * 1) getcwd(2) should return NULL and set errno to EFAULT.
 * 2) getcwd(2) should return NULL and set errno to ENOMEM.
 * 3) getcwd(2) should return NULL and set errno to EINVAL.
 * 4) getcwd(2) should return NULL and set errno to ERANGE.
 * 5) getcwd(2) should return NULL and set errno to ERANGE.
 */

#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include "tst_test.h"
#include "getcwd.h"

static char buffer[5];

static struct t_case {
	char *buf;
	size_t size;
	int exp_err;
	int exp_err2;
} tcases[] = {
	{(void *)-1, PATH_MAX, EFAULT, EFAULT},
	{NULL, (size_t)-1, EFAULT, ENOMEM},
	{buffer, 0, ERANGE, EINVAL},
	{buffer, 1, ERANGE, ERANGE},
	{NULL, 1, ERANGE, ERANGE},
};

static void run(unsigned int n)
{
	struct t_case *tc = &tcases[n];

	tst_getcwd(tc->buf, tc->size, tc->exp_err, tc->exp_err2);
}

static void setup(void)
{
	getcwd_info();
}

static struct tst_test test = {
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.test_variants = TEST_VARIANTS,
};
