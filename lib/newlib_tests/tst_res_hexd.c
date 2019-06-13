// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project
 */

#include <stdio.h>
#include "tst_test.h"

static void do_test(void)
{
	char tmp[] = "Hello from tst_res_hexd";

	tst_res_hexd(TPASS, tmp, sizeof(tmp), "%s%d", "dump", 1);
}

static struct tst_test test = {
	.test_all = do_test,
};
