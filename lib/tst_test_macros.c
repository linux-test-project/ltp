// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Cyril Hrubis <chrubis@suse.cz>
 */

#include <stdio.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_test_macros.h"

bool tst_errno_in_set(int err, const int *exp_errs, int exp_errs_cnt)
{
	int i;

	for (i = 0; i < exp_errs_cnt; i++) {
		if (err == exp_errs[i])
			return 1;
	}

	return 0;
}

const char *tst_errno_names(char *buf, const int *exp_errs, int exp_errs_cnt)
{
	int i;
	char *cb = buf;

	for (i = 0; i < exp_errs_cnt-1; i++)
		cb += sprintf(cb, "%s, ", tst_strerrno(exp_errs[i]));

	cb += sprintf(cb, "%s", tst_strerrno(exp_errs[i]));

	*cb = '\0';

	return buf;
}
