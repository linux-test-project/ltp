// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that lsm_list_modules syscall is raising errors when invalid data is
 * provided.
 */

#include "lsm_common.h"

#define MAX_LSM_NUM 32

static uint64_t lsm_ids[MAX_LSM_NUM];
static uint32_t page_size;
static uint32_t ids_size;
static uint32_t ids_size_small;

static struct tcase {
	uint64_t *ids;
	uint32_t *size;
	uint32_t flags;
	int exp_errno;
	char *msg;
} tcases[] = {
	{
		.size = &ids_size,
		.exp_errno = EFAULT,
		.msg = "ids is NULL",
	},
	{
		.ids = lsm_ids,
		.exp_errno = EFAULT,
		.msg = "size is NULL",
	},
	{
		.ids = lsm_ids,
		.size = &ids_size_small,
		.exp_errno = E2BIG,
		.msg = "size is too small",
	},
	{
		.ids = lsm_ids,
		.size = &ids_size,
		.flags = 1,
		.exp_errno = EINVAL,
		.msg = "flags must be zero",
	},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	memset(lsm_ids, 0, sizeof(lsm_ids));
	ids_size = page_size;
	ids_size_small = 0;

	TST_EXP_FAIL(lsm_list_modules(tc->ids, tc->size, tc->flags),
	      tc->exp_errno,
	      "%s", tc->msg);
}

static void setup(void)
{
	page_size = SAFE_SYSCONF(_SC_PAGESIZE);
}

static struct tst_test test = {
	.test = run,
	.setup = setup,
	.tcnt = ARRAY_SIZE(tcases),
	.min_kver = "6.8",
};
