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
	const char *note;
} tcases[] = {
	{FLAG(TPASS)},
	{FLAG(TFAIL)},
	{FLAG(TBROK)},
	{FLAG(TCONF)},
	{FLAG(TWARN)},
	{FLAG(TINFO)},
	{FLAG(TDEBUG), " (printed only with -D or TST_ENABLE_DEBUG=1)"},
};

static void do_cleanup(void)
{
	tst_brk(TBROK, "TBROK message should be TWARN in cleanup");
}

static void do_test(void)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(tcases); i++)
		tst_res(tcases[i].flag, "%s message%s", tcases[i].str,
			tcases[i].note ?: "");
}

static struct tst_test test = {
	.test_all = do_test,
	.cleanup = do_cleanup,
};
