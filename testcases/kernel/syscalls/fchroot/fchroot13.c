// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test that unprivileged :manpage:`fchroot(2)` into failfs is refused
 * when the process is already chrooted.
 *
 * The root directory is what confines ".." resolution and the failfs root
 * can never be reached by walking up a real mount tree. Moving the root
 * of a chrooted task into failfs would allow it to escape its chroot via
 * :manpage:`openat(2)` with ".." and a pre-opened directory file descriptor,
 * so the kernel refuses the syscall with ``EPERM``.
 *
 * Root is required to create the chroot jail before dropping to an
 * unprivileged user in the forked child.
 */

#define _GNU_SOURCE
#include <pwd.h>
#include <unistd.h>
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/prctl.h"
#include "lapi/syscalls.h"

#define JAILDIR "jail"

static struct passwd *ltpuser;

static void run(void)
{
	if (SAFE_FORK())
		return;

	SAFE_CHROOT(JAILDIR);
	SAFE_CHDIR("/");

	SAFE_SETRESUID(ltpuser->pw_uid, ltpuser->pw_uid,
		ltpuser->pw_uid);

	SAFE_PRCTL(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);

	TST_EXP_FAIL(tst_syscall(__NR_fchroot, FD_FAILFS_ROOT, 0),
		EPERM, "fchroot() from a chrooted process");
}

static void setup(void)
{
	ltpuser = SAFE_GETPWNAM("nobody");
	SAFE_MKDIR(JAILDIR, 0755);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.forks_child = 1,
};
