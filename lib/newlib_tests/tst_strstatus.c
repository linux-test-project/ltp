// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Basic unit test for the tst_strstatus() function.
 */

#include <string.h>
#include "tst_test.h"

static struct tcase {
	int status;
	const char *str;
} tcases[] = {
	{0x0100, "exited with 1"},
	{0x0001, "killed by SIGHUP"},
	{0x137f, "is stopped"},
	{0xffff, "is resumed"},
	{0xff, "invalid status 0xff"},
};

static void do_test(unsigned int n)
{
	const char *str_status = tst_strstatus(tcases[n].status);

	if (strcmp(str_status, tcases[n].str))
		tst_res(TFAIL, "%s != %s", str_status, tcases[n].str);
	else
		tst_res(TPASS, "%s", str_status);
}

static struct tst_test test = {
	.test = do_test,
	.tcnt = ARRAY_SIZE(tcases),
};
