// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test that unprivileged :manpage:`fchroot(2)` into failfs is refused
 * with a shared ``fs_struct``.
 *
 * ``no_new_privs`` is checked on the calling thread, but the root lives in
 * the fs_struct shared with a :manpage:`clone(2)` ``CLONE_FS`` sibling. A
 * sibling without no_new_privs could execute a setuid binary with the
 * failfs root, so entry requires ``fs->users == 1``, the same restriction
 * :manpage:`setns(2)` applies for the mount namespace. The sibling bumps
 * ``fs->users`` to 2, making the syscall fail with ``EINVAL``.
 *
 * Root is required to drop to an unprivileged user in the forked child.
 */

#define _GNU_SOURCE
#include <sys/wait.h>
#include <pwd.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/prctl.h"
#include "lapi/sched.h"
#include "lapi/syscalls.h"

static struct passwd *ltpuser;

static void run(void)
{
	if (SAFE_FORK())
		return;

	struct tst_clone_args args = {
		.flags = CLONE_FS,
		.exit_signal = SIGCHLD,
	};
	pid_t parent = getpid();
	pid_t sib;

	SAFE_SETRESUID(ltpuser->pw_uid, ltpuser->pw_uid,
		ltpuser->pw_uid);

	sib = SAFE_CLONE(&args);
	if (!sib) {
		/*
		 * Parked sibling sharing the fs_struct with the
		 * caller. It must die with the parent, otherwise it
		 * would outlive the test.
		 */
		SAFE_PRCTL(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
		if (getppid() == parent)
			pause();
		exit(0);
	}

	SAFE_PRCTL(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);

	TST_EXP_FAIL(tst_syscall(__NR_fchroot, FD_FAILFS_ROOT, 0),
		EINVAL, "fchroot() with a shared fs_struct");

	SAFE_KILL(sib, SIGKILL);
	SAFE_WAITPID(sib, NULL, 0);
}

static void setup(void)
{
	clone3_supported_by_kernel();

	ltpuser = SAFE_GETPWNAM("nobody");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
};
