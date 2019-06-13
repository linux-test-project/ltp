// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
 */

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_safe_net.h"

static void cleanup(void)
{
	int i;

	tst_brk(TBROK, "TBROK in cleanup");
	SAFE_OPEN("foo", O_RDWR);
	SAFE_FILE_SCANF("foo", "%i", &i);
	SAFE_TOUCH("doo/foo", 0777, NULL);
	SAFE_FOPEN("foo", "r");
	SAFE_SOCKET(AF_UNIX, SOCK_STREAM, -1);
	tst_res(TINFO, "Test still here");
}

static void do_test(void)
{
	tst_res(TPASS, "Passed");
}

static struct tst_test test = {
	.test_all = do_test,
	.cleanup = cleanup,
};
