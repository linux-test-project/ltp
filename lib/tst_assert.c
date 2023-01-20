// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2020 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 * Copyright (c) 2020 Cyril Hrubis <chrubis@suse.cz>
 */
#include <stdio.h>
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

void tst_assert_ulong(const char *file, const int lineno, const char *path, unsigned long val)
{
	unsigned long sys_val;

	safe_file_scanf(file, lineno, NULL, path, "%lu", &sys_val);

	if (val == sys_val) {
		tst_res_(file, lineno, TPASS, "%s = %lu", path, val);
		return;
	}

	tst_res_(file, lineno, TFAIL, "%s != %lu got %lu", path, val, sys_val);
}

void tst_assert_file_int(const char *file, const int lineno, const char *path, const char *prefix, int val)
{
	int sys_val;
	char fmt[1024];

	snprintf(fmt, sizeof(fmt), "%s%%d", prefix);
	file_lines_scanf(file, lineno, NULL, 1, path, fmt, &sys_val);

	if (val == sys_val) {
		tst_res_(file, lineno, TPASS, "%s %s = %d", path, prefix, sys_val);
		return;
	}

	tst_res_(file, lineno, TFAIL, "%s %s != %d got %d", path, prefix, val, sys_val);
}

void tst_assert_str(const char *file, const int lineno, const char *path, const char *val)
{
	char sys_val[1024];

	safe_file_scanf(file, lineno, NULL, path, "%1023s", sys_val);
	if (!strcmp(val, sys_val)) {
		tst_res_(file, lineno, TPASS, "%s = '%s'", path, val);
		return;
	}

	tst_res_(file, lineno, TFAIL, "%s != '%s' got '%s'", path, val, sys_val);
}

void tst_assert_file_str(const char *file, const int lineno, const char *path, const char *prefix, const char *val)
{
	char sys_val[1024];
	char fmt[2048];

	snprintf(fmt, sizeof(fmt), "%s: %%1023s", prefix);
	file_lines_scanf(file, lineno, NULL, 1, path, fmt, sys_val);

	if (!strcmp(val, sys_val)) {
		tst_res_(file, lineno, TPASS, "%s %s = '%s'", path, prefix, sys_val);
		return;
	}

	tst_res_(file, lineno, TFAIL, "%s %s != '%s' got '%s'", path, prefix, val, sys_val);
}
