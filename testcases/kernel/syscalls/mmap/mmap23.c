// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that :manpage:`mmap(2)` with the MAP_32BIT flag fails with ENOMEM
 * when requesting a mapping of size >= 2GB, since it cannot fit within the
 * first 2GB of address space.
 *
 * MAP_32BIT is supported only on x86-64 for 64-bit programs.
 */

#include "tst_test.h"
#include "lapi/mmap.h"

#define MAP_SZ (2UL * TST_GB)

static void run(void)
{
	TST_EXP_FAIL_PTR_VOID(mmap(NULL, MAP_SZ, PROT_READ | PROT_WRITE,
				   MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0),
			      ENOMEM,
			      "mmap(2GB) with MAP_32BIT");

	if (TST_RET_PTR != MAP_FAILED)
		SAFE_MUNMAP(TST_RET_PTR, MAP_SZ);
}

static struct tst_test test = {
	.test_all = run,
	.supported_archs = (const char *const []){
		"x86_64",
		NULL
	},
};
