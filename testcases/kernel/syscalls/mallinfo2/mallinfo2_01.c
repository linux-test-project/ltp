// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@fujitsu.com>
 */

/*\
 * [Description]
 *
 * Basic mallinfo2() test.
 *
 * Test hblkhd member of struct mallinfo2 whether overflow when setting 2G size.
 *
 * Deprecated mallinfo() overflow in this case, that was the point for creating
 * mallinfo2().
 */

#include "mallinfo_common.h"
#include "tst_safe_macros.h"

#ifdef HAVE_MALLINFO2

void test_mallinfo2(void)
{
	struct mallinfo2 info;
	char *buf;
	size_t size = 2UL * 1024UL * 1024UL * 1024UL;

	LTP_VAR_USED(buf) = malloc(size);

	if (!buf)
		tst_brk(TCONF, "Current system can not malloc 2G space, skip it");

	info = mallinfo2();
	if (info.hblkhd < size) {
		print_mallinfo2("Test malloc 2G", &info);
		tst_res(TFAIL, "hblkhd member of struct mallinfo2 overflow?");
	} else {
		tst_res(TPASS, "hblkhd member of struct mallinfo2 doesn't overflow");
	}

	free(buf);
}

static struct tst_test test = {
	.test_all = test_mallinfo2,
};

#else
TST_TEST_TCONF("system doesn't implement non-POSIX mallinfo2()");
#endif
