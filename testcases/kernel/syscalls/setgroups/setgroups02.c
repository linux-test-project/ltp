// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) Linux Test Project, 2003-2023
 * 07/2001 Ported by Wayne Boyer
 */

/*\
 * [Description]
 *
 * Check that root process can setgroups() supplementary group ID and verify
 * that getgroups() returns the previously set ID.
 */

#include <pwd.h>

#include "tst_test.h"
#include "compat_tst_16.h"

static GID_T *groups_get, *groups_set;

static void verify_setgroups(void)
{
	groups_set[0] = 42;

	TST_EXP_PASS(SETGROUPS(1, groups_set));

	TST_EXP_VAL(GETGROUPS(1, groups_get), 1);

	TST_EXP_EQ_LI(groups_get[0], groups_set[0]);

	groups_get[0] = 0;
}

static struct tst_test test = {
	.test_all = verify_setgroups,
	.bufs = (struct tst_buffers []) {
		{&groups_get, .size = sizeof(GID_T)},
		{&groups_set, .size = sizeof(GID_T)},
		{},
	},
	.needs_root = 1,
};
