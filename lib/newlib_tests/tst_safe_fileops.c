// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Linux Test Project
 */

#include <stdio.h>
#include "tst_test.h"

static void do_test(void)
{
	long free;
	long nproc;
	long dummy;

	SAFE_FILE_LINES_SCANF("/proc/meminfo", "MemFree: %ld", &free);
	if (FILE_LINES_SCANF("/proc/stat", "processes %ld", &nproc))
		tst_brk(TBROK, "Could not parse processes");
	tst_res(TPASS, "Free: %ld, nproc: %ld", free, nproc);

	if (FILE_LINES_SCANF("/proc/stat", "non-existent %ld", &dummy))
		tst_res(TPASS, "non-existent not found");
	SAFE_FILE_LINES_SCANF("/proc/stat", "non-existent %ld", &dummy);
}

static struct tst_test test = {
	.test_all = do_test,
};
