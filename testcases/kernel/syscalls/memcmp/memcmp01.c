// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   01/02/2003	Port to LTP	avenkat@us.ibm.com
 *   06/30/2001	Port to Linux	nsharoff@us.ibm.com
 */

/*\
 * [Description]
 *
 * The testcase for buffer comparison by check boundary conditions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "tst_test.h"

#define BSIZE	4096
#define LEN	100

char buf[BSIZE];

static struct test_case {
	char *p;
	char *q;
	int len;
} tcases[] = {
	{&buf[100], &buf[800], LEN},
	{&buf[800], &buf[100], LEN},
};

static void fill(char *str, int len)
{
	register int i;
	for (i = 0; i < len; i++)
		*str++ = 'a';
}

static void setup(void)
{
	register int i;

	for (i = 0; i < BSIZE; i++)
		buf[i] = 0;

	return;
}

static void verify_memcmp(char *p, char *q, int len)
{
	fill(p, len);
	fill(q, len);

	if (memcmp(p, q, len)) {
		tst_res(TFAIL, "memcmp fails - should have succeeded.");
		goto out;
	}

	p[len - 1] = 0;

	if (memcmp(p, q, len) >= 0) {
		tst_res(TFAIL, "memcmp succeeded - should have failed.");
		goto out;
	};

	p[len - 1] = 'a';
	p[0] = 0;

	if (memcmp(p, q, len) >= 0) {
		tst_res(TFAIL, "memcmp succeeded - should have failed.");
		goto out;
	};

	p[0] = 'a';
	q[len - 1] = 0;

	if (memcmp(p, q, len) <= 0) {
		tst_res(TFAIL, "memcmp succeeded - should have failed.");
		goto out;
	};

	q[len - 1] = 'a';
	q[0] = 0;

	if (memcmp(p, q, len) <= 0) {
		tst_res(TFAIL, "memcmp succeeded - should have failed.");
		goto out;
	};

	q[0] = 'a';

	if (memcmp(p, q, len)) {
		tst_res(TFAIL, "memcmp fails - should have succeeded.");
		goto out;
	}

	tst_res(TPASS, "Test passed");
out:
	return;
}

static void run_test(unsigned int nr)
{
	struct test_case *tc = &tcases[nr];

	verify_memcmp(tc->p, tc->q, tc->len);

	return;
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.test = run_test,
};
