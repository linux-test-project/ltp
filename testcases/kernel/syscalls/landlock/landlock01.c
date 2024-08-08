// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * This test verifies that landlock_create_ruleset syscall fails with the right
 * error codes:
 *
 * - EINVAL Unknown flags, or unknown access, or too small size
 * - E2BIG size is too big
 * - EFAULT attr was not a valid address
 * - ENOMSG Empty accesses (i.e., attr->handled_access_fs is 0)
 */

#include "landlock_common.h"

static struct landlock_ruleset_attr *ruleset_attr;
static struct landlock_ruleset_attr *null_attr;
static size_t rule_size;
static size_t rule_small_size;
static size_t rule_big_size;

static struct tcase {
	struct landlock_ruleset_attr **attr;
	uint64_t access_fs;
	size_t *size;
	uint32_t flags;
	int exp_errno;
	char *msg;
} tcases[] = {
	{&ruleset_attr, -1, &rule_size, 0, EINVAL, "Unknown access"},
	{&ruleset_attr, 0, &rule_small_size, 0, EINVAL, "Size is too small"},
	{&ruleset_attr, 0, &rule_size, -1, EINVAL, "Unknown flags"},
	{&ruleset_attr, 0, &rule_big_size, 0, E2BIG, "Size is too big"},
	{&null_attr,    0, &rule_size, 0, EFAULT, "Invalid attr address"},
	{&ruleset_attr, 0, &rule_size, 0, ENOMSG, "Empty accesses"},
};

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	if (*tc->attr)
		(*tc->attr)->handled_access_fs = tc->access_fs;

	TST_EXP_FAIL(tst_syscall(__NR_landlock_create_ruleset,
			*tc->attr, *tc->size, tc->flags),
		tc->exp_errno,
		"%s",
		tc->msg);

	if (TST_RET >= 0)
		SAFE_CLOSE(TST_RET);
}

static void setup(void)
{
	verify_landlock_is_enabled();

	rule_size = sizeof(struct landlock_ruleset_attr);

#ifdef HAVE_STRUCT_LANDLOCK_RULESET_ATTR_HANDLED_ACCESS_NET
	rule_small_size = rule_size - sizeof(uint64_t) - 1;
#else
	rule_small_size = rule_size - 1;
#endif

	rule_big_size = SAFE_SYSCONF(_SC_PAGESIZE) + 1;
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.needs_root = 1,
	.bufs = (struct tst_buffers []) {
		{&ruleset_attr, .size = sizeof(struct landlock_ruleset_attr)},
		{},
	},
	.caps = (struct tst_cap []) {
		TST_CAP(TST_CAP_REQ, CAP_SYS_ADMIN),
		{}
	},
};
