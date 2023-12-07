// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 Petr Vorel <pvorel@suse.cz>
 */

/*
 * Test tst_res() flags.
 */

#include "tst_test.h"

#define FLAG(x) .flag = x, .str = #x
static struct tcase {
	int flag;
	const char *str;
} tcases[] = {
	{FLAG(TPASS)},
	{FLAG(TFAIL)},
	{FLAG(TBROK)},
	{FLAG(TCONF)},
	{FLAG(TWARN)},
	{FLAG(TINFO)},
};

static void do_cleanup(void)
{
	tst_brk(TBROK, "TBROK message should be TWARN in cleanup");
}

static void do_test(void)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(tcases); i++)
		tst_res(tcases[i].flag, "%s message", tcases[i].str);
}

static struct tst_test test = {
	.test_all = do_test,
	.cleanup = do_cleanup,
};
