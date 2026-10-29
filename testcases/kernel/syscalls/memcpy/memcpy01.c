// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   01/02/2003	Port to LTP	avenkat@us.ibm.com
 *   06/30/2001	Port to Linux	nsharoff@us.ibm.com
 */

/*\
 * The testcase for buffer copy by check boundary conditions.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "tst_test.h"

#define BSIZE	4096
#define LEN	100

char buf[BSIZE];

static void clearit(void)
{
	register int i;

	for (i = 0; i < BSIZE; i++)
		buf[i] = 0;
}

static void fill(char *str, int len)
{
	register int i;
	for (i = 0; i < len; i++)
		*str++ = 'a';
}

static int checkit(char *str, int len)
{
	register int i;
	for (i = 0; i < len; i++)
		if (*str++ != 'a')
			return (-1);

	return 0;
}

static struct test_case {
	char *p;
	char *q;
	int len;
} tcases[] = {
	{&buf[100], &buf[800], LEN},
	{&buf[800], &buf[100], LEN},
};

static void setup(void)
{
	clearit();

	return;
}

static void verify_memcpy(char *p, char *q, int len)
{
	fill(p, len);
	memcpy(q, p, LEN);

	if (checkit(q, len)) {
		tst_res(TFAIL, "copy failed - missed data");
		goto out;
	}

	if (p[-1] || p[LEN]) {
		tst_res(TFAIL, "copy failed - 'to' bounds");
		goto out;
	}

	if (q[-1] || q[LEN]) {
		tst_res(TFAIL, "copy failed - 'from' bounds");
		goto out;
	}

	tst_res(TPASS, "Test passed");
out:
	return;
}

static void run_test(unsigned int nr)
{
	struct test_case *tc = &tcases[nr];

	clearit();
	verify_memcpy(tc->p, tc->q, tc->len);

	return;
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.test = run_test,
};
