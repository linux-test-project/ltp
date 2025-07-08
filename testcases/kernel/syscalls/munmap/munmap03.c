// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	07/2001 Ported by Wayne Boyer
 * Copyright (c) 2025 SUSE LLC Ricardo B. Marlière <rbm@suse.com>
 */

/*\
 * Verify that, munmap() fails with errno:
 *
 * - EINVAL, if addresses in the range [addr,addr+len) are outside the valid
 *   range for the address space of a process.
 * - EINVAL, if the len argument is 0.
 * - EINVAL, if the addr argument is not a multiple of the page size as
 *   returned by sysconf().
 */

#include "tst_test.h"

static size_t page_sz;
static char *map_addr;
static char *map_addr_out;
static size_t map_len;
static size_t map_len_zero;

static struct tcase {
	int exp_errno;
	char **addr;
	size_t *len;
} tcases[] = {
	{ EINVAL, &map_addr_out, &map_len },
	{ EINVAL, &map_addr, &map_len_zero },
	{ EINVAL, &map_addr + 1, &map_len },
};

static void run(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	TST_EXP_FAIL(munmap(tc->addr, *tc->len), tc->exp_errno);
}

static void setup(void)
{
	struct rlimit brkval;

	page_sz = SAFE_SYSCONF(_SC_PAGESIZE);
	map_len = page_sz * 2;
	map_addr = SAFE_MMAP(NULL, map_len, PROT_READ | PROT_WRITE,
			     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	SAFE_GETRLIMIT(RLIMIT_DATA, &brkval);
	map_addr_out = (char *)brkval.rlim_max;
}

static void cleanup(void)
{
	if (map_addr)
		SAFE_MUNMAP(map_addr, map_len);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.cleanup = cleanup,
};
