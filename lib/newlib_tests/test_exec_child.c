// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

int main(void)
{
	tst_reinit();
	tst_res(TPASS, "Child passed!");
	return 0;
}
