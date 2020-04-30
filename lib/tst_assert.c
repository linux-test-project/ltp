// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */
#define TST_NO_DEFAULT_MAIN
#include "tst_assert.h"
#include "tst_test.h"

void tst_assert_int(const char *file, const int lineno, const char *path, int val)
{
	int sys_val;

	safe_file_scanf(file, lineno, NULL, path, "%d", &sys_val);

	if (val == sys_val) {
		tst_res_(file, lineno, TPASS, "%s = %d", path, val);
		return;
	}

	tst_res_(file, lineno, TFAIL, "%s != %d got %d", path, val, sys_val);
}

void tst_assert_str(const char *file, const int lineno, const char *path, const char *val)
{
	char sys_val[1024];

	safe_file_scanf(file, lineno, NULL, path, "%1024s", sys_val);
	if (!strcmp(val, sys_val)) {
		tst_res_(file, lineno, TPASS, "%s = '%s'", path, val);
		return;
	}

	tst_res_(file, lineno, TFAIL, "%s != '%s' got '%s'", path, val, sys_val);
}
