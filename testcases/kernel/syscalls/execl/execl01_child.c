// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linux Test Project
 * Copyright (C) 2015 Cyril Hrubis chrubis@suse.cz
 */

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

int main(int argc, char *argv[])
{
	tst_reinit();

	if (argc != 2)
		tst_brk(TFAIL, "argc is %d, expected 2", argc);

	if (strcmp(argv[1], "canary"))
		tst_brk(TFAIL, "argv[1] is %s, expected 'canary'", argv[1]);

	tst_res(TPASS, "%s executed", argv[0]);

	return 0;
}
