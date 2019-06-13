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
 * 2) getcwd(2) should return NULL and set errno to ENOMEM.
 * 3) getcwd(2) should return NULL and set errno to EINVAL.
 * 4) getcwd(2) should return NULL and set errno to ERANGE.
 * 5) getcwd(2) should return NULL and set errno to ERANGE.
 *
 */

#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include "tst_test.h"

static char buffer[5];

static struct t_case {
	char *buf;
	size_t size;
	int exp_err;
} tcases[] = {
	{(void *)-1, PATH_MAX, EFAULT},
	{NULL, (size_t)-1, ENOMEM},
	{buffer, 0, EINVAL},
	{buffer, 1, ERANGE},
	{NULL, 1, ERANGE}
};

static void verify_getcwd(unsigned int n)
{
	struct t_case *tc = &tcases[n];
	char *res;

	errno = 0;
	res = getcwd(tc->buf, tc->size);
	TST_ERR = errno;
	if (res) {
		tst_res(TFAIL, "getcwd() succeeded unexpectedly");
		return;
	}

	if (TST_ERR != tc->exp_err) {
		tst_res(TFAIL | TTERRNO, "getcwd() failed unexpectedly, expected %s",
			tst_strerrno(tc->exp_err));
		return;
	}

	tst_res(TPASS | TTERRNO, "getcwd() failed as expected");
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_getcwd
};
