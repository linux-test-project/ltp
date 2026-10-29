// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Copyright (c) Linux Test Project, 2009-2025
 * Ported from Crackerjack to LTP by Manas Kumar Nayak maknayak@in.ibm.com>
 */

/*\
 * Basic tests for the :manpage:`unshare(2)` syscall.
 *
 * [Algorithm]
 *
 * Calls :manpage:`unshare(2)` for different CLONE_* flags in a child process and
 * expects them to succeed.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/syscall.h>
#include <sched.h>
#include <limits.h>
#include <unistd.h>

#include "tst_test.h"
#include "config.h"

#ifdef HAVE_UNSHARE

#define FLAG_DESC(x) .mode = x, .desc = #x

static struct test_case_t {
	int mode;
	const char *desc;
} tc[] = {
	{FLAG_DESC(CLONE_FILES)},
	{FLAG_DESC(CLONE_FS)},
	{FLAG_DESC(CLONE_NEWNS)},
};

static void run(unsigned int i)
{
	if (!SAFE_FORK())
		TST_EXP_PASS(unshare(tc[i].mode), "unshare(%s)", tc[i].desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.forks_child = 1,
	.needs_tmpdir = 1,
	.needs_root = 1,
	.test = run,
};

#else
TST_TEST_TCONF("unshare is undefined.");
#endif
