// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that :manpage:`sched_getattr(2)` correctly reads back the scheduling
 * attributes of a task configured with :manpage:`sched_setattr(2)`.
 *
 * Root is required (:c:macro:`CAP_SYS_NICE`) to configure the
 * :c:macro:`SCHED_DEADLINE` policy.
 *
 * The test relies on the LTP harness process isolation and resets the
 * scheduling policy to :c:macro:`SCHED_OTHER` after testing to prevent
 * :c:macro:`SCHED_DEADLINE` constraints from leaking across test iterations.
 */

#define _GNU_SOURCE

#include "tst_test.h"
#include "lapi/sched.h"

#define RUNTIME_VAL 10000000
#define PERIOD_VAL 30000000
#define DEADLINE_VAL 30000000

static struct sched_attr *read_attr;

static void reset_sched(void)
{
	struct sched_attr normal = {
		.size = sizeof(normal),
		.sched_policy = SCHED_OTHER,
	};

	SAFE_SCHED_SETATTR(0, &normal, 0);
}

static void run(void)
{
	struct sched_attr attr = {
		.size = sizeof(attr),
		.sched_policy = SCHED_DEADLINE,
		.sched_runtime = RUNTIME_VAL,
		.sched_deadline = DEADLINE_VAL,
		.sched_period = PERIOD_VAL,
	};

	SAFE_SCHED_SETATTR(0, &attr, 0);

	memset((void *)read_attr, 0, sizeof(*read_attr));

	TST_EXP_PASS(sched_getattr(0, read_attr, sizeof(*read_attr), 0),
		     "sched_getattr() with valid parameters");
	if (!TST_PASS)
		return;

	TST_EXP_EQ_LU(read_attr->sched_policy, SCHED_DEADLINE);
	TST_EXP_EQ_LU(read_attr->sched_runtime, RUNTIME_VAL);
	TST_EXP_EQ_LU(read_attr->sched_deadline, DEADLINE_VAL);
	TST_EXP_EQ_LU(read_attr->sched_period, PERIOD_VAL);

	reset_sched();
}

static struct tst_test test = {
	.test_all = run,
	.cleanup = reset_sched,
	.needs_root = 1,
	.bufs = (struct tst_buffers []) {
	    {&read_attr, .size = sizeof(*read_attr)},
	    {},
	},
};
