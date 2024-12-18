// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2010-2017  Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2011-2023
 */
/*\
 * [Description]
 *
 * Out Of Memory (OOM) test
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem.h"

#define OVERCOMMIT_MEMORY "/proc/sys/vm/overcommit_memory"

static void verify_oom(void)
{
	/* we expect mmap to fail before OOM is hit */
	TST_SYS_CONF_LONG_SET(OVERCOMMIT_MEMORY, 2, 1);
	oom(NORMAL, 0, ENOMEM, 0);

	/* with overcommit_memory set to 0 or 1 there's no
	 * guarantee that mmap fails before OOM */
	TST_SYS_CONF_LONG_SET(OVERCOMMIT_MEMORY, 0, 1);
	oom(NORMAL, 0, ENOMEM, 1);

	TST_SYS_CONF_LONG_SET(OVERCOMMIT_MEMORY, 1, 1);
	testoom(0, 0, ENOMEM, 1);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.timeout = TST_UNLIMITED_TIMEOUT,
	.test_all = verify_oom,
	.skip_in_compat = 1,
	.save_restore = (const struct tst_path_val[]) {
		{OVERCOMMIT_MEMORY, NULL, TST_SR_TBROK},
		{}
	},
};
