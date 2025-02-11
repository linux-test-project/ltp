// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Ported from Crackerjack to LTP by Manas Kumar Nayak maknayak@in.ibm.com>
 */

/*\
 * Basic tests for the unshare() syscall.
 *
 * [Algorithm]
 *
 * Calls unshare() for different CLONE_* flags in a child process and expects
 * them to succeed.
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

static struct test_case_t {
	int mode;
	const char *desc;
} tc[] = {
	{CLONE_FILES,	"CLONE_FILES"},
	{CLONE_FS,	"CLONE_FS"},
	{CLONE_NEWNS,	"CLONE_NEWNS"},
};

static void run(unsigned int i)
{
	pid_t pid = SAFE_FORK();
	if (pid == 0)
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
