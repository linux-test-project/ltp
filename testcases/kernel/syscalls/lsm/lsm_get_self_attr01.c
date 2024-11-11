// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that lsm_get_self_attr syscall is raising errors when invalid data is
 * provided.
 */

#include "lsm_common.h"

static struct lsm_ctx *ctx;
static uint32_t ctx_size;
static uint32_t ctx_size_small;

static struct tcase {
	int attr;
	struct lsm_ctx **ctx;
	uint32_t *size;
	uint32_t flags;
	int exp_err;
	char *msg;
} tcases[] = {
	{
		.attr = LSM_ATTR_CURRENT,
		.ctx = &ctx,
		.exp_err = EINVAL,
		.msg = "size is NULL",
	},
	{
		.attr = LSM_ATTR_CURRENT,
		.ctx = &ctx,
		.size = &ctx_size,
		.flags = LSM_FLAG_SINGLE | (LSM_FLAG_SINGLE << 1),
		.exp_err = EINVAL,
		.msg = "flags is invalid",
	},
	{
		.attr = LSM_ATTR_CURRENT,
		.ctx = &ctx,
		.size = &ctx_size_small,
		.exp_err = E2BIG,
		.msg = "size is too smal",
	},
	{
		.attr = LSM_ATTR_CURRENT,
		.ctx = &ctx,
		.size = &ctx_size,
		.flags = LSM_FLAG_SINGLE,
		.exp_err = EINVAL,
		.msg = "flags force to use ctx attributes",
	},
	{
		.attr = LSM_ATTR_CURRENT | LSM_ATTR_PREV,
		.ctx = &ctx,
		.size = &ctx_size,
		.flags = 0,
		.exp_err = EOPNOTSUPP,
		.msg = "flags overset",
	}
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	memset(ctx, 0, LSM_CTX_SIZE_DEFAULT);
	ctx_size = LSM_CTX_SIZE_DEFAULT;
	ctx_size_small = 1;

	TST_EXP_FAIL(lsm_get_self_attr(
		tc->attr, *tc->ctx, tc->size, tc->flags),
		tc->exp_err,
		"%s", tc->msg);
}

static void setup(void)
{
	verify_supported_attr_current();
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.min_kver = "6.8",
	.bufs = (struct tst_buffers[]) {
		{&ctx, .size = LSM_CTX_SIZE_DEFAULT},
		{}
	},
};
