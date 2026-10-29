// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * Tests that all paths shorter than PATH_MAX work fine.
 */

#include "tst_test.h"

static char *path;

static void verify_chdir(void)
{
	int i, fails = 0;

	memset(path, 0, PATH_MAX);

	for (i = 1; i < PATH_MAX; i++) {
		memset(path, '/', i);

		TST_EXP_PASS_SILENT(chdir(path), "chdir('/' * %i)", i);

		fails += !TST_PASS;
	}

	if (!fails)
		tst_res(TPASS, "Path lengths <1,%i> worked correctly", PATH_MAX);
}

static struct tst_test test = {
	.test_all = verify_chdir,
	.bufs = (struct tst_buffers []) {
		{&path, .size = PATH_MAX},
		{}
	}
};
