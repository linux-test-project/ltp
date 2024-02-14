// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2000 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * AUTHOR		: William Roske
 * CO-PILOT		: Dave Fenner
 */
/*\
 * [Description]
 *
 * Calls setgid() with current gid and expects success.
 */

#include "tst_test.h"
#include "compat_tst_16.h"

static gid_t gid;

static void run(void)
{
	TST_EXP_PASS(SETGID(gid));
}

static void setup(void)
{
	gid = getgid();
	GID16_CHECK(gid, setgid);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
};
