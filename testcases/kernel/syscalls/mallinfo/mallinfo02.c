// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

/*\
 * Basic mallinfo() test for malloc() using sbrk or mmap.
 * It size > MMAP_THRESHOLD, it will use mmap. Otherwise, use sbrk.
 */

#include "mallinfo_common.h"
#include "tst_safe_macros.h"

#ifdef HAVE_MALLINFO
void test_mallinfo(void)
{
	struct mallinfo info;
	int size;
	char *buf;

	buf = SAFE_MALLOC(20480);

	info = mallinfo();
	if (info.uordblks > 20480 && info.hblkhd == 0) {
		tst_res(TPASS, "malloc() uses sbrk when size < 128k");
	} else {
		tst_res(TFAIL, "malloc() use mmap when size < 128k");
		print_mallinfo("Test malloc(20480)", &info);
	}
	free(buf);

	info = mallinfo();
	size = MAX(info.fordblks, 131072);

	buf = SAFE_MALLOC(size);
	info = mallinfo();
	if (info.hblkhd > size && info.hblks > 0) {
		tst_res(TPASS, "malloc() uses mmap when size >= 128k");
	} else {
		tst_res(TFAIL, "malloc uses sbrk when size >= 128k");
		print_mallinfo("Test malloc(1024*128)", &info);
	}

	free(buf);
}

static void setup(void)
{
	if (mallopt(M_MMAP_THRESHOLD, 131072) == 0)
		tst_res(TFAIL, "mallopt(M_MMAP_THRESHOLD, 128K) failed");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = test_mallinfo,
};

#else
TST_TEST_TCONF("system doesn't implement non-POSIX mallinfo()");
#endif
