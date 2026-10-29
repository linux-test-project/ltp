// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   01/02/2003	Port to LTP	avenkat@us.ibm.com
 *   06/30/2001	Port to Linux	nsharoff@us.ibm.com
 */

/*\
 * The testcase for test setting of buffer by check boundary conditions.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "tst_test.h"

#define BSIZE 4096

char buf[BSIZE];

static void fill(void)
{
	register int i;

	for (i = 0; i < BSIZE; i++)
		buf[i] = 'a';
}

static int checkit(char *str)
{
	register int i = 0;

	while (!*str++)
		i++;

	return i;
}

static void setup(void)
{
	fill();
}

static void verify_memset(void)
{
	register int i, j;
	char *p = &buf[400];

	for (i = 0; i < 200; i++) {
		fill();
		memset(p, 0, i);
		if ((j = checkit(p)) != i) {
			tst_res(TINFO, "Not enough zero bytes, wanted %d, got %d", i, j);
			break;
		}
		if (!p[-1] || !p[i]) {
			tst_res(TINFO, "Boundary error, clear of %d", i);
			break;
		}
	}

	if (i == 200)
		tst_res(TPASS, "Test passed");
	else
		tst_res(TFAIL, "Test fails");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_memset,
};
