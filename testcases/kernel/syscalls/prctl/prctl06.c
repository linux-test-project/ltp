// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
 * Author: Yang Xu <xuyang2018.jy@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Test PR_GET_NO_NEW_PRIVS and PR_SET_NO_NEW_PRIVS of prctl(2).
 *
 * - Return the value of the no_new_privs bit for the calling thread.
 *   A value of 0 indicates the regular execve(2) behavior.  A value of
 *   1 indicates execve(2) will operate in the privilege-restricting mode.
 * - With no_new_privs set to 1, diables privilege granting operations
 *   at execve-time. For example, a process will not be able to execute a
 *   setuid binary to change their uid or gid if this bit is set. The same
 *   is true for file capabilities.
 * - The setting of this bit is inherited by children created by fork(2),
 *   and preserved across execve(2). We also check NoNewPrivs field in
 *   /proc/self/status if it supports.
 */

#include "prctl06.h"

static uid_t nobody_uid;
static gid_t nobody_gid;
static int proc_flag = 1;
static char *proc_sup = "Yes";

static void do_prctl(void)
{
	char ipc_env_var[1024];
	char *const argv[] = {BIN_PATH, "After execve, parent process", proc_sup, NULL};
	char *const childargv[] = {BIN_PATH, "After execve, child process", proc_sup, NULL};
	char *const envp[] = {ipc_env_var, NULL };
	int childpid;

	check_no_new_privs(0, "parent", proc_flag);

	TEST(prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0));
	if (TST_RET == -1) {
		tst_res(TFAIL | TTERRNO, "prctl(PR_SET_NO_NEW_PRIVS) failed");
		return;
	}
	tst_res(TPASS, "prctl(PR_SET_NO_NEW_PRIVS) succeeded");

	SAFE_SETGID(nobody_gid);
	SAFE_SETUID(nobody_uid);

	sprintf(ipc_env_var, IPC_ENV_VAR "=%s", getenv(IPC_ENV_VAR));

	childpid = SAFE_FORK();
	if (childpid == 0) {
		check_no_new_privs(1, "After fork, child process", proc_flag);
		execve(BIN_PATH, childargv, envp);
		tst_brk(TFAIL | TERRNO,
			"child process failed to execute prctl_execve");

	} else {
		tst_reap_children();
		check_no_new_privs(1, "parent process", proc_flag);
		execve(BIN_PATH, argv, envp);
		tst_brk(TFAIL | TERRNO,
			"parent process failed to execute prctl_execve");
	}
}

static void verify_prctl(void)
{
	int pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		do_prctl();
		exit(0);
	}
}

static void setup(void)
{
	struct passwd *pw;
	int field;

	pw = SAFE_GETPWNAM("nobody");
	nobody_uid = pw->pw_uid;
	nobody_gid = pw->pw_gid;

	SAFE_CP(TESTBIN, TEST_REL_BIN_DIR);

	SAFE_CHOWN(BIN_PATH, 0, 0);
	SAFE_CHMOD(BIN_PATH, SUID_MODE);

	if (FILE_LINES_SCANF(PROC_STATUS, "NoNewPrivs:%d", &field)) {
		tst_res(TCONF, "%s doesn't support NoNewPrivs field", PROC_STATUS);
		proc_flag = 0;
		proc_sup = "No";
	}

	TEST(prctl(PR_GET_NO_NEW_PRIVS, 0, 0, 0, 0));
	if (TST_RET == 0) {
		tst_res(TINFO, "kernel supports PR_GET/SET_NO_NEW_PRIVS");
		return;
	}

	if (TST_ERR == EINVAL)
		tst_brk(TCONF,
			"kernel doesn't support PR_GET/SET_NO_NEW_PRIVS");

	tst_brk(TBROK | TTERRNO,
		"current environment doesn't permit PR_GET/SET_NO_NEW_PRIVS");
}

static const char *const resfile[] = {
	TESTBIN,
	NULL,
};

static struct tst_test test = {
	.resource_files = resfile,
	.setup = setup,
	.test_all = verify_prctl,
	.forks_child = 1,
	.needs_root = 1,
	.mount_device = 1,
	.mntpoint = MNTPOINT,
	.child_needs_reinit = 1,
};
