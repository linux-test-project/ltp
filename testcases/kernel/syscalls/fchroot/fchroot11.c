// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Test unprivileged :manpage:`fchroot(2)` into failfs with no_new_privs
 * set.
 *
 * An unprivileged process may enter failfs when it committed to
 * no_new_privs, since setuid binaries then pose no confused deputy risk
 * anymore. The test also verifies that a process which entered failfs
 * counts as chrooted: it can no longer create a user namespace with
 * :manpage:`unshare(2)` to regain ``CAP_SYS_CHROOT``.
 *
 * Root is required to drop to an unprivileged user in the forked child.
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <pwd.h>
#include <sched.h>
#include <sys/wait.h>
#include "tst_test.h"
#include "lapi/fcntl.h"
#include "lapi/prctl.h"
#include "lapi/sched.h"
#include "lapi/syscalls.h"

static struct passwd *ltpuser;
static int userns_supported;

static void run(void)
{
	if (SAFE_FORK())
		return;

	SAFE_SETRESUID(ltpuser->pw_uid, ltpuser->pw_uid,
		ltpuser->pw_uid);

	SAFE_PRCTL(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);

	TST_EXP_PASS(tst_syscall(__NR_fchroot, FD_FAILFS_ROOT, 0),
		"unprivileged fchroot() with no_new_privs");

	TST_EXP_FAIL2(open("/dev/zero", O_RDONLY), EOPNOTSUPP,
		"absolute lookup after entering failfs");

	if (userns_supported) {
		TST_EXP_FAIL(unshare(CLONE_NEWUSER), EPERM,
			"user namespace creation blocked by the failfs root");
	} else {
		tst_res(TCONF, "unprivileged user namespaces are not supported");
	}
}

static void setup(void)
{
	pid_t pid;
	int status;

	ltpuser = SAFE_GETPWNAM("nobody");

	pid = SAFE_FORK();
	if (!pid) {
		SAFE_SETRESUID(ltpuser->pw_uid, ltpuser->pw_uid,
			ltpuser->pw_uid);
		SAFE_PRCTL(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
		if (unshare(CLONE_NEWUSER) == -1)
			exit(1);
		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		userns_supported = 1;
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1,
};
