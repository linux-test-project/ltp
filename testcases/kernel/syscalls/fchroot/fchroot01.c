// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test that :manpage:`fchroot(2)` with a regular directory fd moves the
 * process root to the directory referenced by the fd.
 *
 * This is the fd-based counterpart of :manpage:`chroot(2)`, introduced in
 * Linux v7.3. Root is required because :manpage:`fchroot(2)` with a regular
 * directory fd requires ``CAP_SYS_CHROOT``.
 *
 * The test runs in a forked child so the root of the parent process is
 * left untouched.
 */

#include <sys/stat.h>
#include <fcntl.h>
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/syscalls.h"

#define JAILDIR "jail"
#define CANARY "/canary"
#define CANARYDIR (JAILDIR CANARY)

static void run(void)
{
	struct stat st;

	if (SAFE_FORK())
		return;

	int dfd = SAFE_OPEN(JAILDIR, O_PATH | O_DIRECTORY);

	TST_EXP_PASS(tst_syscall(__NR_fchroot, dfd, 0),
		"fchroot() with a directory fd");

	TST_EXP_PASS(stat(CANARY, &st),
		"canary file visible under the new root");
}

static void setup(void)
{
	SAFE_MKDIR(JAILDIR, 0755);
	SAFE_TOUCH(CANARYDIR, 0644, NULL);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.forks_child = 1,
};
