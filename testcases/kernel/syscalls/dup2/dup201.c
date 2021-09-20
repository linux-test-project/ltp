// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * 01/2002 Removed EMFILE test - Paul Larson
 */
/*\
 * [Description]
 *
 * Negative tests for dup2() with bad fd (EBADF).
 *
 * - First fd argument is less than 0
 * - First fd argument is getdtablesize()
 * - Second fd argument is less than 0
 * - Second fd argument is getdtablesize()
 *
 */

#include <errno.h>
#include <unistd.h>
#include "tst_test.h"

static int maxfd, mystdout;
static int goodfd = 5;
static int badfd = -1;

static struct tcase {
	int *ofd;
	int *nfd;
} tcases[] = {
	{&badfd, &goodfd},
	{&maxfd, &goodfd},
	{&mystdout, &badfd},
	{&mystdout, &maxfd},
};

static void setup(void)
{
	/* get some test specific values */
	maxfd = getdtablesize();
}

static void run(unsigned int i)
{
	struct tcase *tc = tcases + i;

	TST_EXP_FAIL2(dup2(*tc->ofd, *tc->nfd), EBADF,
			"dup2(%d, %d)", *tc->ofd, *tc->nfd);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
	.setup = setup,
};
