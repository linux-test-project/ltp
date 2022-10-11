// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: Zhao Gongyi <zhaogongyi@huawei.com>
 */

/*\
 * [Description]
 *
 * Check that successful madvise(2) MADV_DONTNEED operation will result in
 * zero-fill-on-demand pages for anonymous private mappings.
 */

#include "tst_test.h"

#define MAP_SIZE (8 * 1024)

static char *addr;

static void run(void)
{
	int i;

	memset(addr, 1, MAP_SIZE);

	TEST(madvise(addr, MAP_SIZE, MADV_DONTNEED));
	if (TST_RET == -1) {
		tst_brk(TBROK | TTERRNO, "madvise(%p, %d, 0x%x) failed",
			addr, MAP_SIZE, MADV_DONTNEED);
	}

	for (i = 0; i < MAP_SIZE; i++) {
		if (addr[i]) {
			tst_res(TFAIL,
				"There are no zero-fill-on-demand pages "
				"for anonymous private mappings");
			return;
		}
	}

	if (i == MAP_SIZE) {
		tst_res(TPASS,
			"There are zero-fill-on-demand pages "
			"for anonymous private mappings");
	}
}

static void setup(void)
{
	addr = SAFE_MMAP(NULL, MAP_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS,
			-1, 0);
}

static void cleanup(void)
{
	if (addr)
		SAFE_MUNMAP(addr, MAP_SIZE);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
};
